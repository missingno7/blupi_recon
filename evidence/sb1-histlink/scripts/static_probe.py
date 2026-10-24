"""BLUPI-LAY-04 control for the two remaining internal helpers."""
import sys,os
sys.dont_write_bytecode=True
os.environ['PYTHONDONTWRITEBYTECODE']='1'
import json,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
from cmpfunc import Coff
source=HERE/'solver-blupi-extern-helpers.cpp'
raw=(HERE/'extern/blupi.cpp').read_bytes().replace(b'static void FinishObjects',b'void FinishObjects').replace(b'static BOOL DoInit',b'BOOL DoInit')
source.write_bytes(raw)
obj=ROOT/'build/w-sb1icf-static-probe/blupi.obj'
manifest=json.loads((ROOT/'recon.json').read_text())
cmd=[sys.executable,'tools/msvc.py',str(source),'--tree','sp3','--out',str(obj),'--log-dir',str(obj.parent/'logs'),
    '--include',str(HERE/'extern'),'--include',r'C:\tools\dx5sdk\INCLUDE']
for define in manifest['toolchain']['defines']:cmd+=['--define',define]
for flag in manifest['toolchain']['flags']:cmd+=['--flag',flag]
run=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,encoding='utf-8',errors='replace')
(HERE/'solver-static-probe.log').write_text(run.stdout+run.stderr)
assert run.returncode==0,run.stdout+run.stderr
a=Coff(ROOT/'build/w-sb1icf-us-extern/objects/10_blupi.obj');b=Coff(obj)
def describe(c):
    return [(f.name,f.section.index,f.data.hex(),[(r.offset-f.start,r.typ,c.sym_by_index[r.symbol_index].name) for r in f.relocs]) for f in c.functions()]
assert describe(a)==describe(b)
result=dict(command=cmd,exit=run.returncode,all_function_payloads_symbolic_relocations_and_section_order_identical=True,
    remaining_helpers=[dict(name=s.name,old_storage=s.storage,new_storage=next(t.storage for t in b.symbols if t.name==s.name and t.section>0),section=s.section)
        for s in a.symbols if s.is_function and s.storage==3 and s.section>0])
(HERE/'solver-static-probe.json').write_text(json.dumps(result,indent=2))
print(result['remaining_helpers'],'all section order/payload/relocation records identical')
