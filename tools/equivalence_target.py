#!/usr/bin/env python3
"""Target-local SB2 behavioral certificates. Never changes comparator/STRICT results.

Every run proves the declared straight-line/acyclic regions symbolically, checks
the rest of the complete function instruction by instruction (including branch
destinations), and independently resolves every symbolic relocation. Certificates
pin emitted bytes, oracle, tools, inventory, CRT and the complete proof receipt.
No native game code is run; oracle bytes are used only by this verifier.
"""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import copy
from dataclasses import replace
import hashlib
import itertools
import json
from pathlib import Path

import capstone as cs
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG
import cmpfunc
import pefile
import targets

ROOT = Path(__file__).resolve().parents[1]
MD = cs.Cs(cs.CS_ARCH_X86, cs.CS_MODE_32)
MD.detail = True
MASK = 0xffffffff
REGS = ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp')
ASSUMPTIONS = [
    'Memory accessed by either body is valid ordinary RAM, stable during each proof region; no MMIO, concurrency or asynchronous register/flags observation.',
    'The active callee stack frame is disjoint from pointed-to objects and outputs; distinct fields of an output POINT do not overlap.',
    'Calls follow the declared x86 ABI (stack arguments and ECX this); arithmetic flags and other scratch registers are not hidden arguments or return values.',
    'Equivalence covers defined memory effects, branch outcomes, ABI-preserved registers and declared return values. Calls retain their targets/arguments except for the separately proved pure SortGetType call commutation; timing, pure-helper invocation order and dead stack spills are excluded.',
    'Relocation checks establish symbol/data identity under the comparator binding model; they do not assert equivalence of otherwise unmatched callees.',
]


def require(ok, message):
    if not ok:
        raise ValueError(message)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def hashed(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True, separators=(',', ':')).encode()).hexdigest()


def declarations(target, manifest):
    """These are absolute target RVAs, never the base manifest's translated RVAs."""
    return manifest.get('target_equivalent_functions', {}).get(target.name, [])


# Expressions denote 32-bit bitvectors. Addition is a canonical affine sum of
# opaque atoms modulo 2**32; other operations are deliberately narrow rewrites.
# Equality of these expressions is an algebraic proof for ALL inputs, not samples.
def add(*terms):
    constant, parts = 0, {}
    for value, scale in terms:
        if isinstance(value, int):
            constant += value * scale
        elif value[0] == '+':
            constant += value[1] * scale
            for atom, coefficient in value[2]:
                parts[atom] = parts.get(atom, 0) + coefficient * scale
        else:
            parts[value] = parts.get(value, 0) + scale
    pairs = tuple(sorted(((a, c & MASK) for a, c in parts.items() if c & MASK), key=repr))
    constant &= MASK
    if not pairs:
        return constant
    if not constant and len(pairs) == 1 and pairs[0][1] == 1:
        return pairs[0][0]
    return ('+', constant, pairs)


def low(value, bits):
    mask = (1 << bits) - 1
    if isinstance(value, int):
        return value & mask
    if bits == 32:
        return value
    if value[0] == 'insert' and bits <= value[1]:
        return low(value[3], bits)
    if value[0] == 'low':
        return low(value[2], min(bits, value[1]))
    return ('low', bits, value)


def bitop(op, a, b):
    if isinstance(a, int) and isinstance(b, int):
        return {'and': lambda: a & b, 'or': lambda: a | b, 'xor': lambda: a ^ b}[op]() & MASK
    if a == b:
        return 0 if op == 'xor' else a
    if op in ('or', 'xor') and (a == 0 or b == 0):
        return b if a == 0 else a
    if op == 'and':
        if a == 0 or b == 0:
            return 0
        for v, m in ((a, b), (b, a)):
            if m == MASK:
                return v
            if isinstance(m, int) and m in (0xff, 0xffff):
                return low(v, 8 if m == 0xff else 16)
    return (op, *sorted((a, b), key=repr))


def alias(name):
    for full, names in (('eax', ('al', 'ah', 'ax')), ('ebx', ('bl', 'bh', 'bx')),
                        ('ecx', ('cl', 'ch', 'cx')), ('edx', ('dl', 'dh', 'dx'))):
        if name in names:
            return full, (0xff00 if name == names[1] else 0xffff if name == names[2] else 0xff)
    return name, MASK


