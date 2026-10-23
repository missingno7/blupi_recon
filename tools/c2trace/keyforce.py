"""Pinned VC5 SP3/RTM expression comparator diagnostic; forced objects are NEVER source.

Run sequentially with .pyenv/python.exe. capture uses a private copy of the
existing c2stub and tools/fnprobe.py -> tools/msvc.py. trace/force use c2dbg.
Diff counts use fnprobe's relocation-masked comparison against the selected target;
zero here is NOT STRICT. JSON includes aligned residue regions.
"""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
sys.path[:0] = [str(ROOT / 'tools'), str(HERE)]
import c2dbg
import cmpfunc
import fnprobe
import pefile
import residues
import c2trace
import profiles

# Diagnostic extents retained from the SP3 tool and scratch/w-rtmtrace/REPORT.md.
# These include unresolved bodies absent from the accepted manifest. They are
# relocation-masked analysis coordinates, never acceptance bindings.
DIAGNOSTICS = {
    'sb1/blupi/us': {'TreatEventBase': (0x1d230, 6616), 'DrawButtons': (0x16f70, 7419),
        'ChangePhase': (0x19310, 6919), '?Build@CDecor@@': (0x4350, 3866)},
    'sb2/blupi/en': {'GetBlupiInfo': (0x5960,80), 'IsBridge': (0x147c0,208),
        'IsDoor': (0x14890,208), 'IsTeleporte': (0x14960,128), 'IsDeepWater': (0x14b00,80),
        'IsOutWater': (0x14b50,80), 'SetMoveTraj': (0x14d80,80),
        '?BltFast@CPixmap@@IAEJHHUtagPOINT@@UtagRECT@@H@Z': (0x28e10,240),
        'MouseBackDraw': (0x29f30,272), 'DrawButtons': (0x1d7d0,7648),
        '?ChangePhase@CEvent@@': (0x1fc50,6912)},
}

COMPARE = profiles.get().compare
FUNCTION = profiles.get().function
CURRENT = profiles.get().current
FRAME = profiles.get().frame
SHA = profiles.get().sha256


def configure(compiler=None, target=None):
    global COMPARE, FUNCTION, CURRENT, FRAME, SHA
    profile = c2trace.configure(compiler, target)
    COMPARE, FUNCTION, CURRENT, FRAME, SHA = (profile.compare, profile.function,
        profile.current, profile.frame, profile.sha256)
    fnprobe.TARGET = profiles.target()
    fnprobe.EXE = fnprobe.TARGET.exe if fnprobe.TARGET else ROOT / 'assets/Speedy Blupi/BLUPI.EXE'
    return profile


def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2) + '\n', encoding='utf-8')


def verify_compiler():
    return profiles.get().verify()


def capture(name, unit):
    verify_compiler()
    profiles.check_capture(c2trace.STUB.parent, check_target=False)
    cap = c2trace.capture_dir(name, 'keyforce')
    if cap.exists():
        raise FileExistsError(cap)
    cap.mkdir(parents=True)
    stub = cap / 'c2stub.exe'
    shutil.copyfile(c2trace.STUB, stub)
    (cap / 'il').mkdir()
    canonical, _, source = unit.partition('=')
    canonical = canonical.replace('\\', '/')
    source = (ROOT / (source or canonical)).resolve()
    obj = fnprobe.compile_unit(canonical, source, cap, ['/B2' + str(stub)], tree=profiles.get().name)
    for f in (cap / 'il').iterdir():
        shutil.copyfile(f, cap / f.name)
    meta = dict(unit=canonical, source=str(source), source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),
                object=str(obj), c2_sha256=verify_compiler(), diagnostic_only=True,
                headers={str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                         for p in source.parent.glob('*.h')})
    write(cap / 'capture.json', meta)
    profiles.stamp(cap)
    print(cap, flush=True)


def symbol(p, ptr, seed):
    if not ptr:
        return None
    ident = p.u32(ptr + 40)
    text = p.rd(p.u32(ptr + 24), 200).split(b'\0')[0].decode('ascii', 'replace')
    return dict(id=ident, name=text, domain='back_end_symbol' if ident >= seed else 'front_end_symbol')


def function_symbol(p, seed):
    return symbol(p, p.u32(p.u32(FRAME)), seed)


