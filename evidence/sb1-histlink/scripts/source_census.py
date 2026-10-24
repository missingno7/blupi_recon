"""Source ancestry/history census, no external-tree modifications."""
import sys
sys.dont_write_bytecode=True
import json,subprocess,re
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
OUT=HERE/'solver-sources'
OUT.mkdir(exist_ok=True)
def git(*args,cwd=ROOT):
    return subprocess.check_output(['git',*args],cwd=cwd)
manifest=json.loads((ROOT/'recon.json').read_text())
accepted={x['symbol'] for x in manifest['accepted_functions'] if not x.get('withdrawn')}
trace=json.loads((ROOT/'build/w-sb1icf-trace-extern/historical-host.json').read_text())
items=next(s for s in trace['sorts'] if s['comparator']==0x432130)['before']
dead=[dict(slot=i,**x,accepted=x['name'] in accepted) for i,x in enumerate(items)
    if 'w-sb1icf' in x['module'] and not(int(x['flags'],16)&0x2000)]
(HERE/'solver-dead-game.json').write_text(json.dumps(dead,indent=2))
print('Dead game COMDATs:',[(x['slot'],x['name']) for x in dead])
history={}
for commit in ['f47a4e6','60c7fab','6b1345f','6ad87f1','699e45d']:
    patch=git('show','--format=fuller',commit,'--','src').decode('utf-8',errors='replace')
    (OUT/(commit+'.diff')).write_text(patch)
    names=[line for line in patch.splitlines() if line.startswith(('+','-')) and re.search(r'\b(?:CDecor::|CPixmap::|Draw.*Big|Get.*Big)',line)]
    history[commit]=names
    print(commit, names[:45])
pb=ROOT/'external/planetblupi'
tree=git('ls-tree','-r','--name-only','b0270ab',cwd=pb).decode().splitlines()
paths=[p for p in tree if Path(p).name in ['fifo.cpp','fifo.h','arrange.cpp','arrange.h','obstacle.cpp','obstacle.h','action.cpp','action.h','text.cpp','pixmap.cpp','sound.cpp','misc.cpp','movie.cpp']]
for name in paths:
    dest=OUT/'pb';dest.mkdir(exist_ok=True)
    (dest/Path(name).name).write_bytes(git('show','b0270ab:'+name,cwd=pb))
print('Historical PB files',paths)
(HERE/'solver-source-census.json').write_text(json.dumps(dict(dead_game=dead,history=history,pb_paths=paths),indent=2))