class State:
    def __init__(self):
        self.regs = {r: ('input', r) for r in REGS}
        self.flags = ('input', 'flags')
        self.stack = {}
        self.reads = set()
        self.writes = []

    def reg(self, name):
        full, mask = alias(name)
        require(mask != 0xff00, 'high-byte register unsupported')
        return low(self.regs[full], mask.bit_length())

    def putreg(self, name, value):
        full, mask = alias(name)
        require(mask != 0xff00, 'high-byte register unsupported')
        self.regs[full] = (value if mask == MASK else low(value, mask.bit_length()) if self.regs[full] == 0 else
                           ('insert', mask.bit_length(), self.regs[full], low(value, mask.bit_length())))

    def address(self, op):
        m = op.mem
        require(not m.segment, 'segmented memory unsupported in proof region')
        return add((m.disp, 1), (self.reg(MD.reg_name(m.base)), 1) if m.base else (0, 1),
                   (self.reg(MD.reg_name(m.index)), m.scale) if m.index else (0, 1))

    def load(self, address, size):
        if (address, size) in self.stack:
            return self.stack[address, size]
        require(not self.writes, 'ordinary read after ordinary write needs an alias proof')
        self.reads.add((address, size))
        return ('memory', size, address)

    def value(self, op):
        if op.type == X86_OP_IMM:
            return op.imm & MASK
        if op.type == X86_OP_REG:
            return self.reg(MD.reg_name(op.reg))
        require(op.type == X86_OP_MEM, 'unsupported operand')
        return self.load(self.address(op), op.size)

    def put(self, op, value):
        if op.type == X86_OP_REG:
            self.putreg(MD.reg_name(op.reg), value)
        else:
            require(op.type == X86_OP_MEM, 'unsupported destination')
            self.writes.append((self.address(op), op.size, low(value, op.size * 8)))

    def step(self, i, pure_calls=None):
        m, o = i.mnemonic, i.operands
        if m == 'mov':
            self.put(o[0], self.value(o[1]))
        elif m == 'lea':
            self.put(o[0], self.address(o[1]))
        elif m in ('movsx', 'movzx'):
            v = low(self.value(o[1]), o[1].size * 8)
            self.put(o[0], ('signed', o[1].size * 8, v) if m == 'movsx' else v)
        elif m == 'cdq':
            self.regs['edx'] = ('sar', self.regs['eax'], 31)
        elif m in ('add', 'sub', 'and', 'or', 'xor'):
            a, b = self.value(o[0]), self.value(o[1])
            v = add((a, 1), (b, 1 if m == 'add' else -1)) if m in ('add', 'sub') else bitop(m, a, b)
            self.put(o[0], v)
            self.flags = (m, o[0].size, a, b)
        elif m in ('sar', 'shl'):
            a, count = self.value(o[0]), self.value(o[1])
            if isinstance(count, int):
                count &= 31
            v = add((a, 1 << count)) if m == 'shl' and isinstance(count, int) else (m, a, count)
            self.put(o[0], v)
            self.flags = (m, o[0].size, a, count)
        elif m in ('cmp', 'test'):
            a, b = self.value(o[0]), self.value(o[1])
            self.flags = (m, o[0].size, *sorted((a, b), key=repr)) if m == 'test' else (m, o[0].size, a, b)
        elif m == 'push':
            v = self.value(o[0])
            self.regs['esp'] = add((self.regs['esp'], 1), (-4, 1))
            self.stack[self.regs['esp'], 4] = v
        elif m == 'pop':
            v = self.load(self.regs['esp'], 4)
            self.put(o[0], v)
            self.regs['esp'] = add((self.regs['esp'], 1), (4, 1))
        elif m == 'call':
            require(pure_calls and i.address in pure_calls, 'call without a checked purity theorem')
            symbol = pure_calls[i.address]
            argument = self.load(self.regs['esp'], 4)
            self.regs['eax'] = ('pure-call', symbol, argument)
            for r in ('ecx', 'edx'):
                self.regs[r] = ('call-clobber', symbol, argument, r)
            self.flags = ('call-clobber', symbol, argument, 'flags')
        elif m.startswith('ret'):
            self.regs['esp'] = add((self.regs['esp'], 1), (4 + (o[0].imm if o else 0), 1))
        else:
            raise ValueError(f'unsupported proof instruction +{i.address:x}: {m} {i.op_str}')


