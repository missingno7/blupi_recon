"""Read-only CRT archive census and conditional member-key substitutions."""
import sys
sys.dont_write_bytecode=True
import json,hashlib
from pathlib import Path
from collections import defaultdict
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
from cmpfunc import Coff
import solver
BUILD=ROOT/'build/w-sb1icf-solver-library'
BUILD.mkdir(exist_ok=True)
def members(data):
    assert data[:8]==b'!<arch>\n'
    pos=8;names=b''
    while pos+60<=len(data):
        h=data[pos:pos+60];assert h[58:60]==b'`\n'
        size=int(h[48:58]);name=h[:16].decode().strip();raw=data[pos+60:pos+60+size]
        if name=='//':names=raw
        elif name!='/':
            if name.startswith('/') and name[1:].isdigit():name=names[int(name[1:]):].split(b'\0')[0].decode().rstrip('/')
            yield name.replace('\\','/').split('/')[-1].lower().rstrip('/'),raw
        pos+=60+size+(size&1)
out={'archives':{},'conditional_replacements':[]}
archives={}
for release in ['sp3','sp2','rtm']:
    path=Path('C:/tools/msvc5_'+release+'/VC/LIB/LIBC.LIB')
    records={}
    for name,raw in members(path.read_bytes()):
        p=BUILD/'member.obj';p.write_bytes(raw)
        try:c=Coff(p)
        except ValueError:continue
        syms={}
        for s in c.symbols:
            if s.section>0 and s.value==0 and (s.is_function or s.storage==2):syms.setdefault(s.section,s.name)
        records[name]=[dict(name=syms.get(s.index,''),size=s.raw_size,relocs=len(s.relocations),
            raw_sha256=hashlib.sha256(s.raw).hexdigest()) for s in c.sections if s.raw_size and s.characteristics&0x20000000 and s.characteristics&0x1000]
    archives[release]=records
    small=[dict(member=k,entry=x) for k,v in records.items() for x in v if x['size']<16]
    out['archives'][release]=dict(library=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest(),
        ordinary_members=len(records),code_comdat_members=sum(bool(v) for v in records.values()),tiny_comdats=small)
    print(release,'tiny COMDATs',small,flush=True)
rich={m['module_index']:m for m in solver.data['rich_modules']}
crt_slots=defaultdict(list)
for i,x in enumerate(solver.items):
    if rich[x['module_index']]['library'].lower().endswith('libc.lib'):
        crt_slots[x['module_index']].append(i)
sp3_mismatches=[]
for module,slots in crt_slots.items():
    name=solver.items[slots[0]]['module'].replace('\\','/').split('/')[-1].lower()
    a=[(solver.items[i]['name'],solver.items[i]['size'],solver.items[i]['relocs']) for i in slots]
    b=[(x['name'],x['size'],x['relocs']) for x in archives['sp3'][name]]
    if a!=b:sp3_mismatches.append(dict(member=name,trace=a,archive=b))
out['sp3_reconstruction_mismatches']=sp3_mismatches
print('SP3 eligible block reconstruction mismatches',len(sp3_mismatches),flush=True)
assert not sp3_mismatches,sp3_mismatches[:1]
for release in ['sp2','rtm']:
    vector=[];changed=[];inserted=[]
    solver.keys[:]=solver.keys[:793]
    solver.gmap[:]=solver.gmap[:793]
    seen=set()
    for i,x in enumerate(solver.items):
        if x['module_index'] not in crt_slots:
            vector.append(i);continue
        module=x['module_index']
        if module in seen:continue
        seen.add(module)
        slots=crt_slots[module]
        name=x['module'].replace('\\','/').split('/')[-1].lower()
        oldbyname={solver.items[j]['name']:j for j in slots}
        replacements=archives[release].get(name,[])
        for new in replacements:
            import bisect
            key=(new['size'],new['relocs'])
            rank=bisect.bisect_left(solver.unique,key)
            rank=2*rank if rank<len(solver.unique) and solver.unique[rank]==key else 2*rank-1
            old=oldbyname.get(new['name'])
            if old is None:
                new_id=len(solver.keys);solver.keys.append(rank);solver.gmap.append(-1);vector.append(new_id)
                inserted.append(dict(member=name,entry=new))
            else:
                vector.append(old)
                if solver.keys[old]!=rank:changed.append(dict(slot=old,member=name,name=new['name'],key=key))
                solver.keys[old]=rank
        for old in slots:
            if solver.items[old]['name'] not in {n['name'] for n in replacements}:changed.append(dict(removed_slot=old,member=name))
    strict,band=solver.passes(vector)
    result=dict(release=release,scope='Conditional same loaded-member order; not a library extraction model or code-equivalence proof',
        count=len(vector),strict=strict,band=band,changed_keys=changed,inserted=inserted,signature=solver.signature(vector))
    out['conditional_replacements'].append(result)
    print(release,'conditional winner constraints',strict,band,'key changes',len(changed),'inserted',len(inserted),flush=True)
    # Restore pinned ranks before the next release.
    import bisect
    solver.keys[:]=[2*bisect.bisect_left(solver.unique,(x['size'],x['relocs'])) for x in solver.items]
(HERE/'solver-libraries.json').write_text(json.dumps(out,indent=2))
