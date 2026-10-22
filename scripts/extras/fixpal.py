"""Make index 0 of an 8 bpp BMP the black one: swap the palette entries and the pixel indices (the picture does not change).
mainui fills the free space of the buttons atlas with index 0, a light index 0 gives light frames around the buttons."""
import sys, struct, collections

def fix(path, out):
    d = bytearray(open(path, 'rb').read())
    off = struct.unpack_from('<I', d, 10)[0]
    hsz = struct.unpack_from('<I', d, 14)[0]
    w, h, planes, bpp = struct.unpack_from('<iiHH', d, 18)
    assert bpp == 8
    pal = 14 + hsz
    entry = lambda i: tuple(d[pal + i * 4: pal + i * 4 + 3])
    # the darkest entry, the most used one among them
    cnt = collections.Counter(d[off:])
    dark = min(range(256), key=lambda i: (sum(entry(i)), -cnt[i]))
    if dark == 0:
        print('already black at 0:', path); return False
    print('swap palette entries 0 and', dark, entry(0), entry(dark))
    for k in range(4):
        d[pal + k], d[pal + dark * 4 + k] = d[pal + dark * 4 + k], d[pal + k]
    tr = bytes(dark if i == 0 else 0 if i == dark else i for i in range(256))
    d[off:] = bytes(d[off:]).translate(tr)
    open(out, 'wb').write(d)
    return True

if __name__ == '__main__':
    fix(sys.argv[1], sys.argv[2])
