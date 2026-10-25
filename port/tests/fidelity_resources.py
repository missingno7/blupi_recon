"""Original executables are parsed as resource data only; never executed.

The RT_STRING length/block decoder here is independent of language_pack.py.
Expectations are generated into the test work dir, never installed as goldens.
"""
import argparse
from pathlib import Path
import struct
import subprocess
import pefile


def resources(path):
    result = {}
    with pefile.PE(str(path)) as pe:
        for kind in pe.DIRECTORY_ENTRY_RESOURCE.entries:
            if kind.id != 6:
                continue
            for block in kind.directory.entries:
                for language in block.directory.entries:
                    record = language.data.struct
                    data = pe.get_data(record.OffsetToData, record.Size)
                    offset = 0
                    for index in range(16):
                        length, = struct.unpack_from('<H', data, offset)
                        offset += 2
                        text = data[offset:offset + length * 2].decode('utf-16le')
                        offset += length * 2
                        if length:
                            sid = 16 * (block.id - 1) + index
                            value = text.encode('cp1252')
                            assert sid not in result or result[sid] == value
                            result[sid] = value
    assert result
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--eggbert2', type=Path, required=True, help='Eggbert 2 game root')
    ap.add_argument('--blupi2', type=Path, required=True, help='Blupi 2 game root')
    ap.add_argument('--blupi2-variants', type=Path, required=True, help='holds deutsch/ and francais/')
    ap.add_argument('--languages', type=Path, required=True, help='port/games/sb2/languages')
    ap.add_argument('--exe', type=Path, required=True)
    ap.add_argument('--work', type=Path, required=True)
    args = ap.parse_args()
    cases = [('eggbert-en', args.eggbert2, 'Eggbert2.exe', ''),
             ('blupi-en', args.blupi2, 'BLUPI.EXE', ''),
             ('blupi-de', args.blupi2, 'Blupi.exe', 'deutsch'),
             ('blupi-fr', args.blupi2, 'Blupi.exe', 'francais')]
    args.work.mkdir(parents=True, exist_ok=True)
    for pack, data, name, variant in cases:
        language = args.blupi2_variants / variant if variant else data
        original = next((p for p in language.iterdir() if p.name.lower() == name.lower()), language / name)
        if not original.is_file():
            raise FileNotFoundError(original)
        expected = resources(original)
        fixture = args.work / (pack + '.bin')
        with fixture.open('wb') as out:
            out.write(struct.pack('<I', len(expected)))
            for sid, value in sorted(expected.items()):
                out.write(struct.pack('<II', sid, len(value)))
                out.write(value)
        subprocess.run([str(args.exe), str(data), str(args.work / pack), pack,
                        str(args.languages / (pack + '.txt')),
                        str(fixture), str(language)], check=True, timeout=30)


if __name__ == '__main__':
    main()
