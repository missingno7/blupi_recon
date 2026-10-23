#!/usr/bin/env python3
"""Target-local SB2 behavioral certificates. Never changes comparator/STRICT results.

Every run proves the declared straight-line/acyclic regions symbolically, checks
the complete continuation by aligned instructions or an inductive product CFG,
and independently resolves every symbolic relocation. Certificates
pin emitted bytes, oracle, tools, inventory, CRT and the complete proof receipt.
No native game code is run; oracle bytes are used only by this verifier.
"""
from __future__ import annotations

import argparse
from contextlib import contextmanager
import copy
from collections import deque
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


def difference_ranges(differences):
    """Enumerate offsets compactly; complete payload hashes bind byte values."""
    ranges = []
    for offset,_,_ in differences:
        offset = int(offset,0)
        if ranges and int(ranges[-1][1],0) == offset:
            ranges[-1][1] = hex(offset+1)
        else:
            ranges.append([hex(offset),hex(offset+1)])
    return ranges


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
    if name in ('si','di','bp','sp'):
        return 'e'+name,0xffff
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


def paths(insns, lo, hi, pure_calls=None, initial=None, invariant_flags=False):
    by = {i.address: i for i in insns}
    pending, result = [(lo, initial or State(), {}, frozenset())], []
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
            if invariant_flags and i.mnemonic in ('sete', 'setne'):
                value = ('zero', state.flags)
                state.put(i.operands[0], value if i.mnemonic == 'sete' else ('not', value))
            else:
                state.step(i, pure_calls)
            if invariant_flags:
                state.flags = defined_flags(state.flags)
            if i.mnemonic.startswith('ret'):
                result.append(('return', state, conditions))
            else:
                pending.append((pc + i.size, state, conditions, visited))
    return result


def defined_flags(flags):
    # CMP x,0 and TEST x,x define identical CF/OF/ZF/SF/PF. AF is
    # undefined for TEST; invariant mode separately rejects any AF observer.
    if flags[0] == 'cmp' and flags[3] == 0:
        return ('zero-flags', flags[1], flags[2])
    if flags[0] == 'test' and flags[2] == flags[3]:
        return ('zero-flags', flags[1], flags[2])
    return flags


def constant_states(insns, relocation_offsets=()):
    """Must constants at each reachable instruction, including back edges.

    Only MOV and zero/all-ones idioms generate facts. Every other write
    conservatively kills its full register, including partial writes. No
    field value, branch feasibility or callee result is assumed.
    """
    by = {i.address: i for i in insns}
    require(not any((i.group(cs.CS_GRP_JUMP) or i.group(cs.CS_GRP_BRANCH_RELATIVE))
                    and not i.mnemonic.startswith('j') and i.mnemonic != 'call' for i in insns),
            'constant invariants need explicitly modeled control transfers')
    incoming, pending = {0: {}}, deque([0])
    while pending:
        pc = pending.popleft()
        i, before = by[pc], incoming[pc]
        after = dict(before)
        for r in i.regs_access()[1]:
            after.pop(alias(i.reg_name(r))[0], None)
        if i.mnemonic == 'call':
            for r in ('eax','ecx','edx'):
                after.pop(r, None)
        o = i.operands
        relocated = any(i.address <= k < i.address+i.size for k in relocation_offsets)
        if o and o[0].type == X86_OP_REG and o[0].size == 4 and not relocated:
            reg = i.reg_name(o[0].reg)
            if i.mnemonic == 'mov':
                if o[1].type == X86_OP_IMM:
                    after[reg] = o[1].imm & MASK
                elif o[1].type == X86_OP_REG and i.reg_name(o[1].reg) in before:
                    after[reg] = before[i.reg_name(o[1].reg)]
            elif i.mnemonic == 'xor' and o[1].type == X86_OP_REG and o[0].reg == o[1].reg:
                after[reg] = 0
            elif i.mnemonic == 'or' and o[1].type == X86_OP_IMM and o[1].imm & MASK == MASK:
                after[reg] = MASK
        if i.mnemonic.startswith('ret'):
            successors = []
        elif i.mnemonic.startswith('j'):
            require(o[0].type == X86_OP_IMM, 'constant invariants need a complete direct CFG')
            successors = [o[0].imm] + ([] if i.mnemonic == 'jmp' else [pc+i.size])
        else:
            successors = [pc+i.size]
        for dest in successors:
            require(dest in by, f'CFG leaves decoded body at +{dest:x}')
            joined = after if dest not in incoming else {r:v for r,v in incoming[dest].items() if after.get(r) == v}
            if dest not in incoming or incoming[dest] != joined:
                incoming[dest] = dict(joined)
                pending.append(dest)
    return incoming


