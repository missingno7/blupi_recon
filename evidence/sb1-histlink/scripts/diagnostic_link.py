"""Historical link after a failed certificate, explicitly layout-only.

Canonical relink aborts on a certificate mismatch even in diagnostic mode.
This driver retains that failure in diagnostic-gate.json, then gives the
unchanged link options/object order to histlink. It grants no acceptance.
"""
import os,sys
sys.dont_write_bytecode=True
os.environ['PYTHONDONTWRITEBYTECODE']='1'
import json,subprocess,hashlib,io,contextlib
from pathlib import Path
import pefile
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
import relink,verify,equivalence,targets
label,objects=sys.argv[1:3]
out=ROOT/'build'/('w-sb1icf-'+label)
out.mkdir(exist_ok=True)
objects=(ROOT/objects).resolve()
target=targets.load('sb1/blupi/us')
manifest=target.manifest()
evidence=sorted(objects.glob('crt_evidence*.obj'))
stream=io.StringIO()
verify.BUILD=out
verify.TARGET=target
verify.DEGRADED.clear()
sound_log=objects.parent/'logs/08_sound.log'
if not sound_log.is_file() or not any('F:\\Profil\\sound.cpp'.lower() in l.lower() for l in sound_log.read_text().splitlines() if l.startswith('command:')):
    verify.DEGRADED['src/sound.cpp']='same sandbox historical-path limitation'
with contextlib.redirect_stdout(stream):
    comparison=verify.run_comparator([objects/f'{i:02d}_{Path(u["source"]).stem}.obj' for i,u in enumerate(manifest['translation_units'])],evidence,manifest)
    by_unit,coverage=verify.report(comparison,[u['source'] for u in manifest['translation_units']])
regressions=verify.accepted_regressions(manifest,by_unit)
certificates={}
pending={}
for case,(symbol,*_) in equivalence.CASES.items():
    unit=next(u for u in comparison if any(f['name']==symbol for f in u['functions']))
    c=equivalence.cmpfunc.Coff(Path(unit['object']))
    f=next(f for f in c.functions() if f.name==symbol)
    pending[case]=(None,c,f)
with contextlib.redirect_stdout(stream):
    receipt=equivalence.audit(comparison,evidence,out,pending)
for case,(_,c,f) in pending.items():
    current=equivalence.certificate(case,c,f,receipt)
    saved=json.loads((ROOT/'evidence/equivalence'/ (case+'.json')).read_text())
    certificates[case]=dict(status='PASS' if current==saved else 'FAIL_PIN',
        differing_top_level_fields=[k for k in current if current[k]!=saved[k]],current=current,saved=saved)
gate=dict(status='FAIL_CERTIFICATE' if any(x['status']!='PASS' for x in certificates.values()) else 'INCOMPLETE',
    layout_only=True,accepted_regressions=regressions,coverage=coverage,certificates=certificates)
(out/'diagnostic-gate.json').write_text(json.dumps(gate,indent=2))
(out/'verify.log').write_text(stream.getvalue())
prior=ROOT/'build/w-sb1icf-link-extern'
rsp=(prior/'link.rsp').read_text(encoding='cp1252')
rsp=rsp.replace(str(ROOT/'build/w-sb1icf-us-extern/objects'),str(objects))
rsp=rsp.replace(str(prior/'BLUPI.EXE'),str(out/'BLUPI.EXE')).replace(str(prior/'BLUPI.map'),str(out/'BLUPI.map'))
(out/'link.rsp').write_text(rsp,encoding='cp1252')
cmd=[sys.executable,'tools/histlink.py','--trace-rich','--report',str(out/'historical-host.json'),'--','@'+str(out/'link.rsp')]
run=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,encoding='utf-8',errors='replace')
(out/'link.log').write_text(run.stdout+run.stderr)
record=dict(command=cmd,exit=run.returncode,objects={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(objects.glob('[0-9]*.obj'))},
    gate_status=gate['status'],canonical_gate_bypassed_for_layout_only=True)
(out/'build.json').write_text(json.dumps(record,indent=2))
if run.returncode:
    print(run.stdout+run.stderr);sys.exit(run.returncode)
oracle=pefile.PE(str(target.exe))
candidate=pefile.PE(str(out/'BLUPI.EXE'))
layout=relink.layout_report(oracle,candidate,relink.read_map(out/'BLUPI.map'),manifest)
layout['candidate_sha256']=hashlib.sha256((out/'BLUPI.EXE').read_bytes()).hexdigest()
layout['rich_header']=dict(oracle=relink.rich_header(target.exe),candidate=relink.rich_header(out/'BLUPI.EXE'))
(out/'layout.json').write_text(json.dumps(layout,indent=2))
print('LAYOUT ONLY; gate',gate['status'],'accepted regressions',regressions)
print('certificates',[(k,v['status'],v['differing_top_level_fields']) for k,v in certificates.items()])
print('layout',layout['accepted_function_summary'])
print('sections',layout['sections'])
print('sha256',layout['candidate_sha256'])
