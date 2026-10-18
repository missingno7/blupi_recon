"""Static ordered CFG comparison of a COFF function and the original PE.

Usage: .pyenv/python.exe scratch/sol-cfg/cfg.py OBJECT SYMBOL RVA CODE_SIZE
       --tag NAME --out scratch/sol-cfg
Never loads or executes the target. CODE_SIZE excludes trailing switch data.
Reports ordered block/edge/condition/call equality, not a byte/STRICT match.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import csv
import difflib
import hashlib
import json
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import capstone
import cmpfunc
import pefile

X = capstone.x86


def number(s):
    return int(s, 0) if isinstance(s, str) else s


def known_bindings(pe):
    """Only manifest entries and verified relocation targets are named here."""
    manifest = json.loads((ROOT / 'recon.json').read_text())
    bindings = {}
    names = {}
    for rec in manifest.get('accepted_functions', []):
        bindings[rec['symbol']] = pe.OPTIONAL_HEADER.ImageBase + number(rec['rva'])
    for rec in manifest.get('symbol_mappings', []):
        if 'va' in rec:
            bindings[rec['symbol']] = number(rec['va'])
    results = ROOT / 'build/verify/results.json'
    if results.exists():
        for unit in json.loads(results.read_text()):
            for f in unit['functions']:
                for match in f.get('matches', []):
                    if match['status'] in ('STRICT', 'FOLDED'):
                        bindings.setdefault(f['name'], pe.OPTIONAL_HEADER.ImageBase + number(match['rva']))
                    for rel in match.get('relocations', []):
                        if rel.get('verified') and rel.get('resolved_symbol_va'):
                            bindings.setdefault(rel['symbol'], number(rel['resolved_symbol_va']))
    for sym, va in bindings.items():
        names.setdefault(va, sym)
    with (ROOT / 'oracle/functions.csv').open() as stream:
        for row in csv.DictReader(stream):
            names.setdefault(pe.OPTIONAL_HEADER.ImageBase + number(row['rva']), row['name'])
    return bindings, names


class Decoder:
    def __init__(self, data, code_limit, base, coff=None, fn=None, pe=None):
        self.data, self.limit, self.base = data, code_limit, base
        self.coff, self.fn, self.pe = coff, fn, pe
        self.rel = {r.offset - fn.start: r for r in fn.relocs} if fn else {}
        self.md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        self.md.detail = True
        self.tables = {}
        self.lookups = {}
        self.ins = {}
        self.succ = {}
        self.calls = []
        self.unresolved = []

    def relocation(self, ins):
        return next((self.rel[k] for k in range(ins.address, ins.address + ins.size) if k in self.rel), None)

    def symbol_target(self, rel):
        sym = self.coff.sym_by_index[rel.symbol_index]
        addend = struct.unpack_from('<I', self.data, rel.offset - self.fn.start)[0]
        return sym, sym.value + addend

    def immediate(self, ins):
        if ins.operands and ins.operands[0].type == X.X86_OP_IMM:
            return ins.operands[0].imm & 0xffffffff
        return None

    def table(self, ins):
        """VC5 absolute indexed-jump table, bounded by valid in-function labels."""
        op = ins.operands[0]
        if op.type != X.X86_OP_MEM or op.mem.scale != 4 or op.mem.base:
            return None
        rel = self.relocation(ins)
        targets = []
        prior = [i for a, i in sorted(self.ins.items()) if ins.address - 48 <= a < ins.address]
        compares = [i for i in prior if i.mnemonic == 'cmp' and i.operands[-1].type == X.X86_OP_IMM]
        count = compares[-1].operands[-1].imm + 1 if compares else None
        # A byte compression map reduces the range-test extent to max(map)+1.
        for prev in prior:
            if not prev.mnemonic.startswith('mov') or not count or not 0 < count < 1024:
                continue
            mem = next((o for o in prev.operands if o.type == X.X86_OP_MEM and o.size == 1), None)
            if mem is None:
                continue
            if self.fn:
                maprel = self.relocation(prev)
                if not maprel:
                    continue
                mapsym, mapstart = self.symbol_target(maprel)
                if mapsym.section != self.fn.section.index:
                    continue
                raw = self.fn.section.raw[mapstart:mapstart + count]
            else:
                raw = self.pe.get_data((mem.mem.disp & 0xffffffff) - self.pe.OPTIONAL_HEADER.ImageBase, count)
            if len(raw) == count:
                count = max(raw) + 1
        if count is not None and not 0 < count < 1024:
            count = None
        if count is None:
            self.unresolved.append({'table_extent': ins.address})
        if self.fn:
            if not rel:
                return None
            sym, start = self.symbol_target(rel)
            sec = self.coff.sections[sym.section - 1]
            rs = {r.offset: r for r in sec.relocations}
            pos = start
            while pos + 4 <= len(sec.raw) and pos in rs and (count is None or len(targets) < count):
                label = self.coff.sym_by_index[rs[pos].symbol_index]
                off = label.value + struct.unpack_from('<I', sec.raw, pos)[0] - self.fn.start
                if label.section != self.fn.section.index or not 0 <= off < self.limit:
                    break
                targets.append(off)
                pos += 4
            location = {'section': sym.section, 'offset': start}
        else:
            start = op.mem.disp & 0xffffffff
            raw = self.pe.get_data(start - self.pe.OPTIONAL_HEADER.ImageBase, 2048)
            for pos in range(0, min(len(raw) - 3, count * 4 if count else len(raw) - 3), 4):
                off = (struct.unpack_from('<I', raw, pos)[0] - self.base) & 0xffffffff
                if not 0 <= off < self.limit:
                    break
                targets.append(off)
            location = {'va': start}
        if not targets:
            return None
        self.tables[ins.address] = dict(location, targets=targets)
        return targets

    def decode(self):
        todo = [0]
        while todo:
            off = todo.pop()
            while 0 <= off < self.limit and off not in self.ins:
                ins = next(self.md.disasm(self.data[off:off + 15], off, count=1), None)
                if not ins:
                    raise ValueError(f'cannot decode +{off:x}')
                self.ins[off] = ins
                after = off + ins.size
                edges = []
                if ins.group(capstone.CS_GRP_JUMP):
                    dest = self.immediate(ins)
                    if dest is not None:
                        edges.append(('taken', dest))
                    else:
                        targets = self.table(ins)
                        if targets is None:
                            self.unresolved.append(off)
                        else:
                            edges += [(f'case{k}', target) for k, target in enumerate(targets)]
                    if ins.mnemonic != 'jmp':
                        edges.append(('fall', after))
                elif not ins.group(capstone.CS_GRP_RET):
                    edges = [('fall', after)] if after < self.limit else []
                self.succ[off] = edges
                if ins.mnemonic == 'call':
                    rel = self.relocation(ins)
                    if self.fn and rel:
                        sym = self.coff.sym_by_index[rel.symbol_index].name
                        self.calls.append({'offset': off, 'symbol': sym, 'kind': 'indirect' if ins.operands[0].type != X.X86_OP_IMM else 'direct'})
                    elif not self.fn:
                        op = ins.operands[0]
                        va = (op.imm + self.base if op.type == X.X86_OP_IMM else op.mem.disp if op.type == X.X86_OP_MEM and not op.mem.base and not op.mem.index else None)
                        self.calls.append({'offset': off, 'va': va & 0xffffffff if va is not None else None,
                                           'symbol': ins.op_str, 'kind': 'direct' if op.type == X.X86_OP_IMM else 'indirect'})
                    else:
                        self.calls.append({'offset': off, 'symbol': ins.op_str, 'kind': 'indirect' if ins.operands[0].type != X.X86_OP_IMM else 'unresolved'})
                if ins.group(capstone.CS_GRP_JUMP) or ins.group(capstone.CS_GRP_RET):
                    todo.extend(t for _, t in edges if 0 <= t < self.limit)
                    break
                off = after
        self.calls.sort(key=lambda c: c['offset'])
        # Relocated non-jump indexed byte tables are switch compression maps.
        for off, ins in self.ins.items():
            for op in ins.operands:
                if op.type != X.X86_OP_MEM or not (op.mem.index or op.mem.base) or op.size != 1:
                    continue
                if self.fn:
                    rel = self.relocation(ins)
                    if not rel:
                        continue
                    sym, start = self.symbol_target(rel)
                    if sym.section != self.fn.section.index or start < max(self.ins):
                        continue
                    sec = self.fn.section
                    prior = [i for a, i in sorted(self.ins.items()) if off - 36 <= a < off and i.mnemonic == 'cmp' and i.operands[-1].type == X.X86_OP_IMM]
                    stop = start + prior[-1].operands[-1].imm + 1 if prior else min([s.value for s in self.coff.symbols if s.section == sym.section and s.value > start] + [len(sec.raw)])
                    raw = sec.raw[start:stop]
                    self.lookups[off] = {'offset': start, 'bytes': list(raw)}
                else:
                    start = op.mem.disp & 0xffffffff
                    if start < self.base + self.limit:
                        continue
                    # Bound a compression map by the preceding unsigned range test.
                    prior = [i for a, i in sorted(self.ins.items()) if off - 36 <= a < off and i.mnemonic == 'cmp' and i.operands[-1].type == X.X86_OP_IMM]
                    if prior:
                        count = prior[-1].operands[-1].imm + 1
                        if 0 < count < 1024:
                            raw = self.pe.get_data(start - self.pe.OPTIONAL_HEADER.ImageBase, count)
                            self.lookups[off] = {'va': start, 'bytes': list(raw)}
        return self

    def blocks(self):
        starts = {0}
        for off, ins in self.ins.items():
            if ins.group(capstone.CS_GRP_JUMP) or ins.group(capstone.CS_GRP_RET):
                starts.update(t for _, t in self.succ[off] if t in self.ins)
        starts = sorted(starts)
        ids = {s: i for i, s in enumerate(starts)}
        blocks = []
        calls = {c['offset']: c for c in self.calls}
        for k, start in enumerate(starts):
            stop = starts[k + 1] if k + 1 < len(starts) else self.limit
            seq = [i for a, i in sorted(self.ins.items()) if start <= a < stop]
            last = seq[-1]
            edges = [(kind, ids.get(target, f'external:{target:x}')) for kind, target in self.succ[last.address]]
            term = last.mnemonic if last.group(capstone.CS_GRP_JUMP) or last.group(capstone.CS_GRP_RET) else 'fall'
            blocks.append({'id': k, 'start': start, 'end': last.address + last.size, 'term': term,
                           'edges': edges, 'calls': [calls[i.address] for i in seq if i.address in calls],
                           'ins': [(i.address, i.mnemonic, i.op_str) for i in seq]})
        return blocks


def stats(blocks):
    predecessors = defaultdict(set)
    for b in blocks:
        for _, target in b['edges']:
            predecessors[target].add(b['id'])
    all_nodes = set(range(len(blocks)))
    dominators = {k: ({0} if k == 0 else set(all_nodes)) for k in all_nodes}
    changed = True
    while changed:
        changed = False
        for k in range(1, len(blocks)):
            ps = [dominators[v] for v in predecessors[k]]
            value = {k} | (set.intersection(*ps) if ps else set())
            if value != dominators[k]:
                dominators[k] = value
                changed = True
    loops = []
    for b in blocks:
        for target in sorted({t for _, t in b['edges'] if isinstance(t, int)}):
            if target not in dominators[b['id']]:
                continue
            nodes, todo = {target, b['id']}, [b['id']] if target != b['id'] else []
            while todo:
                k = todo.pop()
                for pred in predecessors[k]:
                    if pred not in nodes:
                        nodes.add(pred)
                        todo.append(pred)
            loops.append({'header': target, 'tail': b['id'], 'blocks': sorted(nodes)})
    return {'blocks': len(blocks), 'edges': sum(len(b['edges']) for b in blocks),
            'conditions': sum(b['term'].startswith('j') and b['term'] != 'jmp' for b in blocks),
            'jumps': sum(b['term'] == 'jmp' for b in blocks),
            'returns': sum(b['term'].startswith('ret') for b in blocks),
            'calls': dict(sorted(Counter(c['key'] for b in blocks for c in b['calls']).items())),
            'natural_loops': loops,
            'backedges': [(b['id'], target) for b in blocks for _, target in b['edges'] if isinstance(target, int) and target <= b['id']],
            'joins': {str(k): sorted(v) for k, v in predecessors.items() if len(v) > 1}}


def statement_shapes(dec, blocks):
    """Ignore register choice and relocation payload, retain operations and widths.

    This is only a statement-placement diagnostic: e.g. cmp r,zero vs test r,r
    remains different. It must not be used as a byte-match criterion.
    """
    rows = []
    def image_address(value):
        return dec.pe is not None and dec.pe.get_section_by_rva((value & 0xffffffff) - dec.pe.OPTIONAL_HEADER.ImageBase) is not None
    for b in blocks:
        shape = []
        for off, _, _ in b['ins']:
            ins = dec.ins[off]
            ops = []
            relocated = dec.relocation(ins) is not None if dec.fn else False
            if ins.mnemonic == 'call':
                ops = [next(c['key'] for c in b['calls'] if c['offset'] == off)]
            elif ins.group(capstone.CS_GRP_JUMP):
                ops = ['edge']
            else:
                for op in ins.operands:
                    if op.type == X.X86_OP_REG:
                        ops.append(('reg', op.size))
                    elif op.type == X.X86_OP_IMM:
                        ops.append(('imm', 'reloc' if relocated or image_address(op.imm) else op.imm))
                    elif op.type == X.X86_OP_MEM:
                        # Stack/base displacement is real data; absolute addresses
                        # compare as symbolic references because PE != COFF.
                        disp = op.mem.disp if (op.mem.base or op.mem.index) and not relocated and not image_address(op.mem.disp) else 'absolute'
                        ops.append(('mem', op.size, bool(op.mem.base), bool(op.mem.index), op.mem.scale, disp))
            shape.append((ins.mnemonic, ops))
        rows.append(shape)
    return rows


def signature(b):
    return (b['term'], tuple((kind, target) for kind, target in b['edges']), tuple(c['key'] for c in b['calls']))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('object', type=Path)
    p.add_argument('symbol', help='exact decorated symbol or unique substring')
    p.add_argument('rva', type=number)
    p.add_argument('size', type=number, help='oracle code size without switch tables')
    p.add_argument('--tag', default='cfg')
    p.add_argument('--out', type=Path, default=Path(__file__).resolve().parent)
    args = p.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    coff = cmpfunc.Coff(args.object)
    matches = [f for f in coff.functions() if args.symbol == f.name or args.symbol in f.name]
    if len(matches) != 1:
        p.error(f'symbol selected {len(matches)} functions')
    fn = matches[0]
    pe = pefile.PE(str(cmpfunc.DEFAULT_EXE), fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase + args.rva
    decoders = {'candidate': Decoder(fn.data, len(fn.data), 0, coff, fn).decode(),
                'oracle': Decoder(pe.get_data(args.rva, args.size), args.size, base, pe=pe).decode()}
    bindings, names = known_bindings(pe)
    cc, oc = decoders['candidate'].calls, decoders['oracle'].calls
    inferred = {}
    # Unknown callees may be identified by consistent ordered static call correspondence.
    # Keep this evidence separate from verified bindings; it proves no relocations.
    if len(cc) == len(oc):
        proposals = defaultdict(set)
        for a, b in zip(cc, oc):
            if a['symbol'] not in bindings and b['va'] is not None:
                proposals[a['symbol']].add(b['va'])
        inferred = {s: next(iter(v)) for s, v in proposals.items() if len(v) == 1}
    for c in cc:
        va = bindings.get(c['symbol'], inferred.get(c['symbol']))
        # Plain imported call can target an ff25 thunk of the IAT entry.
        c['va'] = va
        c['binding'] = 'verified/manifest' if c['symbol'] in bindings else 'ordered correspondence (diagnostic)' if c['symbol'] in inferred else 'unresolved'
        c['key'] = f"{c['kind']}:{va:08x}" if va is not None else f"{c['kind']}:{c['symbol']}"
    # Both remaining register calls in event.cpp load PostMessage's IAT entry
    # into ESI. Prove each by walking backwards through all predecessor paths
    # until its reaching definition, rejecting disagreement or another writer.
    for which, dec in decoders.items():
        preds = defaultdict(set)
        for off, edges in dec.succ.items():
            for _, target in edges:
                preds[target].add(off)
        for c in dec.calls:
            ins = dec.ins[c['offset']]
            if ins.operands[0].type != X.X86_OP_REG:
                continue
            reg = ins.operands[0].reg
            todo, seen, values = list(preds[c['offset']]), set(), set()
            while todo:
                off = todo.pop()
                if off in seen:
                    continue
                seen.add(off)
                prev = dec.ins[off]
                writes = prev.regs_access()[1]
                if reg in writes:
                    ops = prev.operands
                    if prev.mnemonic == 'mov' and len(ops) == 2 and ops[0].type == X.X86_OP_REG and ops[0].reg == reg and ops[1].type == X.X86_OP_MEM and not ops[1].mem.base and not ops[1].mem.index:
                        rel = dec.relocation(prev) if dec.fn else None
                        va = bindings.get(coff.sym_by_index[rel.symbol_index].name) if rel else ops[1].mem.disp & 0xffffffff
                        values.add(va)
                    else:
                        values.add(None)
                    continue
                if not preds[off]:
                    values.add(None)
                todo.extend(preds[off])
            if len(values) == 1 and None not in values:
                c['va'] = values.pop()
                c['symbol'] = names.get(c['va'], hex(c['va']))
                c['key'] = f"indirect:{c['va']:08x}"
                c['binding'] = 'same IAT load on all reaching-definition paths'
    # Linker ICF aliases are compared by destination, with symbol displayed separately.
    for c in oc:
        c['key'] = f"{c['kind']}:{c['va']:08x}" if c['va'] is not None else f"{c['kind']}:{c['symbol']}"
        c['symbol'] = names.get(c['va'], c['symbol'])
    # Recognize import thunks without executing them.
    for a in cc:
        if a['va'] is None or a['kind'] != 'direct':
            continue
        for b in oc:
            if b['kind'] != 'direct' or b['va'] is None or a['va'] == b['va']:
                continue
            raw = pe.get_data(b['va'] - pe.OPTIONAL_HEADER.ImageBase, 6)
            if raw[:2] == b'\xff\x25' and struct.unpack_from('<I', raw, 2)[0] == a['va']:
                a['key'] = b['key']
    graphs = {which: dec.blocks() for which, dec in decoders.items()}
    summary = {which: stats(bs) for which, bs in graphs.items()}
    diffs = []
    if len(graphs['candidate']) == len(graphs['oracle']):
        for a, b in zip(graphs['candidate'], graphs['oracle']):
            for key in ('term', 'edges'):
                if a[key] != b[key]:
                    diffs.append({'block': a['id'], 'field': key, 'candidate': a[key], 'oracle': b[key]})
            if [c['key'] for c in a['calls']] != [c['key'] for c in b['calls']]:
                diffs.append({'block': a['id'], 'field': 'calls', 'candidate': a['calls'], 'oracle': b['calls']})
    else:
        matcher = difflib.SequenceMatcher(None, [signature(b) for b in graphs['candidate']], [signature(b) for b in graphs['oracle']], autojunk=False)
        diffs = [list(op) for op in matcher.get_opcodes() if op[0] != 'equal']
    for which, bs in graphs.items():
        lines = []
        for b in bs:
            lines.append(f"B{b['id']:03d} +{b['start']:04x}..{b['end']:04x} {b['term']} -> {b['edges']}")
            cm = {c['offset']: f"{c['symbol']} [{c['key']}]" for c in b['calls']}
            for off, mn, op in b['ins']:
                lines.append(f'  +{off:04x} {mn:7s} {cm.get(off, op)}')
        (args.out / f'{args.tag}-{which}-cfg.txt').write_text('\n'.join(lines) + '\n')
    tables = {which: {'jump_tables': dec.tables, 'byte_maps': dec.lookups, 'unresolved_jumps': dec.unresolved,
                       'decoded_instructions': len(dec.ins), 'reachable_bytes': sum(i.size for i in dec.ins.values()),
                       'code_end': max(i.address + i.size for i in dec.ins.values())} for which, dec in decoders.items()}
    table_diffs = []
    for key in ('jump_tables', 'byte_maps'):
        a, b = tables['candidate'][key], tables['oracle'][key]
        if len(a) != len(b):
            table_diffs.append(f'{key}: count {len(a)} != {len(b)}')
        for (ao, at), (bo, bt) in zip(sorted(a.items()), sorted(b.items())):
            if key == 'byte_maps':
                # COFF maps can carry alignment padding. Compare logical oracle extent.
                if at['bytes'][:len(bt['bytes'])] != bt['bytes']:
                    table_diffs.append(f'byte map +{ao:x}/+{bo:x}: contents differ')
            elif len(at['targets']) != len(bt['targets']):
                table_diffs.append(f'jump table +{ao:x}/+{bo:x}: entry counts differ')
    shapes = {w: statement_shapes(decoders[w], bs) for w, bs in graphs.items()}
    statement_diffs = []
    if len(graphs['candidate']) == len(graphs['oracle']):
        for k, (a, b) in enumerate(zip(shapes['candidate'], shapes['oracle'])):
            if a != b:
                statement_diffs.append({'block': k, 'candidate_offset': graphs['candidate'][k]['start'],
                                        'oracle_offset': graphs['oracle'][k]['start'],
                                        'candidate_instructions': len(a), 'oracle_instructions': len(b)})
    result = {'symbol': fn.name, 'rva': args.rva, 'oracle_size': args.size,
              'object': str(args.object), 'object_sha256': hashlib.sha256(args.object.read_bytes()).hexdigest(),
              'stats': summary, 'tables': tables, 'inferred_bindings': {s: hex(v) for s, v in inferred.items()},
              'ordered_structural_differences': diffs, 'table_differences': table_diffs,
              'statement_shape_differences': statement_diffs, 'graphs': graphs}
    (args.out / f'{args.tag}-cfg.json').write_text(json.dumps(result, indent=2))
    for which, s in summary.items():
        print(which, {k: v for k, v in s.items() if k not in ('calls', 'backedges', 'joins', 'natural_loops')},
              f"backedges={len(s['backedges'])} joins={len(s['joins'])} calls={sum(s['calls'].values())}",
              f"tables={len(tables[which]['jump_tables'])} byte_maps={len(tables[which]['byte_maps'])} loops={len(s['natural_loops'])}")
    print('ordered structural differences:', len(diffs), 'table differences:', table_diffs)
    print('unresolved jumps:', {w: d.unresolved for w, d in decoders.items()})
    if diffs:
        print(json.dumps(diffs[:8], indent=2))
    print('Diagnostic ordered correspondence bindings:', len(inferred), '(not STRICT proof)')
    print('Statement-shape differing blocks:', [d['block'] for d in statement_diffs])


if __name__ == '__main__':
    main()
