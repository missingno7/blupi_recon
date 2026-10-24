"""Fresh nine-target source hypothesis runs; record every failure verbatim."""
import os,sys
sys.dont_write_bytecode=True
os.environ['PYTHONDONTWRITEBYTECODE']='1'
import json,subprocess,io,contextlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
import targets,verify
from cmpfunc import Coff
rows=[]
variant=sys.argv[1] if len(sys.argv)>1 else 'solver-tail8'
suffix='sb2forms' if variant=='solver-tail8-sb2forms' else 'tail8'
receipt=HERE/('solver-suite'+('-sb2forms' if suffix=='sb2forms' else '')+'.json')
for target_name in [n for n in targets.names() if n.startswith('sb1/')]:
    slug='us' if target_name=='sb1/blupi/us' else target_name.replace('/','-')
    label='solver-'+slug+'-'+suffix
    out=ROOT/'build'/('w-sb1icf-'+label)
    if not (out/'results.json').is_file():
        cmd=[sys.executable,str(HERE/'run.py'),'verify',target_name,label,variant]
        run=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,encoding='utf-8',errors='replace')
    command=json.loads((HERE/('verify-'+label+'.command.json')).read_text())
    comparison=json.loads((out/'results.json').read_text())
    before=ROOT/'build'/('w-sb1icf-'+slug+'-extern')
    previous=json.loads((before/'results.json').read_text())
    oldrecords={f['name']:f for u in previous for f in u['functions']}
    newrecords={f['name']:f for u in comparison for f in u['functions']}
    raw_changes=[];reloc_changes=[]
    for old,new in zip(previous,comparison):
        a=Coff(Path(old['object']));b=Coff(Path(new['object']))
        af={f.name:f for f in a.functions()};bf={f.name:f for f in b.functions()}
        for name,f in af.items():
            g=bf[name]
            if f.data!=g.data:raw_changes.append(name)
            ra=[(r.offset-f.start,r.typ,a.sym_by_index[r.symbol_index].name) for r in f.relocs]
            rb=[(r.offset-g.start,r.typ,b.sym_by_index[r.symbol_index].name) for r in g.relocs]
            if ra!=rb:reloc_changes.append(name)
    target=targets.load(target_name)
    verify.TARGET=target
    verify.GAME_TEXT_END=target.game_text_end
    verify.GAME_TEXT_SIZE=target.game_text_end-verify.ORACLE_TEXT_START
    verify.DEGRADED.clear();verify.DEGRADED['src/sound.cpp']='historical path unavailable'
    with contextlib.redirect_stdout(io.StringIO()):
        by_unit,coverage=verify.report(comparison,[u['source'] for u in target.manifest()['translation_units']])
    regressions=verify.accepted_regressions(target.manifest(),by_unit)
    log=(HERE/('verify-'+label+'.log')).read_text()
    row=dict(target=target_name,exit=command['exit'],coverage=coverage,accepted_regressions=regressions,
        old_function_count=len(oldrecords),new_function_count=len(newrecords),
        added_records={n:newrecords[n] for n in newrecords if n not in oldrecords},
        changed_existing_records={n:dict(before=oldrecords[n],after=newrecords[n]) for n in oldrecords if oldrecords[n]!=newrecords[n]},
        raw_payload_changes=raw_changes,symbolic_relocation_changes=reloc_changes,
        certificate_log=[l for l in log.splitlines() if 'certificate' in l.lower() or 'EQUIVALENT' in l or 'equivalence' in l.lower()])
    rows.append(row)
    receipt.write_text(json.dumps(rows,indent=2))
    print(target_name,'exit',row['exit'],'accepted regressions',len(regressions),'new',len(row['added_records']),
        'changed records',len(row['changed_existing_records']),'raw changes',len(raw_changes),'symbolic changes',len(reloc_changes),row['certificate_log'][:1],flush=True)