def decode(data):
    insns = list(MD.disasm(data, 0))
    require(sum(i.size for i in insns) == len(data), 'incomplete instruction decode')
    return insns


def live_masks(insns, returns, pure_calls=None):
    """Conservative bit-level register liveness, fixed point over the whole CFG."""
    by = {i.address: i for i in insns}
    live = {pc: {} for pc in by}
    def merge(into, values):
        for r, mask in values.items():
            into[r] = into.get(r, 0) | mask
    def access(i):
        read, write = {}, {}
        rr, ww = i.regs_access()
        for dest, regs in ((read, rr), (write, ww)):
            for r in regs:
                name, mask = alias(i.reg_name(r))
                dest[name] = dest.get(name, 0) | mask
        if (i.mnemonic == 'xor' and len(i.operands) == 2
                and i.operands[0].type == i.operands[1].type == X86_OP_REG
                and i.operands[0].reg == i.operands[1].reg):
            read.pop(alias(i.reg_name(i.operands[0].reg))[0], None)
        if (i.mnemonic == 'or' and i.operands[0].type == X86_OP_REG
                and i.operands[1].type == X86_OP_IMM
                and i.operands[1].imm & ((1 << (8*i.operands[0].size))-1) == (1 << (8*i.operands[0].size))-1):
            read.pop(alias(i.reg_name(i.operands[0].reg))[0], None)
        if i.mnemonic == 'call':
            if not pure_calls or i.address not in pure_calls:
                read['ecx'] = MASK
            for r in ('eax', 'ecx', 'edx', 'eflags'):
                write[r] = MASK
        elif i.mnemonic not in ('cmp', 'test', 'xor', 'and', 'or', 'add', 'sub', 'neg'):
            write.pop('eflags', None)  # no partial/conditional flag kills
        return read, write
    for _ in range(len(insns) + 1):
        changed = False
        for i in reversed(insns):
            if i.mnemonic.startswith('ret'):
                before = {r: MASK for r in ('ebx', 'esi', 'edi', 'ebp', 'esp') + (('eax',) if returns else ())}
            else:
                after = {}
                following = [i.address + i.size]
                if i.mnemonic.startswith('j'):
                    if i.operands[0].type == X86_OP_IMM:
                        following = [i.operands[0].imm] + ([] if i.mnemonic == 'jmp' else following)
                    else:
                        # Unknown successors: keep every register/flag live.
                        # This cannot hide a differing value at a region exit.
                        merge(after, {r: MASK for r in (*REGS, 'eflags')})
                        following = []
                for pc in following:
                    if pc in live:
                        merge(after, live[pc])
                reads, writes = access(i)
                before = {r: m & ~writes.get(r, 0) for r, m in after.items() if m & ~writes.get(r, 0)}
                merge(before, reads)
            if before != live[i.address]:
                live[i.address] = before
                changed = True
        if not changed:
            return live
    raise ValueError('liveness did not converge')


def disjoint_writes(writes):
    """Only reorder writes proven disjoint by a constant modular byte distance."""
    for (a, sa, _), (b, sb, _) in itertools.combinations(writes, 2):
        delta = add((a, 1), (b, -1))
        require(isinstance(delta, int) and sb <= delta <= (1 << 32) - sa,
                'write ordering needs a non-alias proof')
    return tuple(sorted(writes, key=repr))


def paths(insns, lo, hi, pure_calls=None):
    by = {i.address: i for i in insns}
    pending, result = [(lo, State(), {}, frozenset())], []
    while pending:
        pc, state, conditions, visited = pending.pop()
        if pc < lo or pc >= hi:
            result.append((pc, state, conditions))
            continue
        require(pc not in visited, 'loops inside proof regions need a separate invariant')
        visited = visited | {pc}
        i = by[pc]
        if i.mnemonic.startswith('j'):
            require(i.operands[0].type == X86_OP_IMM, 'indirect region jump')
            if i.mnemonic == 'jmp':
                pending.append((i.operands[0].imm, state, conditions, visited))
                continue
            # Overapproximate each distinct comparison as independent. Proving
            # every Boolean assignment is stronger than feasible path sampling.
            atom = (i.mnemonic, state.flags)
            for truth, dest in ((True, i.operands[0].imm), (False, pc + i.size)):
                if atom in conditions and conditions[atom] != truth:
                    continue
                new = dict(conditions)
                new[atom] = truth
                pending.append((dest, copy.deepcopy(state), new, visited))
        else:
            state.step(i, pure_calls)
            if i.mnemonic.startswith('ret'):
                result.append(('return', state, conditions))
            else:
                pending.append((pc + i.size, state, conditions, visited))
    return result


