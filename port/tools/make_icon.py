"""Makes the port's icon from the game's own Blupi sprite (waving).

    python make_icon.py <game folder>

Cuts sprite 47 of the Blupi sheet (IMAGE08/BLUPI000.BLP, rectangle from
table_icon_blupi in src/pixtables.h), makes the sheet's blue key colour
transparent and centres it on a 64x64 canvas. Writes:

    port/res/icon.png   the source image (Linux window icon, Android launcher)
    port/res/icon.ico   Windows: 16-64 px averaged down, 128 and 256 px in
                        whole pixels (stored as PNG)
"""
import os
import re
import struct
import sys
import zlib

SPRITE = 47     # front view, waving
CANVAS = 64

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))


def sprite_rect(index):
    text = open(os.path.join(ROOT, 'src', 'pixtables.h'), encoding='latin-1').read()
    body = re.search(r'table_icon_blupi\[\]\s*=\s*\{(.*?)\};', text, re.S).group(1)
    values = [int(v) for v in re.findall(r'-?\d+', body)]
    x, y, _, _, w, h = values[1 + index * 6:7 + index * 6]
    return x, y, w, h


def read_bmp8(path):
    d = open(path, 'rb').read()
    offset, header = struct.unpack('<II', d[10:18])
    width, height = struct.unpack('<ii', d[18:26])
    colours = struct.unpack('<I', d[46:50])[0] or 256
    palette = [tuple(d[14 + header + i * 4 + 2 - c] for c in range(3)) for i in range(colours)]
    stride = (width + 3) // 4 * 4

    def pixel(x, y):
        rgb = palette[d[offset + stride * (height - 1 - y) + x]]
        return rgb + ((0,) if rgb == (0, 0, 255) else (255,))
    return pixel


def png_bytes(image):
    size = len(image)
    raw = b''.join(b'\0' + b''.join(bytes(p) for p in row) for row in image)
    chunk = lambda t, p: struct.pack('>I', len(p)) + t + p + struct.pack('>I', zlib.crc32(t + p))
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', size, size, 8, 6, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b''))


def scaled(image, size):
    """Whole-pixel enlargement, or an area average (alpha-weighted) when smaller."""
    n = len(image)
    if size >= n:
        k = size // n
        return [[image[y // k][x // k] for x in range(size)] for y in range(size)]
    out = []
    step = n / size
    for ty in range(size):
        row = []
        for tx in range(size):
            acc = [0.0, 0.0, 0.0, 0.0]
            y0, y1, x0, x1 = ty * step, (ty + 1) * step, tx * step, (tx + 1) * step
            for sy in range(int(y0), min(n, int(y1 + 0.999))):
                wy = min(y1, sy + 1) - max(y0, sy)
                for sx in range(int(x0), min(n, int(x1 + 0.999))):
                    w = wy * (min(x1, sx + 1) - max(x0, sx))
                    r, g, b, a = image[sy][sx]
                    acc[0] += r * a * w
                    acc[1] += g * a * w
                    acc[2] += b * a * w
                    acc[3] += a * w
            area = step * step
            a = acc[3] / area
            if acc[3] > 0:
                row.append(tuple(round(acc[c] / acc[3]) for c in range(3)) + (round(a),))
            else:
                row.append((0, 0, 0, 0))
        out.append(row)
    return out


def dib_bytes(image):
    """32-bit BGRA DIB as stored in an ICO: double height, bottom-up, AND mask."""
    size = len(image)
    header = struct.pack('<IiiHHIIiiII', 40, size, size * 2, 1, 32, 0, 0, 0, 0, 0, 0)
    colour = b''.join(bytes((p[2], p[1], p[0], p[3])) for row in reversed(image) for p in row)
    mask_stride = (size + 31) // 32 * 4
    mask = b''
    for row in reversed(image):
        bits = bytearray(mask_stride)
        for x, p in enumerate(row):
            if p[3] == 0:
                bits[x // 8] |= 0x80 >> (x % 8)
        mask += bytes(bits)
    return header + colour + mask


def ico_bytes(images):
    entries, data = [], b''
    offset = 6 + 16 * len(images)
    for image in images:
        size = len(image)
        payload = png_bytes(image) if size >= 128 else dib_bytes(image)
        entries.append(struct.pack('<BBBBHHII', size % 256, size % 256, 0, 0, 1, 32, len(payload), offset + len(data)))
        data += payload
    return struct.pack('<HHH', 0, 1, len(images)) + b''.join(entries) + data


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    pixel = read_bmp8(os.path.join(sys.argv[1], 'IMAGE08', 'BLUPI000.BLP'))
    sx, sy, w, h = sprite_rect(SPRITE)
    x0, y0 = (CANVAS - w) // 2, (CANVAS - h) // 2
    image = [[(0, 0, 0, 0)] * CANVAS for _ in range(CANVAS)]
    for y in range(h):
        for x in range(w):
            p = pixel(sx + x, sy + y)
            image[y0 + y][x0 + x] = p if p[3] else (0, 0, 0, 0)

    out = os.path.join(ROOT, 'port', 'res')
    os.makedirs(out, exist_ok=True)
    open(os.path.join(out, 'icon.png'), 'wb').write(png_bytes(image))
    sizes = [16, 24, 32, 48, 64, 128, 256]
    open(os.path.join(out, 'icon.ico'), 'wb').write(ico_bytes([scaled(image, s) for s in sizes]))
    print('port/res/icon.png, port/res/icon.ico: sprite %d (%dx%d)' % (SPRITE, w, h))


if __name__ == '__main__':
    main()