def tree(p, ptr, seed, depth=0, seen=()):
    if not ptr:
        return None
    if depth > 22 or ptr in seen:
        return dict(incomplete=True, ptr=hex(ptr))
    raw = struct.unpack('<20I', p.rd(ptr, 80))
    kind = raw[2] & 255
    t = dict(ptr=hex(ptr), kind=kind, op=raw[1], key=raw[3], type=raw[2] >> 16)
    descend = lambda q: tree(p, q, seed, depth + 1, seen + (ptr,))
    if kind in (1, 2, 3):
        desc = raw[5]
        t.update(desc=hex(desc), desc_kind=p.rd(desc+4, 1)[0], desc_number=p.u32(desc+28))
        t['expr'] = descend(p.u32(desc+20))
        if not t['expr'] and t['desc_kind'] == 3:
            t['domain'] = 'backend_temp_descriptor'
        else:
            t['sym'] = symbol(p, p.u32(p.u32(desc+8)), seed)
    elif kind == 4:
        t['sym'] = symbol(p, raw[5], seed)
    elif kind in (5, 6):
        t.update(offset=raw[8], base=descend(raw[9]), index=descend(raw[10]))
    elif kind in (7, 10):
        t['value'] = list(raw[6:8])
    elif kind == 8:
        t['value_bytes'] = p.rd(raw[5], 12).hex()
    else:
        children, child = [], raw[6]
        while child and len(children) < 80:
            children.append(descend(child))
            child = p.u32(child)
        t['inputs'] = children
        if child:
            t['incomplete'] = True
    return t


def dependencies(t):
    if not t:
        return []
    if t.get('incomplete'):
        raise ValueError('incomplete snapshot')
    if t['kind'] in (1, 2, 3):
        if t.get('expr'):
            return dependencies(t['expr'])
        if t['desc_kind'] == 3:
            return [dict(id=t['desc_number'], name='temp%d' % t['desc_number'], domain='backend_temp_descriptor')]
        if t.get('sym'):
            return [t['sym']]
        raise ValueError('missing symbol')
    if t['kind'] == 4:
        return [t['sym']]
    if t['kind'] in (5, 6):
        return dependencies(t.get('base'))  # index not hashed by this routine
    return [s for child in t.get('inputs', []) for s in dependencies(child)]


def fold(x):
    x &= 0xffffffff
    return ((x & 65535) ^ (x >> 16)) & 65535


def recipe(t):
    if not t['key']:
        return dict(high=0, constant=0, terms=[], cached_zero=True)
    by_id = {}
    for s in dependencies(t):
        key = (s['domain'], s['id'])
        if key in by_id:
            del by_id[key]
        else:
            by_id[key] = s
    terms = sorted(by_id.values(), key=lambda s: (s['domain'], s['id']))
    low = t['key'] & 65535
    for s in terms:
        low ^= (s['id'] & 65535) if s['domain'] == 'backend_temp_descriptor' else fold(s['id'])
    return dict(high=t['key'] & 0xffff0000, constant=low, terms=terms)


def evaluate(r, front=0, back=0, temp=0):
    low = r['constant']
    for s in r['terms']:
        d = {'front_end_symbol': front, 'back_end_symbol': back, 'backend_temp_descriptor': temp}[s['domain']]
        n = s['id'] + d
        low ^= (n & 65535) if s['domain'] == 'backend_temp_descriptor' else fold(n)
    return r['high'] | low


def sign(a, b):
    return (b > a) - (b < a)


def describe(t):
    if not t:
        return '-'
    if t.get('incomplete'):
        return 'INCOMPLETE'
    k = t['kind']
    if k in (1, 2, 3):
        if t.get('expr'):
            return 't%d=%s' % (t['desc_number'], describe(t['expr']))
        if t.get('desc_kind') == 3:
            return 'temp%d' % t['desc_number']
        s = t.get('sym') or {}
        return '%s#%s' % (s.get('name', '?'), s.get('id', '?'))
    if k == 4:
        s = t.get('sym') or {}
        return '&%s#%s' % (s.get('name', '?'), s.get('id', '?'))
    if k in (5, 6):
        return '%s(%s+%#x)' % ('mem' if k == 6 else 'addr', describe(t.get('base')), t['offset'])
    if k in (7, 10):
        return hex(t['value'][0])
    if k == 8:
        return 'float:' + t['value_bytes']
    return '%x(%s)' % (t['op'], ','.join(describe(x) for x in t.get('inputs', [])))


