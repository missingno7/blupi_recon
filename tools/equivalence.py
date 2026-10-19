#!/usr/bin/env python3
"""Static EQUIVALENT tier for two SB1 residues; never grants STRICT.

Oracle bytes are verifier-only. No native game code is executed. Certificates
bind complete function payloads and symbolic relocations, not COFF timestamps.
"""
from __future__ import annotations
import argparse
import hashlib
import itertools
import json
from pathlib import Path
import sys

import capstone as cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG
import cmpfunc
import pefile

ROOT = Path(__file__).resolve().parents[1]
EXE = ROOT / 'assets/Speedy Blupi/BLUPI.EXE'
md = cs.Cs(cs.CS_ARCH_X86, cs.CS_MODE_32)
md.detail = True
CASES = {
    'ChangePhase': ('?ChangePhase@CEvent@@QAEHI@Z', 0x19310, [(0xc5,0x110),(0x5f0,0x5fc)], 32),
    'TreatEventBase': ('?TreatEventBase@CEvent@@QAEHIIJ@Z', 0x1d230, [(0x41a,0x42f),(0x6f3,0x700)], 8),
}
ASSUMPTIONS = [
    'Object fields are valid ordinary RAM and remain stable during each reordered guard.',
    'Calls obey the declared x86 ABI; scratch registers and arithmetic flags are not hidden arguments.',
    'Equivalence covers defined memory effects, branch outcomes, call targets/arguments and return values, not timing or intermediate register observations.',
    'Callee/data bindings are checked separately; two CRT identities use unique pinned-library signatures and one getter uses raw body identity.',
]


def require(condition, message):
    if not condition:
        raise ValueError(message)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def json_hash(value):
    return hashlib.sha256(json.dumps(value,sort_keys=True,separators=(',',':')).encode()).hexdigest()


def payload_hash(coff, f):
    return json_hash({'bytes':f.data.hex(), 'relocations':[
        [r.offset-f.start,r.typ,coff.sym_by_index[r.symbol_index].name] for r in f.relocs]})


def guard(insns, start, exits, values):
    """Interpret only the finite equality-predicate CFG, never native code."""
    regs = dict(values)
    zf = None
    pc = start
    def value(op):
        if op.type == X86_OP_IMM:
            return op.imm
        if op.type == X86_OP_REG:
            return regs[md.reg_name(op.reg)]
        assert op.type == X86_OP_MEM and md.reg_name(op.mem.base) == 'ebp' and not op.mem.index
        return regs[op.mem.disp]
    for _ in range(64):
        if pc in exits:
            return pc
        i = insns[pc]
        nxt = pc + i.size
        if i.mnemonic == 'mov':
            assert i.operands[0].type == X86_OP_REG
            regs[i.reg_name(i.operands[0].reg)] = value(i.operands[1])
        elif i.mnemonic == 'cmp':
            zf = value(i.operands[0]) == value(i.operands[1])
        elif i.mnemonic in ('je', 'jne'):
            assert zf is not None
            if zf == (i.mnemonic == 'je'):
                nxt = i.operands[0].imm
        else:
            raise AssertionError((hex(pc), i.mnemonic))
        pc = nxt
    raise AssertionError('Unexpected loop')


def dead_before_use(insns, start, dirty):
    """Conservative forward liveness check; each branch is explored.

    At calls require equal explicit targets and ECX (conservatively treated as
    this for every call); caller-saved registers and arithmetic flags cease to
    be obligations under the declared ABI. Partial-register writes never kill
    the obligation for the whole register. Revisited dataflow states need no
    repeat work; an indirect jump with a still-dirty value is not accepted.
    """
    pending, visited, killed = [(start, frozenset(dirty))], set(), set()
    aliases = {'al':'eax','ah':'eax','ax':'eax','cl':'ecx','ch':'ecx','cx':'ecx',
               'dl':'edx','dh':'edx','dx':'edx'}
    while pending:
        pc, dirty = pending.pop()
        if not dirty:
            continue
        if (pc,dirty) in visited:
            continue
        visited.add((pc,dirty))
        i = insns[pc]
        reads, writes = ({i.reg_name(r) for r in group} for group in i.regs_access())
        reads = {aliases.get(r,r) for r in reads}
        if i.mnemonic == 'call':
            reads.add('ecx')
            writes |= {'eax','ecx','edx','eflags'}
        elif i.mnemonic not in ('cmp','test','xor'):
            writes.discard('eflags')  # avoid treating a partial flags write as a full kill
        assert not (dirty & reads), (hex(pc),i.mnemonic,i.op_str,dirty & reads)
        remaining = dirty - writes
        if not remaining:
            killed.add(hex(pc))
            continue
        assert not i.mnemonic.startswith('ret'), ('live-at-return',remaining)
        if i.mnemonic.startswith('j'):
            assert i.operands[0].type == X86_OP_IMM, 'Unhandled indirect jump'
            pending.append((i.operands[0].imm, remaining))
            if i.mnemonic == 'jmp':
                continue
        pending.append((pc+i.size, remaining))
    return sorted(killed)


