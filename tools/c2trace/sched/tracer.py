"""Observe c2 SP3 scheduling windows, DAG, ready queues and selected nodes.
No decision perturbations. Captures/replays are private; every process is reaped.
Usage: tracer.py CAP FUNCTION OUT.json [--line-min N --line-max N]
"""
import argparse
import json
from pathlib import Path
import struct
import lab
import model
dbg=lab.c2dbg
PHASE, WINDOW, READY, SELECT, INSERT = 0x40d9be,0x4315f1,0x43179a,0x431be3,0x43170c
PIPE_RETURN=0x431800

def u16(p,a): return int.from_bytes(p.rd(a,2),'little')
def cstr(p,a): return p.rd(a,250).split(b'\0')[0].decode('ascii','replace') if a else ''
def name(p):
    sym=p.u32(0x495f1c)
    return cstr(p,p.u32(sym+24)) if sym else ''
def tree(p,a,depth=0,seen=()):
    if not a: return None
    if depth>12 or a in seen:return {'ptr':hex(a),'truncated':True}
    n=struct.unpack('<12I',p.rd(a,48)); k=n[2]&255
    o={'ptr':hex(a),'kind':k,'op':n[1],'type':n[2]>>16}
    def child(b):return tree(p,b,depth+1,seen+(a,))
    if k in (1,2,3):
        d=n[5];o.update(desc=hex(d),desc_kind=p.rd(d+4,1)[0],id=p.u32(d+28),reg=p.u32(p.u32(d+8)+28))
        o['binding']=hex(n[6])
        if n[6] and p.rd(n[6]+4,1)[0]==1:
            o['assigned_register']=dbg.REG.get(p.u32(n[6]+28),p.u32(n[6]+28))
        if p.rd(d+4,1)[0]==1: o['register']=dbg.REG.get(p.u32(d+28),p.u32(d+28))
        else:
            s=p.u32(p.u32(d+8));o['symbol']=cstr(p,p.u32(s+24)) if s else ''
        if p.u32(d+20):o['expr']=child(p.u32(d+20))
    elif k==4:o['symbol']=cstr(p,p.u32(n[5]+24))
    elif k in (5,6):
        o.update(offset=lab.colour_trace.signed(n[8]),base=child(n[9]),index=child(n[10]),scale=n[4]&15)
    elif k==7:o['value']=lab.colour_trace.signed(n[6])
    else:o['raw']=[hex(v) for v in n[4:]]
    return o

def tup(p,a):
    n=struct.unpack('<10I',p.rd(a,40))
    return dict(ptr=hex(a),op=n[1],kind=n[2]&255,flags=hex(n[2]),line=n[4]&65535,
                inputs=operands(p,n[6]),outputs=operands(p,n[7]))
def operands(p,a):
    out=[]
    for b in lab.colour_trace.chain(p,a,limit=100):out.append(tree(p,b))
    return out
def node(p,a,edges=False):
    t=p.u32(a+28)
    row=dict(ptr=hex(a),tuple=tup(p,t) if t else None,priority=p.u32(a+44),
             base_priority=p.u32(a+40),earliest=p.u32(a+48),ordinal=u16(p,a+54),
             incoming=u16(p,a+32),outgoing=u16(p,a+34),height=u16(p,a+52),
             soft_incoming=u16(p,a+36),flags=p.rd(a+57,1)[0],flags2=p.rd(a+58,1)[0])
    if edges:
        row['edges']=[dict(ptr=hex(e),dest=hex(p.u32(e+12)),latency=u16(p,e+20),
             kind=p.rd(e+22,1)[0]&31,flags=hex(p.u32(e+16)))
             for e in lab.colour_trace.chain(p,p.u32(a+12),limit=2000)]
    if t:
        op=p.u32(t+4);table=p.u32(0x4930a4)
        row['pair_class']=p.u32(table+op*12+8) if op<0x11b else 0x300
        row['issue_cost']=p.u32(table+op*12+4) if op<0x11b else 0
        row['cost']=p.rd(a+56,1)[0]
    return row
def queue(p):return [node(p,a) for a in lab.colour_trace.chain(p,p.u32(0x493134),16,limit=2000)]

