import struct
import argparse

translation_table = dict.fromkeys(map(ord, ' [](),.~!@#$%^&*{}: '), None)

DOSnames = []

"""
ESP
---

Formato de imagen: bmp, 32x32, 256 colores indexado, usando 64 colores como en la imagen
de ejemplo.

Las imagenes solamente pueden ser de 32x32 pixels, y deben usar la paleta contenida en la
imagen de ejemplo.

El programa convertira la imagen a una matriz de celdas ASCII de 32x16 caracteres, asi que
en realidad, la imagen solamente tendra 32x16 pixels. Sin embargo, existen caracteres ASCII
especiales que contienen dos bloques coloreados, estos caracteres pueden contener dos pixels
de la imagen original:

    Original 1x2 pixels => Final ascii
        -----                	-----
        |P1 |                	|###|
        |   |                	|###|
        -----		    =>     	|###|
        |P2 |                	|   |
        |   |                	|   |
        -----                	-----

Para usar los 16 colores debemos anular la funcion de parpadeo del texto en CGA, TANDY, EGA
 y VGA. Desactivar esta funcion, puede producir problemas, asi que no podremos usar todos los
colores en estos caracteres. Uno de los colores debe ser tomado de los 8 primeros (0-7) el otro
puede tomar valores de 0 a 15.

Los colores mas alla del 15 en la paleta de la imagen de ejemplo, solamente pueden ser utilizados
en celdas de 1x2 pixels con un color unico, porque estos colores usaran otros caracteres 
especiales, que mezclan dos colores de manera uniforme. En este caso, mezclamos negro con los
colores para simular tonos mas oscuros.

    Caracter ASCII mezclando dos colores
                -----
                |# #|
                | # |
                |# #|
                | # |
                |# #|
                -----
                
Si no es posible un color o mezcla de colores, aparecera una E con fondo rojo parpadeando en la
celda con colores incorrectos.
"""


def fputc(value: bytes, fp):
    fp.write(value)


def convert_image(in_file="thumb.bmp", out_file="menu.bin"):
    SEEK_SET = 0
    SEEK_CUR = 1
    E = 69

    # read header
    with open(in_file, "rb") as input_f:
        file_data = input_f.read(54)

    image_width = struct.unpack_from('<i', file_data, 18)[0]
    image_height = struct.unpack_from('<i', file_data, 22)[0]
    print(image_width, image_height)
    if image_height != 32 or image_width != 32:
        print(f"Image must be 32x32 but it is {image_width}x{image_height}")
        exit(1)

    padding = image_width % 4
    num_of_pixels = image_width * image_height

    data = []
    with open(in_file, "rb") as input_f:
        input_f.seek(0x0136 + 1024 - 32, SEEK_SET)
        for y in range(32):
            data += input_f.read(32)
            input_f.seek(-64, SEEK_CUR)

    offset = 0
    with open(out_file, "wb") as fout:
        for y in range(16):
            for x in range(32):
                pix = data[offset]
                pix1 = data[offset + 32]  # Read second pixel of the final ascii cell
                offset += 1
                # colors  16, 32 and 48 are  color  0(black)
                if pix == 16 or pix == 32 or pix == 48:
                    pix = 0

                if pix1 == 16 or pix1 == 32 or pix1 == 48:
                    pix1 = 0

                # one color cells
                if pix == pix1:
                    if pix < 16:
                        fputc(b"\xDB", fout)
                    elif pix < 32:
                        fputc(b"\xB2", fout)  # Pattern bright
                    elif pix < 48:
                        fputc(b"\xB1", fout)  # Pattern middle
                    else:
                        fputc(b"\xB0", fout)  # Pattern dark
                    # Write foreground color, from 0 to 16 (high 4 bits = bkg color = 0)
                    fputc(chr(pix & 15).encode("cp437"), fout)
                else:  # two color cells
                    # Don't use pattern colors
                    if pix > 15:
                        fputc(b"E", fout)
                        fputc(b"\xCF", fout)
                    elif pix1 > 15:
                        fputc(b"E", fout)
                        fputc(b"\xCF", fout)
                    #  both pixels ar greater than 7
                    elif (pix > 7) and (pix1 > 7):
                        fputc(b"E", fout)
                        fputc(b"\xCF", fout)
                    # use 16 foreground colors for the upper block
                    elif (pix > 7) and (pix1 < 8):
                        fputc(b"\xDF", fout)
                        fputc((pix1 << 4) + pix, fout)
                    # use 16 foreground colors for the lower block
                    elif (pix < 8) and (pix1 > 7):
                        fputc(b"\xDC", fout)
                        fputc(chr((pix << 4) + pix1).encode("cp437"), fout)
                    # Both pixels < 8
                    else:
                        fputc(b"\xDC", fout)
                        fputc(chr((pix << 4) + pix1).encode("cp437"), fout)
            offset += 32  # //Skip one row


def main():
    parser = argparse.ArgumentParser(
        prog='Ansi Thumbnail generator',
        description='Generates thumbnail',
        epilog='')
    parser.add_argument('-i', '--input', default="thumb.bmp",
                        help='Input file bmp (32x32) 64 colors')
    parser.add_argument('-o', '--output', default="menu.bin",
                        help='Output file in ansi')
    args = parser.parse_args()
    convert_image(in_file=args.input, out_file=args.output)


if __name__ == '__main__':
    main()
