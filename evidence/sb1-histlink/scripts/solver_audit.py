"""Final static proof/data inventory for the solver follow-up; no output edits."""
import sys
sys.dont_write_bytecode=True
import json,hashlib,struct,subprocess,bisect
from pathlib import Path
from collections import defaultdict,Counter
import pefile
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
from cmpfunc import Coff
import relink,solver
from analyze import qsort
out={}
trace=json.loads((ROOT/'build/w-sb1icf-trace-solver-tail8/historical-host.json').read_text())
s=next(s for s in trace['sorts'] if s['comparator']==0x432130)
assert qsort(s['before'])==s['after']
c=Coff(ROOT/'build/w-sb1icf-solver-us-tail8/objects/16_decmove.obj')
block=[dict(name=f.name,size=f.section.raw_size,relocs=len(f.section.relocations),module='16_decmove.obj') for f in c.functions()]
old=solver.items
indices=[i for i,x in enumerate(old) if Path(x['module']).name=='16_decmove.obj']
identity=lambda x:(x['name'],Path(x['module']).name,x['size'],x['relocs'])
expected=[identity(x) for x in old[:indices[0]]]+[identity(x) for x in block]+[identity(x) for x in old[indices[-1]+1:]]
assert expected==[identity(x) for x in s['before']]
before={x['con']:x for x in s['before']}
folds=[]
for f in trace['folds']:
    keeper,later=before[f['keeper']],before[f['folded']]
    folds.append(dict(keeper=identity(keeper),folded=identity(later)))
new=[x for x in s['before'] if x['name'] in {f.name for f in c.functions()}-{x['name'] for x in old}]
assert len(new)==8 and all(not(int(x['flags'],16)&0x2000) for x in new)
module_identity=lambda x:(Path(x['name']).name if not x['library'] else x['name'],x['module_index'],
    x['library'].lower(),x['archive_data_offset'] if x['library'] else None,x['compiler_stamp'])
assert [module_identity(x) for x in trace['rich_modules']]==[module_identity(x) for x in solver.data['rich_modules']]
out['trace_validation']=dict(count=s['count'],full_replay_exact=True,predicted_input_exact=True,actual_folds=folds,
    added_entries=[{k:v for k,v in x.items() if k not in ['pointer','con','con_fields']} for x in new],
    loaded_member_sequence_identical=True,job_active_processes_at_exit=trace['job_active_processes_at_exit'],tool_files_unchanged=trace['tool_files_unchanged'])
assert hashlib.sha256((ROOT/'build/w-sb1icf-trace-solver-tail8/BLUPI.EXE').read_bytes()).hexdigest()==hashlib.sha256((ROOT/'build/w-sb1icf-solver-link-tail8/BLUPI.EXE').read_bytes()).hexdigest()

out['links']={}
for label in ['solver-link-tail8','solver-link-tail8-guard','solver-link-declarations-only','solver-link-sb2forms']:
    p=ROOT/'build'/('w-sb1icf-'+label)
    layout=json.loads((p/'layout.json').read_text())
    host=json.loads((p/'historical-host.json').read_text())
    assert host['job_active_processes_at_exit']==0 and host['tool_files_unchanged']
    ss=next(s for s in host['sorts'] if s['comparator']==0x432130)
    assert qsort(ss['before'])==ss['after']
    out['links'][label]=dict(summary=layout['accepted_function_summary'],sections=layout['sections'],
        hash=layout['candidate_sha256'],rich=layout['rich_header'],
        gate=json.loads((p/'diagnostic-gate.json').read_text())['status'],full_qsort_replay_exact=True)
assert out['links']['solver-link-declarations-only']['hash']==json.loads((ROOT/'build/w-sb1icf-link-extern/layout.json').read_text())['candidate_sha256']

oracle=pefile.PE(str(ROOT/'assets/Speedy Blupi/BLUPI.EXE'))
candidate=pefile.PE(str(ROOT/'build/w-sb1icf-solver-link-tail8-guard/BLUPI.EXE'))
a=Path(oracle.__data__.name).read_bytes() if getattr(oracle.__data__,'name',None) else bytes(oracle.__data__)
b=(ROOT/'build/w-sb1icf-solver-link-tail8-guard/BLUPI.EXE').read_bytes()
diff={i for i,(x,y) in enumerate(zip(a,b)) if x!=y}
section_diffs={};covered=set()
for sec in oracle.sections:
    offsets=set(range(sec.PointerToRawData,sec.PointerToRawData+sec.SizeOfRawData))
    section_diffs[sec.Name.rstrip(b'\0').decode()]=len(diff&offsets);covered|=offsets
