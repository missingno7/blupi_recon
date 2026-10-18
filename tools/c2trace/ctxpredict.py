"""Predict VC5 SP3 expression-order windows from ONE diagnostic IL capture.

Examples (run with the repo's .pyenv/python.exe):
  tools/c2trace/ctxpredict.py src/decmove.cpp MoveObjectSort
  tools/c2trace/ctxpredict.py <micro.cpp> <Function> --micro
  tools/c2trace/ctxpredict.py src/decblupi.cpp IsVentillo --capture symnum-macro0

This reports c2 comparator decisions, NOT strict object equality. A capture is
one compile, followed by one synchronous c2 debugger replay. No probe sweep.
For another project's input, --micro --sdk dx6sdk --include DIR supplies the
explicit context; all generated files stay in this repository.
"""
from pathlib import Path
import argparse,hashlib,json,os,subprocess,sys
from hashmodel import fold,hash16

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]

def leaves(t):
    if not t:return []
    out=[]
    if t.get('sym',{}).get('id') and not t.get('expr'):
        out.append(t['sym'])
    for x in [t.get('expr'),t.get('base'),t.get('index')]+t.get('inputs',[]):out+=leaves(x)
    return out

def shifted_key(t,n):
    # Preserve a cached key's constant part. c2 sometimes transforms a child
    # before refreshing its parent; do not mistake that for new symbol input.
    return t['key'] ^ hash16(t,0) ^ hash16(t,n)

def comparison(x,n=0):
    a,b=shifted_key(x['left'],n),shifted_key(x['right'],n)
    return (b>a)-(b<a)  # c2 408d58: negative means left key is larger

def bounds(x,n,limit):
    v=comparison(x,n);lo=hi=n
    ids=[s['id'] for s in leaves(x['left'])+leaves(x['right'])]
    floor=max(-limit,16-min(ids)) if ids else -limit
    while lo>floor and comparison(x,lo-1)==v:lo-=1
    while hi<limit and comparison(x,hi+1)==v:hi+=1
    return dict(lo=lo,hi=hi,left=n-lo,right=hi-n,
                left_censored=lo==floor,right_censored=hi==limit)

def decisions(data,func,delta=0,limit=1024):
    found={}
    for i,x in enumerate(data):
        if func not in x['function']:continue
        if x['caller']!='0x4058e1':continue
        a,b=x['left'],x['right']
        if (a['key']>>16)!=(b['key']>>16) or not a['key'] or not b['key']:continue
        symbols=sorted({(s['id'],s['name']) for s in leaves(a)+leaves(b)})
        if not symbols:continue
        if all(comparison(x,n)==comparison(x,delta) for n in range(-limit,limit+1)):continue
        # Retain different phases even when symbol dependencies coincide.
        sig=(tuple(symbols),a['key']>>16,a['key']^b['key'],hash16(a),hash16(b))
        if sig in found:continue
        found[sig]=dict(trace_index=i,function=x['function'],caller=x['caller'],
            symbols=[dict(id=n,name=s,shifted_id=n+delta) for n,s in symbols],
            left_key=hex(shifted_key(a,delta)),right_key=hex(shifted_key(b,delta)),
            comparator=comparison(x,delta),window=bounds(x,delta,limit),
            cached_key_consistent=(hash16(a)==(a['key']&65535) and hash16(b)==(b['key']&65535)))
    return list(found.values())