def audit(verified, evidence_objects, out_dir, pending):
    require(__debug__, 'Equivalence checks require assertions enabled (no -O).')
    OUT = Path(out_dir)
    OUT.mkdir(parents=True, exist_ok=True)
    OBJ = next(Path(u['object']) for u in verified if any(f['name'] == CASES['ChangePhase'][0] for f in u['functions']))
    coff, pe = cmpfunc.Coff(OBJ), pefile.PE(str(EXE))
    cases = [(name, rva, windows, expected) for name,(symbol,rva,windows,expected) in CASES.items() if name in pending]
    functions, decoded, receipt = {}, {}, {}
    for name, rva, windows, expected in cases:
        f = next(f for f in coff.functions() if name + '@' in f.name)
        functions[name] = f
        raw = pe.get_data(rva, len(f.data))
        assert len(raw) == len(f.data)
        masked = {k for r in f.relocs for k in range(r.offset-f.start, r.offset-f.start+4)}
        different = [k for k, (a,b) in enumerate(zip(f.data, raw)) if a != b and k not in masked]
        assert len(different) == expected
        assert all(any(lo <= k < hi for lo,hi in windows) for k in different)
        assert not any(lo <= k < hi for lo,hi in windows for k in masked)
        decoded[name] = [{i.address:i for i in md.disasm(data, 0)} for data in (f.data, raw)]
        receipt[name] = dict(nonrelocated_differing_bytes=len(different),
                             all_differences_inside_checked_windows=True)

    if 'ChangePhase' in decoded:
        cp = decoded['ChangePhase']
        excluded = {0x62e, 0x605, 0x62f, 0x632, 0x631, 0x630}
        phases = sorted(excluded | {0x5f5, 0xffffffff})
        for phase, old, demo in itertools.product(phases, (0x5f5, 0xffffffff), (0, 0xffffffff)):
            values = {'esi':phase, 'edi':0, 'ebx':0x5f5, 0x6d28:old, 0x9918:demo}
            exits = [guard(d, 0xc5, {0x110, 0x11b}, values) for d in cp]
            expected = 0x110 if not demo and (old == 0x5f5 or phase != 0x5f5) and phase not in excluded else 0x11b
            assert exits == [expected, expected]
        for phase in (0x5f5, 0x609, 0xffffffff):
            exits = [guard(d, 0x5f0, {0x5fc, 0x616}, {'esi':phase,'ebx':0x5f5}) for d in cp]
            assert exits == [0x5fc if phase in (0x5f5, 0x609) else 0x616] * 2
        receipt['ChangePhase']['guard_equivalence_classes'] = 32
        receipt['ChangePhase']['writeinfo_equivalence_classes'] = 3
        receipt['ChangePhase']['guard_dirty_register_kills'] = {
            hex(pc): [dead_before_use(d,pc,{'eax','eflags'}) for d in cp] for pc in (0x110,0x11b)}
        receipt['ChangePhase']['writeinfo_dirty_flags_kills'] = {
            hex(pc): [dead_before_use(d,pc,{'eflags'}) for d in cp] for pc in (0x5fc,0x616)}


    if 'TreatEventBase' in decoded:
        # Validate algebraic toggle equivalence for every 32-bit old flag by requiring
        # the exact same instruction sequence after an ECX/EDX alpha-renaming.
        teb = decoded['TreatEventBase']
        def texts(d, lo, hi):
            return [f'{i.mnemonic} {i.op_str}' for pc,i in d.items() if lo <= pc < hi]
        a, b = [texts(d,0x41a,0x42f) for d in teb]
        assert a == ['mov ecx, dword ptr [ebp + 0x6d40]', 'xor edx, edx', 'test ecx, ecx',
                     'sete dl', 'mov dword ptr [ebp + 0x6d40], edx', 'mov esi, edx']
        def rename(s):
            return s.replace('ecx','TEMP').replace('edx','ecx').replace('TEMP','edx').replace('dl','cl')
        assert [rename(s) for s in a] == b
        # Both define both temporaries before use, write !(old != 0) to the same memory
        # and ESI, and leave identical TEST flags. Only ECX/EDX may differ at the join.
        receipt['TreatEventBase']['toggle_dirty_register_kills'] = [dead_before_use(d,0x42f,{'ecx','edx'}) for d in teb]
        a, b = [texts(d,0x6f3,0x702) for d in teb]
        assert a == ['mov eax, dword ptr [ebp + 0x6d50]', 'push 0', 'push 0', 'push 0x10', 'push eax', 'call esi']
        assert [s.replace('eax','edx') for s in a] == b
        receipt['TreatEventBase']['postmessage_same_target_and_stack_arguments'] = True
    # Check all relocation targets separately. No STRICT result is added.
    paths = [Path(u['object']).resolve() for u in verified] + list(evidence_objects)
    text = next(s for s in pe.sections if s.Name.startswith(b'.text'))
    at = {functions[name].name:rva for name,rva,_,_ in cases}
    shared, globals_, fdefs, ddefs, locations, folded, issues, bindings = cmpfunc.collect_global_state(
        paths, pe, text.get_data(), text.VirtualAddress, at, {})
    starts = {pe.OPTIONAL_HEADER.ImageBase+r for r in cmpfunc.oracle_function_starts()}
    # A callee used only by an unmatched caller may lack an observed symbol binding
    # even though its own body has an independently verified location. Use the
    # existing STRICT/FOLDED body receipts, and named PE imports, for those bindings.

    bindings_by_name = {}
    for unit in verified:
        for f in unit['functions']:
            if f['status'] in ('STRICT','FOLDED') and len(f['matches']) == 1:
                va = pe.OPTIONAL_HEADER.ImageBase + int(f['matches'][0]['rva'],16)
                bindings_by_name.setdefault(f['name'],set()).add(va)
    for name, vas in bindings_by_name.items():
        if len(vas) == 1:
            va, = vas
            key = 'global:' + name
            assert key not in globals_ or globals_[key] == va
            globals_[key] = va
    # GetTrueColorBack has no STRICT caller that establishes folding, but behavior
    # at its observed call target is independently identical to its zero-relocation
    # body and the already-STRICT CSound::GetEnable owner. This is a conditional
    # semantic binding, not a change to the verifier's folding acceptance.
    pixmap = next(c for c in map(cmpfunc.Coff, paths) if any(f.name == '?GetTrueColorBack@CPixmap@@QAEHXZ' for f in c.functions()))
    getter = next(f for f in pixmap.functions() if f.name == '?GetTrueColorBack@CPixmap@@QAEHXZ')
    owner_va, = bindings_by_name['?GetEnable@CSound@@QAEHXZ']
    assert not getter.relocs and len(getter.data) == 16
    assert getter.data == pe.get_data(owner_va-pe.OPTIONAL_HEADER.ImageBase,16)
    globals_['global:'+getter.name] = owner_va
    receipt['semantic_fold_binding'] = dict(symbol=getter.name,va=hex(owner_va),
        evidence='16 raw-identical bytes, no relocations; target is STRICT CSound::GetEnable owner',
        strict_acceptance_changed=False)
    imports = cmpfunc.import_map(pe)
    for symbol in coff.symbols:
        if symbol.name.startswith('__imp_') and cmpfunc.normalize_import(symbol.name) in imports:
            va = imports[cmpfunc.normalize_import(symbol.name)]
            key = cmpfunc.symbol_map_key(OBJ,symbol)
            assert key not in globals_ or globals_[key] == va
            globals_[key] = va

    # Identify the two CRT file-enumeration targets independently in the pinned
    # library. These are unique relocation-masked body signatures, not a new claim
    # of complete CRT behavioral equivalence.
    lib = Path('C:/tools/msvc5_sp3/VC/LIB/LIBC.LIB').read_bytes()
    assert lib.startswith(b'!<arch>\n')
    offset, names = 8, b''
    while offset + 60 <= len(lib):
        header = lib[offset:offset+60]
        size = int(header[48:58])
        name = header[:16].decode().strip()
        data = lib[offset+60:offset+60+size]
        offset += 60 + size + (size & 1)
        if name == '//':
            names = data
            continue
        if name.startswith('/') and name[1:].isdigit():
            name = names[int(name[1:]):].split(b'\0',1)[0].decode()
        if name.replace('\\','/').split('/')[-1].rstrip('/') != 'findfile.obj':
            continue
        member = OUT/'findfile.obj'
        member.write_bytes(data)
        receipt['crt_target_identity'] = {}
        for f in cmpfunc.Coff(member).functions():
            if f.name not in ('__findfirst','__findnext'):
                continue
            hits = cmpfunc.candidates_for(f,text.get_data())
            assert len(hits) == 1
            va = pe.OPTIONAL_HEADER.ImageBase + text.VirtualAddress + hits[0]
            globals_['global:'+f.name] = va
            receipt['crt_target_identity'][f.name] = dict(va=hex(va),size=len(f.data),
                proof='Unique relocation-masked signature in pinned SP3 LIBC findfile.obj')
        break
    assert len(receipt.get('crt_target_identity',{})) == 2
    for name,rva,_,_ in cases:
        status, rows = cmpfunc.resolve_sites(functions[name],rva-text.VirtualAddress,text.get_data(),
            text.VirtualAddress,pe.OPTIONAL_HEADER.ImageBase,pe,coff,OBJ,shared,fdefs,ddefs,
            imports,{},globals_,starts)
        receipt[name]['relocation_status'] = status
        receipt[name]['verified_relocations'] = sum(r['verified'] for r in rows)
        receipt[name]['relocations'] = rows
        assert status == 'STRICT' and all(r['verified'] for r in rows)
        print(name, 'relocation-only audit:', sum(r['verified'] for r in rows),'/',len(rows),'verified')


    return receipt


