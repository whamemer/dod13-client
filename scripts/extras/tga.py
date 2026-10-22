"""Reader of uncompressed true color TGA files (24 and 32 bpp)."""
import struct


def read_tga(path):
    d = open(path, 'rb').read()
    idlen, cmap, itype = d[0], d[1], d[2]
    assert itype == 2 and cmap == 0, 'only uncompressed true color is supported: ' + path
    w, h = struct.unpack_from('<HH', d, 12)
    bpp, desc = d[16], d[17]
    step = bpp // 8
    off = 18 + idlen
    rows = []
    for y in range(h):
        sy = y if desc & 0x20 else h - 1 - y          # bit 5 set: the rows go from the top
        row = d[off + sy * w * step: off + (sy + 1) * w * step]
        rows.append([(row[i * step + 2], row[i * step + 1], row[i * step]) for i in range(w)])
    return w, h, rows
