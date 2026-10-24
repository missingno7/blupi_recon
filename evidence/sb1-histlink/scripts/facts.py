"""Collect final static diagnostics and verify evidence consistency."""
import sys
sys.dont_write_bytecode=True
import json
import hashlib
import subprocess
from collections import Counter
from pathlib import Path
import capstone
import pefile
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
import relink
from analyze import qsort

labels=['link-base','link-providers-only','link-sessions-only','link-extern',
    'link-extern-defaultlibs','link-guard-base','link-guard-extern']
out={'links':{},'one_factor_validation':{},'canonical_hashes':{}}
for label in labels:
    dest=ROOT/'build'/('w-sb1icf-'+label)
    layout=json.loads((dest/'layout.json').read_text())
    gate=json.loads((dest/'gate.json').read_text())
    host=json.loads((dest/'historical-host.json').read_text())
    assert host['job_active_processes_at_exit']==0 and host['tool_files_unchanged']
    symbols=relink.read_map(dest/'BLUPI.map')
    names=['??1CButton@@QAE@XZ','?GetHide@CJauge@@QAEHXZ','?GetNbSessions@CNetwork@@QAEHXZ',
        '?GetEnable@CSound@@QAEHXZ','?BuildUp@CEvent@@IAEHUtagPOINT@@H@Z',
        '?ArrayUnwindFilter@@YAHPAU_EXCEPTION_POINTERS@@@Z','_strncnt',
        '?_CallMemberFunction0@@YGXPAX0@Z','__chkstk']
    out['links'][label]=dict(sha256=layout['candidate_sha256'],
        gate=gate['status'],regressions=gate['regressions'],
        layout=layout['accepted_function_summary'],entry=hex(layout['candidate']['entry_rva']),
        sections=layout['sections'],rich=layout['rich_header'],
        alias_locations=[dict(name=x['name'],rva=hex(x['va']-0x400000),owner=x['owner']) for x in symbols if x['name'] in names])
    s=next(s for s in host['sorts'] if s['comparator']==0x432130)
    assert qsort(s['before'])==s['after']
    if label in ['link-providers-only','link-sessions-only']:
        expected=json.loads((ROOT/'build/w-sb1icf-link-base/historical-host.json').read_text())
        bs=next(s for s in expected['sorts'] if s['comparator']==0x432130)
        vector=list(bs['before'])
        i=358 if label.endswith('providers-only') else 364
        vector[i],vector[i+1]=vector[i+1],vector[i]
        fields=lambda a:[(x['name'],x['size'],x['relocs']) for x in a]
        assert fields(vector)==fields(s['before'])
        # Compare full permutations by input slot, avoiding pointer identity and duplicate names.
        pred=qsort(vector)
        pred_indices=[next(i for i,y in enumerate(vector) if y['pointer']==x['pointer']) for x in pred]
        actual_indices=[next(i for i,y in enumerate(s['before']) if y['pointer']==x['pointer']) for x in s['after']]
        assert pred_indices==actual_indices
        out['one_factor_validation'][label]=dict(count=s['count'],input_order_predicted=True,
            full_permutation_predicted=True)

assert out['links']['link-guard-base']['sha256']=='557ca92e6ec8c71cfaf5e9f2d55969d3ebcd614c771c9010e074c6ad82ba3533'
assert out['links']['link-extern-defaultlibs']['sha256']==out['links']['link-extern']['sha256']

bs=json.loads((ROOT/'build/w-sb1icf-trace-base/historical-host.json').read_text())
es=json.loads((ROOT/'build/w-sb1icf-trace-extern/historical-host.json').read_text())
canonical_module=lambda x:(Path(x['name']).name if 'w-sb1icf' in x['name'] else x['name'],x['module_index'],x['archive_data_offset'] if x['library'] else None,x['compiler_stamp'])
out['rich_loaded_member_sequence_identical']=[canonical_module(x) for x in bs['rich_modules']]==[canonical_module(x) for x in es['rich_modules']]
assert out['rich_loaded_member_sequence_identical']
out['import_sorts_identical']=[]
for a,b in zip(bs['sorts'][:2],es['sorts'][:2]):
    strip=lambda v:[(x['name'],x['module_index']) for x in v]
    out['import_sorts_identical'].append(strip(a['before'])==strip(b['before']) and strip(a['after'])==strip(b['after']))
assert all(out['import_sorts_identical'])

# Preserve the exact static disassembly supporting the comparator/first-winner rule.
p=pefile.PE(r'C:\tools\msvc5_sp3\VC\BIN\LINK.EXE')
c=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
lines=[]
for a,n in [(0x431f30,0x78),(0x431fb0,0x178),(0x432130,0x98),(0x432970,0x120),(0x4604d8,0xb7)]:
    lines.append('\nVA '+hex(a))
    lines.extend(f'{i.address:08x}  {i.mnemonic:8s} {i.op_str}' for i in c.disasm(p.get_data(a-0x400000,n),a))
(HERE/'link-icf-disassembly.txt').write_text('\n'.join(lines))
for directory in ['src','tools','msvc5kb','docs','evidence']:
    for p in (ROOT/directory).rglob('*'):
        if p.is_file() and '__pycache__' not in p.parts:
            out['canonical_hashes'][str(p.relative_to(ROOT))]=hashlib.sha256(p.read_bytes()).hexdigest()
for name in ['recon.json','targets.json','AGENTS.md']:
    p=ROOT/name
    out['canonical_hashes'][name]=hashlib.sha256(p.read_bytes()).hexdigest()
out['git_head']=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
out['git_status']=subprocess.check_output(['git','status','--short'],cwd=ROOT,text=True)
(HERE/'facts.json').write_text(json.dumps(out,indent=2))
print('Verified all seven link receipts, 793-entry factorial replays, identical loaded modules/import sorts, clean tool-tree exits.')
print('Git HEAD',out['git_head'],'status',repr(out['git_status']))
for label,item in out['links'].items():
    print(label,item['gate'],item['layout']['at_original_rva'],item['sections'][0]['same_section_offset_equal_bytes'],item['sha256'])
