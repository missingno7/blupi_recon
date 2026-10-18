"""Observe c2's expression-order comparator; no debuggee mutations except breakpoints."""
from pathlib import Path
import sys,json,struct,os
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools/c2trace'))
import c2dbg
SELECTORS=[os.environ['SYMNUM_FUNCTION']] if 'SYMNUM_FUNCTION' in os.environ else ['MoveObjectSort','MockeryDetect','IsDeepWater','IsVentillo','Target']

def cstr(p,a): return p.rd(a,180).split(b'\0')[0].decode('ascii','replace') if a else ''
def symbol(p,s):
    return dict(id=p.u32(s+40),name=cstr(p,p.u32(s+24)))
def tree(p,a,depth=0):
    if not a or depth>18: return None
    raw=struct.unpack('<20I',p.rd(a,80))
    kind=raw[2]&255
    d=dict(kind=kind,op=raw[1],key=raw[3])
    if kind in [1,2,3]:
        desc=raw[5]
        d['sym']=symbol(p,p.u32(p.u32(desc+8)))
        d['desc_kind']=p.rd(desc+4,1).hex()
        d['desc_number']=p.u32(desc+28)
        d['expr']=tree(p,p.u32(desc+20),depth+1)
    elif kind==4:
        d['sym']=symbol(p,raw[5])
    elif kind in [5,6]:
        d['offset']=raw[8]
        d['base']=tree(p,raw[9],depth+1)
        d['index']=tree(p,raw[10],depth+1)
    elif kind==7:
        d['value']=raw[6:8]
    elif kind==8:
        d['value_bytes']=p.rd(raw[5],12).hex()
    else:
        children=[]
        a=raw[6]
        for _ in range(80):
            if not a: break
            children.append(tree(p,a,depth+1)); a=p.u32(a)
        d['inputs']=children
    return d

for name in sys.argv[1:]:
    out=[]
    functions={}
    def hit(p,c,a):
        f=p.u32(0x48fb48)
        s=p.u32(f)
        fn=cstr(p,p.u32(s+24))
        if a==0x42491e:
            if any(x in fn for x in SELECTORS):
                functions[fn]=dict(frontend_id=p.u32(s+40),next_backend_id=p.u32(0x491050))
                p.arm(0x408d58)
            elif 0x408d58 in p.orig:
                p.wr(0x408d58,p.orig.pop(0x408d58))
            return
        if any(x in fn for x in SELECTORS):
            out.append(dict(function=fn,caller=hex(p.u32(c.Esp)),left=tree(p,c.Ecx),right=tree(p,c.Edx)))
    c2dbg.run(ROOT/'build/c2trace/cap'/name,[0x42491e],hit)
    out_dir=ROOT/'build/c2trace/order'; out_dir.mkdir(parents=True,exist_ok=True)
    dest=out_dir/(name+'-order.json')
    dest.write_text(json.dumps(out,indent=1))
    (out_dir/(name+'-functions.json')).write_text(json.dumps(functions,indent=1))
    print(name,len(out),'comparisons',dest.name)
