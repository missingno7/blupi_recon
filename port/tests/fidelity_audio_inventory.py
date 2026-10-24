"""Read-only inventory of original SOUND WAVs, never execute game binaries.

The mixer fixture independently tests every format admitted below. This inventory
fails if a new original format would fall outside that fixture's proof scope.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--assets", type=Path, required=True)
    parser.add_argument("--receipt", type=Path)
    args = parser.parse_args()
    formats, folders = Counter(), Counter()
    rows = []
    for folder in sorted(args.assets.rglob("*")):
        if not folder.is_dir() or folder.name.lower() != "sound":
            continue
        for path in sorted(folder.iterdir()):
            if not path.is_file():
                continue
            with path.open("rb") as stream:
                header = stream.read(44)
            if header[:4] != b"RIFF" or header[8:12] != b"WAVE":
                continue  # original MIDI files also use the .BLP suffix
            assert len(header) == 44, f"short header: {path}"
            tag, channels, rate, avg, align, bits = struct.unpack_from("<HHIIHH", header, 20)
            size = struct.unpack_from("<I", header, 40)[0]
            fmt = (rate, bits, channels)
            assert fmt in {(11025, 8, 1), (11025, 16, 1),
                           (22050, 8, 1), (22050, 16, 1)}, f"untested format {fmt}: {path}"
            assert tag == 1 and align == channels * bits // 8 and avg == rate * align, path
            assert header[12:16] == b"fmt " and header[36:40] == b"data", path
            assert struct.unpack_from("<I", header, 16)[0] == 16, path
            assert size >= 4 and size % align == 0 and path.stat().st_size >= 44 + size, path
            relative = path.relative_to(args.assets).as_posix()
            formats[str(fmt)] += 1
            folders[path.parent.relative_to(args.assets).as_posix()] += 1
            rows.append({"path": relative, "format": fmt, "data_bytes": size,
                         "header_sha256": hashlib.sha256(header).hexdigest()})
    assert rows, "no original SOUND WAVs available"
    receipt = {"formats": dict(sorted(formats.items())), "folders": dict(sorted(folders.items())),
               "wav_count": len(rows), "files": rows}
    if args.receipt:
        args.receipt.parent.mkdir(parents=True, exist_ok=True)
        args.receipt.write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    print(f"{len(rows)} original WAVs, {len(folders)} SOUND folders: {dict(formats)}")


if __name__ == "__main__":
    main()