def replay(cap, breakpoints, hit):
    """Pinned, bounded replay; c2dbg synchronously reaps the child on every exit."""
    verify_compiler()
    return c2dbg.run(cap, breakpoints, hit)


def seed_of(cap):
    gl = next(cap.glob('*gl')).read_bytes()
    if gl[:3] != b'\x11\x86\x03':
        raise ValueError('unsupported IL header')
    return int.from_bytes(gl[7:11], 'little')


def trace(cap, selectors):
    seed = seed_of(cap)
    events, funcs, counters = [], [], {}
    active = [False]

    def hit(p, c, addr):
        if addr == FUNCTION:
            s = function_symbol(p, seed)
            s['next_backend_id'] = p.u32(profiles.get().next_symbol)
            s['current_pointer_agrees'] = p.u32(CURRENT) == p.u32(p.u32(FRAME))
            s['frame_pointer_agrees'] = p.u32(FRAME) == c.Ecx
            active[0] = any(x in s['name'] for x in selectors)
            funcs.append(s)
            if active[0]:
                p.arm(COMPARE)
            elif COMPARE in p.orig:
                p.wr(COMPARE, p.orig.pop(COMPARE))
            return
        if not active[0]:
            return
        s = function_symbol(p, seed)
        if not any(x in s['name'] for x in selectors):
            raise ValueError('current function pointer disagrees with function gate')
        left, right = tree(p, c.Ecx, seed), tree(p, c.Edx, seed)
        idx = counters.get(s['name'], 0)
        counters[s['name']] = idx + 1
        row = dict(event=len(events), index=idx, function=s, caller=hex(p.u32(c.Esp)),
                   left=left, right=right, result=sign(left['key'], right['key']))
        for label, t in [('a', left), ('b', right)]:
            row[label + '_text'] = describe(t)
            try:
                row[label + '_recipe'] = recipe(t)
            except ValueError as e:
                row[label + '_recipe_error'] = str(e)
        events.append(row)

    obj = replay(cap, [FUNCTION], hit)
    shutil.copyfile(obj, cap / 'baseline.obj')
    meta = json.loads((cap / 'capture.json').read_text())
    original = {f.name:f.data for f in cmpfunc.Coff(Path(meta['object'])).functions()}
    reproduced = {f.name:f.data for f in cmpfunc.Coff(obj).functions()}
    if original != reproduced:
        raise RuntimeError('unforced replay did not reproduce capture function bytes')
    write(cap / 'keys.json', dict(seed=seed, c2_sha256=SHA, functions=funcs, events=events))
    for name, count in counters.items():
        report = compare(cap / 'baseline.obj', name)
        write(cap / (name.split('@')[0].lstrip('?') + '-baseline.json'), report)
        print(name, count, 'comparisons;', report['fixed_diff_count'], 'oracle fixed-byte diffs;',
              len(report['aligned']['regions']), 'regions', flush=True)
    print('keys:', cap / 'keys.json')


def target(selector):
    selected = profiles.target() or profiles.targets.load()
    known = [v for k,v in DIAGNOSTICS.get(selected.name, {}).items() if k in selector or selector in k]
    if len(known) == 1:
        return known[0]
    # manifest() intentionally removes excluded functions; use the independent
    # inventory/base manifest plus target coordinate overrides for diagnostics.
    manifest = json.loads(selected.manifest_path.read_text())
    rows = [x.copy() for x in manifest['accepted_functions']
            if x['symbol'] == selector or selector in x['symbol']]
    if len(rows) != 1:
        raise ValueError('No unique known oracle coordinates for ' + selector)
    row = rows[0]
    rva = int(str(row['rva']), 0)
    if selected.address_map:
        rva = selected.translate(0x400000 + rva, 'code') - 0x400000
    override = selected.accepted_overrides.get(row['symbol'], {})
    return int(str(override.get('rva',rva)),0), override.get('size',row['size'])


