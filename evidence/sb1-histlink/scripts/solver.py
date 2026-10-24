"""Exact VC5 qsort inverse diagnostics. Model vectors are never linker inputs.

One unknown insertion represents a distinct, non-equivalent executable COMDAT.
Only its comparison key is specified; liveness does not affect qsort. Do not
interpret a satisfying vector edit as source evidence or an accepted build.
"""
import sys
sys.dont_write_bytecode=True
import json,time,random,bisect
from pathlib import Path
from collections import defaultdict
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
from analyze import qsort
data=json.loads((ROOT/'build/w-sb1icf-trace-extern/historical-host.json').read_text())
trace=next(s for s in data['sorts'] if s['comparator']==0x432130)
items=trace['before']
unique=sorted({(x['size'],x['relocs']) for x in items})
keys=[2*bisect.bisect_left(unique,(x['size'],x['relocs'])) for x in items]+[0]
groups=defaultdict(set)
for fold in data['folds']:groups[fold['keeper']].update([fold['keeper'],fold['folded']])
conid={x['con']:i for i,x in enumerate(items)}
classes=[{conid[c] for c in cs} for cs in groups.values()]
wanted_names=['?_CallMemberFunction0@@YGXPAX0@Z','??1CMenu@@QAE@XZ','?GetEnable@CSound@@QAEHXZ',
    '?PlayUp@CEvent@@IAEHUtagPOINT@@H@Z','?GetHide@CJauge@@QAEHXZ',
    '?ArrayUnwindFilter@@YAHPAU_EXCEPTION_POINTERS@@@Z','_strncnt']
targets=[]
for cs,name in zip(classes,wanted_names):
    candidates=[i for i in cs if items[i]['name']==name and (name!='_strncnt' or items[i]['module'].endswith('aw_map.obj'))]
    assert len(candidates)==1,(cs,name)
    targets.append(candidates[0])
gmap=[-1]*794
for g,cs in enumerate(classes):
    for i in cs:gmap[i]=g
interesting=sorted({keys[x] for x in targets})
MINKEY,MAXKEY=interesting[0],interesting[-1]

def sort_relevant(vector):
    """Same swaps as QSORT.C; skip only disjoint key ranges without constraints."""
    a=list(vector)
    def sort(lo,hi,klo,khi):
        if hi<=lo or khi<MINKEY or klo>MAXKEY:return
        size=hi-lo+1
        if size<=8:
            while hi>lo:
                m=lo
                for p in range(lo+1,hi+1):
                    if keys[a[p]]>keys[a[m]]:m=p
                a[m],a[hi]=a[hi],a[m]
                hi-=1
            return
        mid=lo+size//2
        a[mid],a[lo]=a[lo],a[mid]
        low,high=lo,hi+1
        pivot=keys[a[lo]]
        while True:
            low+=1
            while low<=hi and keys[a[low]]<=pivot:low+=1
            high-=1
            while high>lo and keys[a[high]]>=pivot:high-=1
            if high<low:break
            a[low],a[high]=a[high],a[low]
        a[lo],a[high]=a[high],a[lo]
        sort(lo,high-1,klo,pivot)
        sort(low,hi,pivot,khi)
    sort(0,len(a)-1,-1,2*len(unique)+2)
    return a

def signature(vector,full=False):
    order=(qsort(vector,key=lambda i:keys[i]) if full else sort_relevant(vector))
    wins=[-1]*len(classes)
    for i in order:
        g=gmap[i]
        if g>=0 and wins[g]<0:wins[g]=i
    return wins

def passes(vector):
    wins=signature(vector)
    # Strict preserves the current same-TU label PlayUp as well. Band constraint
    # allows its indistinguishable siblings, still retaining CallMemberFunction0.
    band=wins[3]>=0 and all(w==t for g,(w,t) in enumerate(zip(wins,targets)) if g!=3)
    return band and wins[3]==targets[3],band

def ranges(values):
    result=[]
    for v in sorted(set(values)):
        if result and v==result[-1][1]+1:result[-1][1]=v
        else:result.append([v,v])
    return result

def entry(i):
    x=items[i]
    return dict(slot=i,name=x['name'],module=x['module'],size=x['size'],relocs=x['relocs'],live=bool(int(x['flags'],16)&0x2000))

def validate():
    assert [items[i] for i in qsort(list(range(793)),key=lambda i:keys[i])]==trace['after']
    random.seed(510)
    for n in range(600):
        a=list(range(793))
        if n%3==0:a.pop(random.randrange(793))
        elif n%3==1:a.insert(random.randrange(794),a.pop(random.randrange(793)))
        else:
            keys[793]=random.randrange(2*len(unique)+1)
            a.insert(random.randrange(794),793)
        assert signature(a)==signature(a,True),n
    print('Validated full endpoint and 600 modified-vector constraint signatures.',flush=True)