def constant_join(before, after, lives, constants, full_writes=()):
    """Dropping a unary fact at a join must not silently strengthen R.

    If two independently known constants differ, equality cannot become an
    obligation at a successor unless the common instruction fully overwrites
    that register from equal inputs. Partial writes cannot discharge this.
    """
    for r in REGS:
        if r not in lives[0].get(after[0],{}) or r not in lives[1].get(after[1],{}):
            continue
        if all(r in constants[s].get(after[s],{}) for s in (0,1)) or r in full_writes:
            continue
        a,b = (constants[s].get(before[s],{}).get(r) for s in (0,1))
        require(a is None or b is None or a == b, f'constant relation lost at join for {r}: {before}->{after}')


def full_register_writes(i):
    result = {alias(i.reg_name(r))[0] for r in i.regs_access()[1] if alias(i.reg_name(r))[1] == MASK}
    if i.mnemonic == 'call':
        result.update(('eax','ecx','edx'))
    return result


def prove_region(a, b, region, mapping, lives, returns, pure_calls, constants=None):
    lo, hi, other_lo, other_hi = region
    initial = [State(), State()]
    if constants:
        for r in REGS:
            ca, cb = constants[0][lo].get(r), constants[1][other_lo].get(r)
            both_live = r in lives[0][lo] and r in lives[1][other_lo]
            for side, c in enumerate((ca, cb)):
                other = (cb, ca)[side]
                initial[side].regs[r] = (c if c is not None else other if both_live and other is not None
                                        else ('input', r) if both_live else ('private', side, r))
        # Stack addresses must correspond even if a region doesn't itself read ESP.
        initial[0].regs['esp'] = initial[1].regs['esp'] = ('input','esp')
        if 'eflags' not in lives[0][lo] or 'eflags' not in lives[1][other_lo]:
            for side in (0,1):
                initial[side].flags = ('private-flags',side)
    runs = [paths(insns, start, end, calls, initial[side], bool(constants)) for side,(insns,start,end,calls) in enumerate(
            ((a, lo, hi, pure_calls[0]), (b, other_lo, other_hi, pure_calls[1])))]
    atoms = sorted({atom for run in runs for _, _, cond in run for atom in cond}, key=repr)
    require(len(atoms) <= 14, 'region Boolean proof too large')
    def observation(side, pc, state):
        dest = mapping[pc] if side == 0 and pc != 'return' else pc
        if pc == 'return':
            masks = {r: MASK for r in ('ebx', 'esi', 'edi', 'ebp', 'esp') + (('eax',) if returns else ())}
        else:
            origin = pc if side == 0 else hi if pc == other_hi else next(k for k,v in mapping.items() if v == pc)
            masks = dict(lives[0].get(origin, {}))
            for r, mask in lives[1].get(dest, {}).items():
                masks[r] = masks.get(r, 0) | mask
        values = {r: low(state.regs[r], mask.bit_length()) for r, mask in masks.items() if r in REGS}
        if constants and pc != 'return':
            origin = pc if side == 0 else hi if pc == other_hi else next(k for k,v in mapping.items() if v == pc)
            for r in list(values):
                if r == 'esp':
                    continue
                # The join relation is equality of all mutually live unknowns;
                # must-constant registers may instead carry their independent
                # proven constants. A private dead register is unconstrained.
                own_pc = pc
                c = constants[side].get(own_pc, {}).get(r)
                if c is not None:
                    require(low(state.regs[r], masks[r].bit_length()) == low(c, masks[r].bit_length()),
                            f'constant postcondition failed for {r} at +{own_pc:x}')
                if r not in lives[0].get(origin,{}) or r not in lives[1].get(dest,{}):
                    del values[r]
                elif r in constants[0].get(origin,{}) and r in constants[1].get(dest,{}):
                    del values[r]
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


