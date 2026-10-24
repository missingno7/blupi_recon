"""Factorial, model-only diagnosis of the two measured callback order swaps.

No model vector is supplied to LINK, no generated reorder candidate exists.
"""
import sys
sys.dont_write_bytecode=True
import json
import hashlib
from collections import defaultdict, Counter
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
from analyze import qsort

def load(mode):
    d=json.loads((ROOT/f'build/w-sb1icf-trace-{mode}/historical-host.json').read_text())
    return d,next(s for s in d['sorts'] if s['comparator']==0x432130)

base,bs=load('base')
ext,es=load('extern')
groups=defaultdict(set)
for f in ext['folds']:
    groups[f['keeper']].update([f['keeper'],f['folded']])
ident={x['con']:(x['name'],x['module'].replace('\\','/').split('/')[-1]) for x in es['before']}
classes=[{ident[c] for c in cs} for cs in groups.values()]
def identity(x): return x['name'],x['module'].replace('\\','/').split('/')[-1]
rows=[]
for providers,sessions in [(False,False),(True,False),(False,True),(True,True)]:
    vector=list(bs['before'])
    if providers: vector[358],vector[359]=vector[359],vector[358]
    if sessions: vector[364],vector[365]=vector[365],vector[364]
    sorted_vector=qsort(vector)
    row=dict(providers_external=providers,sessions_external=sessions,
        winners=[identity(next(x for x in sorted_vector if identity(x) in cs)) for cs in classes])
    rows.append(row)
out=dict(scope='Model-only; the two endpoints validated against full real traces and fold events.',
    groups=[sorted(cs) for cs in classes],factorial=rows,
    changed_slots=[dict(index=i,base=identity(bs['before'][i]),extern=identity(es['before'][i]),
        before_key=(bs['before'][i]['size'],bs['before'][i]['relocs']),
        after_key=(es['before'][i]['size'],es['before'][i]['relocs'])) for i in [358,359,364,365]])
for mode,d,s in [('base',base,bs),('extern',ext,es)]:
    prediction=qsort(s['before'])
    assert prediction==s['after']
    actual=defaultdict(set)
    for f in d['folds']: actual[f['keeper']].add(f['folded'])
    actual_validation=[]
    for k,fs in actual.items():
        expected=next(x for x in prediction if x['con'] in fs|{k})['con']
        assert k==expected
        actual_validation.append(dict(keeper=identity(next(x for x in prediction if x['con']==k)),fold_count=len(fs)))
    out[mode]=dict(full_permutation_exact=True,all_folds_first_sorted_equivalent=True,folds=actual_validation,
        population_count=s['count'],live_bit_set=sum(bool(int(x['flags'],16)&0x2000) for x in s['before']),
        code_comdats=sum(bool(int(x['flags'],16)&0x1000) for x in s['before']),
        module_count=len(d['rich_modules']),stamps=dict(Counter(x['compiler_stamp'] for x in d['rich_modules'])),
        trace_sha256=hashlib.sha256((ROOT/f'build/w-sb1icf-trace-{mode}/BLUPI.EXE').read_bytes()).hexdigest(),
        reference_sha256=hashlib.sha256((ROOT/f'build/w-sb1icf-link-{mode}/BLUPI.EXE').read_bytes()).hexdigest())
    assert out[mode]['trace_sha256']==out[mode]['reference_sha256']
(HERE/'causality.json').write_text(json.dumps(out,indent=2))
print(json.dumps(rows,indent=2))
