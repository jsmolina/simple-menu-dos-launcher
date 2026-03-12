# -*- coding: utf-8 -*-
"""
Ansi Thumbnail generator — Palette Auto (by Copilot)
---------------------------------------------------
Entrada: BMP indexado 8 bpp (256 colores) de 32x32.
Salida: matriz de 32x16 celdas (carácter + atributo), 2 bytes por celda.

Cambio clave:
- En lugar de asumir que el índice codifica (base, sombra) como (idx & 15, idx >> 4),
  ahora se lee la **paleta real del BMP** y cada índice se **cuantiza** al color
  ANSI de 16 colores más cercano. El nivel de sombreado (█▓▒░) se decide por
  **luminancia** del color de paleta.

Formato de salida (compat.): por celda (x,y) se escriben 2 bytes:
  Byte 0: código de carácter CP437 (▀=223, ▄=220, █=219, ▓=178, ▒=177, ░=176)
  Byte 1: atributo (BG<<4 | FG) con colores 0..15.

Opciones:
  --allow-bright-bg   Permite fondo brillante (8..15). Desactivado por defecto.
  --no-dither         Desactiva el dithering 2×2 en celdas monocolor.
  --charset           'shades' (█▓▒░, por defecto) o 'ascii' (rampa densidad).

Notas:
- Requiere BMP 32×32, 8 bpp, sin compresión.
- Se espera una paleta de 256 entradas (BGRA). Si hay menos, se completa con 0s.
"""

import argparse
import struct
from typing import List, Tuple

# CP437 codes
CP437_FULL = 219  # █
CP437_LOWER = 220 # ▄
CP437_UPPER = 223 # ▀
CP437_SHADE_LIGHT = 176  # ░
CP437_SHADE_MED = 177    # ▒
CP437_SHADE_DARK = 178   # ▓

ASCII_RAMP = " .:-=+*#%@"  # 10 niveles
BAYER_2x2 = ((0,2),(3,1))

# ANSI 16-color palette (approx. EGA)
ANSI16 = [
    (0,0,0),       # 0 black
    (0,0,170),     # 1 blue
    (0,170,0),     # 2 green
    (0,170,170),   # 3 cyan
    (170,0,0),     # 4 red
    (170,0,170),   # 5 magenta
    (170,85,0),    # 6 brown / dark yellow
    (170,170,170), # 7 light gray
    (85,85,85),    # 8 dark gray (bright black)
    (85,85,255),   # 9 bright blue
    (85,255,85),   # 10 bright green
    (85,255,255),  # 11 bright cyan
    (255,85,85),   # 12 bright red
    (255,85,255),  # 13 bright magenta
    (255,255,85),  # 14 bright yellow
    (255,255,255), # 15 white
]


def clamp(v: int, lo: int, hi: int) -> int:
    return max(lo, min(hi, v))


