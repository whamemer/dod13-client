"""Draws the textures of the touch buttons (touch/gfx/*.png, 128x128, RGBA): a dark olive disc with a light icon.
The icons are made of discs, rings, strokes and polygons, no picture is taken from another project.
usage: gen_touch_icons.py <extras dir> [contact sheet.png]"""
import sys, os, math
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bmptool import write_png_rgba, write_png

S = 128
SS = 3                      # subsamples per axis
BG = (44, 48, 38)
RIM = (116, 112, 84)
FG = (234, 228, 198)


class Shape:
    def __init__(self, f):
        self.f = f

    def __call__(self, x, y):
        return self.f(x, y)

    def __or__(self, other):
        return Shape(lambda x, y: self.f(x, y) or other.f(x, y))

    def __sub__(self, other):
        return Shape(lambda x, y: self.f(x, y) and not other.f(x, y))


def disc(cx, cy, r):
    return Shape(lambda x, y: (x - cx) ** 2 + (y - cy) ** 2 <= r * r)


def ring(cx, cy, r1, r2):
    return Shape(lambda x, y: r1 * r1 <= (x - cx) ** 2 + (y - cy) ** 2 <= r2 * r2)


def seg(x1, y1, x2, y2, w):
    dx, dy = x2 - x1, y2 - y1
    l2 = dx * dx + dy * dy

    def f(x, y):
        t = 0.0 if l2 == 0 else max(0.0, min(1.0, ((x - x1) * dx + (y - y1) * dy) / l2))
        px, py = x1 + t * dx, y1 + t * dy
        return (x - px) ** 2 + (y - py) ** 2 <= (w / 2.0) ** 2
    return Shape(f)


def poly(pts):
    n = len(pts)

    def f(x, y):
        inside = False
        j = n - 1
        for i in range(n):
            xi, yi = pts[i]; xj, yj = pts[j]
            if (yi > y) != (yj > y) and x < (xj - xi) * (y - yi) / (yj - yi) + xi:
                inside = not inside
            j = i
        return inside
    return Shape(f)


def rrect(x0, y0, x1, y1, r):
    def f(x, y):
        if x < x0 or x > x1 or y < y0 or y > y1:
            return False
        cx = min(max(x, x0 + r), x1 - r)
        cy = min(max(y, y0 + r), y1 - r)
        return (x - cx) ** 2 + (y - cy) ** 2 <= r * r
    return Shape(f)


def arc(cx, cy, r, w, a0, a1):
    """a ring sector, the angles in degrees, they grow clockwise on the screen (y goes down)"""
    def f(x, y):
        d = math.hypot(x - cx, y - cy)
        if abs(d - r) > w / 2.0:
            return False
        a = math.degrees(math.atan2(y - cy, x - cx)) % 360.0
        lo, hi = a0 % 360.0, a1 % 360.0
        return lo <= a <= hi if lo <= hi else (a >= lo or a <= hi)
    return Shape(f)


def mirror(shape):
    return Shape(lambda x, y: shape.f(S - x, y))


def flip(shape):
    return Shape(lambda x, y: shape.f(x, S - y))


def rot(shape, deg, cx=64, cy=64):
    c, s = math.cos(math.radians(-deg)), math.sin(math.radians(-deg))
    return Shape(lambda x, y: shape.f(cx + (x - cx) * c - (y - cy) * s, cy + (x - cx) * s + (y - cy) * c))


# ---- the icons ----------------------------------------------------------------------------------------------------------

def icon_attack():
    return ring(64, 64, 20, 27) | seg(64, 20, 64, 44, 7) | seg(64, 84, 64, 108, 7) | seg(20, 64, 44, 64, 7) | seg(84, 64, 108, 64, 7) | disc(64, 64, 4.5)


def icon_attack2():
    return (ring(64, 64, 34, 40) | seg(64, 22, 64, 106, 3.5) | seg(22, 64, 106, 64, 3.5)) - disc(64, 64, 9) | ring(64, 64, 9, 13)