def main():
    p=argparse.ArgumentParser(description=__doc__,formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('tu');p.add_argument('function');p.add_argument('--capture')
    p.add_argument('--micro',action='store_true');p.add_argument('--sdk',default='dx5sdk')
    p.add_argument('--include',action='append',default=[])
    p.add_argument('--flag',action='append',default=[]);p.add_argument('--define',action='append',default=[])
    p.add_argument('--delta',type=int,default=0);p.add_argument('--radius',type=int,default=1024)
    p.add_argument('--json',type=Path);a=p.parse_args()
    name=a.capture or 'sympredict-'+hashlib.sha256((a.tu+'|'+a.function).encode()).hexdigest()[:12]
    if not a.capture:
        if a.micro:
            sys.path.insert(0,str(ROOT/'tools/c2trace'));import c2trace
            c2trace._fresh_il()
            src=Path(a.tu).resolve()
            cmd=[sys.executable,str(ROOT/'tools/msvc.py'),str(src),'--out',str(ROOT/'build/c2trace/predict'/('predict-'+name+'.obj')),
                 '--log-dir',str(ROOT/'build/c2trace/predict/logs'),'--include',str(src.parent),'--include',str(Path('C:/tools')/a.sdk/'INCLUDE')]
            for i in a.include:cmd+=['--include',i]
            for f in ['/W3','/GX','/O2','/ML','/Gd','/Gy','/B2'+str(c2trace.STUB)]+a.flag:cmd+=['--flag',f]
            for d in a.define:cmd+=['--define',d]
            subprocess.run(cmd,check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            c2trace._keep(name)
        else:
            subprocess.run([sys.executable,str(ROOT/'tools/c2trace/c2trace.py'),'capture',name,a.tu],check=True)
    trace=ROOT/'build/c2trace/order'/(name+'-order.json')
    if not trace.exists() or not a.capture:
        env=os.environ.copy();env['SYMNUM_FUNCTION']=a.function
        subprocess.run([sys.executable,str(HERE/'trace_order.py'),name],check=True,env=env)
    data=json.loads(trace.read_text())
    meta=ROOT/'build/c2trace/order'/(name+'-functions.json')
    functions=json.loads(meta.read_text()) if meta.exists() else {x['function']:{} for x in data}
    functions={k:v for k,v in functions.items() if a.function in k}
    if not functions:raise SystemExit('Requested function was not observed; check its name and emission/inlining.')
    rows=decisions(data,a.function,a.delta,a.radius)
    cap=ROOT/'build/c2trace/cap'/name
    gl=next(cap.glob('*gl')).read_bytes()
    # Record 0x11: version/mode fields then the raw next front-end number.
    fe_end=int.from_bytes(gl[7:11],'little') if gl[:1]==b'\x11' else None
    sys.path.insert(0,str(ROOT/'tools'));import cmpfunc
    coff=cmpfunc.Coff(cap/'trace.obj')
    static_symbols={}
    for f in coff.functions():
        if a.function in f.name:
            for r in f.relocs:
                s=coff.sym_by_index[r.symbol_index].name
                if '$S'in s:static_symbols[s]=int(s.rsplit('$S',1)[1])
    result=dict(tu=a.tu,function=a.function,capture=name,next_frontend_id=fe_end,delta=a.delta,
        function_counters=functions,static_symbols=static_symbols,
        scope='Uniform shift of real symbol IDs; fixed IR topology, costs and c2 allocation history.',
        decisions=rows,limits=['Comparator windows are not strict byte-match windows.',
            'A later pass can reverse an earlier choice; inspect every reported phase.',
            'The capture context must include the exact flags, headers, PCH and source.',
            'No lexical declaration estimator or general register-allocation predictor is claimed.'])
    if a.json:a.json.write_text(json.dumps(result,indent=2)+'\n')
    print('next front-end ID:',fe_end,'delta:',a.delta,'context-sensitive comparisons:',len(rows))
    if functions:print('function counters:',functions)
    if static_symbols:print('static symbol suffixes:',static_symbols)
    for x in rows:
        syms=', '.join('%s=%d'%(s['name'] or '?',s['shifted_id']) for s in x['symbols'])
        w=x['window'];print('  #%d keys %s / %s cmp=%d: delta [%d,%d], left %d / right %d; %s'%(
            x['trace_index'],x['left_key'],x['right_key'],x['comparator'],w['lo'],w['hi'],w['left'],w['right'],syms))
    print('Conditional comparator prediction; not a strict-match claim. '+('No changing key found in the radius.' if not rows else ''))

if __name__=='__main__':main()