def structure(function, raw, regions, returns, coff=None, pure_symbol=None, constant_invariants=False,
              region_relocations=()):
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
    region_sites = dict(region_relocations)
    symbolic_raw = bytearray(raw)
    for r in function.relocs:
        off, size = r.offset - function.start, cmpfunc.relocation_size(r.typ)
        owner = next(i for i in aa if i.address <= off and off + size <= i.address + i.size)
        if off in region_sites:
            other = region_sites[off]
            j = next(i for i in bb if i.address <= other and other+size <= i.address+i.size)
            require(r.typ == 6 and size == 4 and off == owner.address+owner.disp_offset
                    and other == j.address+j.disp_offset and owner.disp_size == j.disp_size == 4,
                    'region relocation must be an audited absolute memory displacement')
            require(any(x <= owner.address < y and z <= j.address < w for x,y,z,w in regions),
                    'region relocation outside paired regions')
            sites[off] = other
            symbolic_raw[other:other+size] = function.data[off:off+size]
            continue
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
                require(mapping.get(i.operands[0].imm) == j.operands[0].imm,
                        f'branch destination changed/enters region interior at +{i.address:x}: +{i.operands[0].imm:x}/+{j.operands[0].imm:x}')
            else:
                require(i.bytes == j.bytes and not regions, 'indirect jump needs an explicit jump-table proof')
        else:
            left, right = bytearray(i.bytes), bytearray(j.bytes)
            require(len(left) == len(right), 'outside-region instruction size differs')
            for off, size in reloc_by_insn.get(i.address, []):
                left[off:off+size] = right[off:off+size] = b'\0' * size
            require(left == right, f'unproved outside-region instruction +{i.address:x}: {i.mnemonic} {i.op_str}')
    lives = [live_masks(a, returns, pure_calls[0]), live_masks(b, returns, pure_calls[1])]
    constants = None
    if constant_invariants:
        constants = [constant_states(a, [r.offset-function.start for r in function.relocs]),
                     constant_states(b, sites.values())]
        require(not any('esp' in state for states in constants for state in states.values()),
                'constant ESP needs a separate stack relation')
        # No instruction may observe AF or expose flags as a whole: this makes
        # the CMP-zero/TEST identity valid for every flags consumer here.
        for insns in (aa,bb):
            for i in insns:
                if any(i.reg_name(r) == 'eflags' for r in i.regs_access()[0]):
                    require(i.mnemonic.startswith(('j','set','cmov','rep')) or i.mnemonic in ('adc','sbb'),
                            f'unsupported flags observer in invariant proof: {i.mnemonic}')
        for x,y in zip(ta,tb):
            if x[0] != 'i':
                continue
            i,j = x[2],y[2]
            ca,cb = constants[0].get(i.address,{}), constants[1].get(j.address,{})
            reads = {alias(i.reg_name(r))[0] for r in i.regs_access()[0]}
            if i.mnemonic == 'call':
                reads.add('ecx')
            if i.mnemonic.startswith('ret'):
                reads.update(('ebx','esi','edi','ebp','esp') + (('eax',) if returns else ()))
            if i.mnemonic == 'xor' and len(i.operands)==2 and i.operands[0].type==i.operands[1].type==X86_OP_REG and i.operands[0].reg==i.operands[1].reg:
                reads.discard(alias(i.reg_name(i.operands[0].reg))[0])
            require(not any(r in ca and r in cb and ca[r] != cb[r] for r in reads),
                    f'unchanged instruction reads unequal constants at +{i.address:x}')
            if not i.mnemonic.startswith('ret'):
                successors = [(i.address+i.size,j.address+j.size)]
                if i.mnemonic.startswith('j'):
                    successors = [(i.operands[0].imm,j.operands[0].imm)] + ([] if i.mnemonic=='jmp' else successors)
                for successor in successors:
                    constant_join((i.address,j.address),successor,lives,constants,full_register_writes(i))
    proof_b = decode(bytes(symbolic_raw)) if region_sites else b
    checks = [prove_region(a, proof_b, r, mapping, lives, returns, pure_calls, constants) for r in regions]
    object_mask = {k for r in function.relocs for k in range(r.offset-function.start, r.offset-function.start+4)}
    oracle_mask = {k for off in sites.values() for k in range(off,off+4)}
    differences = [[hex(k), x, y] for k,(x,y) in enumerate(zip(function.data, raw))
                   if x != y and k not in object_mask.intersection(oracle_mask)]
    receipt = dict(regions=checks, nonrelocation_differences=differences,
                       differing_bytes=len(differences), outside_region_instructions_checked=sum(x[0]=='i' for x in ta),
                       object_padding_bytes=len(function.data)-(aa[-1].address+aa[-1].size),
                       oracle_padding_bytes=len(raw)-(bb[-1].address+bb[-1].size))
    if constants:
        receipt.pop('nonrelocation_differences')
        receipt['nonrelocation_difference_ranges'] = difference_ranges(differences)
        receipt['difference_values_sha256'] = hashed(differences)
        receipt['constant_invariants'] = dict(method='whole-CFG must constants, meet at every predecessor/back edge; ABI call kills',
            object_reachable_instructions=len(constants[0]), oracle_reachable_instructions=len(constants[1]),
            states_sha256=hashed(constants), relation='equal mutually live unknown registers; independently proven must constants; equal observable memory, stack and defined flags')
    require(set(region_sites) <= set(sites), 'unused region relocation declaration')
    return sites, receipt


