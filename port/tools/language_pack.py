"""Writes a language pack (port/docs/LANGUAGES.md) from a game executable.

    python language_pack.py EXE --game sb1 --edition blupi --code de --name Deutsch
                            [--variant de] [--base BASE_EXE] [-o FILE]

Reads the executable's string table and its character table (table_char), and
writes the pack: the header, the [chars] entries where the character table
differs from BASE_EXE's (the edition's English build), and every string. Only
data is read; the executable is never run.
"""
import argparse
import sys

import pefile

ACCENTS = {0x02: 'acute', 0x03: 'grave', 0x7E: 'tilde'}
# table_char's printable ASCII entries (0x20-0x7f) are the same in every build
# and locate the table in .data.
ASCII_FIRST, ASCII_LAST = 0x20, 0x80


def strings(pe):
    out = {}
    for t in pe.DIRECTORY_ENTRY_RESOURCE.entries:
        if t.id != 6:
            continue
        for block in t.directory.entries:
            for lang in block.directory.entries:
                data = pe.get_data(lang.data.struct.OffsetToData, lang.data.struct.Size)
                pos = 0
                for i in range(16):
                    n = int.from_bytes(data[pos:pos + 2], 'little')
                    pos += 2
                    if n:
                        out[(block.id - 1) * 16 + i] = data[pos:pos + 2 * n].decode('utf-16-le')
                        pos += 2 * n
    return out


def table_char(pe, reference=None):
    data = [s for s in pe.sections if s.Name.startswith(b'.data')][0].get_data()
    if reference is None:
        # first build: the ASCII glyph ranks equal the codes from 0x21 to 0x7e
        probe = b''.join(bytes([c, 0, 0, 0xFF, 0, 0]) for c in range(0x21, 0x7F))
        k = data.find(probe)
        if k < 0:
            sys.exit('table_char not found')
        return data[k - 0x21 * 6:k - 0x21 * 6 + 1536]
    probe = reference[ASCII_FIRST * 6:ASCII_LAST * 6]
    k = data.find(probe)
    if k < 0:
        sys.exit('table_char not found')
    start = k - ASCII_FIRST * 6
    return data[start:start + 1536]


def signed(b):
    return b - 256 if b > 127 else b


def letter(code):
    return bytes([code]).decode('cp1252', errors='replace')


def describe(code, entry, base):
    """How a pack writes table_char[code] = entry, given the base table."""
    rank, bx, by, accent, ax, ay = entry
    if accent == 0xFF and bx == 0 and by == 0:
        # a glyph of the font: name a letter that already draws it
        for c in list(range(0x61, 0x7B)) + list(range(0xE0, 0x100)) + list(range(0x21, 0x7F)):
            if base[c * 6:c * 6 + 6] == entry:
                return letter(c)
        return '#%02x' % rank
    base_letter = None
    for c in range(0x21, 0x7F):
        if base[c * 6] == rank and base[c * 6 + 3] == 0xFF:
            base_letter = letter(c)
            break
    name = ACCENTS.get(accent, '#%02x' % accent)
    out = '%s + %s @ %d,%d' % (base_letter or '#%02x' % rank, name, signed(ax), signed(ay))
    if bx or by:
        out += ' base @ %d,%d' % (signed(bx), signed(by))
    return out


def quote(text):
    text = text.replace('\\', '\\\\').replace('\n', '\\n')
    if text != text.strip(' ') or (text.startswith('"') and text.endswith('"')):
        text = '"' + text + '"'
    return text


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('exe')
    ap.add_argument('--game', required=True)
    ap.add_argument('--edition', required=True)
    ap.add_argument('--code', required=True)
    ap.add_argument('--name', required=True)
    ap.add_argument('--variant', help='language variant of the game code (default: the code)')
    ap.add_argument('--base', help="the edition's English executable (default: EXE itself)")
    ap.add_argument('-o', '--output')
    args = ap.parse_args()

    pe = pefile.PE(args.exe)
    base_pe = pefile.PE(args.base) if args.base else pe
    base = table_char(base_pe)
    table = table_char(pe, base)

    lines = [
        '# Speedy Blupi language pack (port/docs/LANGUAGES.md)',
        'name    = %s' % args.name,
        'code    = %s' % args.code,
        'game    = %s' % args.game,
        'edition = %s' % args.edition,
        'variant = %s' % (args.variant or args.code),
        '',
        '[chars]',
    ]
    for code in range(256):
        entry = table[code * 6:code * 6 + 6]
        if entry != base[code * 6:code * 6 + 6]:
            lines.append('%s = %s' % (letter(code), describe(code, entry, base)))
    lines += ['', '[strings]']
    for sid, text in sorted(strings(pe).items()):
        lines.append('%d = %s' % (sid, quote(text)))
    out = '\n'.join(lines) + '\n'
    if args.output:
        open(args.output, 'w', encoding='utf-8', newline='\n').write(out)
    else:
        sys.stdout.write(out)


if __name__ == '__main__':
    main()