def prove_region(a, b, region, mapping, lives, returns, pure_calls):
    lo, hi, other_lo, other_hi = region
    runs = [paths(insns, start, end, calls) for insns, start, end, calls in
            ((a, lo, hi, pure_calls[0]), (b, other_lo, other_hi, pure_calls[1]))]
    atoms = sorted({atom for run in runs for _, _, cond in run for atom in cond}, key=repr)
    require(len(atoms) <= 14, 'region Boolean proof too large')
    def observation(side, pc, state):
        dest = mapping[pc] if side == 0 and pc != 'return' else pc
        if pc == 'return':
            masks = {r: MASK for r in ('ebx', 'esi', 'edi', 'ebp', 'esp') + (('eax',) if returns else ())}
        else:
            masks = dict(lives[0].get(pc if side == 0 else next(k for k,v in mapping.items() if v == pc), {}))
            for r, mask in lives[1].get(dest, {}).items():
                masks[r] = masks.get(r, 0) | mask
        values = {r: low(state.regs[r], mask.bit_length()) for r, mask in masks.items() if r in REGS}
        if masks.get('eflags'):
            values['eflags'] = state.flags
        active_stack = []
        for (address, size), value in state.stack.items():
            delta = add((address, 1), (state.regs['esp'], -1))
            require(isinstance(delta, int), 'stack offset not constant at region exit')
            if delta < 0x80000000:
                active_stack.append(((address, size), value))
        return (dest, values, sorted(state.reads, key=repr), disjoint_writes(state.writes),
                sorted(active_stack, key=repr))
    observations = [[(observation(side, pc, s), cond) for pc, s, cond in run] for side, run in enumerate(runs)]
    for bits in itertools.product((False, True), repeat=len(atoms)):
        assignment = dict(zip(atoms, bits))
        selected = [[obs for obs, cond in run if all(assignment[k] == v for k,v in cond.items())] for run in observations]
        require(len(selected[0]) == len(selected[1]) == 1, 'region path coverage/termination failed')
        require(selected[0] == selected[1], f'region {region}: unequal symbolic effects: {selected}')
    return dict(object_window=[hex(lo), hex(hi)], oracle_window=[hex(other_lo), hex(other_hi)],
                boolean_atoms=len(atoms), exhaustive_boolean_assignments=1 << len(atoms),
                symbolic_outcomes_sha256=hashed([obs for obs, _ in observations[0]]))


