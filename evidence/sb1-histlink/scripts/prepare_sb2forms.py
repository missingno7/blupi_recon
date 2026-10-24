"""Separate source-authenticity control using the current SB2 helper forms."""
import sys
sys.dont_write_bytecode=True
import re,json,shutil,difflib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
dest=HERE/'solver-tail8-sb2forms'
shutil.copytree(HERE/'solver-tail8',dest,dirs_exist_ok=True)
shutil.copyfile(HERE/'solver-tail8.units.json',HERE/'solver-tail8-sb2forms.units.json')
names=['FlushBalleTraj','SetBalleTraj','IsBalleTraj','FlushMoveTraj','SetMoveTraj','IsMoveTraj','AscenseurShift','MoveAscenseurDetect']
def end(text,m):
    depth=1;i=m.end()
    while depth:
        if text[i]=='{':depth+=1
        elif text[i]=='}':depth-=1
        i+=1
    return i
text=(dest/'decmove.cpp').read_bytes().decode('latin1')
provenance={}
accepted=json.loads((ROOT/'games/sb2/recon.json').read_text())['accepted_functions']
for name in names:
    pattern=r'^(?:void|BOOL|int)\s+CDecor::'+name+r'\([^;{}]*\)\s*\{'
    matches=[]
    for p in (ROOT/'games/sb2/src').glob('*.cpp'):
        raw=p.read_bytes().decode('latin1');m=re.search(pattern,raw,re.M)
        if m:matches.append((p,raw,m))
    assert len(matches)==1,(name,len(matches))
    p,source,m=matches[0]
    old=re.search(pattern,text,re.M);assert old
    text=text[:old.start()]+source[m.start():end(source,m)]+text[end(text,old):]
    provenance[name]=dict(source=str(p.relative_to(ROOT)),accepted_entries=[a for a in accepted if a['symbol'].startswith('?'+name+'@CDecor@@')])
(dest/'decmove.cpp').write_bytes(text.encode('latin1'))
p=dest/'decor.h';p.write_bytes(p.read_bytes().replace(b'MAXCELX * MAXCELY / 8 + 1',b'((TRAJBYTEX) * MAXCELY)'))
(HERE/'solver-sb2forms-provenance.json').write_text(json.dumps(provenance,indent=2))
(HERE/'solver-tail8-sb2forms.diff').write_text(''.join(''.join(difflib.unified_diff(
    (ROOT/'src'/n).read_text().splitlines(True),(dest/n).read_text().splitlines(True),fromfile='src/'+n,tofile='src/'+n))
    for n in ['network.cpp','network.h','decor.h','decmove.cpp']))
print('Prepared current SB2 helper forms; SB1 tail placement remains explicitly hypothetical.')