def cells():
    rows=[dict(rank=-1,representative=[1,0],lower_inclusive=[1,0],upper_exclusive=list(unique[0]))]
    for i,k in enumerate(unique):
        rows.append(dict(rank=2*i,representative=list(k),exact=list(k)))
        if i+1<len(unique):
            right=unique[i+1]
            if k[0]==right[0] and right[1]<=k[1]+1:continue
            representative=(k[0],k[1]+1) if k[1]<65535 else (k[0]+1,0)
            if representative>=right:continue
            rows.append(dict(rank=2*i+1,representative=list(representative),lower_exclusive=list(k),upper_exclusive=list(right)))
        else:rows.append(dict(rank=2*i+1,representative=[k[0]+1,0],lower_exclusive=list(k),upper_exclusive=None))
    return rows

def save(name,value):
    (HERE/('solver-'+name+'.json')).write_text(json.dumps(value,indent=2))

def main():
    mode=sys.argv[1] if len(sys.argv)>1 else 'remove'
    validate()
    save('constraints',dict(count=793,classes=[[entry(i) for i in sorted(cs)] for cs in classes],
        targets=[entry(i) for i in targets],key_cells=cells(),base_signature=signature(list(range(793)))))
    start=time.time()
    if mode=='remove':
        rows=[]
        for i in range(793):
            a=list(range(793));a.pop(i)
            strict,band=passes(a)
            if band:rows.append(dict(entry=entry(i),strict=strict))
        save('remove',dict(tested=793,seconds=time.time()-start,solutions=rows))
        print('remove:',len(rows),'band solutions;',sum(x['strict'] for x in rows),'strict',flush=True)
    elif mode=='remove2':
        rows=[]
        count=0
        protected={t for g,t in enumerate(targets) if g!=3}
        for i in range(793):
            if i in protected:continue
            strict_positions=[];band_positions=[]
            for j in range(i+1,793):
                if j in protected:continue
                a=list(range(793));a.pop(j);a.pop(i)
                strict,band=passes(a);count+=1
                if band:band_positions.append(j)
                if strict:strict_positions.append(j)
            if band_positions:rows.append(dict(entry=entry(i),strict_seconds=ranges(strict_positions),band_seconds=ranges(band_positions)))
        save('remove2',dict(tested=count,seconds=time.time()-start,solutions=rows))
        print('remove pairs:',count,'families',len(rows),'seconds',round(time.time()-start,1),flush=True)
    elif mode=='insert':
        rows=[]
        allcells=cells()
        for n,cell in enumerate(allcells):
            keys[793]=cell['rank']
            strict_positions=[];band_positions=[]
            for p in range(794):
                a=list(range(793));a.insert(p,793)
                strict,band=passes(a)
                if band:band_positions.append(p)
                if strict:strict_positions.append(p)
            if band_positions:rows.append(dict(cell=cell,strict_positions=ranges(strict_positions),band_positions=ranges(band_positions)))
            if n%20==0 or n==len(allcells)-1:
                save('insert',dict(cells_completed=n+1,cells_total=len(allcells),tested=(n+1)*794,seconds=time.time()-start,solutions=rows))
                print('insert cells',n+1,'/',len(allcells),'solution families',len(rows),'seconds',round(time.time()-start,1),flush=True)
    elif mode=='move':
        rows=[]
        for i in range(793):
            short=list(range(793));short.pop(i)
            strict_positions=[];band_positions=[]
            for p in range(793):
                if p==i:continue
                a=short[:];a.insert(p,i)
                strict,band=passes(a)
                if band:band_positions.append(p)
                if strict:strict_positions.append(p)
            if band_positions:rows.append(dict(entry=entry(i),strict_positions=ranges(strict_positions),band_positions=ranges(band_positions)))
            if i%40==0 or i==792:
                save('move',dict(entries_completed=i+1,entries_total=793,tested=(i+1)*792,seconds=time.time()-start,solutions=rows))
                print('move entries',i+1,'/793','families',len(rows),'seconds',round(time.time()-start,1),flush=True)
    elif mode=='members':
        blocks=[]
        for i,x in enumerate(items):
            if not blocks or blocks[-1]['module_index']!=x['module_index']:blocks.append(dict(module=x['module'],module_index=x['module_index'],slots=[]))
            blocks[-1]['slots'].append(i)
        remove=[];move=[]
        for block in blocks:
            ids=block['slots'];ss=set(ids)
            short=[i for i in range(793) if i not in ss]
            strict,band=passes(short)
            if band:remove.append(dict(block=block,strict=strict))
            # Destination boundary before another actual module in the reduced vector.
            destinations=[0]+[p for p in range(1,len(short)) if items[short[p]]['module_index']!=items[short[p-1]]['module_index']]+[len(short)]
            solutions=[]
            for p in destinations:
                a=short[:p]+ids+short[p:]
                strict,band=passes(a)
                if band:solutions.append(dict(destination=p,strict=strict))
            if solutions:move.append(dict(block=block,solutions=solutions))
        save('members',dict(blocks=blocks,remove_solutions=remove,move_solutions=move,seconds=time.time()-start))
        print('member removals',len(remove),'member move families',len(move),flush=True)
    else:raise ValueError(mode)

if __name__=='__main__':main()
