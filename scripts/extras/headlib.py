"""Letters and composer for the banners head_*.bmp (460x80, a fraktur font, the letters have descenders so the whole 80 rows are copied).
The same method as wonbtn.py: advance widths fitted from the known texts, cuts snapped to the valleys of the column profile."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bmptool import read_bmp
from advance import solve

W, H = 460, 80

HEAD_TEXTS = {'head_advanced': 'advanced', 'head_audio': 'audio', 'head_config': 'configuration', 'head_controls': 'controls',
              'head_creategame': 'create game', 'head_createroom': 'create room', 'head_custom': 'custom game', 'head_customize': 'customize',
              'head_filter': 'filter', 'head_gameopts': 'options', 'head_gore': 'content control', 'head_inetgames': 'internet games',
              'head_keyboard': 'keyboard', 'head_lan': 'lan game', 'head_load': 'load game', 'head_multi': 'multiplayer',
              'head_newgame': 'new game', 'head_readme': 'readme text', 'head_room': 'chat room', 'head_rooms': 'chat rooms',
              'head_save': 'save game', 'head_single': 'single player', 'head_specgames': 'spectate games', 'head_video': 'video',
              'head_vidmodes': 'video modes', 'head_vidoptions': 'video options'}


def load_gray(path):
    im = read_bmp(path)
    return [[im.palette[im.rows[y][x]][2] if im.bpp == 8 else im.rgb(x, y)[0] for x in range(im.w)] for y in range(im.h)]


def profile(rows):
    h = len(rows)
    return [sum(rows[y][x] for y in range(h)) / float(h) for x in range(len(rows[0]))]


def ink_extent(rows, thr=6):
    p = profile(rows)
    xs = [x for x in range(len(p)) if p[x] > thr]
    return min(xs), max(xs)


class HeadLibrary:
    def __init__(self, shell, exclude=()):
        self.imgs = {}
        for name, text in HEAD_TEXTS.items():
            if name in exclude:
                continue
            p = os.path.join(shell, name + '.bmp')
            if os.path.exists(p):
                self.imgs[name] = (text, load_gray(p))
        self.fit()
        self.glyphs = {}
        for name, (text, rows) in self.imgs.items():
            for ch, xa, xb, cost in self.segment(rows, text):
                self.glyphs.setdefault(ch, []).append((cost, xb - xa, name, clean_glyph([r[xa:xb] for r in rows])))

    def fit(self):
        letters = sorted(set(''.join(t for t, _ in self.imgs.values()).replace(' ', '')))
        names = letters + [' ']
        idx = {c: i for i, c in enumerate(names)}
        n = len(names)
        A = [[0.0] * n for _ in range(n)]; b = [0.0] * n
        for text, rows in self.imgs.values():
            x0, x1 = ink_extent(rows)
            v = [0.0] * n
            for ch in text: v[idx[ch]] += 1.0
            for i in range(n):
                for j in range(n): A[i][j] += v[i] * v[j]
                b[i] += v[i] * (x1 - x0 + 1)
        # a prior toward the mean advance keeps the letters that are in few words (y, b, w) sane
        mean = sum(len(rows[0]) for _, rows in self.imgs.values()) * 0 + 21.0
        for i in range(n):
            A[i][i] += 3.0
            b[i] += 3.0 * (mean if names[i] != " " else 16.0)
        sol = solve(A, b)
        self.adv = {c: sol[idx[c]] for c in names}
        self.err = []
        for text, rows in self.imgs.values():
            x0, x1 = ink_extent(rows)
            self.err.append(abs(sum(self.adv[c] for c in text) - (x1 - x0 + 1)))

    def segment(self, rows, text):
        x0, x1 = ink_extent(rows)
        k = (x1 - x0 + 1) / sum(self.adv[c] for c in text)
        prof = profile(rows)
        bounds = []; cum = 0.0
        for c in text:
            bounds.append(x0 + cum * k); cum += self.adv[c] * k
        bounds.append(x0 + cum * k)
        snapped = [x0]
        for b in bounds[1:-1]:
            bi = int(round(b))
            cand = sorted((prof[bi + d], abs(d), bi + d) for d in range(-3, 4) if 0 < bi + d < len(prof) - 1)
            snapped.append(cand[0][2])
        snapped.append(x1 + 1)
        out = []
        for i, c in enumerate(text):
            if c == ' ':
                continue
            xa, xb = snapped[i], snapped[i + 1]
            cost = (prof[xa] if i > 0 else 0) + (prof[xb] if i < len(text) - 1 else 0)
            out.append((c, xa, xb, cost))
        return out

    def glyph(self, ch, prefer=None):
        lst = self.glyphs[ch]
        if prefer:
            pl = [g for g in lst if g[2] in prefer]
            if pl:
                lst = pl
        med = sorted(g[1] for g in lst)[len(lst) // 2]
        return min(lst, key=lambda g: g[0] + 2 * abs(g[1] - med))[3]

    def compose(self, text, start_x=22, prefer=None):
        out = [[0] * W for _ in range(H)]
        x = start_x
        for ch in text:
            if ch == ' ':
                x += int(round(self.adv[' ']))
                continue
            g = self.glyph(ch, (prefer or {}).get(ch))
            for y in range(H):
                for i in range(len(g[0])):
                    if x + i < W:
                        out[y][x + i] = max(out[y][x + i], g[y][i])
            x += len(g[0])
        return out


def clean_glyph(g, thr=30):
    """Removes the small spots at the cut edges: pieces of the neighbors that the cut did not separate."""
    h = len(g); w = len(g[0])
    seen = [[False] * w for _ in range(h)]
    out = [list(r) for r in g]
    for y in range(h):
        for x in range(w):
            if g[y][x] > thr and not seen[y][x]:
                comp = []; stack = [(y, x)]; seen[y][x] = True
                while stack:
                    cy, cx = stack.pop(); comp.append((cy, cx))
                    for dy in (-1, 0, 1):
                        for dx in (-1, 0, 1):
                            ny, nx = cy + dy, cx + dx
                            if 0 <= ny < h and 0 <= nx < w and not seen[ny][nx] and g[ny][nx] > thr:
                                seen[ny][nx] = True; stack.append((ny, nx))
                xs = [c[1] for c in comp]; ys = [c[0] for c in comp]
                at_edge = min(xs) <= 1 or max(xs) >= w - 2
                if at_edge and len(comp) < 14 and (max(ys) - min(ys)) < 10:
                    for cy, cx in comp:
                        for dy in (-1, 0, 1):
                            for dx in (-1, 0, 1):
                                ny, nx = cy + dy, cx + dx
                                if 0 <= ny < h and 0 <= nx < w and (min(xs) <= 1 and nx <= 2 or max(xs) >= w - 2 and nx >= w - 3):
                                    out[ny][nx] = 0
    return out