def structure(function, raw, regions, returns, coff=None, pure_symbol=None):
    a, b = decode(function.data), decode(raw)
    require(bool(regions), 'equivalence needs an explicit proof region')
    for side, insns in enumerate((a, b)):
        boundaries = {i.address for i in insns} | {len(raw)}
        previous = 0
        for r in regions:
            lo, hi = r[2*side:2*side+2]
            require(previous <= lo < hi and lo in boundaries and hi in boundaries,
                    'overlapping/unordered region or non-instruction boundary')
            previous = hi
    # Padding remains bound by the payload hashes and the difference enumeration.
    # It is permitted only after a terminating instruction; branch checks below
    # reject any entry into that padding.
    def trim(insns):
        while insns and insns[-1].mnemonic == 'nop':
            insns = insns[:-1]
        require(insns[-1].mnemonic.startswith('ret') or insns[-1].mnemonic == 'jmp', 'nonterminal padding')
        return insns
    aa, bb = trim(a), trim(b)
    def tokens(insns, side):
        result, covered = [], set()
        for i in insns:
            index = next((n for n, r in enumerate(regions) if r[2*side] <= i.address < r[2*side+1]), None)
            if index is None:
                result.append(('i', i.address, i))
            elif index not in covered:
                require(i.address == regions[index][2*side], 'region not at instruction boundary')
                result.append(('r', i.address, index))
                covered.add(index)
        require(len(covered) == len(regions), 'missing proof region')
        return result
    ta, tb = tokens(aa, 0), tokens(bb, 1)
    require(len(ta) == len(tb), 'outside-region instruction count differs')
    mapping = {x[1]: y[1] for x,y in zip(ta,tb)}
    mapping[len(function.data)] = len(raw)
    for r in regions:
        mapping[r[1]] = r[3]
    for x,y in zip(ta,tb):
        require(x[0] == y[0], 'region alignment differs')
        if x[0] == 'r':
            require(x[2] == y[2], 'region order differs')
    sites, reloc_by_insn, pure_calls = {}, {}, [{}, {}]
    inside_calls = []
    for r in function.relocs:
        off, size = r.offset - function.start, cmpfunc.relocation_size(r.typ)
        owner = next(i for i in aa if i.address <= off and off + size <= i.address + i.size)
        if owner.address not in mapping:
            require(pure_symbol and coff.sym_by_index[r.symbol_index].name == pure_symbol
                    and r.typ == 0x14 and owner.mnemonic == 'call' and off == owner.address+1,
                    'relocation inside proof region lacks a purity theorem')
            inside_calls.append((owner, off))
            continue
        sites[off] = mapping[owner.address] + off - owner.address
        reloc_by_insn.setdefault(owner.address, []).append((off - owner.address, size))
    if inside_calls:
        other_calls = [i for i in bb if i.mnemonic == 'call' and any(r[2] <= i.address < r[3] for r in regions)]
        require(len(inside_calls) == len(other_calls), 'pure call count changed')
        for (i, off), j in zip(inside_calls, other_calls):
            require(i.bytes[0] == j.bytes[0] == 0xe8 and i.size == j.size == 5, 'pure direct call encoding changed')
            sites[off] = j.address+1
            pure_calls[0][i.address] = pure_calls[1][j.address] = pure_symbol
    for x,y in zip(ta,tb):
        if x[0] != 'i':
            continue
        i,j = x[2],y[2]
        require(i.mnemonic == j.mnemonic, f'outside region opcode +{i.address:x}')
        if i.mnemonic.startswith('j'):
            if i.operands[0].type == j.operands[0].type == X86_OP_IMM:
                require(mapping.get(i.operands[0].imm) == j.operands[0].imm, 'branch destination changed/enters region interior')
            else:
                require(i.bytes == j.bytes and not regions, 'indirect jump needs an explicit jump-table proof')
        else:
            left, right = bytearray(i.bytes), bytearray(j.bytes)
            require(len(left) == len(right), 'outside-region instruction size differs')
            for off, size in reloc_by_insn.get(i.address, []):
                left[off:off+size] = right[off:off+size] = b'\0' * size
            require(left == right, f'unproved outside-region instruction +{i.address:x}: {i.mnemonic} {i.op_str}')
    lives = [live_masks(a, returns, pure_calls[0]), live_masks(b, returns, pure_calls[1])]
    checks = [prove_region(a, b, r, mapping, lives, returns, pure_calls) for r in regions]
    object_mask = {k for r in function.relocs for k in range(r.offset-function.start, r.offset-function.start+4)}
    oracle_mask = {k for off in sites.values() for k in range(off,off+4)}
    differences = [[hex(k), x, y] for k,(x,y) in enumerate(zip(function.data, raw))
                   if x != y and k not in object_mask.intersection(oracle_mask)]
    return sites, dict(regions=checks, nonrelocation_differences=differences,
                       differing_bytes=len(differences), outside_region_instructions_checked=sum(x[0]=='i' for x in ta),
                       object_padding_bytes=len(function.data)-(aa[-1].address+aa[-1].size),
                       oracle_padding_bytes=len(raw)-(bb[-1].address+bb[-1].size))


@contextmanager
def inventory(target):
    starts, tus = cmpfunc.oracle_function_starts, cmpfunc.oracle_tu_at_rva
    try:
        cmpfunc.oracle_function_starts = lambda: starts(target.oracle_dir/'functions.csv')
        cmpfunc.oracle_tu_at_rva = lambda: tus(target.oracle_dir/'tu_map.csv')
        yield
    finally:
        cmpfunc.oracle_function_starts, cmpfunc.oracle_tu_at_rva = starts, tus


