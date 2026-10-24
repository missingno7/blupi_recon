"""Source-backed solver hypothesis: trajectory six + elevator Shift/Detect.

Exact historical definitions and their original relative positions, no stubs
or dummy references. The named tail view is explicitly a hypothesis; the
unassigned remainder preserves the independently proven storage extent.
"""
import sys
sys.dont_write_bytecode=True
import re,json,shutil,difflib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
dest=HERE/'solver-tail8'
shutil.copytree(HERE/'extern',dest,dirs_exist_ok=True)
old=HERE/'solver-sources/history-60c7fab-parent'
names=['FlushBalleTraj','SetBalleTraj','IsBalleTraj','FlushMoveTraj','SetMoveTraj','IsMoveTraj','AscenseurShift','MoveAscenseurDetect']
def functions(text):
    rows=[]
    for m in re.finditer(r'^(?:void|BOOL|int|RECT|POINT)\s+CDecor::(\w+)\([^;{}]*\)\s*\{',text,re.M):
        depth=1;i=m.end()
        while depth:
            if text[i]=='{':depth+=1
            elif text[i]=='}':depth-=1
            i+=1
        rows.append((m[1],m.start(),i,text[m.start():i]))
    return rows
text=(dest/'decmove.cpp').read_bytes().decode('latin1')
historical=(old/'decmove.cpp').read_bytes().decode('latin1')
rows=functions(historical)
for n,(name,start,end,body) in enumerate(rows):
    if name not in names:continue
    assert not re.search(r'\bCDecor::'+name+r'\s*\(',text)
    next_pos=None
    for later,*_ in rows[n+1:]:
        m=re.search(r'^(?:void|BOOL|int|RECT|POINT)\s+CDecor::'+later+r'\s*\(',text,re.M)
        if m:next_pos=m.start();break
    if next_pos is None:text+='\r\n\r\n'+body+'\r\n'
    else:text=text[:next_pos]+body+'\r\n\r\n'+text[next_pos:]
(dest/'decmove.cpp').write_bytes(text.encode('latin1'))
header=(dest/'decor.h').read_bytes().decode('latin1')
oldheader=(old/'decor.h').read_bytes().decode('latin1')
lines=oldheader.splitlines(True)
for n,line in enumerate(lines):
    m=re.search(r'\b('+'|'.join(names)+r')\s*\(',line)
    if not m or re.search(r'\b'+m[1]+r'\s*\(',header):continue
    next_pos=None
    for later in lines[n+1:]:
        mm=re.search(r'\b(\w+)\s*\(',later)
        if not mm:continue
        actual=re.search(r'\b'+mm[1]+r'\s*\(',header)
        if actual:
            next_pos=header.rfind('\n',0,actual.start())+1;break
    assert next_pos is not None,m[1]
    header=header[:next_pos]+line+header[next_pos:]
needle='\tBYTE\t\tm_unknownTail[100044];'
assert needle in header
replacement='''\t// [HYPOTHESIS, scratch only] SB2-derived trajectory storage; no SB1 access proves these names/offsets.
\tint         m_blupiTimeNoAsc;
\tBYTE        m_balleTraj[MAXCELX * MAXCELY / 8 + 1];
\tBYTE        m_moveTraj[MAXCELX * MAXCELY / 8 + 1];
\t// [EVIDENCE] The complete tail remains the independently proven 100,044 bytes.
\tBYTE        m_unknownTail[100044 - sizeof(int) - 2 * (MAXCELX * MAXCELY / 8 + 1)];'''.replace('\n','\r\n')
header=header.replace(needle,replacement)
(dest/'decor.h').write_bytes(header.encode('latin1'))
src=ROOT/'src'
def affected(p,seen=None):
    seen=set() if seen is None else seen
    if p.name in ['network.h','decor.h']:return True
    if p.name in seen or not p.is_file():return False
    seen.add(p.name)
    return any(affected(src/x,seen) for x in re.findall(r'#\s*include\s*["<]([^">]+)[">]',p.read_text(errors='replace')))
manifest=json.loads((ROOT/'recon.json').read_text())
units=[u['source'] for u in manifest['translation_units'] if affected(ROOT/u['source'])]
(HERE/'solver-tail8.units.json').write_text(json.dumps(units,indent=2))
diff=''.join(''.join(difflib.unified_diff((src/name).read_text().splitlines(True),
    (dest/name).read_text().splitlines(True),fromfile='src/'+name,tofile='src/'+name))
    for name in ['network.cpp','network.h','decor.h','decmove.cpp'])
(HERE/'solver-tail8.diff').write_text(diff)
control=HERE/'solver-declarations-only'
shutil.copytree(dest,control,dirs_exist_ok=True)
shutil.copyfile(HERE/'extern/decmove.cpp',control/'decmove.cpp')
(HERE/'solver-declarations-only.units.json').write_text(json.dumps(units,indent=2))
print('Prepared eight exact historical bodies, source order preserved; header dependents:',units)
