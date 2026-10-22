"""Makes the background of the console, gfx/shell/conback.bmp, from the menu background of Day of Defeat Beta 1.0 - 1.3b.
The menu background of that version is cut into the tiles resource/background/640_*_loading.tga, BackgroundLayout.txt tells where
every tile goes; they are put together to 640x480 and darkened so that the orange text of the console is readable over it.

The engine draws the console background opaque, stretches it to the width of the screen with the 4:3 proportions and aligns its
bottom edge with the bottom of the console. On a 16:9 screen only the lower 75% of the picture is on the screen, the upper part is
above the screen. So the picture is squeezed vertically to the part of the image that is visible on the screen of the given
aspect (16:9 by default, there it has the right proportions) and the rest of the image on top is a dark continuation of its
upper edge. On a 4:3 screen the whole image is visible: the picture is a bit flat there and has a dark band on top.

usage: gen_conback.py <dod_addon/resource dir of the beta> <extras>/gfx/shell/conback.bmp [brightness, 0.5 by default] [aspect, 1.7778 by default]"""
import sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from tga import read_tga
from bmptool import write_bmp24


def load_picture(res):
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

    return width, height, canvas


def squeeze_rows(canvas, new_h):
    """a box filter over the rows"""
    h = len(canvas)
    w = len(canvas[0])
    ratio = h / float(new_h)
    out = []

    for y in range(new_h):
        y0, y1 = y * ratio, (y + 1) * ratio
        acc = [[0.0, 0.0, 0.0] for _ in range(w)]
        total = 0.0

        for r in range(int(y0), min(h, int(math.ceil(y1)))):
            wgt = min(y1, r + 1) - max(y0, r)
            total += wgt

            for x in range(w):
                p = canvas[r][x]
                acc[x][0] += p[0] * wgt
                acc[x][1] += p[1] * wgt
                acc[x][2] += p[2] * wgt

        out.append([(a[0] / total, a[1] / total, a[2] / total) for a in acc])

    return out


def blur_row(row, radius):
    w = len(row)
    out = []

    for x in range(w):
        lo, hi = max(0, x - radius), min(w, x + radius + 1)
        n = hi - lo
        out.append((sum(p[0] for p in row[lo:hi]) / n, sum(p[1] for p in row[lo:hi]) / n, sum(p[2] for p in row[lo:hi]) / n))

    return out


def main():
    res = sys.argv[1]
    out = sys.argv[2]
    factor = float(sys.argv[3]) if len(sys.argv) > 3 else 0.5
    aspect = float(sys.argv[4]) if len(sys.argv) > 4 else 16.0 / 9.0

    width, height, canvas = load_picture(res)

    # the part of the image (in the 4:3 proportions of the engine) that is on a screen of this aspect
    visible = min(1.0, (1.0 / aspect) / (height / float(width)))
    pic_h = int(round(height * visible))
    picture = squeeze_rows(canvas, pic_h) if pic_h < height else [[tuple(p) for p in r] for r in canvas]

    # the dark continuation of the upper edge: its colors, blurred, fade to dark upwards
    top = height - pic_h
    edge = [tuple(sum(picture[r][x][k] for r in range(min(4, pic_h))) / min(4, pic_h) for k in range(3)) for x in range(width)]
    edge = blur_row(edge, 24)
    filler = []

    for y in range(top):
        t = y / float(max(1, top))
        g = 0.3 + 0.7 * (t * t * (3.0 - 2.0 * t))
        filler.append([(p[0] * g, p[1] * g, p[2] * g) for p in edge])

    image = filler + picture
    image = [[(int(r * factor + 0.5), int(g * factor + 0.5), int(b * factor + 0.5)) for r, g, b in row] for row in image]
    write_bmp24(out, width, height, image)
    print('%dx%d, brightness %.2f, aspect %.3f: the picture is the lower %d rows, %d rows of the continuation on top -> %s' % (width, height, factor, aspect, pic_h, top, out))


if __name__ == '__main__':
    main()
