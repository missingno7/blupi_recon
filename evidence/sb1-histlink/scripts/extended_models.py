"""Two bounded current-SB2-form models; no source/link mutations."""
import sys
sys.dont_write_bytecode=True
import json,re,bisect
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
from cmpfunc import Coff
import solver
original=solver.items
original_names={x['name']:i for i,x in enumerate(original)}
original_keys=solver.keys[:793]
original_groups=solver.gmap[:793]
history=json.loads((HERE/'solver-source-models.json').read_text())['history'][0]
rows=history['new_functions']
historical_functions=Coff(ROOT/'build/w-sb1icf-source-keys/history-decmove-60.obj').functions()
current=Coff(ROOT/'build/w-sb1icf-solver-us-sb2forms/objects/16_decmove.obj')
current_keys={f.name:(f.section.raw_size,len(f.relocs)) for f in current.functions()}
def reset(names,keys):
    solver.keys[:]=original_keys
    solver.gmap[:]=original_groups
    result={}
    for name,key in zip(names,keys):
        rank=bisect.bisect_left(solver.unique,tuple(key))
        result[name]=len(solver.keys)
        solver.keys.append(2*rank if rank<len(solver.unique) and solver.unique[rank]==tuple(key) else 2*rank-1)
        solver.gmap.append(-1)
    return result
keys=[current_keys.get(r['name'],tuple(r['key'])) for r in rows]
ids=reset([r['name'] for r in rows],keys)
old=[i for i,x in enumerate(original) if Path(x['module']).name=='16_decmove.obj']
solutions=[]
for mask in range(1,1<<len(rows)):
    names={r['name'] for n,r in enumerate(rows) if mask>>n&1}
    replacement=[original_names[f.name] if f.name in original_names else ids[f.name]
        for f in historical_functions if f.name in original_names or f.name in names]
    vector=list(range(old[0]))+replacement+list(range(old[-1]+1,793))
    strict,band=solver.passes(vector)
    if band:
        closed=all(dep in original_names or dep in names for r in rows if r['name'] in names for dep in r['cdecor_dependencies'])
        solutions.append(dict(names=sorted(names),strict=strict,closed=closed))
(HERE/'solver-corrected-family.json').write_text(json.dumps(dict(
    scope='Eight current compiled keys plus four historical measured keys; not all twelve current forms',
    keys=keys,tested=4095,solutions=solutions),indent=2))
print('Mixed current/historical key model:',len(solutions),'solutions;',sum(x['closed'] for x in solutions),'dependency closed')

new_names=[f.name for f in current.functions() if f.name not in original_names]
ids=reset(new_names,[current_keys[n] for n in new_names])
short={n.split('@')[0][1:]:n for n in new_names}
order=re.findall(r'^(?:void|BOOL|int|RECT|POINT)\s+CDecor::(\w+)\s*\(',
    (ROOT/'games/sb2/src/decblupi.cpp').read_text(),re.M)
db=[(i,x['name'].split('@')[0][1:]) for i,x in enumerate(original) if Path(x['module']).name=='17_decblupi.obj']
lookup={name:i for i,name in db}
vector=list(range(793));insertions=[]
for name in ['FlushBalleTraj','SetBalleTraj','IsBalleTraj','FlushMoveTraj','SetMoveTraj','IsMoveTraj']:
    pos=order.index(name)
    anchor=next((lookup[n] for n in order[pos+1:] if n in lookup),None)
    destination=vector.index(anchor) if anchor is not None else vector.index(db[-1][0])+1
    vector.insert(destination,ids[short[name]])
    insertions.append(dict(name=name,before_original_slot=anchor,destination=destination))
for name in ['AscenseurShift','MoveAscenseurDetect']:
    symbol=short[name];pos=next(i for i,f in enumerate(historical_functions) if f.name==symbol)
    anchor=next((original_names[f.name] for f in historical_functions[pos+1:] if f.name in original_names),None)
    destination=vector.index(anchor) if anchor is not None else len(vector)
    vector.insert(destination,ids[symbol]);insertions.append(dict(name=name,before_original_slot=anchor,destination=destination))
result=dict(scope='Current measured SB1 keys for SB2 helper forms; six placed before their next shared decblupi source neighbour without reordering existing SB1 functions. Existing keys held fixed; not a compiled TU-move proof.',
    insertions=insertions,passes=solver.passes(vector),signature=solver.signature(vector))
(HERE/'solver-sb2-tu-model.json').write_text(json.dumps(result,indent=2))
print('Current SB2 TU grouping model:',result['passes'],result['signature'])