def icon_reload():
    a1 = 300.0
    px, py = 64 + 30 * math.cos(math.radians(a1)), 64 + 30 * math.sin(math.radians(a1))
    tx, ty = -math.sin(math.radians(a1)), math.cos(math.radians(a1))
    nx, ny = math.cos(math.radians(a1)), math.sin(math.radians(a1))
    head = poly([(px + tx * 18, py + ty * 18), (px + nx * 16, py + ny * 16), (px - nx * 16, py - ny * 16)])
    return arc(64, 64, 30, 10, 20, a1) | head


def icon_jump():
    return poly([(64, 22), (96, 58), (77, 58), (77, 102), (51, 102), (51, 58), (32, 58)])


def icon_duck():
    return flip(icon_jump())


def icon_prone():
    return disc(30, 66, 10) | seg(46, 68, 98, 68, 15) | seg(98, 68, 112, 80, 11) | seg(22, 96, 106, 96, 5)


def icon_use():
    return seg(48, 30, 48, 98, 11) | seg(48, 32, 86, 32, 11) | seg(48, 64, 82, 64, 11) | seg(48, 96, 86, 96, 11)


def icon_knife():
    blade = poly([(60, 74), (100, 28), (108, 34), (84, 78), (70, 84)])
    return blade | seg(32, 100, 62, 74, 13) | seg(58, 66, 76, 86, 6)


def icon_pistol():
    return rrect(24, 40, 106, 60, 4) | poly([(40, 58), (66, 58), (56, 100), (34, 100)]) | ring(76, 66, 7, 11) | rrect(98, 44, 112, 54, 2)


def icon_rifle():
    stock = poly([(14, 56), (44, 54), (44, 74), (28, 86), (12, 78)])
    return stock | rrect(44, 52, 80, 70, 3) | rrect(80, 57, 114, 65, 2) | seg(52, 70, 56, 90, 8) | seg(82, 38, 82, 52, 4)


def icon_grenade():
    return disc(62, 76, 25) | rrect(54, 36, 70, 54, 3) | poly([(70, 42), (96, 36), (98, 46), (70, 54)]) | ring(44, 38, 6, 11) - rrect(40, 48, 48, 52, 0)


def icon_prev():
    return seg(80, 28, 46, 64, 13) | seg(46, 64, 80, 100, 13)


def icon_next():
    return mirror(icon_prev())


def icon_score():
    return disc(36, 40, 7) | disc(36, 64, 7) | disc(36, 88, 7) | seg(54, 40, 98, 40, 9) | seg(54, 64, 98, 64, 9) | seg(54, 88, 98, 88, 9)


def bubble():
    return rrect(22, 28, 106, 82, 14) | poly([(38, 78), (38, 104), (68, 80)])


def icon_chat():
    return bubble() - (disc(46, 55, 6) | disc(64, 55, 6) | disc(82, 55, 6))


def icon_chat_team():
    return bubble() - (seg(46, 44, 82, 44, 9) | seg(64, 44, 64, 70, 9))


def icon_signals():
    return bubble() - (seg(64, 38, 64, 58, 10) | disc(64, 70, 5.5))


def icon_changeteam():
    return seg(26, 48, 90, 48, 11) | poly([(88, 30), (112, 48), (88, 66)]) | seg(38, 82, 102, 82, 11) | poly([(40, 64), (16, 82), (40, 100)])


def icon_changeclass():
    dome = disc(64, 68, 34) - rrect(20, 68, 108, 110, 0)
    return dome | rrect(22, 66, 106, 80, 7) | seg(64, 36, 64, 52, 5)


def icon_cmdmenu():
    return rrect(30, 30, 60, 60, 6) | rrect(68, 30, 98, 60, 6) | rrect(30, 68, 60, 98, 6) | rrect(68, 68, 98, 98, 6)


def icon_menu():
    return seg(32, 38, 96, 38, 11) | seg(32, 64, 96, 64, 11) | seg(32, 90, 96, 90, 11)