def library_functions(library, out):
    data = library.read_bytes()
    require(data.startswith(b'!<arch>\n'), 'CRT is not an archive')
    offset = 8
    while offset + 60 <= len(data):
        header = data[offset:offset+60]
        size = int(header[48:58])
        body = data[offset+60:offset+60+size]
        offset += 60 + size + (size & 1)
        # Only object members can be COFF, not archive name/symbol tables.
        if body[:2] != b'\x4c\x01':
            continue
        path = out/'crt-member.obj'
        path.write_bytes(body)
        coff = cmpfunc.Coff(path)
        yield coff, coff.functions()


def bindings(target, manifest, comparison, evidence, pe, out, library):
    tx = next(s for s in pe.sections if s.Name.startswith(b'.text'))
    td, tr, ib = tx.get_data(), tx.VirtualAddress, pe.OPTIONAL_HEADER.ImageBase
    paths_ = [Path(u['object']).resolve() for u in comparison] + list(evidence)
    at = {a['symbol']: int(str(a['rva']), 0) for a in manifest.get('accepted_functions', [])}
    at.update({a['symbol']: int(str(a['rva']), 0) for a in manifest.get('forced_locations', [])})
    explicit = {a['symbol']: int(str(a['va']), 0) for a in manifest.get('symbol_mappings', [])}
    with inventory(target):
        shared, global_, fd, dd, _, _, _, _ = cmpfunc.collect_global_state(paths_, pe, td, tr, at, explicit)
        starts = {ib+r for r in cmpfunc.oracle_function_starts()}
    for unit in comparison:
        for row in unit['functions']:
            if row['status'] in ('STRICT', 'FOLDED') and len(row['matches']) == 1:
                key, va = 'global:'+row['name'], ib+int(row['matches'][0]['rva'], 16)
                require(key not in global_ or global_[key] == va, 'conflicting independently checked binding')
                global_[key] = va
    # Missing CRT names must have a unique complete, raw-identical library body
    # WITHOUT relocations. This is stricter than a masked signature identity.
    needed = {r['symbol'] for r in declarations(target, manifest)}
    missing = set()
    for path in paths_:
        coff = cmpfunc.Coff(path)
        for f in coff.functions():
            if f.name in needed:
                for r in f.relocs:
                    name = coff.sym_by_index[r.symbol_index].name
                    if 'global:'+name not in global_:
                        missing.add(name)
    crt = {}
    if missing:
        for coff, functions in library_functions(library, out):
            for f in functions:
                if f.name not in missing or f.relocs:
                    continue
                hits = cmpfunc.candidates_for(f, td)
                if len(hits) == 1 and f.data == td[hits[0]:hits[0]+len(f.data)]:
                    require(f.name not in crt, 'ambiguous CRT library definition')
                    va = ib+tr+hits[0]
                    require(va in starts, 'CRT signature not at an oracle function entry')
                    global_['global:'+f.name] = va
                    crt[f.name] = dict(va=hex(va), size=len(f.data), body_sha256=hashlib.sha256(f.data).hexdigest(),
                                       proof='unique complete raw-identical relocation-free CRT library body')
    return (shared, global_, fd, dd, cmpfunc.import_map(pe), explicit, starts), crt


