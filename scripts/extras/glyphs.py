import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from bmptool import read_bmp

TEXTS = {1: 'resume game', 3: 'configuration', 6: 'view readme', 7: 'quit', 8: 'multiplayer', 14: 'cancel', 15: 'game options',
         16: 'video', 17: 'audio', 18: 'controls', 19: 'done', 20: 'quickstart', 21: 'use defaults', 22: 'ok', 23: 'video options',
         24: 'video modes', 25: 'advanced', 27: 'delete', 28: 'internet games', 29: 'chat rooms', 30: 'lan games', 31: 'customize',
         32: 'skip', 33: 'exit', 34: 'connect', 35: 'refresh', 36: 'filter', 38: 'create', 39: 'create game', 41: 'list rooms',
         42: 'search', 43: 'servers', 44: 'join', 45: 'find', 46: 'create room', 47: 'join game', 49: 'find game', 50: 'start game',
         51: 'view info', 52: 'update', 53: 'add server', 54: 'disconnect', 55: 'console', 58: 'visit dod', 59: 'how to play',
         61: 'three d info', 62: 'custom game', 63: 'activate', 64: 'install', 65: 'visit site', 67: 'deactivate', 69: 'spectate'}

def cell_rows(im, idx, st):
    return [[im.palette[im.rows[idx * 78 + st * 26 + y][x]][0] for x in range(im.w)] for y in range(26)]

def segments(rows, thr=40, mingap=1):
    w = len(rows[0])
    ink = [max(rows[y][x] for y in range(26)) > thr for x in range(w)]
    segs = []; x = 0
    while x < w:
        if ink[x]:
            s = x
            while x < w and ink[x]: x += 1
            segs.append([s, x - 1])
        else:
            x += 1
    return segs

if __name__ == '__main__':
    im = read_bmp(sys.argv[1])
    ok = bad = 0
    for idx, t in sorted(TEXTS.items()):
        rows = cell_rows(im, idx, 0)
        segs = segments(rows)
        n = len(t.replace(' ', ''))
        flag = 'ok ' if len(segs) == n else 'BAD'
        if len(segs) == n: ok += 1
        else: bad += 1
        top = min((y for y in range(26) if max(rows[y]) > 40), default=-1); bot = max((y for y in range(26) if max(rows[y]) > 40), default=-1)
        print('%s %2d %-16s letters=%2d segs=%2d rows %d..%d' % (flag, idx, t, n, len(segs), top, bot))
    print('ok', ok, 'bad', bad)
