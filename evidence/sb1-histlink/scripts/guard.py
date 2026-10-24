"""Snapshot attested guard objects without changing them; historical-sound control.

These are preserved objects, not fresh builds, and are reported separately.
"""
import sys
sys.dont_write_bytecode=True
import json
import hashlib
import shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
prior=json.loads((ROOT/'build/w-emitorder-sb1-guard/build.json').read_text())
manifest=json.loads((ROOT/'recon.json').read_text())
affected=json.loads((HERE/'affected.json').read_text())
records={}
for mode in ['base','extern']:
    out=ROOT/'build'/('w-sb1icf-guard-'+mode)/'objects'
    out.mkdir(parents=True,exist_ok=True)
    logs=out.parent/'logs'
    logs.mkdir(exist_ok=True)
    for unit,info in prior['objects'].items():
        original=Path(info['path'])
        assert hashlib.sha256(original.read_bytes()).hexdigest()==info['sha256'],original
        source=ROOT/'build/w-sb1icf-us-extern/objects'/original.name if mode=='extern' and unit in affected else original
        shutil.copyfile(source,out/original.name)
        log=source.parent.parent/'logs'/(source.stem+'.log')
        if log.is_file():
            shutil.copyfile(log,logs/log.name)
        records[str(out/original.name)]=dict(source=str(source),sha256=hashlib.sha256(source.read_bytes()).hexdigest(),guard_sha256=info['sha256'])
(HERE/'guard-inputs.json').write_text(json.dumps(records,indent=2))
print('Preserved guard and header-candidate object snapshots ready.')
