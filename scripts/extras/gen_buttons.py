"""Makes the buttons of the Xash3D main menu that the Day of Defeat atlas does not have:
 - the labels that Xash3D shows and the game never had (new game, load game, ...) go into empty cells of btns_main.bmp;
 - the buttons that are new in Xash3D (touch, gamepad, gyroscope, favorites) are separate 156x78 bmp files.
The letters are cut out of the labels that exist, see wonbtn.py."""
import sys, os, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bmptool import read_bmp, write_png
from wonbtn import Library, make_states, write_gray_bmp, W, CELL_H

SHELL = sys.argv[1]            # .../gfx/shell
PREVIEW = sys.argv[2] if len(sys.argv) > 2 else None

ATLAS_CELLS = {0: 'new game', 2: 'hazard course', 4: 'load game', 12: 'save game', 13: 'load game'}
FILES = {'btn_touch': 'touch', 'btn_touch_options': 'touch options', 'btn_touch_buttons': 'touch buttons', 'btn_touch_save': 'save',
         'btn_touch_editor': 'editor', 'btn_touch_select': 'select', 'btn_touch_reset': 'reset', 'btn_gamepad': 'gamepad',
         'btn_gyro': 'gyroscope', 'btn_favorite': 'favorite', 'btn_unfavorite': 'unfavorite'}

atlas_path = os.path.join(SHELL, 'btns_main.bmp')
im = read_bmp(atlas_path)
lib = Library(im)

previews = []

# the separate files
for name, text in FILES.items():
    states = make_states(lib.compose(text))
    rows = []
    for st in states:
        rows += st
    write_gray_bmp(os.path.join(SHELL, name + '.bmp'), rows)
    previews.append((text, states))

# the empty cells of the atlas
d = bytearray(open(atlas_path, 'rb').read())
off = struct.unpack_from('<I', d, 10)[0]
h = im.h
stride = (im.w + 3) & ~3
# gray -> palette index
gray_of = [im.palette[i][0] if im.palette[i][0] == im.palette[i][1] == im.palette[i][2] else None for i in range(256)]


def nearest(v):
    best = None
    for i in range(256):
        if gray_of[i] is None:
            continue
        key = (abs(gray_of[i] - v), i)
        if best is None or key < best:
            best = key
    return best[1]


lut = [nearest(v) for v in range(256)]
assert lut[0] == 0, 'index 0 has to be black'
for cell, text in ATLAS_CELLS.items():
    states = make_states(lib.compose(text))
    for st in range(3):
        for y in range(CELL_H):
            row = cell * 78 + st * 26 + y
            base = off + (h - 1 - row) * stride
            for x in range(W):
                d[base + x] = lut[states[st][y][x]]
    previews.append((text + ' (atlas %d)' % cell, states))
open(atlas_path, 'wb').write(d)

if PREVIEW:
    rows = []
    scale = 2
    for text, states in previews:
        for st in states:
            for r in st[:20]:
                line = bytearray()
                for v in r:
                    line += bytes([min(255, v)] * 3) * scale
                for _ in range(scale):
                    rows.append(bytes(line))
        rows.append(bytes([120, 0, 0]) * (W * scale))
    write_png(PREVIEW, W * scale, len(rows), rows)
print('done:', len(FILES), 'files,', len(ATLAS_CELLS), 'atlas cells')
