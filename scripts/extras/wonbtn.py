"""Glyph library and composer for the labels of btns_main.bmp (the WON-menu style buttons of Day of Defeat).

The atlas holds 3 states of 156x26 pixels per button: 0 normal, 1 hover boost (0.4 of the normal one), 2 pressed (normal with a glow).
All labels are set in one font, all the letters are 15 rows high (rows 2..16), so a new label is composed from the letters of the
labels that exist: the letters are cut out of the normal state with the advance widths of the font (fitted by least squares from the
known texts), the other two states are made from the composed one with the relations measured on the original labels.
"""
import sys, os, math, struct
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bmptool import read_bmp
from glyphs import cell_rows, TEXTS
from advance import fit, ink_extent

W, CELL_H = 156, 26
START_X = 11
# the crossbar of the f starts a column left of the valley that separates it from the letter before
LEFT_EXT = {"f": 1}


def profile(rows):
    return [sum(rows[y][x] for y in range(CELL_H)) / float(CELL_H) for x in range(len(rows[0]))]


class Library:
    def __init__(self, im, exclude_texts=()):
        self.im = im
        texts = {c: t for c, t in TEXTS.items() if t not in exclude_texts}
        self.adv, _ = fit(im, texts)
        self.glyphs = {}      # letter -> list of (cost, width, rows)
        for cell, t in texts.items():
            rows = cell_rows(im, cell, 0)
            for ch, xa, xb, cost in self.segment(rows, t):
                xa -= LEFT_EXT.get(ch, 0)
                self.glyphs.setdefault(ch, []).append((cost, xb - xa, [r[xa:xb] for r in rows]))
        self.spacew = int(round(self.adv[' ']))

    def segment(self, rows, text):
        x0, x1 = ink_extent(rows)
        span = x1 - x0 + 1
        total = sum(self.adv[c] for c in text)
        k = span / total
        prof = profile(rows)
        bounds = []; cum = 0.0
        for c in text:
            bounds.append(x0 + cum * k); cum += self.adv[c] * k
        bounds.append(x0 + cum * k)
        # snap the inner boundaries to the closest valley
        snapped = [x0]
        for b in bounds[1:-1]:
            bi = int(round(b))
            cand = [(prof[bi + d], abs(d), bi + d) for d in range(-2, 3) if 0 < bi + d < len(prof) - 1]
            cand.sort()
            snapped.append(cand[0][2])
        snapped.append(x1 + 1)
        out = []
        for i, c in enumerate(text):
            if c == ' ':
                continue
            xa, xb = snapped[i], snapped[i + 1]
            cost = (prof[xa] if i > 0 else 0) + (prof[xb] if xb < len(prof) and i < len(text) - 1 else 0)
            out.append((c, xa, xb, cost))
        return out

    def glyph(self, ch):
        if ch in self.glyphs:
            # the cleanest cut, the width close to the median one
            lst = sorted(self.glyphs[ch], key=lambda g: g[1])
            med = lst[len(lst) // 2][1]
            return min(self.glyphs[ch], key=lambda g: g[0] + 6 * abs(g[1] - med))[2]
        if ch == 'b' and 'd' in self.glyphs:
            g = self.glyph('d')
            return [list(reversed(r)) for r in g]
        if ch == '/' and 'l' in self.glyphs:
            g = self.glyph('l')
            w = len(g[0])
            out = [[0] * (w + 8) for _ in range(CELL_H)]
            for y in range(CELL_H):
                sh = int(round((16 - y) * 0.45)) + 2
                for x in range(w):
                    if 0 <= x + sh < w + 8:
                        out[y][x + sh] = max(out[y][x + sh], g[y][x])
            return out
        raise KeyError(ch)

    def compose(self, text):
        out = [[0] * W for _ in range(CELL_H)]
        x = START_X
        for ch in text:
            if ch == ' ':
                x += self.spacew
                continue
            g = self.glyph(ch)
            w = len(g[0])
            for y in range(CELL_H):
                for i in range(w):
                    if x + i < W:
                        out[y][x + i] = max(out[y][x + i], g[y][i])
            x += w
        return out


def gauss_blur(rows, sigma):
    r = int(math.ceil(sigma * 3))
    k = [math.exp(-(i * i) / (2 * sigma * sigma)) for i in range(-r, r + 1)]
    s = sum(k); k = [v / s for v in k]
    h = len(rows); w = len(rows[0])
    tmp = [[0.0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            tmp[y][x] = sum(rows[y][x + i] * k[i + r] for i in range(-r, r + 1) if 0 <= x + i < w)
    out = [[0.0] * w for _ in range(h)]
    for x in range(w):
        for y in range(h):
            out[y][x] = sum(tmp[y + i][x] * k[i + r] for i in range(-r, r + 1) if 0 <= y + i < h)
    return out


def make_states(s0):
    s1 = [[int(round(v * 0.40)) for v in r] for r in s0]
    bl = gauss_blur(s0, 2.5)
    s2 = [[min(255, int(round(0.66 * s0[y][x] + 0.58 * bl[y][x]))) for x in range(len(s0[0]))] for y in range(len(s0))]
    return [s0, s1, s2]


def write_gray_bmp(path, rows_topdown):
    """8 bpp BMP with a gray palette, index 0 is black."""
    h = len(rows_topdown); w = len(rows_topdown[0])
    stride = (w + 3) & ~3
    pal = b''.join(bytes((i, i, i, 0)) for i in range(256))
    off = 14 + 40 + 1024
    data = b''.join(bytes(rows_topdown[y]) + b'\0' * (stride - w) for y in range(h - 1, -1, -1))
    hdr = b'BM' + struct.pack('<IHHI', off + len(data), 0, 0, off)
    info = struct.pack('<IiiHHIIiiII', 40, w, h, 1, 8, 0, len(data), 3780, 3780, 256, 0)
    open(path, 'wb').write(hdr + info + pal + data)