def trace(cap,selector,out,lo=0,hi=65535):
    events=[];errors=[];state={'active':False,'phase':None,'windows':0,'selected':False,'returns':{}}
    def ret(p,c,payload):
        a=p.u32(c.Esp); state['returns'].setdefault(a,[]).append(payload);p.arm(a)
    def hit(p,c,a):
        try:onhit(p,c,a)
        except Exception as e:
            if len(errors)<8:errors.append(hex(a)+': '+repr(e))
    def onhit(p,c,a):
        if a in state['returns'] and state['returns'][a]:
            what=state['returns'][a].pop()
            if isinstance(what,dict) and what['kind']=='insert':
                what['actual_queue']=queue(p);events.append(what)
            elif what=='window' and state['selected']:
                events.append(dict(stage='window-end',window=state['windows']))
                state['selected']=False
            return
        if a==PHASE:
            state['active']=selector in name(p);state['phase']=c.Edx
            if state['active']:events.append(dict(stage='phase',phase=c.Edx,function=name(p)))
            return
        if a not in (WINDOW,READY,SELECT,INSERT,PIPE_RETURN):return
        if not state['active']:return
        if a==WINDOW:
            state['windows']+=1
            start,end=p.u32(0x493224),p.u32(0x493228)
            tuples=[]
            for t in lab.colour_trace.chain(p,start,limit=200):
                tuples.append(tup(p,t))
                if t==end:break
            state['selected']=any(lo<=t['line']<=hi for t in tuples)
            events.append(dict(stage='window-lines',window=state['windows'],phase=state['phase'],
                               lines=[t['line'] for t in tuples]))
            if state['selected']:
                nodes=[node(p,n,True) for n in lab.colour_trace.chain(p,p.u32(c.Ecx),limit=2000)]
                row=dict(stage='window',window=state['windows'],phase=state['phase'],
                    graph=hex(c.Ecx),tuples=tuples,nodes=nodes,model=p.u32(0x49309c),
                    issue_width=p.u32(0x493078),tables=hex(p.u32(0x4930a4)),
                    pipe_picker=hex(p.u32(0x493074)),preallocation=p.u32(0x49321c),
                    weights=list(struct.unpack('<8h',p.rd(p.u32(0x493070),16))))
                row['prediction_before_schedule']=model.schedule(row)
                # Persist the prediction before c2 executes a scheduler decision.
                registered.write(json.dumps(dict(window=row['window'],prediction=row['prediction_before_schedule']))+'\n')
                registered.flush()
                events.append(row)
                ret(p,c,'window')
        elif state['selected']:
            if a==PIPE_RETURN:
                first=p.u32(0x4930b0)
                events.append(dict(stage='pipe-try',window=state['windows'],cycle=p.u32(0x4930ac),
                    slot=c.Ebp,node=node(p,c.Esi),actual=lab.colour_trace.signed((c.Eax&65535) | (0xffff0000 if c.Eax&32768 else 0)),
                    first=node(p,first) if c.Ebp and first else None,resources=[
                        dict(capacity=u16(p,0x4930e0+12*i),busy=lab.colour_trace.signed(p.u32(0x4930e8+12*i)))
                        for i in range(u16(p,0x493138))],rep_block=lab.colour_trace.signed(p.u32(0x493084))))
                return
            stage={READY:'cycle',SELECT:'select',INSERT:'insert'}[a]
            e=dict(stage=stage,window=state['windows'],cycle=p.u32(0x4930ac),ready=queue(p))
            if a in (SELECT,INSERT):e['node']=node(p,c.Ecx)
            events.append(e)
            if a==INSERT:ret(p,c,dict(kind='insert',stage='insert-result',window=state['windows'],node=e['node'],before=e['ready']))
    with open(str(out)+'.predictions.jsonl','w',encoding='utf-8') as registered:
        obj=lab.replay(cap,[PHASE,WINDOW,READY,SELECT,INSERT,PIPE_RETURN],hit)
    result=dict(events=events,errors=errors,object=str(obj),payload_equality=True)
    lab.write(out,result)
    if errors:raise RuntimeError(errors)
    print(str(out),len(events),'events; payload equality verified')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('cap');p.add_argument('function');p.add_argument('out')
    p.add_argument('--line-min',type=int,default=0);p.add_argument('--line-max',type=int,default=65535)
    a=p.parse_args();trace(Path(a.cap),a.function,Path(a.out),a.line_min,a.line_max)
