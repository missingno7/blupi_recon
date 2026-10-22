"""Exports the game's images (IMAGE16 / IMAGE08 *.BLP, which are BMP files) as PNG.

    python export_graphics.py <game folder> <output folder>

For each image name the IMAGE16 version is used when there is one (24-bit, the
true-colour screens), otherwise the IMAGE08 one (8-bit, palette kept: an
indexed PNG, so an edited image converts back without colour drift). Pixels
are unchanged; pure blue (0, 0, 255) is the transparent colour of the sprite
sheets, as in the game (CPixmap::SetTransparent). An index.md lists every
image with its size, depth and use.
"""
import os
import struct
import sys
import zlib

# What the game uses each image for (port/game/main.cpp DoInit, src/event.cpp
# phase table, src/decblock.cpp). Sprite sheets are cut by rectangle tables
# (src/pixtables.h), not by a uniform grid.
USES = {
    'BLUPI000': 'Blupi sprites (player 1, channel 2); sheet cut by table_icon_blupi',
    'BLUPI001': 'Blupi sprites, player 2 colours (channel 11)',
    'BLUPI002': 'Blupi sprites, player 3 colours (channel 12)',
    'BLUPI003': 'Blupi sprites, player 4 colours (channel 13)',
    'OBJECT': 'decor tiles and objects (channel 1); cut by table_icon_object',
    'ELEMENT': 'effects, HUD icons, cursors (channel 10); cut by table_icon_element',
    'EXPLO': 'explosions and clouds (channel 9); cut by table_icon_explo',
    'BUTTON00': 'buttons: 40x40 grid, 6 per row (channel 4); row 0 = button states, then icons',
    'JAUGE': 'gauges (channel 5)',
    'TEXT': 'main font (channel 6)',
    'LITTLE': 'small font (channel 7)',
    'MAP': 'minimap frame (channel 8)',
    'TEMP': 'scratch surface for the hide effect (channel 14)',
    'INIT': 'title screen',
}
for i in range(24):
    USES['DECOR%03d' % i] = 'parallax background of a world (channel 3)'
SCREENS = ('BYE CLEAR CREATE GAMER GREAD GWRITE HELP INFO INSERT LOST MOVIE MULTI MUSIC NAME READ '
           'REGION SERVICE SESSION SETUP STOP WIN WRITE').split()
for s in SCREENS:
    USES.setdefault(s, 'menu screen background')


def read_bmp(path):
    d = open(path, 'rb').read()
    if d[:2] != b'BM':
        raise ValueError('not a BMP')
    off = struct.unpack_from('<I', d, 10)[0]
    hs = struct.unpack_from('<I', d, 14)[0]
    w, h = struct.unpack_from('<ii', d, 18)
    bpp = struct.unpack_from('<H', d, 28)[0]
    comp = struct.unpack_from('<I', d, 30)[0]
    if comp != 0 or bpp not in (8, 24):
        raise ValueError('unsupported BMP (bpp %d, compression %d)' % (bpp, comp))
    top_down = h < 0
    h = abs(h)
    stride = ((w * bpp + 31) // 32) * 4
    rows = []
    for y in range(h):
        src = y if top_down else h - 1 - y
        rows.append(d[off + src * stride: off + src * stride + (w * bpp) // 8])
    palette = None
    if bpp == 8:
        count = struct.unpack_from('<I', d, 46)[0] or 256
        base = 14 + hs
        palette = [d[base + i * 4: base + i * 4 + 3][::-1] for i in range(count)]
    return w, h, bpp, rows, palette


def chunk(kind, data):
    return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff)


def write_png(path, w, h, bpp, rows, palette):
    raw = bytearray()
    for row in rows:
        raw.append(0)
        if bpp == 24:
            for i in range(0, len(row), 3):
                raw += row[i + 2:i + 3] + row[i + 1:i + 2] + row[i:i + 1]   # BGR -> RGB
        else:
            raw += row
    if bpp == 24:
        header = struct.pack('>IIBBBBB', w, h, 8, 2, 0, 0, 0)
        extra = b''
    else:
        header = struct.pack('>IIBBBBB', w, h, 8, 3, 0, 0, 0)
        extra = chunk(b'PLTE', b''.join(palette))
    with open(path, 'wb') as f:
        f.write(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', header) + extra +
                chunk(b'IDAT', zlib.compress(bytes(raw), 9)) + chunk(b'IEND', b''))


def main():
    root, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    found = {}
    for sub in ('IMAGE08', 'IMAGE16'):   # IMAGE16 last: it wins
        folder = os.path.join(root, sub)
        for name in sorted(os.listdir(folder)):
            stem, ext = os.path.splitext(name)
            if ext.upper() == '.BLP':
                found[stem.upper()] = (sub, os.path.join(folder, name))
    lines = ['# Speedy Blupi images', '',
             'Exported from the game folder by `port/tools/export_graphics.py`. Pixels unchanged.',
             'Pure blue (0, 0, 255) is the transparent colour of the sprite sheets. 8-bit images are',
             'indexed PNGs with their own palette; edit them in a palette-aware editor to keep it.', '',
             '| Image | From | Size | Depth | Use |', '|---|---|---|---|---|']
    for stem in sorted(found):
        sub, path = found[stem]
        w, h, bpp, rows, palette = read_bmp(path)
        png = stem.lower() + '.png'
        write_png(os.path.join(out, png), w, h, bpp, rows, palette)
        depth = '24-bit RGB' if bpp == 24 else '8-bit indexed'
        lines.append('| %s | %s | %dx%d | %s | %s |' % (png, sub, w, h, depth, USES.get(stem, '')))
        print('%-14s %-8s %4dx%-4d %s' % (png, sub, w, h, depth))
    with open(os.path.join(out, 'index.md'), 'w', encoding='utf-8', newline='\n') as f:
        f.write('\n'.join(lines) + '\n')


if __name__ == '__main__':
    main()
