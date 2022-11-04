import os, glob
import shutil
import string
import argparse

translation_table = dict.fromkeys(map(ord, ' [](),.~!@#$%^&*{}: '), None)

DOSnames = []


def fputc(number: int, fp):
    fp.write(chr(number).encode('utf-8'))


def convert_image(in_file="thumb.bmp", out_file="menu.bin"):
    SEEK_SET = 0
    SEEK_CUR = 1
    E = 69

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
                        fputc(219, fout)
                    elif pix < 32:
                        fputc(178, fout)  # Pattern bright
                    elif pix < 48:
                        fputc(177, fout)  # Pattern middle
                    else:
                        fputc(176, fout)  # Pattern dark
                    # Write foreground color, from 0 to 16 (high 4 bits = bkg color = 0)
                    fputc(pix & 15, fout)
                else:  # two color cells
                    # Don't use pattern colors
                    if pix > 15:
                        fputc(E, fout)
                        fputc(0xCF, fout)
                    elif pix1 > 15:
                        fputc(E, fout)
                        fputc(0xCF, fout)
                    #  both pixels ar greater than 7
                    elif (pix > 7) and (pix1 > 7):
                        fputc(E, fout)
                        fputc(0xCF, fout)
                    # use 16 foreground colors for the upper block
                    elif (pix > 7) and (pix1 < 8):
                        fputc(223, fout)
                        fputc((pix1 << 4) + pix, fout)
                    # use 16 foreground colors for the lower block
                    elif (pix < 8) and (pix1 > 7):
                        fputc(220, fout)
                        fputc((pix << 4) + pix1, fout)
                    # Both pixels < 8
                    else:
                        fputc(220, fout)
                        fputc((pix << 4) + pix1, fout)
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