def read_bmp_indexed_and_palette(path: str) -> Tuple[int, int, List[List[int]], List[Tuple[int,int,int]]]:
    with open(path, 'rb') as f:
        header = f.read(54)
        if len(header) < 54 or header[0:2] != b'BM':
            raise ValueError("El archivo no parece un BMP válido ('BM').")
        file_size = struct.unpack_from('<I', header, 2)[0]
        image_offset = struct.unpack_from('<I', header, 10)[0]
        dib_size = struct.unpack_from('<I', header, 14)[0]
        width = struct.unpack_from('<I', header, 18)[0]
        height = struct.unpack_from('<I', header, 22)[0]
        planes = struct.unpack_from('<H', header, 26)[0]
        bpp = struct.unpack_from('<H', header, 28)[0]
        compression = struct.unpack_from('<I', header, 30)[0]
        if bpp != 8 or compression != 0:
            raise ValueError("BMP debe ser 8 bpp indexado y sin compresión.")
        if width != 32 or height != 32:
            raise ValueError(f"La imagen debe ser 32x32, pero es {width}x{height}.")

        # Leer paleta entre el fin del DIB y el comienzo de los datos
        palette_entries = (image_offset - 54) // 4  # entradas BGRA
        if palette_entries <= 0:
            palette_entries = 256
        f.seek(54)
        pal_raw = f.read(palette_entries * 4)
        palette_rgb: List[Tuple[int,int,int]] = []
        for i in range(palette_entries):
            b, g, r, a = pal_raw[i*4:(i+1)*4]
            palette_rgb.append((r, g, b))
        # Completar a 256 si hiciera falta
        while len(palette_rgb) < 256:
            palette_rgb.append((0,0,0))

        # Leer píxeles
        f.seek(image_offset)
        row_size = ((width + 3) // 4) * 4
        raw = f.read(row_size * height)
        if len(raw) < row_size * height:
            raise ValueError("Datos de imagen incompletos.")
        rows: List[List[int]] = []
        for r in range(height):
            start = r * row_size
            row = list(raw[start:start + width])
            rows.append(row)
        rows.reverse()  # top-first
        return width, height, rows, palette_rgb


def nearest_ansi_base(r: int, g: int, b: int) -> int:
    best_d = 1e12
    best_i = 0
    for i, (R,G,B) in enumerate(ANSI16):
        # Euclidean RGB distance
        d = (R-r)*(R-r) + (G-g)*(G-g) + (B-b)*(B-b)
        if d < best_d:
            best_d = d
            best_i = i
    return best_i


def shade_level_from_luma(r: int, g: int, b: int) -> int:
    # ITU-R BT.601 luma approximation
    Y = 0.299*r + 0.587*g + 0.114*b
    # thresholds tuned for █▓▒░ distribution (0=█ sólido, 3=░ claro)
    if Y >= 200: return 0  # █
    if Y >= 140: return 1  # ▓
    if Y >= 90:  return 2  # ▒
    return 3               # ░


def build_index_lut(palette_rgb: List[Tuple[int,int,int]]) -> Tuple[List[int], List[int]]:
    """Devuelve dos arrays de 256: base_color[256], shade_level[256]."""
    base = [0]*256
    shade = [0]*256
    for i, (r,g,b) in enumerate(palette_rgb):
        base[i] = nearest_ansi_base(r,g,b)
        shade[i] = shade_level_from_luma(r,g,b)
    return base, shade


def pick_shade_char(avg_level: float, x: int, y: int, dither: bool) -> int:
    if not dither:
        idx = int(round(avg_level))
    else:
        threshold = BAYER_2x2[y % 2][x % 2]
        idx = int(avg_level)
        frac = avg_level - idx
        if (frac * 4) > threshold:
            idx += 1
    idx = clamp(idx, 0, 3)
    return [CP437_FULL, CP437_SHADE_DARK, CP437_SHADE_MED, CP437_SHADE_LIGHT][idx]


def pick_ascii_char(avg_level: float, x: int, y: int, dither: bool) -> int:
    steps = len(ASCII_RAMP) - 1
    base = avg_level / 3.0 * steps
    if dither:
        threshold = BAYER_2x2[y % 2][x % 2]
        base += (threshold - 1.5) * 0.15
    idx = clamp(int(round(base)), 0, steps)
    return ord(ASCII_RAMP[idx])


def compose_attr(bg: int, fg: int) -> int:
    return ((bg & 0x0F) << 4) | (fg & 0x0F)


def map_cell(top_idx: int, bottom_idx: int, x: int, y: int,
             idx2base: List[int], idx2shade: List[int],
             allow_bright_bg: bool, charset: str, dither: bool) -> Tuple[int,int]:
    tb, ts = idx2base[top_idx], idx2shade[top_idx]
    bb, bs = idx2base[bottom_idx], idx2shade[bottom_idx]

    if tb == bb:
        base = tb
        avg_level = (ts + bs) / 2.0
        if charset == 'ascii':
            ch = pick_ascii_char(avg_level, x, y, dither)
        else:
            ch = pick_shade_char(avg_level, x, y, dither)
        attr = compose_attr(0, base)  # fondo negro para monocolor
        return ch, attr

    # Dos colores distintos: semibloques
    if not allow_bright_bg:
        if tb >= 8 and bb < 8:
            return CP437_UPPER, compose_attr(bb, tb)
        if bb >= 8 and tb < 8:
            return CP437_LOWER, compose_attr(tb, bb)
        # Ambos brillantes o ambos no: remapear BG si es brillante
        bg = bb & 7
        return CP437_UPPER, compose_attr(bg, tb)
    else:
        return CP437_UPPER, compose_attr(bb, tb)


def convert_image(in_file: str, out_file: str,
                  allow_bright_bg: bool, charset: str, dither: bool) -> None:
    w, h, rows, palette_rgb = read_bmp_indexed_and_palette(in_file)
    idx2base, idx2shade = build_index_lut(palette_rgb)

    if not out_file:
        out_file = in_file.rsplit('.', 1)[0] + '.bin'
    if out_file.lower().endswith('.bmp'):
        out_file = out_file[:-4] + '.bin'

    with open(out_file, 'wb') as out:
        for cy in range(0, h // 2):
            y_top = cy * 2
            y_bottom = y_top + 1
            for cx in range(0, w):
                t = rows[y_top][cx]
                b = rows[y_bottom][cx]
                ch, attr = map_cell(t, b, cx, cy, idx2base, idx2shade,
                                     allow_bright_bg, charset, dither)
                out.write(bytes((ch, attr)))


def main():
    parser = argparse.ArgumentParser(
        prog='Ansi Thumbnail generator — palette auto',
        description='Convierte un BMP indexado 32x32 a matriz ANSI 32x16 (char+attr) usando cuantización a ANSI16.'
    )
    parser.add_argument('-i', '--input', default='thumb.bmp', help='BMP 32x32, 8bpp, sin compresión')
    parser.add_argument('-o', '--output', default='menu_img.bin', help='Salida .bin (2 bytes/celda)')
    parser.add_argument('--allow-bright-bg', action='store_true', help='Permitir fondo brillante (8..15).')
    parser.add_argument('--no-dither', dest='dither', action='store_false', default=True, help='Desactiva dithering 2×2 en monocolor.')
    parser.add_argument('--charset', choices=['shades', 'ascii'], default='shades', help='Conjunto para monocolor: shades (█▓▒░) o ascii.')
    args = parser.parse_args()

    convert_image(args.input, args.output, args.allow_bright_bg, args.charset, args.dither)

if __name__ == '__main__':
    main()