def pure_sort_callee(comparison, pe, symbol):
    """The only call reordering lemma: SortGetType is a total pure integer map."""
    require(symbol == '?SortGetType@@YAHH@Z', 'unknown purity theorem')
    matches = [(u,r) for u in comparison for r in u['functions'] if r['name'] == symbol]
    require(len(matches) == 1, 'pure callee must have one emitted definition')
    unit, row = matches[0]
    require(row['status'] == 'STRICT' and len(row['matches']) == 1, 'pure callee must independently be STRICT')
    c = cmpfunc.Coff(Path(unit['object']))
    f, = [f for f in c.functions() if f.name == symbol]
    require(not f.relocs and len(f.data) == 48, 'pure callee payload changed')
    va = int(row['matches'][0]['rva'],16)
    require(f.data == pe.get_data(va,48), 'pure callee not raw-identical')
    texts = [i.mnemonic+' '+i.op_str for i in decode(f.data) if i.mnemonic != 'nop']
    require(texts == ['mov eax, dword ptr [esp + 4]', 'cmp eax, 2', 'je 0x26',
        'cmp eax, 3', 'je 0x26', 'cmp eax, 0x60', 'je 0x26', 'cmp eax, 0x61', 'je 0x26',
        'xor ecx, ecx', 'cmp eax, 0xc', 'setne cl', 'add ecx, 2', 'mov eax, ecx', 'ret ',
        'mov eax, 1', 'ret '], 'pure callee instruction theorem changed')
    # All paths read only their stack argument and change only EAX/ECX/flags;
    # no writes, other reads, calls, loops, traps or external references exist.
    return dict(symbol=symbol, rva=hex(va), size=48, body_sha256=hashlib.sha256(f.data).hexdigest(),
                theorem='F(x)=1 for x in {2,3,96,97}; F(12)=2; F(x)=3 otherwise. Total, read-only stack argument, no external effects.')


