import os, glob
import shutil
import string
import argparse

translation_table = dict.fromkeys(map(ord, ' [](),.~!@#$%^&*{}: '), None)

DOSnames = []


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
    for w in os.listdir(directory):
        filename = w.lower()
        if filename.endswith('run.bat') or \
                filename.endswith('.exe') or \
                filename.endswith('.bat') or \
                filename.endswith('.com'):
            return w


def find_setup(directory):
    for w in os.listdir(directory):
        filename = w.lower()
        if filename.endswith('setup.exe') or \
                filename.endswith('setsound.exe') or \
                filename.endswith('install.exe'):
            return w

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
        f.write(f'#path\texecutable\tsetup\r\n'.encode('utf-8'))
        for origin in in_dirs:
            for dirname in os.listdir(origin):
                if dirname.startswith('.'):
                    continue
                fulldirpath = os.path.join(origin, dirname)
                executable = find_executable(fulldirpath)
                setup_exec = find_setup(fulldirpath)
                shortname = dir_to_dos(dirname)
                f.write(f'{dos_path}\\GAMES\\{shortname}\t{executable}\t{setup_exec}\t9000\t{dirname}'.encode('utf-8'))
                f.write(b'\r\n')
                current_game_dir = os.path.join(games_dir, shortname)
                print(f'Copy {dirname} to {current_game_dir}...')
                shutil.copytree(fulldirpath, current_game_dir)


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
