"""Compare raw frames written by the cross-build oracle (port/docs/ROADMAP.md 0.3).

Usage: oracle_diff.py RUN_DIR [FRAME ...]

For each frame present in RUN_DIR/hist-frames and RUN_DIR/port-frames, prints
the number of differing pixels and their bounding box, and writes
RUN_DIR/diff_<frame>.ppm: historical | port | difference mask (red).
Frame files: "BLPF", int32 width, height, bpp, [256 RGBX palette if 8 bpp],
rows of raw pixels (8 bpp indices or little-endian RGB565).
"""
import os
import struct
import sys


def load(path):
    data = open(path, 'rb').read()
    assert data[:4] == b'BLPF', path
    w, h, bpp = struct.unpack('<iii', data[4:16])
    off = 16
    pal = None
    if bpp == 8:
        pal = [tuple(data[off + i * 4: off + i * 4 + 3]) for i in range(256)]
        off += 1024
    step = bpp // 8
    pixels = []
    for i in range(w * h):
        p = data[off + i * step: off + (i + 1) * step]
        pixels.append(p[0] if bpp == 8 else struct.unpack('<H', p)[0])
    return w, h, bpp, pal, pixels


def rgb(bpp, pal, v):
    if bpp == 8:
        return pal[v]
    r, g, b = (v >> 11) & 31, (v >> 5) & 63, v & 31
    return (r << 3 | r >> 2, g << 2 | g >> 4, b << 3 | b >> 2)


def compare(run, frame):
    name = 'frame_%06d.raw' % frame
    a = load(os.path.join(run, 'hist-frames', name))
    b = load(os.path.join(run, 'port-frames', name))
    w, h, bpp = a[0], a[1], a[2]
    if (w, h, bpp) != b[:3]:
        print(frame, 'format differs', a[:3], b[:3])
        return
    diff = [i for i in range(w * h) if a[4][i] != b[4][i]]
    palette_diff = bpp == 8 and a[3] != b[3]
    if not diff:
        print('%6d identical%s' % (frame, ' (palette differs)' if palette_diff else ''))
        return
    xs = [i % w for i in diff]
    ys = [i // w for i in diff]
    print('%6d %7d pixels differ, box x %d..%d y %d..%d%s' % (
        frame, len(diff), min(xs), max(xs), min(ys), max(ys), ' (palette differs)' if palette_diff else ''))
    out = bytearray()
    mask = set(diff)
    for y in range(h):
        for img in (a, b):
            for x in range(w):
                out += bytes(rgb(bpp, img[3], img[4][y * w + x]))
        for x in range(w):
            out += b'\xff\x00\x00' if (y * w + x) in mask else b'\x20\x20\x20'
    path = os.path.join(run, 'diff_%06d.ppm' % frame)
    with open(path, 'wb') as f:
        f.write(b'P6\n%d %d\n255\n' % (w * 3, h))
        f.write(out)


def main():
    run = sys.argv[1]
    frames = [int(x) for x in sys.argv[2:]]
    if not frames:
        frames = sorted(int(n[6:12]) for n in os.listdir(os.path.join(run, 'hist-frames'))
                        if os.path.exists(os.path.join(run, 'port-frames', n)))
    for frame in frames:
        compare(run, frame)


if __name__ == '__main__':
    main()
