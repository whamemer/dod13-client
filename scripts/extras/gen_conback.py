"""Makes the background of the console, gfx/shell/conback.bmp, from the menu background of Day of Defeat Beta 1.0 - 1.3b.
The menu background of that version is cut into the tiles resource/background/640_*_loading.tga, BackgroundLayout.txt tells where
every tile goes; they are put together to 640x480 and darkened so that the orange text of the console is readable over it.
The engine draws the console background opaque and stretches it to 4:3, so the size and the proportions stay as they are.
usage: gen_conback.py <dod_addon/resource dir of the beta> <extras>/gfx/shell/conback.bmp [brightness, 0.5 by default]"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tga import read_tga
from bmptool import write_bmp24


def main():
    res = sys.argv[1]
    out = sys.argv[2]
    factor = float(sys.argv[3]) if len(sys.argv) > 3 else 0.5

    width = height = 0
    tiles = []
    for line in open(os.path.join(res, 'BackgroundLayout.txt'), encoding='utf-8', errors='replace'):
        parts = line.split()

        if len(parts) == 3 and parts[0] == 'resolution':
            width, height = int(parts[1]), int(parts[2])
        elif len(parts) == 4 and parts[0].endswith('.tga'):
            tiles.append((parts[0], int(parts[2]), int(parts[3])))

    assert width and height and tiles, 'BackgroundLayout.txt was not understood'

    canvas = [[(0, 0, 0)] * width for _ in range(height)]

    for name, x0, y0 in tiles:
        w, h, rows = read_tga(os.path.join(res, '..', name))
        for y in range(h):
            canvas[y0 + y][x0:x0 + w] = rows[y]

    canvas = [[(int(r * factor + 0.5), int(g * factor + 0.5), int(b * factor + 0.5)) for r, g, b in row] for row in canvas]
    write_bmp24(out, width, height, canvas)
    print('%dx%d, brightness %.2f -> %s' % (width, height, factor, out))


if __name__ == '__main__':
    main()
