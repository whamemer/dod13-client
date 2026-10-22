"""Tiny BMP reader / PNG writer without PIL: indexed 8 bpp and 24/32 bpp bottom-up BMP."""
import struct, zlib, sys


class Image:
    def __init__(self, w, h, rows, palette=None, bpp=8):
        self.w, self.h, self.rows, self.palette, self.bpp = w, h, rows, palette, bpp  # rows: top-down, each bytes of indices or BGR(A)

    def rgb(self, x, y):
        r = self.rows[y]
        if self.bpp == 8:
            b, g, rr, _ = self.palette[r[x]]
            return (rr, g, b)
        if self.bpp == 24:
            return (r[x * 3 + 2], r[x * 3 + 1], r[x * 3])
        return (r[x * 4 + 2], r[x * 4 + 1], r[x * 4])


def read_bmp(path):
    d = open(path, 'rb').read()
    assert d[:2] == b'BM', path
    off = struct.unpack_from('<I', d, 10)[0]
    hsz = struct.unpack_from('<I', d, 14)[0]
    w, h, planes, bpp, comp, isz, xp, yp, clr, imp = struct.unpack_from('<iiHHIIiiII', d, 18)
    assert comp == 0, 'compressed bmp'
    pal = None
    if bpp == 8:
        n = clr or 256
        pal = [struct.unpack_from('<BBBB', d, 14 + hsz + i * 4) for i in range(n)]
        while len(pal) < 256:
            pal.append((0, 0, 0, 0))
    stride = ((w * bpp + 31) // 32) * 4
    rows = []
    top_down = h < 0
    ah = abs(h)
    for y in range(ah):
        sy = y if top_down else ah - 1 - y
        rows.append(d[off + sy * stride: off + sy * stride + (w * bpp + 7) // 8])
    return Image(w, ah, rows, pal, bpp)


def write_png(path, w, h, rgb_rows):
    raw = b''.join(b'\x00' + bytes(r) for r in rgb_rows)

    def chunk(t, data):
        c = struct.pack('>I', len(data)) + t + data
        return c + struct.pack('>I', zlib.crc32(t + data) & 0xffffffff)

    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(raw, 6)) + chunk(b'IEND', b'')
    open(path, 'wb').write(png)


def crop_to_png(img, path, x0, y0, w, h, scale=1, bg=None):
    rows = []
    for y in range(y0, min(y0 + h, img.h)):
        row = bytearray()
        for x in range(x0, min(x0 + w, img.w)):
            p = img.rgb(x, y)
            row += bytes(p) * scale
        for _ in range(scale):
            rows.append(bytes(row))
    write_png(path, min(w, img.w - x0) * scale, len(rows), rows)


if __name__ == '__main__':
    img = read_bmp(sys.argv[1])
    print(img.w, img.h, img.bpp)
    if len(sys.argv) > 2:
        x0, y0, w, h, sc = [int(v) for v in sys.argv[3:8]] if len(sys.argv) > 7 else (0, 0, img.w, img.h, 1)
        crop_to_png(img, sys.argv[2], x0, y0, w, h, sc)


def write_png_rgba(path, w, h, rgba_rows):
    """rgba_rows: top-down, every row is w*4 bytes."""
    raw = b''.join(b'\x00' + bytes(r) for r in rgba_rows)

    def chunk(t, data):
        c = struct.pack('>I', len(data)) + t + data
        return c + struct.pack('>I', zlib.crc32(t + data) & 0xffffffff)

    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, 6, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b'')
    open(path, 'wb').write(png)


def write_bmp24(path, w, h, rgb_rows):
    """24 bpp BMP; rgb_rows: top-down, every row is a list of (r, g, b)."""
    stride = (w * 3 + 3) & ~3
    data = b''.join(bytes(v for p in rgb_rows[y] for v in (p[2], p[1], p[0])) + b'\0' * (stride - w * 3) for y in range(h - 1, -1, -1))
    hdr = b'BM' + struct.pack('<IHHI', 54 + len(data), 0, 0, 54)
    info = struct.pack('<IiiHHIIiiII', 40, w, h, 1, 24, 0, len(data), 3780, 3780, 0, 0)
    open(path, 'wb').write(hdr + info + data)