out['whole_file_comparison']=dict(same_size=len(a)==len(b),size=len(a),differing_bytes=len(diff),
    differing_bytes_by_raw_section=section_diffs,outside_sections=[dict(offset=hex(i),oracle=a[i],candidate=b[i]) for i in sorted(diff-covered)])

def imports(pe):
    return {i.address:(d.dll.decode().lower(),i.name.decode() if i.name else '#'+str(i.ordinal)) for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports}
ia,ib=imports(oracle),imports(candidate)
text=next(s for s in oracle.sections if s.Name.rstrip(b'\0')==b'.text')
aa=oracle.get_data(text.VirtualAddress,text.Misc_VirtualSize)
bb=candidate.get_data(text.VirtualAddress,text.Misc_VirtualSize)
iat_fields=[];iat_diff=set()
for off in range(len(aa)-3):
    av,bv=struct.unpack_from('<I',aa,off)[0],struct.unpack_from('<I',bb,off)[0]
    if av in ia and bv in ib and ia[av]==ib[bv] and av!=bv:
        iat_fields.append(dict(rva=hex(text.VirtualAddress+off),symbol=ia[av],oracle=hex(av),candidate=hex(bv)))
        iat_diff.update(off+j for j in range(4) if aa[off+j]!=bb[off+j])
remaining=[off for off,(x,y) in enumerate(zip(aa,bb)) if x!=y and off not in iat_diff]
cases={name:(rva,len(next(f for f in Coff(ROOT/'build/w-sb1icf-solver-us-tail8/objects/13_event.obj').functions() if f.name==symbol).data))
    for name,(symbol,rva,*_) in __import__('equivalence').CASES.items()}
out['text_residual_diagnostic']=dict(iat_identity_fields=iat_fields,differing_bytes_in_iat_fields=len(iat_diff),
    remaining_count=len(remaining),remaining_rvas=[hex(text.VirtualAddress+i) for i in remaining],
    equivalence_region_counts={name:sum(rva<=text.VirtualAddress+i<rva+size for i in remaining) for name,(rva,size) in cases.items()})
thunks=set()
for d in oracle.DIRECTORY_ENTRY_IMPORT:
    for rva in [d.struct.FirstThunk,d.struct.OriginalFirstThunk]:thunks.update(range(rva,rva+4*(len(d.imports)+1)))
rdata=next(s for s in oracle.sections if s.Name.rstrip(b'\0')==b'.rdata')
rda=oracle.get_data(rdata.VirtualAddress,rdata.Misc_VirtualSize);rdb=candidate.get_data(rdata.VirtualAddress,rdata.Misc_VirtualSize)
out['rdata_residual_diagnostic']=dict(outside_import_thunk_arrays=[hex(rdata.VirtualAddress+i) for i,(x,y) in enumerate(zip(rda,rdb)) if x!=y and rdata.VirtualAddress+i not in thunks])

map_symbols=relink.read_map(ROOT/'build/w-sb1icf-solver-link-tail8-guard/BLUPI.map')
wanted=['?GetHide@CJauge@@QAEHXZ','??1CMenu@@QAE@XZ','?GetEnable@CSound@@QAEHXZ',
    '?ArrayUnwindFilter@@YAHPAU_EXCEPTION_POINTERS@@@Z','_strncnt','?BuildUp@CEvent@@IAEHUtagPOINT@@H@Z',
    '?_CallMemberFunction0@@YGXPAX0@Z','__chkstk']
out['winner_locations']=[dict(name=x['name'],rva=hex(x['va']-0x400000),owner=x['owner']) for x in map_symbols if x['name'] in wanted]
sb2=json.loads((ROOT/'games/sb2/recon.json').read_text())
out['sb2_family_evidence']=[x for x in sb2['accepted_functions'] if 'AscenseurShift' in x['symbol'] or 'MoveAscenseurDetect' in x['symbol']]
out['git_head']=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
out['git_status']=subprocess.check_output(['git','status','--short'],cwd=ROOT,text=True)
out['source_hashes']={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (ROOT/'src').iterdir() if p.is_file()}
(HERE/'solver-audit.json').write_text(json.dumps(out,indent=2))
print('801-entry real trace matches the predicted input and complete qsort replay; eight added entries all REF-dead.')
print('Whole file',out['whole_file_comparison'])
print('Text differences: IAT identity fields',len(iat_diff),'remaining',len(remaining),'by equivalence case',out['text_residual_diagnostic']['equivalence_region_counts'])
print('rdata outside import thunk arrays',out['rdata_residual_diagnostic'])
print('SB2 evidence',out['sb2_family_evidence'])
print('git status',repr(out['git_status']))