def icon_map():
    return poly([(22, 42), (46, 32), (46, 92), (22, 102)]) | poly([(51, 32), (77, 42), (77, 102), (51, 92)]) | poly([(82, 42), (106, 32), (106, 92), (82, 102)])


def icon_mapzoom():
    return (ring(54, 54, 24, 33) | seg(77, 77, 102, 102, 13) | seg(54, 40, 54, 68, 6) | seg(40, 54, 68, 54, 6))


def icon_settings():
    g = disc(64, 64, 29)
    for i in range(8):
        g = g | rot(rrect(56, 20, 72, 44, 3), i * 45)
    return g - disc(64, 64, 11)


def icon_drop():
    return poly([(64, 82), (90, 54), (75, 54), (75, 26), (53, 26), (53, 54), (38, 54)]) | seg(34, 102, 94, 102, 10) | seg(34, 102, 34, 88, 10) | seg(94, 102, 94, 88, 10)


def icon_voice():
    return seg(64, 30, 64, 62, 26) | arc(64, 60, 30, 8, 10, 170) | seg(64, 90, 64, 102, 8) | seg(46, 104, 82, 104, 8)


ICONS = {
    'attack': icon_attack, 'attack2': icon_attack2, 'reload': icon_reload, 'jump': icon_jump, 'duck': icon_duck, 'prone': icon_prone,
    'use': icon_use, 'knife': icon_knife, 'pistol': icon_pistol, 'rifle': icon_rifle, 'grenade': icon_grenade, 'prev': icon_prev,
    'next': icon_next, 'score': icon_score, 'chat': icon_chat, 'chat_team': icon_chat_team, 'signals': icon_signals,
    'changeteam': icon_changeteam, 'changeclass': icon_changeclass, 'cmdmenu': icon_cmdmenu, 'menu': icon_menu, 'map': icon_map,
    'mapzoom': icon_mapzoom, 'settings': icon_settings, 'drop': icon_drop, 'voice': icon_voice,
}

BASE = disc(64, 64, 61.5)
RIMRING = ring(64, 64, 56.5, 61.5)


def render(icon):
    shape = icon()
    rows = []
    off = [(i + 0.5) / SS for i in range(SS)]
    for y in range(S):
        row = bytearray()
        for x in range(S):
            r = g = b = a = 0
            for oy in off:
                for ox in off:
                    px, py = x + ox, y + oy
                    if not BASE(px, py):
                        continue
                    c = FG if shape(px, py) else (RIM if RIMRING(px, py) else BG)
                    r += c[0]; g += c[1]; b += c[2]; a += 255
            n = SS * SS
            if a:
                cnt = a // 255
                row += bytes((r // cnt, g // cnt, b // cnt, a // n))
            else:
                row += bytes((0, 0, 0, 0))
        rows.append(bytes(row))
    return rows


def main():
    out = os.path.join(sys.argv[1], 'touch', 'gfx')
    os.makedirs(out, exist_ok=True)
    sheet = []
    names = sorted(ICONS)
    for name in names:
        rows = render(ICONS[name])
        write_png_rgba(os.path.join(out, name + '.png'), S, S, rows)
        sheet.append((name, rows))
    if len(sys.argv) > 2:
        cols = 7
        cell = 64
        nrows = (len(sheet) + cols - 1) // cols
        img = [[(70, 74, 60)] * (cols * cell) for _ in range(nrows * cell)]
        for i, (name, rows) in enumerate(sheet):
            ox, oy = (i % cols) * cell, (i // cols) * cell
            for y in range(cell):
                for x in range(cell):
                    px = rows[y * 2][x * 8:x * 8 + 4]
                    al = px[3] / 255.0
                    bgc = img[oy + y][ox + x]
                    img[oy + y][ox + x] = tuple(int(px[k] * al + bgc[k] * (1 - al)) for k in range(3))
        write_png(sys.argv[2], cols * cell, nrows * cell, [bytes(v for p in r for v in p) for r in img])
    print('icons:', len(names))


if __name__ == '__main__':
    main()
