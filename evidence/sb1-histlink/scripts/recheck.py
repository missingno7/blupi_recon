"""Recheck fresh object sets with worker-local target inventory shadow paths.

No recompilation: reuse exactly the objects from the fresh serial suite.
All canonical comparator records, regression logic, certificates run unchanged.
"""
import sys
sys.dont_write_bytecode=True
import os
os.environ['PYTHONDONTWRITEBYTECODE']='1'
import json
import subprocess
import io
import contextlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
import verify as v
import targets
import equivalence_target
original_run=subprocess.run
def local_run(command,*args,**kwargs):
    if len(command)>1 and Path(command[1]).name=='cmpfunc_target.py':
        command=list(command)
        command[1]=str(HERE/'cmpfunc_target_local.py')
    return original_run(command,*args,**kwargs)
subprocess.run=local_run
rows=[]
for name in targets.names():
    if not name.startswith('sb1/') or name=='sb1/blupi/us':continue
    target=targets.load(name)
    manifest=target.manifest()
    for mode in ['base','extern']:
        old=ROOT/'build'/('w-sb1icf-'+target.slug+'-'+mode)
        out=ROOT/'build'/('w-sb1icf-recheck-'+target.slug+'-'+mode)
        out.mkdir(exist_ok=True)
        v.BUILD=out
        v.TARGET=target
        v.GAME_TEXT_END=target.game_text_end
        v.GAME_TEXT_SIZE=target.game_text_end-v.ORACLE_TEXT_START
        v.DEGRADED.clear()
        v.DEGRADED['src/sound.cpp']='historical build path unavailable, same fresh objects'
        objects=sorted((old/'objects').glob('[0-9]*.obj'))
        evidence=sorted((old/'objects').glob('crt_evidence*.obj'))
        stream=io.StringIO()
        with contextlib.redirect_stdout(stream):
            result=v.run_comparator(objects,evidence,manifest)
            by_unit,coverage=v.report(result,[u['source'] for u in manifest['translation_units']])
            regressions=v.accepted_regressions(manifest,by_unit)
            certificates=equivalence_target.check(target,manifest,result,evidence,out)
        (out/'recheck.log').write_text(stream.getvalue())
        prior=json.loads((old/'results.json').read_text())
        row=dict(target=name,mode=mode,results_identical_to_fresh_suite=result==prior,
            regressions=regressions,coverage=coverage,certificates=certificates)
        assert row['results_identical_to_fresh_suite']
        (out/'receipt.json').write_text(json.dumps(row,indent=2))
        rows.append(row)
        print(name,mode,'same results; regressions',len(regressions),flush=True)
(HERE/'recheck.json').write_text(json.dumps(rows,indent=2))