def certificate(name, coff, function, receipt):
    return dict(schema=1, status='EQUIVALENT', symbol=function.name,
                rva=hex(CASES[name][1]), scope='static-local-equivalence',
                assumptions=ASSUMPTIONS, payload_sha256=payload_hash(coff, function),
                oracle_sha256=digest(EXE), checker_sha256=digest(__file__),
                comparator_sha256=digest(ROOT/'tools/cmpfunc.py'),
                capstone_version=cs.__version__,
                crt_library_sha256=digest('C:/tools/msvc5_sp3/VC/LIB/LIBC.LIB'),
                checks={k:v for k,v in receipt[name].items() if k != 'relocations'},
                semantic_fold_binding=receipt['semantic_fold_binding'],
                crt_target_identity=receipt['crt_target_identity'])


def validate_certificate(saved, current, name):
    require(current == saved, f'{name}: equivalence certificate changed; review required')


def check(manifest, comparison, evidence_objects, out_dir, write=False):
    """Fresh proof plus pinned certificate; raw comparator statuses stay intact."""
    require(__debug__, 'Equivalence checks require assertions enabled (no -O).')
    entries = manifest.get('equivalent_functions', [])
    pending, results = {}, {}
    for entry in entries:
        name = entry['name']
        require(name in CASES and entry['symbol'] == CASES[name][0], 'Unknown equivalence case')
        matches = [(u,f) for u in comparison for f in u['functions'] if f['name'] == entry['symbol']]
        require(len(matches) == 1, f'{name}: expected exactly one emitted body')
        unit, row = matches[0]
        if row['status'] in ('STRICT', 'FOLDED'):
            results[name] = dict(status=row['status'], certificate_required=False)
            continue
        require(row['status'] == 'MISMATCH', f'{name}: unexpected byte status {row["status"]}')
        coff = cmpfunc.Coff(Path(unit['object']))
        function = next(f for f in coff.functions() if f.name == entry['symbol'])
        pending[name] = (entry, coff, function)
    if pending:
        receipt = audit(comparison, evidence_objects, out_dir, pending)
        for name, (entry, coff, function) in pending.items():
            current = certificate(name, coff, function, receipt)
            path = ROOT/entry['certificate']
            if write:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(json.dumps(current, indent=2)+'\n', encoding='utf-8')
            else:
                saved = json.loads(path.read_text(encoding='utf-8'))
                validate_certificate(saved, current, name)
            results[name] = dict(status='EQUIVALENT', byte_status='MISMATCH',
                                 certificate=entry['certificate'])
            print(f'{name}: EQUIVALENT (static local proof; byte status MISMATCH)')
    Path(out_dir).mkdir(parents=True, exist_ok=True)
    (Path(out_dir)/'equivalence.json').write_text(json.dumps(results,indent=2)+'\n',encoding='utf-8')
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--objects-dir', type=Path, required=True)
    parser.add_argument('--out-dir', type=Path, default=ROOT/'build/equivalence')
    parser.add_argument('--write-certificates', action='store_true')
    args = parser.parse_args()
    import verify
    manifest = json.loads((ROOT/'recon.json').read_text())
    args.out_dir.mkdir(parents=True, exist_ok=True)
    verify.BUILD = args.out_dir.resolve()
    objects = sorted(args.objects_dir.resolve().glob('*.obj'))
    evidence = [p for p in objects if p.name.startswith('crt_')]
    objects = [p for p in objects if p not in evidence]
    comparison = verify.run_comparator(objects, evidence, manifest)
    by_unit, _ = verify.report(comparison, [u['source'] for u in manifest['translation_units']])
    require(not verify.accepted_regressions(manifest, by_unit), 'Strict peer regression')
    check(manifest, comparison, evidence, args.out_dir, args.write_certificates)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
