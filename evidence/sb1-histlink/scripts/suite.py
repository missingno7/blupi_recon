"""Fresh serial all-target baseline/candidate acceptance and payload audit."""
import sys
sys.dont_write_bytecode=True
import os
os.environ['PYTHONDONTWRITEBYTECODE']='1'
import json
import subprocess
import hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
from cmpfunc import Coff
import targets

def audit(target):
    slug='us' if target=='sb1/blupi/us' else target.replace('/','-')
    arrays=[]
    payloads=[]
    for mode in ['base','extern']:
        out=ROOT/'build'/('w-sb1icf-'+slug+'-'+mode)
        results=json.loads((out/'results.json').read_text())
        records={}
        bodies={}
        for item in results:
            name=Path(item['object']).name
            records[name]={f['name']:f for f in item['functions']}
            c=Coff(Path(item['object']))
            bodies[name]={f.name:(f.data.hex(),[(r.offset-f.start,r.typ,c.sym_by_index[r.symbol_index].name) for r in f.relocs]) for f in c.functions()}
        blob=(json.dumps(records,sort_keys=True,indent=2)+'\n').encode()
        (out/'function-records-by-symbol.json').write_bytes(blob)
        arrays.append(blob)
        payloads.append(bodies)
    return dict(target=target, function_records_byte_identical=arrays[0]==arrays[1],
        records_sha256=[hashlib.sha256(x).hexdigest() for x in arrays],
        function_payloads_and_symbolic_relocations_identical=payloads[0]==payloads[1],
        differing_function_records=[name for unit,fs in json.loads(arrays[0]).items()
            for name,f in fs.items() if f != json.loads(arrays[1])[unit][name]],
        function_count=sum(len(x) for x in records.values()))

names=sys.argv[1:] or [t for t in targets.names() if t.startswith('sb1/')]
receipt=HERE/'suite.json'
rows=json.loads(receipt.read_text()) if receipt.exists() else {}
for target in names:
    slug='us' if target=='sb1/blupi/us' else target.replace('/','-')
    for mode in ['base','extern']:
        label=slug+'-'+mode
        if not (ROOT/'build'/('w-sb1icf-'+label)/'results.json').is_file():
            cmd=[sys.executable,str(HERE/'run.py'),'verify',target,label,mode]
            r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,encoding='utf-8',errors='replace')
            print(target,mode,'exit',r.returncode,flush=True)
            if r.returncode:
                print('Verifier rejected candidate; see complete log.',flush=True)
        command=json.loads((HERE/('verify-'+label+'.command.json')).read_text())
    rows[target]=audit(target)
    rows[target]['exit_codes']=[json.loads((HERE/('verify-'+slug+'-'+m+'.command.json')).read_text())['exit'] for m in ['base','extern']]
    logs=[(HERE/('verify-'+slug+'-'+m+'.log')).read_text() for m in ['base','extern']]
    rows[target]['certificates']=[[l.strip() for l in log.splitlines() if 'EQUIVALENT' in l or 'certificate' in l.lower()] for log in logs]
    rows[target]['sound_path_incomplete']=[('regression gate INCOMPLETE' in log) for log in logs]
    print(rows[target],flush=True)
    receipt.write_text(json.dumps(rows,indent=2))
