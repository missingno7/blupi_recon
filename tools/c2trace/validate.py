"""Static signature/algebra checks and unforced live hash/comparator return validation.

  .pyenv/python.exe tools/c2trace/validate.py --compiler rtm

Default TU: games/sb2/src/decblock.cpp; observes DrawInfo and GetBlupiInfo.
Compiles through msvc.py, replays with a 180-second bound, compares every emitted
function to the normal capture object, and writes build/c2trace[-rtm]/validation.json.
No oracle match claim is made. --static-only omits capture/replay.
"""
import argparse
import json
from pathlib import Path
import shutil

import capstone
import pefile
import c2trace
import hashmodel
import keyforce as k
import profiles


def code(profile, start, end):
    pe = pefile.PE(str(profile.c2))
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    out = []
    for i in md.disasm(pe.get_data(start - 0x400000, end - start), start):
        ops = []
        for o in i.operands:
            if o.type == capstone.x86.X86_OP_REG:
                ops.append(('reg', i.reg_name(o.reg)))
            elif o.type == capstone.x86.X86_OP_IMM:
                value = 'target' if i.group(capstone.CS_GRP_JUMP) or i.group(capstone.CS_GRP_CALL) else o.imm
                ops.append(('imm', value))
            elif o.type == capstone.x86.X86_OP_MEM:
                v = o.mem
                ops.append(('mem', i.reg_name(v.base), i.reg_name(v.index), v.scale,
                            'address' if v.disp > 0x400000 else v.disp, o.size))
        out.append((i.mnemonic, ops))
    return out


def static():
    sp3, rtm = profiles.PROFILES['sp3'], profiles.PROFILES['rtm']
    digests = {p.name: p.verify() for p in (sp3, rtm)}
    result = {}
    for name, start, end in [('key', 'key', 'hash'), ('hash', 'hash', 'hash_end')]:
        a = code(sp3, getattr(sp3, start), getattr(sp3, end))
        b = code(rtm, getattr(rtm, start), getattr(rtm, end))
        result[name] = dict(sp3_instructions=len(a), rtm_instructions=len(b), normalized_equal=a == b)
        if a != b:
            raise RuntimeError('Key/hash algebra comparison failed: ' + name)
    return dict(compiler_sha256=digests, routines=result,
                scope='Normalized compiler-analysis instructions; not strict game equality')


def live(cap, selectors):
    profile = profiles.get()
    seed = k.seed_of(cap)
    state = dict(active=False, hash_pending=None, compare_pending=None)
    records, functions = [], []

    def hit(p, c, addr):
        for kind in ['hash', 'compare']:
            pending = state[kind + '_pending']
            if pending and addr == pending['return']:
                actual = c.Eax & 65535 if kind == 'hash' else (c.Eax if c.Eax < 0x80000000 else c.Eax - 0x100000000)
                pending.update(actual=actual, agrees=actual == pending['predicted'])
                records.append(pending)
                state[kind + '_pending'] = None
                if kind == 'hash':
                    p.arm(profile.hash)
                return
        if addr == profile.function:
            s = k.function_symbol(p, seed)
            s['current_pointer_agrees'] = p.u32(profile.current) == p.u32(p.u32(profile.frame))
            s['frame_pointer_agrees'] = p.u32(profile.frame) == c.Ecx
            functions.append(s)
            state['function'] = s['name']
            state['active'] = any(x in s['name'] for x in selectors)
            for bp in [profile.hash, profile.compare]:
                if state['active']:
                    p.arm(bp)
                elif bp in p.orig:
                    p.wr(bp, p.orig.pop(bp))
            return
        if not state['active']:
            return
        ret = p.u32(c.Esp)
        if addr == profile.hash:
            expected = hashmodel.hash16(k.tree(p, c.Ecx, seed))
            kind = 'hash'
            # Observe outermost actual return; recursive calls run normally.
            p.wr(profile.hash, p.orig.pop(profile.hash))
        elif addr == profile.compare:
            expected = k.sign(p.u32(c.Ecx + 12), p.u32(c.Edx + 12))
            kind = 'compare'
        else:
            raise ValueError(hex(addr))
        state[kind + '_pending'] = dict(kind=kind, function=state['function'], predicted=expected, **{'return': ret})
        p.arm(ret, oneshot=True)

    meta = json.loads((cap / 'capture.json').read_text())
    obj = k.replay(cap, [profile.function], hit)
    out = cap / 'live-validation.obj'
    shutil.copyfile(obj, out)
    base = {f.name: f.data for f in k.cmpfunc.Coff(Path(meta['object'])).functions()}
    new = {f.name: f.data for f in k.cmpfunc.Coff(out).functions()}
    result = dict(unforced_function_bytes_identical=base == new, emitted_functions=len(new), records=records,
        functions=functions, hash_observations=sum(x['kind'] == 'hash' for x in records),
        comparator_observations=sum(x['kind'] == 'compare' for x in records),
        errors=[x for x in records if not x['agrees']])
    if not (base == new and not result['errors'] and result['hash_observations'] and result['comparator_observations']):
        raise RuntimeError('Live return/byte validation failed: ' + str({x:y for x,y in result.items() if x not in ('records','functions')}))
    if any(not s['current_pointer_agrees'] or not s['frame_pointer_agrees'] for s in functions):
        raise RuntimeError('Function frame/current symbol disagreement')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    profiles.add_arguments(parser)
    parser.add_argument('--tu', default='games/sb2/src/decblock.cpp')
    parser.add_argument('--function', action='append', default=[])
    parser.add_argument('--capture', help='reuse existing keyforce capture instead of compiling')
    parser.add_argument('--static-only', action='store_true')
    parser.add_argument('--json', type=Path)
    args = parser.parse_args()
    profile = k.configure(args.compiler, args.target)
    result = dict(compiler=profile.name, static=static())
    if not args.static_only:
        if args.capture:
            cap = profile.work / 'keyforce' / args.capture
        else:
            c2trace.build_stub()
            # A new, isolated capture on every invocation; no overwrite of user captures.
            import uuid
            folder = profile.work / 'keyforce'
            folder.mkdir(parents=True, exist_ok=True)
            cap = folder / ('validate-' + uuid.uuid4().hex[:12])
            k.capture(cap.name, args.tu)
        result['live'] = live(cap, args.function or ['DrawInfo', 'GetBlupiInfo'])
    dest = args.json or profile.work / 'validation.json'
    dest.parent.mkdir(parents=True, exist_ok=True)
    k.write(dest, result)
    print('static:', result['static']['routines'])
    if 'live' in result:
        print('live:', {x:y for x,y in result['live'].items() if x not in ('records','functions','errors')},
              'errors:', len(result['live']['errors']))
    print(dest)


if __name__ == '__main__':
    main()
