#!/usr/bin/env python3
"""List or restore research inputs retained during the recovery milestone cleanup.

The local archive is ignored build/recovery/research.zip. No executable is run.
Restoration only writes missing files beneath this repository's build/ or scratch/.
"""
import argparse
import fnmatch
import hashlib
import json
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('list', 'restore'))
    parser.add_argument('pattern', help='original repository-relative path or glob')
    args = parser.parse_args()
    with zipfile.ZipFile(ROOT/'build/recovery/research.zip') as archive:
        index = json.loads(archive.read('index.json'))
        matches = sorted(p for p in index if fnmatch.fnmatchcase(p, args.pattern.replace('\\','/')))
        if not matches:
            raise SystemExit('No archived paths match')
        for name in matches:
            row = index[name]
            if args.action == 'restore':
                destination = (ROOT/name).resolve()
                if not any(destination.is_relative_to(ROOT/folder) for folder in ('build','scratch')):
                    raise ValueError(f'Unsafe archive path: {name}')
                data = archive.read('blobs/'+row['sha256'])
                if len(data) != row['size'] or hashlib.sha256(data).hexdigest() != row['sha256']:
                    raise ValueError(f'Archive integrity failure: {name}')
                destination.parent.mkdir(parents=True, exist_ok=True)
                # Never overwrite canonical inputs or later experiment work.
                with destination.open('xb') as stream:
                    stream.write(data)
            print(f"{row['size']:>9} {name}")


if __name__ == '__main__':
    main()
