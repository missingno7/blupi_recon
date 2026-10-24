"""Declaration-context windows of every function of one TU, from ONE capture and ONE c2 replay.

  tools/c2trace/ctxwindows.py games/sb2/src/decblupi.cpp --include games/sb2/src [--define D ...] [--sdk dx5sdk]
                              [--json OUT] [--function SUBSTRING] [--compiler sp3|rtm] [--target GAME/EDITION/LANGUAGE]
SP3 is the default; a target selects its compiler and edition defines. Both
direct-swap and merge-sort callers are surveyed. --legacy-model replays the old
direct-swap-only, recursive-child-hash model for measured comparisons.

For each function with context-sensitive comparisons (msvc5kb BLUPI-CTX-03, docs/SYMBOL_ORDER.md) it prints the
intersection of the current-decision windows: the uniform shift N of real front-end symbol IDs (declarations added
or removed before the function's symbols) over which every one of its comparator decisions stays as it is now.
Same model and limits as ctxpredict.py, which it reuses: a prediction of comparator decisions, not of strict bytes;
fixed expression topology, costs and allocation history; a capture is only valid for the exact source it was made
from. Diagnostic only.
"""
import argparse, json, os, subprocess, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
import ctxpredict  # noqa: E402  (decisions(), window bounds)
import c2trace  # noqa: E402
import profiles


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    profiles.add_arguments(p)
    p.add_argument('--legacy-model',action='store_true',help='old SP3 model for before/after comparison')
    p.add_argument('tu')
    p.add_argument('--include', action='append', default=[])
    p.add_argument('--define', action='append', default=[])
    p.add_argument('--sdk', default='dx5sdk')
    p.add_argument('--function', default='', help='only functions whose decorated name contains this')
    p.add_argument('--radius', type=int, default=1024)
    p.add_argument('--json', type=Path)
    a = p.parse_args()
    profile = c2trace.configure(a.compiler,a.target)
    ctxpredict.LEGACY_MODEL = a.legacy_model
    src = Path(a.tu).resolve()
    name = 'ctxwin-' + src.stem
    c2trace._fresh_il()
    src, include_root = c2trace.source_context(src, profile.work / 'predict')
    cmd = [sys.executable, str(ROOT / 'tools/msvc.py'), str(src), '--tree', profile.name, '--out', str(profile.work / 'predict' / (name + '.obj')),
           '--log-dir', str(profile.work / 'predict/logs'), '--include', str(src.parent),
           '--include', str(include_root),
           '--include', str(Path('C:/tools') / a.sdk / 'INCLUDE')]
    for i in a.include:
        cmd += ['--include', str(Path(i).resolve())]
    for f in ['/W3', '/GX', '/O2', '/ML', '/Gd', '/Gy', '/B2' + str(c2trace.STUB)]:
        cmd += ['--flag', f]
    for d in ['WIN32', 'NDEBUG', '_WINDOWS', 'nullptr=0'] + (profiles.target().defines if profiles.target() else []) + a.define:
        cmd += ['--define', d]
    subprocess.run(cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    c2trace._keep(name)
    env = os.environ.copy()
    env['SYMNUM_FUNCTION'] = a.function
    subprocess.run([sys.executable, str(HERE / 'trace_order.py'), name]+profiles.cli_args(), check=True, env=env, stdout=subprocess.PIPE)
    data = json.loads((profile.work / 'order' / (name + '-order.json')).read_text())
    functions = json.loads((profile.work / 'order' / (name + '-functions.json')).read_text())
    result = {}
    for fn in sorted(functions):
        rows = ctxpredict.decisions([x for x in data if x['function'] == fn], fn, 0, a.radius)
        if not rows:
            continue
        lo = max(r['window']['lo'] for r in rows)
        hi = min(r['window']['hi'] for r in rows)
        result[fn] = dict(frontend_id=functions[fn].get('frontend_id'), decisions=len(rows), lo=lo, hi=hi,
                          windows=[(r['window']['lo'], r['window']['hi']) for r in rows])
    if a.json:
        a.json.write_text(json.dumps(result, indent=1) + '\n')
    for fn, r in sorted(result.items(), key=lambda kv: kv[1]['frontend_id'] or 0):
        print(f"{r['frontend_id']!s:>6} [{r['lo']:>5},{r['hi']:>5}] {r['decisions']:>3}  {fn}")
    print(f'{len(result)} context-sensitive functions of {len(functions)} traced; window = shift range keeping '
          'every current decision (not a strict-match claim)')


if __name__ == '__main__':
    main()
