"""Map compiled ancestry functions to solver constraints without linking."""
import sys
sys.dont_write_bytecode=True
import json,itertools,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
from cmpfunc import Coff
from analyze import qsort
import solver
items=solver.items
baseline=list(range(793))
gmap0=solver.gmap[:]
keys0=solver.keys[:793]
def quantize(k):
    import bisect
    i=bisect.bisect_left(solver.unique,tuple(k))
    return 2*i if i<len(solver.unique) and solver.unique[i]==tuple(k) else 2*i-1

original={}
for p in (ROOT/'build/w-sb1icf-us-extern/objects').glob('[0-9]*.obj'):
    c=Coff(p)
    for f in c.functions():
        original[f.name]=(f.data,[(r.offset-f.start,r.typ,c.sym_by_index[r.symbol_index].name) for r in f.relocs])
group_bodies={}
for g,cs in enumerate(solver.classes):
    for i in cs:
        name=items[i]['name']
        if name in original:group_bodies[original[name][0]]=g

models=json.loads((HERE/'solver-source-keys.json').read_text())
newitems=[]
for row in models:
    if row['exit']:continue
    c=Coff(Path(row['object']))
    funcs={f.name:f for f in c.functions()}
    row['existing_payload_differences']=[]
    for name,f in funcs.items():
        payload=(f.data,[(r.offset-f.start,r.typ,c.sym_by_index[r.symbol_index].name) for r in f.relocs])
        if name in original and payload!=original[name]:row['existing_payload_differences'].append(name)
    for x in row['comdats']:
        sec=c.sections[x['section']-1]
        x['id']=len(newitems)+793
        x['existing']=x['name'] in original
        x['group']=group_bodies.get(sec.raw,-1) if not sec.relocations else -1
        x['source_label']=row['label']
        x['cdecor_dependencies']=sorted({c.sym_by_index[r.symbol_index].name for r in sec.relocations
            if '@CDecor@@' in c.sym_by_index[r.symbol_index].name and c.sym_by_index[r.symbol_index].is_function})
        newitems.append(x)
solver.keys[:]=keys0+[quantize(x['key']) for x in newitems]
solver.gmap[:]=gmap0[:793]+[x['group'] for x in newitems]
def passes(a):return solver.passes(a)
boundaries=[0]+[i for i in range(1,464) if i==463 or items[i]['module']!=items[i-1]['module']]
out=dict(game_module_boundaries=boundaries,models=models,standalone=[],history=[],subsets=[])
for row in models:
    if row['exit']:continue
    entries=row['comdats']
    if row['label'].startswith('history-'):
        stem='text' if row['label']=='history-text' else 'decmove'
        old=[i for i,x in enumerate(items) if Path(x['module']).stem.endswith('_'+stem)]
        oldnames={items[i]['name']:i for i in old}
        new=[x for x in entries if not x['existing']]
        solutions=[]
        # Every subset retains the historical definition order. Existing entry
        # keys are held fixed here; full-TU payload differences stay explicit.
        for mask in range(1,1<<len(new)):
            wanted={x['id'] for n,x in enumerate(new) if mask>>n&1}
            replacement=[oldnames[x['name']] if x['name'] in oldnames else x['id'] for x in entries if x['name'] in oldnames or x['id'] in wanted]
            if len({oldnames[x['name']] for x in entries if x['name'] in oldnames})!=len(old):continue
            a=baseline[:old[0]]+replacement+baseline[old[-1]+1:]
            strict,band=passes(a)
            if band:solutions.append(dict(added=[x['name'] for x in new if x['id'] in wanted],strict=strict))
        dependency_closed=[s for s in solutions if all(dep in original or dep in s['added']
            for x in new if x['name'] in s['added'] for dep in x['cdecor_dependencies'])]
        out['history'].append(dict(label=row['label'],new_functions=new,solutions=solutions,dependency_closed_solutions=dependency_closed,
            existing_payload_differences=row['existing_payload_differences']))
        print(row['label'],'new',[(x['name'],x['key']) for x in new], 'solutions',len(solutions),'dependency closed',len(dependency_closed),flush=True)
    else:
        ids=[x['id'] for x in entries]
        solutions=[]
        if not any(x['existing'] for x in entries):
            for p in range(794):
                strict,band=passes(baseline[:p]+ids+baseline[p:])
                if band:solutions.append(dict(position=p,natural_module_boundary=p in boundaries,strict=strict))
        out['standalone'].append(dict(label=row['label'],solutions=solutions,collisions=[x['name'] for x in entries if x['existing']]))
        print(row['label'],'whole TU solutions',solutions[:25],'total',len(solutions),flush=True)
        # A bounded ancestry-subset screen: one/two functions, kept in source
        # order, as an unanchored dead TU at a legal game-object boundary.
        eligible=[x for x in entries if not x['existing']]
        solutions=[]
        for n in [1,2]:
            for combo in itertools.combinations(eligible,n):
                ids=[x['id'] for x in combo]
                for p in boundaries:
                    strict,band=passes(baseline[:p]+ids+baseline[p:])
                    if band:solutions.append(dict(names=[x['name'] for x in combo],position=p,strict=strict))
        out['subsets'].append(dict(label=row['label'],solutions=solutions))
        print(row['label'],'1/2-function boundary subset solutions',len(solutions),flush=True)
(HERE/'solver-source-models.json').write_text(json.dumps(out,indent=2))
