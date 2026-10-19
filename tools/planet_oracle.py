#!/usr/bin/env python3
"""Reproduce the bounded Planet Blupi source/binary study (static game analysis).

Inputs: official ISOs extracted under build/planet-forensics/disc{,-it};
read-only external/planetblupi Git history. See docs/PLANET_ORACLE.md.
Generated sources, objects and reports remain in ignored build/planet-forensics.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import sys
import shutil
import time
from datetime import datetime, timezone

import pefile
from cmpfunc import Coff, candidates_for
import cmpfunc

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'build/planet-forensics'
REF = 'b0270ab432ac5c65337a6f16dd525a11d371f4b8'
REPO = ROOT / 'external/planetblupi'


def sha(data):
    return hashlib.sha256(data).hexdigest()


def payloads(coff):
    return {f.name: (f.data, [(r.offset - f.start, r.typ,
                             coff.sym_by_index[r.symbol_index].name) for r in f.relocs])
            for f in coff.functions()}


def save(name, data):
    OUT.mkdir(parents=True, exist_ok=True)
    (OUT / name).write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')


def prepare():
    def git(*args):
        return subprocess.check_output(['git', '-C', str(REPO), *args])
    files = git('ls-tree', '--name-only', REF).decode().splitlines()
    receipts = []
    dest = OUT / 'source'
    dest.mkdir(parents=True, exist_ok=True)
    for name in files:
        if Path(name).suffix not in ('.cpp', '.h'):
            continue
        raw = git('show', f'{REF}:{name}')
        # VC5 pragma-once aliases are case-sensitive even on this filesystem.
        adapted = re.sub(rb'(#include\s+")([^"\r\n]+)(")',
                         lambda m: m[1] + m[2].lower() + m[3], raw)
        (dest / name).write_bytes(adapted)
        receipts.append(dict(file=name, original=sha(raw), adapted=sha(adapted),
                             changed=raw != adapted))
    save('source.json', dict(commit=REF, files=receipts))
    print(f'Prepared {len(receipts)} source/header files; only quoted include case normalized.')


def inventory():
    receipts = []
    for path in sorted(OUT.rglob('BLUPI.EXE')):
        pe = pefile.PE(str(path))
        versions = {}
        for group in getattr(pe, 'FileInfo', []):
            for info in group:
                for table in getattr(info, 'StringTable', []):
                    versions.update({k.decode(errors='replace'): v.decode(errors='replace')
                                     for k, v in table.entries.items()})
        imports = {d.dll.decode(): [i.name.decode() if i.name else f'#{i.ordinal}'
                                    for i in d.imports]
                   for d in getattr(pe, 'DIRECTORY_ENTRY_IMPORT', [])}
        record = dict(file=str(path.relative_to(OUT)), size=path.stat().st_size,
                      sha256=sha(path.read_bytes()),
                      timestamp=datetime.fromtimestamp(pe.FILE_HEADER.TimeDateStamp,
                                                       timezone.utc).isoformat(),
                      linker=[pe.OPTIONAL_HEADER.MajorLinkerVersion, pe.OPTIONAL_HEADER.MinorLinkerVersion],
                      entry_rva=hex(pe.OPTIONAL_HEADER.AddressOfEntryPoint), versions=versions,
                      rich=pe.parse_rich_header(),
                      debug=[dict(type=d.struct.Type, size=d.struct.SizeOfData)
                             for d in getattr(pe, 'DIRECTORY_ENTRY_DEBUG', [])],
                      imports=imports, sections=[dict(name=s.Name.rstrip(b'\0').decode(),
                          rva=hex(s.VirtualAddress), virtual_size=s.Misc_VirtualSize,
                          raw_size=s.SizeOfRawData, sha256=sha(s.get_data())) for s in pe.sections])
        receipts.append(record)
        print(record['file'], record['sha256'], record['timestamp'], record['linker'], versions)
    # No Rich bytes are expected here; convert if an additional input has them.
    save('inventory.json', json.loads(json.dumps(receipts, default=lambda b: b.hex())))
    save('archives.json', [dict(file=str(p.relative_to(OUT)), size=p.stat().st_size,
                               sha256=sha(p.read_bytes()))
                          for p in sorted(OUT.glob('*.7z')) + sorted((OUT / 'distribution').glob('*.iso'))])


def version_control():
    dest = OUT / 'version'
    dest.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(OUT / 'source/event.cpp', dest / 'event.cpp')
    subprocess.run(['git', 'apply', '--directory=build/planet-forensics/version',
                    str(ROOT / 'tools/fixtures/planet_event_v18.patch')], cwd=ROOT, check=True)
    print('Prepared binary-evidenced version reconstruction; not pristine 1997 source.')


def compile_units(tree, units, source=None, label=None, extra=()):
    for unit in units:
        cmd = [sys.executable, str(ROOT / 'tools/msvc.py'), str((source or OUT / 'source') / f'{unit}.cpp'),
               '--tree', tree, '--out', str(OUT / (label or tree) / f'{unit}.obj'),
               '--log-dir', str(OUT / (label or tree) / 'logs'), '--include', r'C:\tools\dx5sdk\include',
               '--include', r'C:\tools\msvc5_sp3\VC\INCLUDE', '--include', str(OUT / 'source'),
               '--define', 'WIN32', '--define', 'NDEBUG', '--define', '_WINDOWS',
               '--define', 'nullptr=0']
        cmd += ['--flag=' + x for x in ('/W3', '/GX', '/O2', '/ML', '/Gd', '/Gy')]
        cmd += ['--flag=' + x for x in extra]
        result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        print(unit, tree, 'exit', result.returncode)
        if result.returncode:
            print(result.stdout.decode('utf-8', errors='replace'))
            raise SystemExit(result.returncode)


def scan(tree):
    reports = []
    for exe in (OUT / 'disc/ENGLISH/BLUPI.EXE', OUT / 'disc-it/ITALIAN/BLUPI.EXE'):
        pe = pefile.PE(str(exe))
        section = next(s for s in pe.sections if s.Name.startswith(b'.text'))
        raw = section.get_data()
        for obj in sorted((OUT / tree).glob('*.obj')):
            coff = Coff(obj)
            functions = []
            for f in coff.functions():
                hits = candidates_for(f, raw)
                functions.append(dict(name=f.name, size=len(f.data), relocs=len(f.relocs),
                    rvas=[hex(h + section.VirtualAddress) for h in hits],
                    raw_equal=bool(hits) and not f.relocs,
                    payload_sha256=sha(f.data),
                    relocation_sha256=sha(repr([(r.offset - f.start, r.typ,
                        coff.sym_by_index[r.symbol_index].name) for r in f.relocs]).encode())))
            reports.append(dict(exe=str(exe.relative_to(OUT)), object=str(obj.relative_to(OUT)),
                                functions=functions))
            hit = sum(bool(f['rvas']) for f in functions)
            print(tree, exe.parent.name, obj.name, hit, '/', len(functions), 'relocation-masked hits')
            for f in functions:
                if any(n in f['name'] for n in ('TreatEventBase', 'ChangePhase', 'DrawInfo', 'BltFast')):
                    print(' ', f['name'], f['size'], f['rvas'])
    save(f'{tree}/scan.json', reports)


def compare(tree):
    # Never let the SB1 function map / TU map certify a Planet address.
    cmpfunc.oracle_function_starts = lambda: set()
    cmpfunc.oracle_tu_at_rva = lambda: {}
    sys.argv = ['cmpfunc', *map(str, sorted((OUT / tree).glob('*.obj'))),
                '--exe', str(OUT / 'disc-it/ITALIAN/BLUPI.EXE'),
                '--json-out', str(OUT / tree / 'compare-planet.json'), '--summary']
    cmpfunc.main()


def matrix():
    rows = []
    for unit in ('event', 'pixmap', 'button', 'decor'):
        base = Coff(OUT / 'sp3' / f'{unit}.obj')
        for tree in ('rtm', 'sp2', 'nt5-1592', 'nt5-1691'):
            other = Coff(OUT / tree / f'{unit}.obj')
            a, b = payloads(base), payloads(other)
            changed = [n for n in a.keys() | b.keys() if a.get(n) != b.get(n)]
            rows.append(dict(unit=unit, tree=tree, total=len(a), changed=changed))
            print(tree, unit, len(changed), '/', len(a), changed)
    save('matrix.json', rows)


def capture(units, source=None, label=None):
    for unit in units:
        folder = OUT / f'capture-{label or unit}'
        (folder / 'il').mkdir(parents=True, exist_ok=True)
        stub = folder / 'c2stub.exe'
        shutil.copyfile(ROOT / 'build/c2trace/stub/c2stub.exe', stub)
        compile_units('sp3', [unit], source=source, label=folder.name, extra=['/B2' + str(stub)])


def booltrace(units):
    from types import SimpleNamespace
    sys.path.insert(0, str(ROOT / 'tools/c2trace'))
    import booltemps
    import colour_trace
    booltemps.c2dbg = SimpleNamespace(run=colour_trace.owned_run)
    for unit in units:
        cap = OUT / f'capture-{unit}' / 'il'
        rows = booltemps.run(cap)
        save(f'capture-{unit}/booleans.json', rows)
        a, b = Coff(OUT / 'sp3' / f'{unit}.obj'), Coff(cap / 'trace.obj')
        equal = payloads(a) == payloads(b)
        save(f'capture-{unit}/audit.json', dict(native_payloads_and_symbolic_relocations_equal=equal))
        if not equal:
            raise RuntimeError('Observed replay changed function payloads')
        print(unit, 'boolean temps', len(rows), 'native payload audit passed')
        for row in rows:
            print(row['func'], row['line'], row['rotor'], row['pref'], row['reg'], row['path'])


def phase_trace(label, unit, function, native_tree):
    """Observe only queue keys and chosen registers; avoid a whole use-graph dump."""
    sys.path.insert(0, str(ROOT / 'tools/c2trace'))
    import c2dbg
    import colour_trace
    import booltemps
    cap = OUT / f'capture-{label or unit}' / 'il'
    state = dict(active=False, selection=None, rows=[])
    started = time.monotonic()
    def node(p, addr):
        operand = p.u32(addr)
        kind = p.rd(operand + 4, 1)[0]
        if kind != 13:
            return None
        literal = p.u32(operand + 0x28)
        if p.rd(literal + 8, 1)[0] != 7:
            return None
        return dict(lr=p.u32(addr+0x1c), value=p.u32(literal+0x18),
                    priority=colour_trace.signed(p.u32(addr+0xc)),
                    refs=p.u32(addr+0x24), tie=p.u32(addr+0x40))
    def hit(p, c, addr):
        if time.monotonic() - started > 60:
            raise TimeoutError('Bounded phase observation exceeded 60 seconds')
        if addr == 0x414a64:
            state['active'] = function in booltemps.fname(p)
            return
        if addr == 0x42a8aa:
            state['active'] = False
        if not state['active']:
            return
        if addr in (0x41b6fa, 0x41b785):
            row = node(p, c.Ecx)
            if row:
                row['stage'] = 'sort' if addr == 0x41b6fa else 'select'
                state['rows'].append(row)
            if addr == 0x41b785:
                state['selection'] = row
        elif addr == 0x41b90e and state['selection']:
            state['selection']['register'] = c2dbg.REG.get(c.Edi)
    obj = colour_trace.owned_run(cap, [0x414a64, 0x42a8aa, 0x41b6fa, 0x41b785, 0x41b90e], hit)
    normal = OUT / native_tree / f'{unit}.obj'
    a, b = Coff(normal), Coff(obj)
    if payloads(a) != payloads(b):
        raise RuntimeError('Observation changed function payloads or symbolic relocations')
    name = next(f.name for f in a.functions() if function in f.name)
    save(f'capture-{label or unit}/phase.json', dict(function=name, rows=state['rows'],
        native_bytes_equal=True, native_payloads_and_symbolic_relocations_equal=True))
    print(json.dumps(state['rows'], indent=1))


def cross_version():
    """Scan existing SB1 Display probes; do not attribute absent hits to compilers."""
    paths = [ROOT / 'assets/Speedy Blupi/BLUPI.EXE',
             Path('C:/tools/download/openeggbert/Speedy_Blupi_Demo/exe/Blupi.exe'),
             Path('C:/tools/download/eggbert/zip/egames.exe'),
             Path('C:/tools/download/openeggbert/Speedy_Blupi_II/exe/BLUPI.EXE')]
    functions = {tree: next(f for f in Coff(ROOT / f'build/fnprobe/planet-sb1-{tree}/pixmap.obj').functions()
                            if f.name == '?Display@CPixmap@@QAEHXZ') for tree in ('sp3', 'rtm')}
    rows = []
    for path in paths:
        pe = pefile.PE(str(path))
        section = next(s for s in pe.sections if s.Name.startswith(b'.text'))
        matches = {tree: [hex(section.VirtualAddress + h) for h in candidates_for(f, section.get_data())]
                   for tree, f in functions.items()}
        rows.append(dict(file=str(path), sha256=sha(path.read_bytes()), matches=matches))
        print(path, matches)
    save('cross-version.json', rows)


def crt():
    """Read five discriminating CRT archive members; never invoke a game."""
    wanted = {'mbctype.obj', 'output.obj', 'gmtime.obj', 'wincrt0.obj', 'strchr.obj'}
    rows = []
    for tree in ('rtm', 'sp2', 'sp3'):
        path = Path(f'C:/tools/msvc5_{tree}/VC/LIB/LIBC.LIB')
        blob, offset, names = path.read_bytes(), 8, b''
        assert blob.startswith(b'!<arch>\n')
        while offset + 60 <= len(blob):
            header = blob[offset:offset+60]
            size = int(header[48:58])
            name = header[:16].decode().strip()
            data = blob[offset+60:offset+60+size]
            offset += 60 + size + (size & 1)
            if name == '//':
                names = data
                continue
            if name.startswith('/') and name[1:].isdigit():
                name = names[int(name[1:]):].split(b'\0', 1)[0].decode()
            base = name.replace('\\', '/').split('/')[-1].rstrip('/')
            if base not in wanted:
                continue
            obj = OUT / 'crt' / tree / base
            obj.parent.mkdir(parents=True, exist_ok=True)
            obj.write_bytes(data)
            for exe in (OUT / 'disc/ENGLISH/BLUPI.EXE', OUT / 'disc-it/ITALIAN/BLUPI.EXE'):
                pe = pefile.PE(str(exe))
                section = next(s for s in pe.sections if s.Name.startswith(b'.text'))
                for f in Coff(obj).functions():
                    hits = candidates_for(f, section.get_data())
                    if hits:
                        rows.append(dict(tree=tree, exe=str(exe.relative_to(OUT)), member=name,
                            archive_sha256=sha(blob), object_sha256=sha(data), symbol=f.name,
                            size=len(f.data), relocations=len(f.relocs),
                            rvas=[hex(h + section.VirtualAddress) for h in hits]))
    save('crt.json', rows)
    for r in rows:
        print(r['tree'], r['exe'], r['symbol'], r['size'], r['rvas'], 'masked' if r['relocations'] else 'raw')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['prepare', 'inventory', 'version', 'compile', 'scan', 'compare', 'matrix', 'capture', 'booltrace', 'phase', 'crt', 'cross'])
    parser.add_argument('--tree', default='sp3')
    parser.add_argument('--units', nargs='+', default=['event', 'pixmap', 'button', 'decor'])
    parser.add_argument('--source', type=Path)
    parser.add_argument('--label')
    parser.add_argument('--extra-flag', action='append', default=[])
    parser.add_argument('--function', default='ChangePhase@CEvent')
    parser.add_argument('--native-tree', default='sp3')
    args = parser.parse_args()
    if args.action == 'prepare': prepare()
    elif args.action == 'inventory': inventory()
    elif args.action == 'version': version_control()
    elif args.action == 'compile': compile_units(args.tree, args.units, args.source, args.label, args.extra_flag)
    elif args.action == 'scan': scan(args.tree)
    elif args.action == 'compare': compare(args.tree)
    elif args.action == 'matrix': matrix()
    elif args.action == 'capture': capture(args.units, args.source, args.label)
    elif args.action == 'booltrace': booltrace(args.units)
    elif args.action == 'phase': phase_trace(args.label, args.units[0], args.function, args.native_tree)
    elif args.action == 'crt': crt()
    elif args.action == 'cross': cross_version()


if __name__ == '__main__':
    main()
