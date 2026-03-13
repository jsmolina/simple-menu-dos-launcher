import os
import shutil
import string
import argparse
import struct
from typing import List, Tuple

translation_table = dict.fromkeys(map(ord, ' [](),.~!@#$%^&*{}: '), None)

DOSnames = []


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
            raise ValueError("The file does not appear to be a valid BMP ('BM').")
        file_size = struct.unpack_from('<I', header, 2)[0]
        image_offset = struct.unpack_from('<I', header, 10)[0]
        dib_size = struct.unpack_from('<I', header, 14)[0]
        width = struct.unpack_from('<I', header, 18)[0]
        height = struct.unpack_from('<I', header, 22)[0]
        planes = struct.unpack_from('<H', header, 26)[0]
        bpp = struct.unpack_from('<H', header, 28)[0]
        compression = struct.unpack_from('<I', header, 30)[0]
        if bpp != 8 or compression != 0:
            raise ValueError("BMP should be 8 bpp indexed and uncompressed.")
        if width != 32 or height != 32:
            raise ValueError(f"Image should be 32x32, but is {width}x{height}.")

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
            raise ValueError("Incomplete image data.")
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



def dir_to_dos(longname):
    # FAT12 doesn't support unicode - avert thine eyes
    dname = longname.encode('ascii', 'ignore').decode()
    # Truncate basename while keeping extension
    if len(longname) > 12:
        dname = dname.translate(translation_table)
    dname = str.upper(dname)
    dname = dname[0:8]
    dname = dname.replace(' ', '')
    collided = dname

    # Do we have a collision?
    if dname in DOSnames:
        for i in string.ascii_uppercase + string.digits:
            dname = longname.translate(translation_table) + i
            dname = str.upper(dname)
            dname = dname[0:8]
            if dname not in DOSnames:
                break
        else:
            # If we still have a collision, we need to mangle the name some more
            for i in string.ascii_uppercase + string.digits:
                for j in string.ascii_uppercase + string.digits:
                    oldname = dname
                    dname = longname.translate(translation_table)[0:6] + i + j + longname[-4:]
                    dname = str.upper(dname)

                    if dname not in DOSnames:
                        break
                if dname not in DOSnames:
                    break

    # If we got here, too many collisions (need more code!)
    if dname in DOSnames:
        print('Namespace collision converting', longname, 'to', dname)
        print('Ask the progammer to enhance the collision algorithm.')
        exit(8)

    dname = dname[0:8]
    dname = dname.replace(' ', '')
    DOSnames.append(dname)
    return dname


def find_executable(directory):
    execs = []
    executable = None

    for w in os.listdir(directory):
        filename = w.lower()
        if filename == 'run.bat' or filename == 'exec.bat':
            executable = w

        if filename == "setup.exe" or filename == "install.exe":
            continue

        if filename.endswith('.exe') or \
            filename.endswith('.bat') or \
            filename.endswith('.com'):
            # add to user decision
            execs.append(w)

    if len(execs) > 1:
        print(f"*** Multiple execs in {directory} *** ")
        for i, exec in enumerate(execs):
            print(f"{i} - {exec}")
    elif len(execs) is 1:
        executable = execs[0]

    while not executable:
        option = int(input("Which one? "))
        if option < len(execs):
            executable = execs[option]

    if executable != "exec.bat":
        with open(os.path.join(directory, 'exec.bat'), 'wb') as f:
            f.write(f"{executable}".encode("utf-8"))
        executable = "exec.bat"

    print(f"Executable will be {executable}")
    return executable



def find_setup(directory):
    for w in os.listdir(directory):
        filename = w.lower()
        if filename.endswith('setup.exe') or \
                filename.endswith('setsound.exe') or \
                filename.endswith('install.exe') or \
                filename.endswith('config.bat'):
            return w

def find_image(directory):
    thumbnail = os.path.join(directory, 'thumbnail.bmp')
    if not os.path.isfile(thumbnail):
        return
    out_file = os.path.join(directory, 'menu_img.bin')
    print("Found thumbnail.bmp, converting to menu_img.bin...")
    try:
        convert_image(thumbnail, out_file,
                      allow_bright_bg=False, charset='shades', dither=False)
    except ValueError as e:
        print(f'Warning: could not convert {thumbnail}: {e}')


def main(in_dirs: list, out: str, dos_path: str):
    print(f'Creating {out}...')
    os.mkdir(out)
    print(f'Generating LIST.txt from {in_dirs}...')
    list_txt = os.path.join(out, 'LIST.TXT')
    print(f'Generating PCXT list in {list_txt}...')
    games_dir = os.path.join(out, 'GAMES')
    print(f'Creating games directory in {games_dir}...')
    print(f'Generating PCXT LIST.TXT {list_txt}...')
    with open(list_txt, 'wb') as f:
        comment = '# path\texecutable\tsetup\tother\tname'
        comment_line = f'{comment:<108}\r\n'
        f.write(comment_line.encode('utf-8'))
        for origin in in_dirs:
            for dirname in sorted(os.listdir(origin)):
                if dirname.startswith('.'):
                    continue
                fulldirpath = os.path.join(origin, dirname)
                executable = find_executable(fulldirpath)
                setup_exec = find_setup(fulldirpath) or ""
                find_image(fulldirpath)
                shortname = dir_to_dos(dirname)
                gamename = dirname[0:32]
                game_dir = f"{dos_path}\\GAMES\\{shortname}"
                other = "9000"
                line = f'{game_dir:<32}\t{executable:<16}\t{setup_exec:<16}\t{other:<8}\t{gamename:<32}\r\n'
                f.write(line.encode('utf-8'))
                current_game_dir = os.path.join(games_dir, shortname)
                print(f'Copy {dirname} to {current_game_dir}...')
                shutil.copytree(fulldirpath, current_game_dir)
        if os.path.isfile(os.path.join(os.getcwd(), 'PCXTMENU.COM')):
            print('Copying PCXTMENU.COM to output directory...')
            shutil.copyfile('PCXTMENU.COM', os.path.join(out, 'PCXTMENU.COM'))
        else:
            print('Warning: PCXTMENU.COM not found in current directory, make sure to copy it to the output directory after generation.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog='RLoader generator',
        description='Generates RLoader structure',
        epilog='')
    parser.add_argument('-i', '--in-dirs', action='append', required=True,
                        help='Input dirs, pass more than once for multiple')
    parser.add_argument('-o', '--out', required=True, help='Output Dir')
    parser.add_argument('-d', '--dos-path', help='Path where you will copy the MENU in DOS '
                                                 '(double slash for *nix) e.g. C:\\\\MENU',
                        required=True)
    args = parser.parse_args()
    main(in_dirs=args.in_dirs, out=args.out, dos_path=args.dos_path)