def check(target, manifest, comparison, evidence_objects, out_dir, write=False):
    entries = declarations(target, manifest)
    if not entries:
        return {}
    require(__debug__, 'equivalence requires assertions enabled (no -O)')
    require(target.name.startswith('sb2/') and target.oracle_dir is not None, 'SB2 inventory required')
    target.check_exe()
    out = Path(out_dir).resolve()
    require(any(out.is_relative_to(ROOT/d) for d in ('build', 'scratch')), 'proof output must be inside build/ or scratch/')
    out.mkdir(parents=True, exist_ok=True)
    import msvc
    tree = msvc.resolve_tree(manifest['toolchain']['tree'])
    _, vc, _, _ = msvc.tool_layout(tree, None)
    library = vc/'LIB/LIBC.LIB'
    pe = pefile.PE(str(target.exe))
    tx = next(s for s in pe.sections if s.Name.startswith(b'.text'))
    state, crt = bindings(target, manifest, comparison, evidence_objects, pe, out, library)
    results = {}
    require(len({e['symbol'] for e in entries}) == len(entries), 'duplicate equivalence declarations')
    for entry in entries:
        name, symbol = entry['name'], entry['symbol']
        matches = [(u, f) for u in comparison for f in u['functions'] if f['name'] == symbol]
        require(len(matches) == 1, f'{name}: expected exactly one emitted body')
        unit, row = matches[0]
        rva, size = int(str(entry['rva']),0), entry['size']
        require(row['code_size'] == size, f'{name}: function extent changed')
        require(Path(unit['object']).stem.endswith('_'+Path(entry['tu']).stem), f'{name}: TU changed')
        require(pe.OPTIONAL_HEADER.ImageBase+rva in state[-1], f'{name}: not an oracle entry')
        require(row['status'] in ('MISMATCH', 'STRICT', 'FOLDED'), f'{name}: unexpected byte status')
        if row['status'] in ('STRICT','FOLDED'):
            require(len(row['matches']) == 1 and int(row['matches'][0]['rva'],16) == rva, f'{name}: strict location changed')
            results[name] = dict(status=row['status'], certificate_required=False)
            continue
        coff = cmpfunc.Coff(Path(unit['object']))
        f, = [f for f in coff.functions() if f.name == symbol]
        raw = pe.get_data(rva, size)
        require(len(f.data) == len(raw) == size, f'{name}: incomplete payload')
        regions = [tuple(int(str(x),0) for x in r) for r in entry['regions']]
        pure = pure_sort_callee(comparison, pe, entry['pure_callee']) if entry.get('pure_callee') else None
        sites, checks = structure(f, raw, regions, entry['returns_value'], coff, entry.get('pure_callee'))
        if pure:
            checks['pure_callee_identity'] = pure
        shared, globals_, fd, dd, imports, explicit, starts = state
        rr = []
        for reloc in f.relocs:
            offset = reloc.offset-f.start
            # The original call site may move when a region changes length.
            # Resolve the actual original field, preserving the COFF addend.
            one = replace(f, relocs=[reloc])
            status, rows = cmpfunc.resolve_sites(one, rva-tx.VirtualAddress+sites[offset]-offset,
                tx.get_data(), tx.VirtualAddress, pe.OPTIONAL_HEADER.ImageBase, pe, coff, Path(unit['object']),
                shared, fd, dd, imports, explicit, globals_, starts)
            require(status == 'STRICT' and all(r['verified'] for r in rows), f'{name}: unresolved relocation {rows}')
            rr.extend(dict(r, oracle_offset=sites[offset]) for r in rows)
        checks['verified_relocations'] = len(rr)
        checks['relocations_sha256'] = hashed(rr)
        current = dict(schema=1, status='EQUIVALENT', target=target.name, declaration=entry,
                       scope='static-local-equivalence', assumptions=ASSUMPTIONS,
                       payload_sha256=hashed(dict(bytes=f.data.hex(), relocations=[
                           [r.offset-f.start,r.typ,coff.sym_by_index[r.symbol_index].name] for r in f.relocs])),
                       oracle_sha256=target.sha256, oracle_function_sha256=hashlib.sha256(raw).hexdigest(),
                       checker_sha256=digest(__file__), comparator_sha256=digest(ROOT/'tools/cmpfunc.py'),
                       comparator_wrapper_sha256=digest(ROOT/'tools/cmpfunc_target.py'),
                       capstone_version=cs.__version__, crt_library_sha256=digest(library),
                       inventory_sha256={p:digest(target.oracle_dir/p) for p in ('functions.csv','tu_map.csv')},
                       crt_target_identity=crt, checks=checks)
        path = (ROOT/entry['certificate']).resolve()
        require(path.parent == (ROOT/'evidence/equivalence'/target.slug).resolve() and path.suffix == '.json',
                'certificate outside target evidence directory')
        if write:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(json.dumps(current, indent=2)+'\n', encoding='utf-8')
        else:
            require(json.loads(path.read_text(encoding='utf-8')) == current, f'{name}: equivalence certificate changed; review required')
        results[name] = dict(status='EQUIVALENT', byte_status=row['status'], size=size, rva=hex(rva),
                             certificate=entry['certificate'])
        print(f'{name}: EQUIVALENT ({size:,} bytes; static local proof; byte status MISMATCH)')
    accepted = [r for r in results.values() if r['status'] == 'EQUIVALENT']
    print(f'EQUIVALENT tier: {len(accepted)} functions, {sum(r["size"] for r in accepted):,} bytes (excluded from STRICT coverage)')
    (out/'equivalence.json').write_text(json.dumps(results, indent=2)+'\n', encoding='utf-8')
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    targets.add_argument(parser)
    parser.add_argument('--objects-dir', type=Path, required=True)
    parser.add_argument('--out-dir', type=Path, default=ROOT/'build/w-equiv-audit')
    parser.add_argument('--write-certificates', action='store_true')
    args = parser.parse_args()
    target = targets.load(args.target)
    import verify
    verify.TARGET, verify.BUILD = target, args.out_dir.resolve()
    verify.GAME_TEXT_END = target.game_text_end
    verify.GAME_TEXT_SIZE = target.game_text_end - verify.ORACLE_TEXT_START
    verify.BUILD.mkdir(parents=True, exist_ok=True)
    objects = sorted(args.objects_dir.resolve().glob('*.obj'))
    evidence = [p for p in objects if p.name.startswith('crt_')]
    comparison = verify.run_comparator([p for p in objects if p not in evidence], evidence, target.manifest())
    by, _ = verify.report(comparison, [u['source'] for u in target.manifest()['translation_units']])
    for index, unit in enumerate(target.manifest()['translation_units']):
        if unit.get('build_path'):
            log = args.objects_dir.parent/'logs'/f'{index:02d}_{Path(unit["source"]).stem}.log'
            command = log.read_text(encoding='utf-8') if log.is_file() else ''
            if unit['build_path'].lower() not in command.lower():
                verify.DEGRADED[unit['source']] = 'historical source path not attested in reused object log'
                print(f'gate INCOMPLETE: {unit["source"]}: {verify.DEGRADED[unit["source"]]}')
    require(not verify.accepted_regressions(target.manifest(), by), 'strict peer regression')
    check(target, target.manifest(), comparison, evidence, verify.BUILD, args.write_certificates)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
