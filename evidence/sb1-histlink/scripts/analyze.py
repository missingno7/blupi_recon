"""Read-only ICF trace analysis and VC5 QSORT.C replay; never a link input."""
import sys
sys.dont_write_bytecode = True
import json
import hashlib
from collections import defaultdict, Counter
from pathlib import Path
ROOT = Path(__file__).resolve().parents[2]
HERE = ROOT / 'scratch/w-sb1icf'
sys.path.insert(0, str(ROOT / 'tools'))
from cmpfunc import Coff
import relink

def qsort(a, key=lambda x: (x['size'], x['relocs'])):
    a = list(a)
    def sort(lo, hi):
        size = hi-lo+1
        if size < 2:
            return
        if size <= 8:
            while hi > lo:
                m = lo
                for p in range(lo+1, hi+1):
                    if key(a[p]) > key(a[m]):
                        m = p
                a[m], a[hi] = a[hi], a[m]
                hi -= 1
            return
        mid = lo + size//2
        a[mid], a[lo] = a[lo], a[mid]
        low, high = lo, hi+1
        while True:
            low += 1
            while low <= hi and key(a[low]) <= key(a[lo]):
                low += 1
            high -= 1
            while high > lo and key(a[high]) >= key(a[lo]):
                high -= 1
            if high < low:
                break
            a[low], a[high] = a[high], a[low]
        a[lo], a[high] = a[high], a[lo]
        sort(lo, high-1)
        sort(low, hi)
    sort(0, len(a)-1)
    return a

def trace(label):
    d = json.loads((ROOT / f'build/w-sb1icf-{label}/historical-host.json').read_text())
    return next(s for s in d['sorts'] if s['comparator'] == 0x432130)

def main():
    base, ext = trace('link-base'), trace('link-extern')
    out = {}
    for label, s in [('base', base), ('extern', ext)]:
        out[label] = dict(count=s['count'], replay_exact=(qsort(s['before']) == s['after']))
        print(label, out[label])
    changes = [dict(index=i, before=b, after=e) for i,(b,e) in enumerate(zip(base['before'], ext['before']))
        if (b['name'], b['size'], b['relocs']) != (e['name'], e['size'], e['relocs'])]
    out['input_changes'] = changes
    print('Changed ICF inputs:', [(x['index'], x['before']['name'], x['after']['name']) for x in changes])
    # Pointer-independent survivor ranks. Same-object aliases are not necessarily ICF.
    groups = [
        ['?GetHide@CJauge@@QAEHXZ', '?GetNbSessions@CNetwork@@QAEHXZ'],
        ['??1CButton@@QAE@XZ','??1CJauge@@QAE@XZ','??1CMenu@@QAE@XZ','?OutputDebug@@YAXPAD@Z'],
        ['?BuildUp@CEvent@@IAEHUtagPOINT@@H@Z','?PlayDown@CEvent@@IAEHUtagPOINT@@H@Z','?PlayMove@CEvent@@IAEHUtagPOINT@@H@Z','?PlayUp@CEvent@@IAEHUtagPOINT@@H@Z'],
        ['?GetEnable@CSound@@QAEHXZ','?GetNbProviders@CNetwork@@QAEHXZ','?GetTrueColorDecor@CPixmap@@QAEHXZ','?GetTrueColorBack@CPixmap@@QAEHXZ'],
        ['?FrameUnwindFilter@@YAHPAU_EXCEPTION_POINTERS@@@Z','?ArrayUnwindFilter@@YAHPAU_EXCEPTION_POINTERS@@@Z'],
        ['_strncnt'],
        ['?_CallMemberFunction0@@YGXPAX0@Z','?_CallMemberFunction1@@YGXPAX00@Z','?_CallMemberFunction2@@YGXPAX00H@Z'],
        ['__alloca_probe','__chkstk']]
    out['groups'] = []
    for names in groups:
        group = dict(names=names)
        for label,s in [('base',base),('extern',ext)]:
            group[label] = dict(initial=[dict(index=i,**{k:v for k,v in x.items() if k!='pointer'}) for i,x in enumerate(s['before']) if x['name'] in names],
                sorted=[dict(index=i,initial_index=next(j for j,y in enumerate(s['before']) if y['pointer']==x['pointer']), **{k:v for k,v in x.items() if k!='pointer'}) for i,x in enumerate(s['after']) if x['name'] in names])
        out['groups'].append(group)
        print('GROUP', names, '\n', group['base'], '\n', group['extern'])
    # Exact report-function comparison (object path envelopes deliberately excluded).
    a=json.loads((ROOT/'build/w-sb1icf-us-base/results.json').read_text())
    b=json.loads((ROOT/'build/w-sb1icf-us-extern/results.json').read_text())
    out['complete_per_symbol_function_records_identical']=[{f['name']:f for f in x['functions']} for x in a]==[{f['name']:f for f in x['functions']} for x in b]
    print('Complete per-symbol function records identical:',out['complete_per_symbol_function_records_identical'])
    # All file-static functions: emission-order evidence can expose further fixes.
    static=[]
    for p in sorted((ROOT/'build/w-sb1icf-us-base/objects').glob('[0-9]*.obj')):
        c=Coff(p)
        for s in c.symbols:
            if s.is_function and s.section>0 and s.storage==3:
                static.append(dict(object=p.name,name=s.name,section=s.section,symbol_index=s.index))
    out['file_static_functions']=static
    print('Static functions:',static)
    (HERE/'analysis.json').write_text(json.dumps(out,indent=2))

if __name__=='__main__': main()