def compare(obj, selector):
    coff = cmpfunc.Coff(obj)
    found = [f for f in coff.functions() if selector == f.name or selector in f.name]
    if len(found) != 1:
        raise ValueError('ambiguous function: %r' % [f.name for f in found])
    f = found[0]
    rva, size = target(selector)
    pe = pefile.PE(str(fnprobe.EXE), fast_load=True)
    mask = set()
    for r in f.relocs:
        mask.update(range(r.offset-f.start, r.offset-f.start+4))
    oracle = pe.get_data(rva, len(f.data))
    offsets = [i for i, (a, b) in enumerate(zip(f.data, oracle)) if a != b and i not in mask]
    aligned = residues.analyse(f, coff, pe.get_data(rva, max(size, len(f.data))+1024),
                               pe.OPTIONAL_HEADER.ImageBase+rva, size)
    return dict(function=f.name, object=str(obj), oracle_rva=hex(rva), size=len(f.data), oracle_size=size,
                fixed_diff_count=len(offsets), fixed_diff_offsets=offsets, aligned=aligned,
                comparison='fnprobe relocation-masked fixed bytes against oracle; NOT STRICT')


def force(cap, selector, indices, forced=None):
    data = json.loads((cap / 'keys.json').read_text())
    rows = [e for e in data['events'] if selector in e['function']['name']]
    chosen = {i: next(e for e in rows if e['index'] == i) for i in indices}
    if any(e['result'] == 0 for e in chosen.values()) and forced is None:
        raise ValueError('equal keys: specify --result')
    state = dict(index=-1, active=False, pending=None, value=0, forced=[])

    def hit(p, c, addr):
        if addr == state['pending']:
            c.Eax = state['value'] & 0xffffffff
            state['pending'] = None
            state['forced'].append(state['index'])
            return
        if addr == FUNCTION:
            s = function_symbol(p, data['seed'])
            state['active'] = selector in s['name']
            if state['active']:
                state['index'] = -1
                p.arm(COMPARE)
            elif COMPARE in p.orig:
                p.wr(COMPARE, p.orig.pop(COMPARE))
            return
        if not state['active']:
            return
        state['index'] += 1
        if state['index'] not in chosen:
            return
        row = chosen[state['index']]
        # Earlier forces can change the comparison stream. Never silently use
        # an unrelated subsequent event when a diagnostic combination diverges.
        if (p.u32(c.Ecx+12), p.u32(c.Edx+12), hex(p.u32(c.Esp))) != (
                row['left']['key'], row['right']['key'], row['caller']):
            raise ValueError('comparison stream diverged at %d' % state['index'])
        state['pending'] = p.u32(c.Esp)
        state['value'] = -row['result'] if forced is None else forced
        p.arm(state['pending'], oneshot=True)

    obj = replay(cap, [FUNCTION], hit)
    if sorted(state['forced']) != sorted(indices):
        raise RuntimeError('expected forces %r, observed %r' % (indices, state['forced']))
    stem = selector.split('@')[0].lstrip('?') + '-force-' + '-'.join(map(str, indices))
    if forced is not None:
        stem += '-r' + str(forced)
    saved = cap / (stem + '.obj')
    shutil.copyfile(obj, saved)
    baseline = compare(cap / 'baseline.obj', selector)
    report = compare(saved, selector)
    base_coff, new_coff = cmpfunc.Coff(cap / 'baseline.obj'), cmpfunc.Coff(saved)
    old = {f.name: f.data for f in base_coff.functions()}
    new = {f.name: f.data for f in new_coff.functions()}
    # Compare only fixed object bytes here; the oracle comparison above is
    # always the primary result.
    before = old[report['function']]
    after = new[report['function']]
    changed = [i for i in range(max(len(before), len(after)))
               if before[i:i+1] != after[i:i+1]]
    report.update(diagnostic_only=True, indices=indices,
                  forced_result=forced if forced is not None else 'reverse each unforced result',
                  baseline_fixed_diff_count=baseline['fixed_diff_count'],
                  fixed_diff_delta=report['fixed_diff_count']-baseline['fixed_diff_count'],
                  removed_diff_offsets=sorted(set(baseline['fixed_diff_offsets'])-set(report['fixed_diff_offsets'])),
                  added_diff_offsets=sorted(set(report['fixed_diff_offsets'])-set(baseline['fixed_diff_offsets'])),
                  object_changed_offsets=changed,
                  changed_functions=[n for n in old if old[n] != new.get(n)],
                  selected_comparisons=[dict(index=e['index'], caller=e['caller'],
                      a=e['a_text'], b=e['b_text'], a_recipe=e.get('a_recipe'), b_recipe=e.get('b_recipe'))
                      for e in chosen.values()])
    write(cap / (stem + '.json'), report)
    print(stem, 'oracle', baseline['fixed_diff_count'], '->', report['fixed_diff_count'],
          'regions', len(baseline['aligned']['regions']), '->', len(report['aligned']['regions']),
          'changed', len(changed), 'removed', len(report['removed_diff_offsets']),
          'added', len(report['added_diff_offsets']), flush=True)
    return report


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.epilog = 'Common options (any position): --compiler {sp3,rtm} --target GAME/EDITION/LANGUAGE'
    sub = p.add_subparsers(dest='command', required=True)
    s = sub.add_parser('capture'); s.add_argument('name'); s.add_argument('unit')
    s = sub.add_parser('trace'); s.add_argument('name'); s.add_argument('function', nargs='+')
    s = sub.add_parser('list'); s.add_argument('name'); s.add_argument('function'); s.add_argument('--equal-cost', action='store_true'); s.add_argument('--events', help='event-index ranges A-B[,C-D] (one source region in each pass)')
    s = sub.add_parser('force'); s.add_argument('name'); s.add_argument('function'); s.add_argument('indices', nargs='+', type=int); s.add_argument('--result', type=int, choices=(-1, 0, 1))
    s = sub.add_parser('predict'); s.add_argument('name'); s.add_argument('function'); s.add_argument('index', type=int); s.add_argument('--radius', type=int, default=1024)
    # Accept profile options before or after the subcommand.
    options = argparse.ArgumentParser(add_help=False)
    profiles.add_arguments(options)
    common, rest = options.parse_known_args()
    a = p.parse_args(rest)
    configure(common.compiler, common.target)
    cap = c2trace.capture_dir(a.name, 'keyforce')
    if a.command != 'capture':
        profiles.check_capture(cap)
    if a.command == 'capture':
        capture(a.name, a.unit)
    elif a.command == 'trace':
        trace(cap, a.function)
    elif a.command == 'force':
        force(cap, a.function, a.indices, a.result)
    else:
        data = json.loads((cap / 'keys.json').read_text())
        rows = [e for e in data['events'] if a.function in e['function']['name']]
        if a.command == 'list':
            spans = [tuple(int(x) for x in r.split('-')) for r in a.events.split(',')] if a.events else None
            if spans:
                rows = [e for e in rows if any(lo <= e['index'] <= hi for lo, hi in spans)]
            shown = 0
            for e in rows:
                if a.equal_cost and e['left']['key'] >> 16 != e['right']['key'] >> 16:
                    continue
                shown += 1
                print('#%d %s %08x / %08x = %+d | %s | %s' % (e['index'], e['caller'], e['left']['key'], e['right']['key'], e['result'], e['a_text'], e['b_text']))
                for label in ('a', 'b'):
                    r = e.get(label+'_recipe')
                    print('  '+label+': '+(', '.join('%s=%d [%s]' % (s['name'],s['id'],s['domain']) for s in r['terms']) if r else 'unknown'))
            # Unequal cost halves decide a comparison before any hash bit, so no symbol-ID context can flip it.
            print('%d comparisons, %d listed%s' % (len(rows), shown, ' (equal cost halves only)' if a.equal_cost else ''))
        else:
            e = next(e for e in rows if e['index'] == a.index)
            result = dict(index=a.index, a=e['a_recipe'], b=e['b_recipe'], baseline=e['result'], domains={})
            for domain in ('both', 'front', 'back', 'temp'):
                good = []
                for n in range(-a.radius, a.radius+1):
                    args = dict(front=n if domain in ('front', 'both') else 0,
                                back=n if domain in ('back', 'both') else 0,
                                temp=n if domain == 'temp' else 0)
                    if sign(evaluate(e['a_recipe'], **args), evaluate(e['b_recipe'], **args)) == -e['result']:
                        good.append(n)
                # Contiguous finite windows; no global periodicity assertion.
                windows = []
                for n in good:
                    if windows and n == windows[-1][1]+1:
                        windows[-1][1] = n
                    else:
                        windows.append([n, n])
                result['domains'][domain] = windows
            write(cap / ('predict-%s-%d.json' % (a.function.split('@')[0].lstrip('?'), a.index)), result)
            print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
