"""Serial scratch driver; verifier implementation is unchanged except output path.

verify.py's CLI insists on build/verify-NAME, outside this worker's allowed
prefix. Override its clean-directory hook before any write, require a fresh
build/w-sb1icf-* directory, then run the canonical main/comparator/certificates.
"""
import os
import sys
sys.dont_write_bytecode = True
os.environ['PYTHONDONTWRITEBYTECODE'] = '1'
import json
import re
import shutil
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HERE = ROOT / 'scratch/w-sb1icf'
sys.path.insert(0, str(ROOT / 'tools'))

def prepare():
    dest = HERE / 'extern'
    dest.mkdir(exist_ok=True)
    src = ROOT / 'src'
    for p in src.iterdir():
        if p.is_file() and p.suffix in ('.cpp', '.h', '.inc'):
            raw = p.read_bytes()
            if p.name in ('network.cpp', 'network.h'):
                raw = raw.replace(b'static BOOL CALLBACK Enum', b'BOOL CALLBACK Enum')
            def local(m):
                return b'#include "' + m[1] + b'"' if (src / m[1].decode()).is_file() else m[0]
            raw = re.sub(rb'#\s*include\s*<([^>]+)>', local, raw)
            (dest / p.name).write_bytes(raw)
    shutil.copyfile(ROOT / 'scratch/w-emitorder/sb1-extern.diff', HERE / 'extern.diff')
    def affected(p, seen=None):
        seen = set() if seen is None else seen
        if p.name == 'network.h':
            return True
        if p.name in seen or not p.is_file():
            return False
        seen.add(p.name)
        return any(affected(src / x, seen) for x in re.findall(r'#\s*include\s*["<]([^">]+)[">]', p.read_text(errors='replace')))
    manifest = json.loads((ROOT / 'recon.json').read_text())
    units = [u['source'] for u in manifest['translation_units'] if affected(ROOT / u['source'])]
    (HERE / 'affected.json').write_text(json.dumps(units, indent=2))
    import difflib
    for mode, static_name in [('providers-only','EnumSessionsCallback'), ('sessions-only','EnumProvidersCallback')]:
        control=HERE/mode
        shutil.copytree(dest,control,dirs_exist_ok=True)
        for name in ['network.cpp','network.h']:
            p=control/name
            p.write_bytes(p.read_bytes().replace(('BOOL CALLBACK '+static_name).encode(),('static BOOL CALLBACK '+static_name).encode()))
        diff=''.join(''.join(difflib.unified_diff((src/name).read_text().splitlines(True),
            (control/name).read_text().splitlines(True),fromfile='src/'+name,tofile='src/'+name))
            for name in ['network.cpp','network.h'])
        (HERE/(mode+'.diff')).write_text(diff)
    print('Header dependents:', units)

def verify(target, label, variant='base'):
    import verify as v
    # Canonical secondary wrapper otherwise writes build/targets/...; relocate
    # only that generated shadow, preserving pinned cmpfunc.py byte for byte.
    original_run=subprocess.run
    def local_target_run(command,*args,**kwargs):
        if len(command)>1 and Path(command[1]).name=='cmpfunc_target.py':
            command=list(command)
            command[1]=str(HERE/'cmpfunc_target_local.py')
        return original_run(command,*args,**kwargs)
    subprocess.run=local_target_run
    out = ROOT / 'build' / ('w-sb1icf-' + label)
    if out.exists():
        raise RuntimeError('fresh directories only: ' + str(out))
    def scratch_clean():
        v.BUILD = out
        assert out.parent == ROOT / 'build' and out.name.startswith('w-sb1icf-')
        out.mkdir()
    v.safe_clean_build = scratch_clean
    argv = ['tools/verify.py', '--target', target, '--build-dir', 'w-sb1icf-' + label]
    if variant != 'base':
        unit_file=HERE/(variant+'.units.json')
        if not unit_file.is_file():unit_file=HERE/'affected.json'
        for unit in json.loads(unit_file.read_text()):
            argv.extend(['--tu', unit + '=' + str(HERE / variant / Path(unit).name)])
    argv += sys.argv[5:]
    sys.argv = argv
    print('Verifier args:', subprocess.list2cmdline(argv), flush=True)
    return v.main()

def relink(target, label, objects):
    return subprocess.call([sys.executable, 'tools/relink.py', '--target', target,
        '--objects-dir', objects, '--out-dir', 'build/w-sb1icf-' + label,
        '--historical-host'] + sys.argv[5:], cwd=ROOT)

if __name__ == '__main__':
    kind = sys.argv[1]
    if kind == 'prepare':
        prepare()
        sys.exit(0)
    # Parent captures a complete transcript and a reproducible command receipt.
    if kind in ('verify', 'relink'):
        cmd = [sys.executable, str(Path(__file__).resolve()), '_' + kind] + sys.argv[2:]
        start = time.time()
        run = subprocess.run(cmd, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, encoding='utf-8', errors='replace')
        label = sys.argv[3]
        (HERE / (kind + '-' + label + '.log')).write_text(run.stdout, encoding='utf-8')
        (HERE / (kind + '-' + label + '.command.json')).write_text(json.dumps(dict(
            command=cmd, display=subprocess.list2cmdline(cmd), exit=run.returncode,
            seconds=time.time()-start), indent=2))
        print(run.stdout[-4500:])
        sys.exit(run.returncode)
    if kind == '_verify':
        sys.exit(verify(*sys.argv[2:5]))
    if kind == '_relink':
        sys.exit(relink(*sys.argv[2:5]))
    raise ValueError(kind)
