"""Positive verified-call witnesses across nine original SB1 builds.

Absence of a witness is not proof of absence/liveness: unmatched callers and
inlining are outside this census. No linker/game executable is run.
"""
import sys
sys.dont_write_bytecode=True
import json
from pathlib import Path
from collections import defaultdict
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
import targets
dead=json.loads((HERE/'solver-dead-game.json').read_text())
deadnames={x['name'] for x in dead}
rows={}
for name in [n for n in targets.names() if n.startswith('sb1/')]:
    slug='us' if name=='sb1/blupi/us' else name.replace('/','-')
    results=json.loads((ROOT/'build'/('w-sb1icf-'+slug+'-base')/'results.json').read_text())
    refs=defaultdict(list)
    for unit in results:
        for f in unit['functions']:
            if f['status'] not in ['STRICT','FOLDED']:continue
            for match in f['matches']:
                for r in match.get('relocations',[]):
                    if r.get('verified') and r['symbol'] in deadnames:
                        refs[r['symbol']].append(dict(caller=f['name'],caller_rva=match['rva'],target_va=r.get('effective_target_va')))
    rows[name]=dict(function_count=sum(len(u['functions']) for u in results),positive_references_to_primary_ref_dead_functions=dict(refs))
primary=rows['sb1/blupi/us']['positive_references_to_primary_ref_dead_functions']
new=[dict(target=t,symbol=n,witnesses=w) for t,row in rows.items() for n,w in row['positive_references_to_primary_ref_dead_functions'].items() if n not in primary]
(HERE/'solver-sb1-family-refs.json').write_text(json.dumps(dict(scope=__doc__,targets=rows,secondary_only_positive_witnesses=new),indent=2))
print('Reconstructed baseline function sets all',sorted({x['function_count'] for x in rows.values()}),'functions; secondary-only positive witnesses:',new)
