"""Unapplied inventory/expectation proposal, tested only in private copies.

Canonical code is unchanged. Certificate comparison is collected rather than
raised so every semantic proof runs; all changed pins remain FAIL_PIN and
are not written or treated as accepted certificates.
"""
import os,sys
sys.dont_write_bytecode=True
os.environ['PYTHONDONTWRITEBYTECODE']='1'
import csv,json,copy,io,contextlib,subprocess,difflib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
import targets,verify,equivalence_target
original_run=subprocess.run
def local_run(command,*args,**kwargs):
    if len(command)>1 and Path(command[1]).name=='cmpfunc_target.py':
        command=list(command);command[1]=str(HERE/'cmpfunc_target_local.py')
    return original_run(command,*args,**kwargs)
subprocess.run=local_run
rows=[];patch=[];expectations={};patched_inventory_paths=set()
for name in [n for n in targets.names() if n.startswith('sb1/eggbert/')]:
    target=targets.load(name)
    manifest=target.manifest()
    hide=next(x for x in manifest['accepted_functions'] if 'GetHide@CJauge' in x['symbol'])
    sessions=next(x for x in manifest['accepted_functions'] if 'GetNbSessions@CNetwork' in x['symbol'])
    rva=int(str(hide['rva']),0)
    proposed=copy.deepcopy(target)
    dest=HERE/'proposed-inventory'/target.slug
    dest.mkdir(parents=True,exist_ok=True)
    shutil=__import__('shutil')
    shutil.copyfile(target.oracle_dir/'functions.csv',dest/'functions.csv')
    if (target.oracle_dir/'address_map.json').is_file():
        shutil.copyfile(target.oracle_dir/'address_map.json',dest/'address_map.json')
    raw=(target.oracle_dir/'tu_map.csv').read_text(encoding='utf-8-sig')
    reader=csv.DictReader(io.StringIO(raw));fields=reader.fieldnames;inventory=list(reader)
    row=next(x for x in inventory if int(x['rva'],0)==rva)
    old=dict(row)
    row['tu']='network.cpp'
    row['best_source']='sb2:network.cpp:CNetwork::GetNbSessions'
    row['evidence']='[EVIDENCE, PROPOSAL] Shared getter lies between the independently located CNetwork::EnumSessions and GetSessionName bodies; unique identical CJauge::GetHide bytes do not establish physical ownership.'
    stream=io.StringIO(newline='');w=csv.DictWriter(stream,fieldnames=fields,lineterminator='\n');w.writeheader();w.writerows(inventory)
    new=stream.getvalue()
    (dest/'tu_map.csv').write_text(new,encoding='utf-8')
    inventory_path=str((target.oracle_dir/'tu_map.csv').relative_to(ROOT)).replace('\\','/')
    # All six Eggbert targets currently inherit the same English base inventory.
    # Keep their independent validation copies, but emit its proposed edit once.
    if inventory_path not in patched_inventory_paths:
        patch.extend(difflib.unified_diff(raw.splitlines(True),new.splitlines(True),
            fromfile=inventory_path,tofile=inventory_path))
        patched_inventory_paths.add(inventory_path)
    proposed.oracle_dir=dest
    hide['status']='folded';hide['into']=dict(symbol=sessions['symbol'],tu='src/network.cpp',rva=hex(rva))
    sessions.pop('status',None);sessions.pop('into',None)
    expectations[name]={'accepted_overrides':{
        hide['symbol']:dict(status='folded',into=hide['into']),sessions['symbol']:dict(status='strict')}}
    oldout=ROOT/'build'/('w-sb1icf-'+target.slug+'-extern')
    out=ROOT/'build'/('w-sb1icf-owner-proposal-'+target.slug)
    out.mkdir(exist_ok=True)
    verify.TARGET=proposed;verify.BUILD=out
    verify.GAME_TEXT_END=target.game_text_end;verify.GAME_TEXT_SIZE=target.game_text_end-verify.ORACLE_TEXT_START
    verify.DEGRADED.clear();verify.DEGRADED['src/sound.cpp']='same sandbox sound path limitation'
    objects=sorted((oldout/'objects').glob('[0-9]*.obj'));evidence=sorted((oldout/'objects').glob('crt_evidence*.obj'))
    stream=io.StringIO();pins=[]
    def collect(saved,current,case):
        pins.append(dict(case=case,status='PASS' if saved==current else 'FAIL_PIN',
            differing_top_level_fields=[k for k in current if saved.get(k)!=current[k]],saved=saved,current=current))
    original_validate=equivalence_target.validate_certificate
    equivalence_target.validate_certificate=collect
    try:
        with contextlib.redirect_stdout(stream):
            comparison=verify.run_comparator(objects,evidence,manifest)
            by_unit,coverage=verify.report(comparison,[u['source'] for u in manifest['translation_units']])
            regressions=verify.accepted_regressions(manifest,by_unit)
            semantic_results=equivalence_target.check(proposed,manifest,comparison,evidence,out)
    finally:equivalence_target.validate_certificate=original_validate
    (out/'proposal.log').write_text(stream.getvalue())
    records={f['name']:f for u in comparison for f in u['functions']}
    previous={f['name']:f for u in json.loads((oldout/'results.json').read_text()) for f in u['functions']}
    changes=[k for k in records if records[k]!=previous[k]]
    around=[dict(x) for x in inventory if abs(int(x['rva'],0)-rva)<400]
    result=dict(target=name,old_row=old,proposed_row=row,neighbouring_inventory_rows=around,
        proposed_expectations=expectations[name],accepted_regressions=regressions,coverage=coverage,
        differing_records=changes,getter_records={k:records[k] for k in [hide['symbol'],sessions['symbol']]},
        semantic_proofs_completed=True,certificate_comparisons=pins,
        overall_status='PROPOSAL_ONLY_FAIL_PIN' if any(p['status']!='PASS' for p in pins) else 'PROPOSAL_ONLY')
    assert not regressions,regressions
    assert records[hide['symbol']]['status']=='FOLDED' and records[sessions['symbol']]['status']=='STRICT'
    assert all(p['differing_top_level_fields']==['inventory_sha256'] for p in pins),pins
    (out/'proposal-receipt.json').write_text(json.dumps(result,indent=2))
    (out/'gate.json').write_text(json.dumps(dict(status='FAIL_PIN_PROPOSAL_ONLY',
        accepted_regressions=regressions,semantic_proofs_completed=True,
        pin_comparisons=[dict(case=p['case'],status=p['status'],differing_top_level_fields=p['differing_top_level_fields']) for p in pins]),indent=2))
    rows.append(result)
    (HERE/'solver-owner-proposal.json').write_text(json.dumps(rows,indent=2))
    print(name,'GetNbSessions STRICT; GetHide FOLDED; accepted regressions 0; semantic proofs complete; pins differ only inventory hash',flush=True)
(HERE/'solver-eggbert-inventory.diff').write_text(''.join(patch))
(HERE/'solver-eggbert-owner-overrides.json').write_text(json.dumps(expectations,indent=2))
