import sys, os, math, string
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bmptool import read_bmp
from glyphs import cell_rows, TEXTS

def solve(A, b):
    n = len(A); M = [row[:] + [b[i]] for i, row in enumerate(A)]
    for c in range(n):
        p = max(range(c, n), key=lambda r: abs(M[r][c])); M[c], M[p] = M[p], M[c]
        pv = M[c][c]
        for j in range(c, n + 1): M[c][j] /= pv
        for r in range(n):
            if r != c and M[r][c] != 0:
                f = M[r][c]
                for j in range(c, n + 1): M[r][j] -= f * M[c][j]
    return [M[i][n] for i in range(n)]

def ink_extent(rows, thr=18):
    w = len(rows[0])
    p = [sum(rows[y][x] for y in range(26)) / 26.0 for x in range(w)]
    xs = [x for x in range(w) if p[x] > thr]
    return min(xs), max(xs)

def fit(im, texts=TEXTS):
    letters = sorted(set(''.join(texts.values()).replace(' ', '')))
    names = letters + [' ', '#']        # '#' is the constant (the bearings of the ends)
    idx = {c: i for i, c in enumerate(names)}
    n = len(names)
    A = [[0.0] * n for _ in range(n)]; b = [0.0] * n
    rows_eq = []
    for cell, t in texts.items():
        rows = cell_rows(im, cell, 0)
        x0, x1 = ink_extent(rows)
        span = x1 - x0 + 1
        v = [0.0] * n
        for ch in t: v[idx[ch]] += 1.0
        v[idx['#']] = -1.0
        rows_eq.append((cell, t, v, span))
        for i in range(n):
            for j in range(n): A[i][j] += v[i] * v[j]
            b[i] += v[i] * span
    for i in range(n): A[i][i] += 0.05     # a little ridge
    sol = solve(A, b)
    adv = {c: sol[idx[c]] for c in names}
    res = []
    for cell, t, v, span in rows_eq:
        pred = sum(v[i] * sol[i] for i in range(n))
        res.append((t, span, pred))
    return adv, res

if __name__ == '__main__':
    im = read_bmp(sys.argv[1])
    adv, res = fit(im)
    print(' '.join('%s=%.1f' % (c, adv[c]) for c in sorted(adv)))
    err = [abs(s - p) for t, s, p in res]
    print('mean abs err %.2f max %.2f' % (sum(err) / len(err), max(err)))
    for t, s, p in sorted(res, key=lambda r: -abs(r[1] - r[2]))[:8]: print('  %-16s span %3d pred %.1f' % (t, s, p))