def cfg_structure(function, raw, regions, returns):
    """Inductive product-CFG proof, allowing finite constant-reload stutters.

    Every paired boundary has the same live-unknown/independent-must-constant
    relation as prove_region. Plain MOV r32,imm32 and direct unconditional jumps
    may move across joins; each one-sided MOV is proved as an empty-other-side
    region. No calls, stores, flags writes or arbitrary instructions stutter.
    Finite epsilon closures rule out unmatched silent divergence. Both complete
    reachable instruction sets, branch arms and relocation fields are covered.
    """
    decoded = [decode(function.data),decode(raw)]
    bodies = []
    for insns in decoded:
        body = list(insns)
        while body and body[-1].mnemonic == 'nop':
            body.pop()
        require(body and body[-1].mnemonic.startswith('ret'), 'CFG proof needs terminal padding')
        bodies.append(body)
    by = [{i.address:i for i in insns} for insns in bodies]
    region_at = [{},{}]
    for side in (0,1):
        previous = 0
        for n,r in enumerate(regions):
            lo,hi = r[2*side:2*side+2]
            require(previous <= lo < hi and lo in by[side] and hi in by[side], 'CFG region boundary/order')
            region_at[side][lo] = n
            previous = hi
        for i in bodies[side]:
            if i.mnemonic.startswith('j'):
                require(i.operands[0].type == X86_OP_IMM, 'CFG indirect branch is unproved')
                dest = i.operands[0].imm
                require(dest in by[side], 'CFG branch leaves complete body')
                require(not any(r[2*side] < dest < r[2*side+1] for r in regions), 'CFG jump enters region interior')
            if any(i.reg_name(r) == 'eflags' for r in i.regs_access()[0]):
                require(i.mnemonic.startswith(('j','set','cmov','rep')) or i.mnemonic in ('adc','sbb'), 'CFG unsupported flags observer')
    object_sites = {r.offset-function.start:r for r in function.relocs}
    lives = [live_masks(insns,returns) for insns in bodies]
    # Oracle constant-generating immediates inside .text never carry data
    # relocations in this method: all such fields pair with an object field.
    # Exclude every plausible absolute image pointer conservatively as well.
    oracle_addresses = [i.address+i.imm_offset for i in bodies[1] if i.imm_size == 4 and
                        any(o.type == X86_OP_IMM and 0x400000 <= o.imm < 0x10000000 for o in i.operands)]
    constants = [constant_states(bodies[0],object_sites),constant_states(bodies[1],oracle_addresses)]
    require(not any('esp' in s for states in constants for s in states.values()), 'CFG constant ESP unproved')
    seen,covered,sites,checks,stutters = set(),[set(),set()],{},[],[]
    pending = [(0,0,None,None)]
    flags_cache = {}
    def flag_key(side,lo,hi):
        if (side,lo,hi) in flags_cache:
            return flags_cache[side,lo,hi]
        state = State()
        state.regs.update(constants[side][lo])
        pc = lo
        while pc < hi:
            i = by[side][pc]
            state.step(i)
            pc += i.size
        require(pc == hi,'flag-key instruction boundary')
        flags_cache[side,lo,hi] = defined_flags(state.flags)
        return flags_cache[side,lo,hi]
    def silent(side, pc, other, known):
        chain = set()
        while pc not in region_at[side]:
            require(pc not in chain, 'CFG unmatched silent cycle')
            i = by[side][pc]
            is_reload = (i.mnemonic == 'mov' and i.operands[0].type == X86_OP_REG and i.operands[0].size == 4
                         and i.operands[1].type == X86_OP_IMM)
            if is_reload:
                if i.bytes == by[1-side][other].bytes:
                    break  # execute matching materializations synchronously
                # Relocated addresses must be paired and independently audited.
                if side == 0 and any(i.address<=k<i.address+i.size for k in object_sites):
                    break
                if side == 1 and i.address+i.imm_offset in oracle_addresses:
                    break
                r = (pc,pc+i.size,other,other) if side == 0 else (other,other,pc,pc+i.size)
                proof = prove_region(*decoded,r,{r[0]:r[2],r[1]:r[3]},lives,returns,[{},{}],constants)
                stutters.append(dict(side=side,offset=hex(pc),proof=proof))
                dest = pc+i.size
            elif i.mnemonic == 'jmp':
                dest = i.operands[0].imm
                stutters.append(dict(side=side,offset=hex(pc),jump_to=hex(dest)))
            elif i.mnemonic in ('cmp','test') and known:
                j = by[side].get(pc+i.size)
                key = flag_key(side,pc,pc+i.size)
                inverse = {'je':'jne','jne':'je'}
                if not (j and j.mnemonic in (known[2],inverse.get(known[2])) and key == known[side]):
                    break
                truth = known[3] if j.mnemonic == known[2] else not known[3]
                dest = j.operands[0].imm if truth else j.address+j.size
                if 'eflags' in lives[side][dest] or 'eflags' in lives[1-side][other]:
                    break  # synchronize the comparison instead of discarding flags
                # Since the paired guard was observed, only literal-register
                # loads and direct jumps have executed. No ordinary memory
                # write/call or unknown-register assignment can invalidate its
                # identical symbolic operands. This extra stable read therefore
                # has the known outcome; it is not a sampled branch condition.
                stutters.append(dict(side=side,offset=hex(pc),repeated_guard=repr(key),
                                     conditional=j.mnemonic,truth=truth,prior_conditional=known[2],
                                     prior_truth=known[3],jump_to=hex(dest)))
                covered[side].add(j.address)
            else:
                break
            before = (pc,other) if side == 0 else (other,pc)
            after = (dest,other) if side == 0 else (other,dest)
            # MOV's stronger symbolic postcondition was checked above; this
            # also checks precision loss through jump/repeated-guard joins.
            if not is_reload:
                constant_join(before,after,lives,constants)
            chain.add(pc)
            covered[side].add(pc)
            pc = dest
        return pc
    while pending:
        x,y,last_flags,known = pending.pop()
        incoming_pair = (hex(x),hex(y))
        # Close epsilon transitions in a fixed order, with local relational
        # proofs for each MOV and a bounded, acyclic closure on each side.
        x = silent(0,x,y,known)
        y = silent(1,y,x,known)
        visit = (x,y,last_flags,known)
        if visit in seen:
            continue
        seen.add(visit)
        ix,iy = region_at[0].get(x),region_at[1].get(y)
        if ix is not None or iy is not None:
            require(ix == iy and ix is not None, f'CFG region correspondence +{x:x}/+{y:x}')
            r = regions[ix]
            proof = prove_region(*decoded,r,{r[0]:r[2],r[1]:r[3]},lives,returns,[{},{}],constants)
            checks.append(proof)
            for side in (0,1):
                covered[side].update(i.address for i in bodies[side] if r[2*side]<=i.address<r[2*side+1])
            require(not any(r[0]<=k<r[1] for k in object_sites), 'CFG region relocation needs a separate theorem')
            keys = tuple(flag_key(s,r[2*s],r[2*s+1]) for s in (0,1))
            pending.append((r[1],r[3],keys,None))
            continue
        i,j = by[0][x],by[1][y]
        require(i.mnemonic == j.mnemonic, f'CFG opcode +{x:x}/+{y:x}: {i.mnemonic}/{j.mnemonic}')
        covered[0].add(x); covered[1].add(y)
        ca,cb = constants[0][x],constants[1][y]
        reads = {alias(i.reg_name(r))[0] for r in i.regs_access()[0]}
        if i.mnemonic == 'call': reads.add('ecx')
        if i.mnemonic.startswith('ret'): reads.update(('ebx','esi','edi','ebp','esp')+(('eax',) if returns else ()))
        if i.mnemonic == 'xor' and len(i.operands)==2 and i.operands[0].type==i.operands[1].type==X86_OP_REG and i.operands[0].reg==i.operands[1].reg:
            reads.discard(alias(i.reg_name(i.operands[0].reg))[0])
        require(not any(r in ca and r in cb and ca[r] != cb[r] for r in reads), f'CFG unequal constant read +{x:x}/+{y:x}')
        if i.mnemonic.startswith('j'):
            for truth,xx,yy in ((True,i.operands[0].imm,j.operands[0].imm),(False,x+i.size,y+j.size)):
                constant_join((x,y),(xx,yy),lives,constants)
                fact = (*last_flags,i.mnemonic,truth) if last_flags and i.mnemonic in ('je','jne') else None
                pending.append((xx,yy,last_flags,fact))
        else:
            left,right = bytearray(i.bytes),bytearray(j.bytes)
            require(len(left)==len(right), f'CFG instruction extent +{x:x}/+{y:x}')
            for off,reloc in object_sites.items():
                if x <= off < x+i.size:
                    size = cmpfunc.relocation_size(reloc.typ)
                    require(off+size<=x+i.size,'CFG split relocation')
                    other = y+off-x
                    require(off not in sites or sites[off]==other,'CFG inconsistent relocation pairing')
                    sites[off]=other
                    left[off-x:off-x+size]=right[off-x:off-x+size]=b'\0'*size
            require(left==right, f'CFG unproved instruction +{x:x}/+{y:x}: {i.mnemonic} {i.op_str} / {j.op_str}; incoming={incoming_pair}; guard={known}')
            if not i.mnemonic.startswith('ret'):
                constant_join((x,y),(x+i.size,y+j.size),lives,constants,full_register_writes(i))
                literal_move = (i.mnemonic == 'mov' and i.operands[0].type == X86_OP_REG
                                and i.operands[1].type == X86_OP_IMM and i.operands[0].size == 4
                                and not any(x<=off<x+i.size for off in object_sites))
                keys = tuple(flag_key(s,p,p+k.size) for s,p,k in ((0,x,i),(1,y,j))) if i.mnemonic in ('cmp','test') else last_flags if literal_move else None
                pending.append((x+i.size,y+j.size,keys,known if literal_move else None))
    require(all(covered[s] == set(by[s]) for s in (0,1)), 'CFG incomplete instruction coverage')
    require(set(sites)==set(object_sites) and len(set(sites.values()))==len(sites), 'CFG incomplete/nonbijective relocation pairing')
    object_mask = {k for off in object_sites for k in range(off,off+4)}
    oracle_mask = {k for off in sites.values() for k in range(off,off+4)}
    differences = [[hex(k),x,y] for k,(x,y) in enumerate(zip(function.data,raw)) if x!=y and k not in object_mask.intersection(oracle_mask)]
    return sites, dict(method='inductive product CFG with finite MOV-constant/JMP stutters and stable repeated-guard elimination',
        paired_boundaries=len(seen), regions=checks, stutters_sha256=hashed(stutters),
        stuttered_direct_jumps=sum('jump_to' in s and 'repeated_guard' not in s for s in stutters),
        constant_reload_proofs=[s for s in stutters if 'proof' in s],
        repeated_guard_proofs=[s for s in stutters if 'repeated_guard' in s],
        covered_instructions=[len(x) for x in covered],constant_states_sha256=hashed(constants),
        object_padding_bytes=len(function.data)-(bodies[0][-1].address+bodies[0][-1].size),
        oracle_padding_bytes=len(raw)-(bodies[1][-1].address+bodies[1][-1].size),
        instruction_pairs_sha256=hashed(sorted(seen,key=repr)),
        nonrelocation_difference_ranges=difference_ranges(differences),
        difference_values_sha256=hashed(differences),differing_bytes=len(differences))


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
    imports = cmpfunc.import_map(pe)
    for name in sorted(missing):
        if name.startswith('__imp_') and cmpfunc.normalize_import(name) in imports:
            # PE import-by-name identity is independent of the caller bytes.
            global_['global:'+name] = imports[cmpfunc.normalize_import(name)]
    if missing:
        for coff, functions in library_functions(library, out):
            for f in functions:
                if f.name not in missing:
                    continue
                hits = cmpfunc.candidates_for(f, td)
                if f.relocs:
                    # Same narrow identity method as SB1's pinned certificate:
                    # a unique complete library signature identifies these CRT
                    # names. This is NOT a strict byte/body-equivalence claim.
                    if f.name not in ('__findfirst','__findnext'):
                        continue
                    require(len(f.data) == {'__findfirst':304,'__findnext':288}[f.name],
                            'file-enumeration CRT signature extent changed')
                    require(len(hits) == 1, 'file-enumeration CRT signature is not unique')
                    va = ib+tr+hits[0]
                    require(va in starts and f.name not in crt, 'ambiguous CRT signature entry')
                    global_['global:'+f.name] = va
                    crt[f.name] = dict(va=hex(va), size=len(f.data),
                        library_payload_sha256=hashed(dict(bytes=f.data.hex(),relocations=[
                            [r.offset-f.start,r.typ,coff.sym_by_index[r.symbol_index].name] for r in f.relocs])),
                        oracle_body_sha256=hashlib.sha256(td[hits[0]:hits[0]+len(f.data)]).hexdigest(),
                        proof='unique complete relocation-masked pinned-LIBC signature; target identity only, NOT STRICT or CRT behavioral equivalence')
                elif len(hits) == 1 and f.data == td[hits[0]:hits[0]+len(f.data)]:
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
    certified_entries = {}
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
        if entry.get('proof_method') == 'cfg-constants':
            sites,checks = cfg_structure(f,raw,regions,entry['returns_value'])
        else:
            require('proof_method' not in entry, 'unknown proof method')
            sites, checks = structure(f, raw, regions, entry['returns_value'], coff, entry.get('pure_callee'),
                entry.get('constant_invariants', False),
                [tuple(int(str(x),0) for x in pair) for pair in entry.get('region_relocations',[])])
        if pure:
            checks['pure_callee_identity'] = pure
        shared, globals_, fd, dd, imports, explicit, starts = state
        rr = []
        for reloc in f.relocs:
            offset = reloc.offset-f.start
            # The original call site may move when a region changes length.
            # Resolve the actual original field, preserving the COFF addend.
            one = replace(f, relocs=[reloc])
            site_explicit = explicit
            if coff.sym_by_index[reloc.symbol_index].name == f.name and sites[offset] != offset:
                # resolve_sites' self-call shortcut assumes an unshifted body.
                # A recursive edge instead binds to this proof's checked entry,
                # not to the temporary site-displacement adjustment below.
                self_va = pe.OPTIONAL_HEADER.ImageBase+rva
                require(globals_.get('global:'+f.name) in (None,self_va), 'recursive entry binding conflict')
                site_explicit = dict(explicit, **{f.name:self_va})
            status, rows = cmpfunc.resolve_sites(one, rva-tx.VirtualAddress+sites[offset]-offset,
                tx.get_data(), tx.VirtualAddress, pe.OPTIONAL_HEADER.ImageBase, pe, coff, Path(unit['object']),
                shared, fd, dd, imports, site_explicit, globals_, starts)
            require(status == 'STRICT' and all(r['verified'] for r in rows), f'{name}: unresolved relocation {rows}')
            rr.extend(dict(r, oracle_offset=sites[offset]) for r in rows)
        checks['verified_relocations'] = len(rr)
        checks['relocations_sha256'] = hashed(rr)
        dependencies = {coff.sym_by_index[r.symbol_index].name:certified_entries[coff.sym_by_index[r.symbol_index].name]
                        for r in f.relocs if coff.sym_by_index[r.symbol_index].name in certified_entries}
        if dependencies:
            checks['previously_proved_equivalent_callee_entries'] = dependencies
        current = dict(schema=1, status='EQUIVALENT', target=target.name, declaration=entry,
                       scope='static-local-equivalence', assumptions=ASSUMPTIONS,
                       payload_sha256=hashed(dict(bytes=f.data.hex(), relocations=[
                           [r.offset-f.start,r.typ,coff.sym_by_index[r.symbol_index].name] for r in f.relocs])),
                       oracle_sha256=target.sha256, oracle_function_sha256=hashlib.sha256(raw).hexdigest(),
                       checker_sha256=digest(__file__), comparator_sha256=digest(ROOT/'tools/cmpfunc.py'),
                       comparator_wrapper_sha256=digest(ROOT/'tools/cmpfunc_target.py'),
                       capstone_version=cs.__version__, crt_library_sha256=digest(library),
                       inventory_sha256={p:digest(target.oracle_dir/p) for p in ('functions.csv','tu_map.csv')},
                       crt_target_identity={k:v for k,v in crt.items() if k not in ('__findfirst','__findnext')
                           or any(coff.sym_by_index[r.symbol_index].name == k for r in f.relocs)}, checks=checks)
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
        key,va = 'global:'+f.name,pe.OPTIONAL_HEADER.ImageBase+rva
        if key not in globals_:
            # Identity of this already-proved target-local entry is available
            # to later certificates. It is NOT a STRICT byte-body candidate:
            # shared/function_defs and comparator status remain unchanged.
            globals_[key] = va
            certified_entries[f.name] = dict(rva=hex(rva),certificate=entry['certificate'],
                                             payload_sha256=current['payload_sha256'])
        else:
            require(globals_[key] == va,'certified callee entry conflicts with run-wide identity')
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
