"""Observe upstream cuts, region adoption and rematerialization with VC5 SP3.

Usage:
  .pyenv\\python.exe tools/c2trace/cut_trace.py CAPTURE [--target GAME/EDITION/LANGUAGE] [--prioflip]

CAPTURE names an existing directory under build/c2trace/cap. This observer
checks 439619 adoption, 45a444 singleton cuts, 437d7c cut members,
41b06a instruction pressure, and 4380cf liveness-mask construction from the
captured IL. The existing cost_trace.py and split_trace.py cover weighted
cost/delta accounting and downstream flow/partition checks separately. The
legacy JSON key "before" is an insertion anchor: 401462 inserts AFTER it.
All owned replay files and JSON outputs stay under
build/c2trace/alloc-observers/. No original game executable is launched.
"""
import argparse
from pathlib import Path
import sys, json, shutil, time
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/c2trace/alloc-observers'
sys.path[:0]=[str(ROOT/'tools/c2trace'),str(ROOT/'tools')]
import profiles

def arguments():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('capture',help='capture name under build/c2trace/cap')
 parser.add_argument('--prioflip',action='store_true',help='diagnostic: force zero22 priority to -79 and compare with a control replay')
 profiles.add_arguments(parser)
 args=parser.parse_args()
 if args.compiler not in (None,'sp3'):
  parser.error('cut/adoption tracing is calibrated only for VC5 SP3')
 profiles.configure('sp3',args.target)
 profiles.require_sp3('cut/adoption/rematerialization tracing')
 cap=profiles.get().work/'cap'/args.capture
 if Path(args.capture).name!=args.capture or not cap.is_dir():
  parser.error('capture must be an existing name under build/c2trace/cap')
 profiles.check_capture(cap)
 return args,cap

args,cap=arguments()
name,diagnostic=args.capture,args.prioflip
label=name+('-prioflip' if diagnostic else '')
OUT.mkdir(parents=True,exist_ok=True)
import colour_trace as ct, cmpfunc

class Snap:
 def __init__(self,p):self.p=p;self.pages={};self.alloc_cache={}
 def rd(self,a,n):
  b=bytearray()
  while n:
   page,off=a&~4095,a&4095
   if page not in self.pages:self.pages[page]=self.p.rd(page,4096)
   k=min(n,4096-off);b+=self.pages[page][off:off+k];a+=k;n-=k
  return bytes(b)
 def u32(self,a):return int.from_bytes(self.rd(a,4),'little')
 def u8(self,a):return self.rd(a,1)[0]

def bs(p,a):return set(ct.bitset(p,a))
def operands(p,t,off):return list(ct.chain(p,p.u32(t+off)))
def tuples(p,b):
 end=p.u32(b+0x20)
 for t in ct.chain(p,p.u32(b+0x1c)):
  if t==end:break
  yield t
def lr(p,a):
 v=p.u32(a);kind=p.u8(v+4);val=None
 if kind==13:
  u=p.u32(v+0x28)
  if p.u8(u+8)==7:val=ct.signed(p.u32(u+0x18))
 return dict(ptr=hex(a),lr=p.u32(a+0x1c),kind=kind,value=val,
  source_id=p.u32(v+0x1c),refs=p.u32(a+0x24),priority=ct.signed(p.u32(a+0xc)),
  benefit=ct.signed(p.u32(a+0x3c)),available=sorted(bs(p,p.u32(a+0x20))),
  binding=p.u32(a+0x10))
def tup(p,t):return dict(tuple=hex(t),op=p.u32(t+4),line=p.u32(t+0x10)&65535)
def operand(p,u):
 tag=p.u8(u+8);a=p.u32(u+0x18)
 row=dict(tag=tag,flags=p.u32(u+0x10),value=a)
 if tag==1:
  k=p.u8(a+4);row.update(kind=k,identity=p.u32(a+0x1c))
  if k==2:row['range']=lr(p,a)
 return row

def adopt(p,s,b,available,mode,universe,singleton,rematerialize=False):
 """439619: per-block allowed-colour scan; input sets copied, no native writes."""
 first=bs(p,p.u32(b+0x24)); remain=set(available);cuts=[];steps=[]
 if rematerialize:remain=set(universe)-first
 def cut(t,reason):
  merge=reason=='no-exit-colour' and p.u8(t+8)==26
  cuts.append(dict(before=hex(t),forced=sorted(singleton),reason=reason,action='merge' if merge else 'insert',
   members_before=sorted(bs(p,p.u32(t+0x14))) if merge else []))
 for t in tuples(p,b):
  if not p.u8(t+9)&1:
   if rematerialize:continue
   if p.u8(t+8)==26 and p.u32(s+0x1c) in bs(p,p.u32(t+0x14)):
    if mode:remain.clear()
    else:return dict(result=0,available=sorted(available),cuts=cuts,steps=steps,reason='existing-cut')
   continue
  reads=operands(p,t,0x18);writes=operands(p,t,0x1c)
  # Native pre-scans for selected reads before processing any read last-use.
  if p.u32(t+4)!=0x13a:
   for u in reads:
    if p.u8(u+8)!=1 or p.u32(u+0x18)!=s:continue
    if remain:
     available=set(remain)
     steps.append(dict(**tup(p,t),reason='selected-read',remaining=sorted(remain),available=sorted(available)))
    else:
     remain=set(universe)-first;cut(p.u32(t+0xc),'no-colour-before-read')
     if not mode:return dict(result=0,available=sorted(available),cuts=cuts,steps=steps,reason='no-colour-before-read')
  for u in reads:
   if p.u8(u+8)!=1:continue
   a=p.u32(u+0x18);k=p.u8(a+4)
   last=p.u8(u+0x11)&16 and not p.u8(u+0x10)&128
   if k==2:
    reg=p.u32(a+0x10)
    if reg and last:first.discard(p.u32(reg+0x1c))
   elif k==1:
    reg=p.u32(a+8);aux=p.u32(u+0x14)
    if p.u8(aux+4)!=3:remain.discard(p.u32(reg+0x1c))
    elif last:first.discard(p.u32(reg+0x1c))
  if p.u8(t+0x12)&2 and not p.u8(s+6)&2:
   nonbyte=bs(p,p.u32(0x4931c8));remain &= nonbyte
   if not remain:
    remain=(set(universe)-first)&nonbyte;cut(p.u32(t+0xc),'byte-constraint')
    if not mode:return dict(result=0,available=sorted(available),cuts=cuts,steps=steps,reason='byte-constraint')
  for u in writes:
   tag=p.u8(u+8)
   if tag==9:remain-=bs(p,p.u32(u+0x14))
   if tag!=1:continue
   a=p.u32(u+0x18);k=p.u8(a+4)
   if k==2:
    reg=p.u32(a+0x10)
    if reg:remain.discard(p.u32(reg+0x1c));first.add(p.u32(reg+0x1c))
   elif k==1:
    reg=p.u32(a+8);remain.discard(p.u32(reg+0x1c))
    if p.u8(p.u32(u+0x14)+4)==3:first.add(p.u32(reg+0x1c))
  if p.u32(t+4)!=0x139 and any(p.u8(u+8)==1 and p.u32(u+0x18)==s and not p.u8(u+0x10)&15 for u in writes):
   remain=set(universe)-first
   if not mode:return dict(result=0,available=sorted(available),cuts=cuts,steps=steps,reason='selected-write')
  steps.append(dict(**tup(p,t),reason='tuple-end',first=sorted(first),remaining=sorted(remain),available=sorted(available)))
 if rematerialize:return dict(cuts=cuts,steps=steps,first=sorted(first),remaining=sorted(remain))
 if remain:
  available = set(remain);result=1;reason='adopt'
 else:
  cut(p.u32(p.u32(b+0x20)+0xc),'no-exit-colour');result=0;reason='no-exit-colour'
 return dict(result=result,available=sorted(available),cuts=cuts,steps=steps,reason=reason)

def masks(p,head,batch):
 """4380cf: masks from native tuples, cut membership and the batch bit map."""
 data={};extra=0
 for link in ct.chain(p,head):
  b=p.u32(link+8);G=U=K=A=H=0
  for t in reversed(list(tuples(p,b))):
   if not p.u8(t+9)&1:
    if p.u8(t+8)==26:
     if not p.u8(t+0x12):
      members=bs(p,p.u32(t+0x14))&batch&bs(p,p.u32(b+0x48))
      m=0
      for n in members:m|=p.u32(ct.ranges(p)[n]+0x10)
     else:
      m=0
      for x in ct.chain(p,p.u32(t+0x14)):m|=p.u32(p.u32(x+4)+0x10)
     U&=~m;K|=m;A|=m
    continue
   for u in operands(p,t,0x1c):
    if p.u8(u+8)!=1:continue
    a=p.u32(u+0x18)
    if p.u8(a+4)!=2 or not p.u8(a+5)&2:continue
    m=p.u32(a+0x10)
    if not K&m and not p.u8(u+0x10)&15:G|=m;K|=m
    U&=~m;A&=~m;H|=m;extra|=p.u32(u+0xc)
   for u in operands(p,t,0x18):
    tag=p.u8(u+8)
    if tag==1:
     a=p.u32(u+0x18)
     if p.u8(a+4)!=2 or not p.u8(a+5)&2:continue
     m=p.u32(a+0x10)
     if not K&m:G|=m
     U|=m
     if p.u8(u+0x11)&16:H|=m;A&=~m
    elif tag==2 and p.u32(u+0xc):A|=p.u32(ct.ranges(p)[p.u32(u+0xc)]+0x10)
  data[b]={0x2c:G,0x30:U,0x34:K,0x50:A,0x54:H,0x38:U}
 return data,extra

dest=OUT/(label+'-cut-replay');dest.mkdir(exist_ok=True)
for f in cap.iterdir():
 if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,dest/f.name)
st=dict(active=False,cost=0,selection=None,adopt=None,remat=None,returns={})
events=[];errors=[];checks={};started=time.monotonic()
DETAIL=(0x41a985,0x439385,0x439619,0x437d7c,0x4380cf,0x40ee1d,0x41b06a,0x45a444,0x45a4a7)
if diagnostic:DETAIL+=(0x41b6fa,)
def disarm(p,a):
 if a in p.orig:p.wr(a,p.orig.pop(a))
def check(kind,ok,**detail):
 checks[kind]=checks.get(kind,0)+1
 if not ok:errors.append(dict(kind=kind,**detail))
def ret(p,c,what):
 a=p.u32(c.Esp);st['returns'].setdefault(a,[]).append(what);p.arm(a)
def hit(p,c,a):
 if a==0x414a64:
  root=p.u32(0x48fb48);sy=p.u32(root);fn=p.rd(p.u32(sy+24),180).split(b'\0')[0].decode('ascii','replace')
  st.update(active='ChangePhase@' in fn,root=root,fn=fn,cost=0,selection=None,adopt=None,remat=None)
  if st['active']:
   for bp in DETAIL:p.arm(bp)
  return
 if a==0x42a8aa:
  for bp in DETAIL:disarm(p,bp)
  disarm(p,0x41b06a)
  st['active']=False;return
 if not st['active']:return
 q=Snap(p)
 if a==0x41b6fa:
  n=lr(q,c.Ecx)
  if (n['value'],n['refs'])==(0,22) and not st.get('forced'):
   p.wr(c.Ecx+0xc,(-79&0xffffffff).to_bytes(4,'little'));st['forced']=True
   events.append(dict(stage='diagnostic-force',before=n,priority=-79))
  return
 if a in st['returns']:
  what=st['returns'][a].pop()
  if not st['returns'][a]:del st['returns'][a];disarm(p,a)
  if what['type']=='adopt':
   disarm(p,0x439804)
   e=what['event'];model=e['predicted'];actual=sorted(bs(q,e.pop('_available_ptr')))
   e.update(result=c.Eax,available_after=actual)
   check('adopt-result',c.Eax==model['result'],event=e)
   check('adopt-available',actual==model['available'],event=e)
   actualcuts=[dict(before=x['tuple'],forced=x['prediction']['forced']) for x in e['native_cuts']]
   expected=[dict(before=x['before'],forced=x['forced']) for x in model['cuts'] if x['action']=='insert']
   check('adopt-cuts',actualcuts==expected,event=e)
   for x in model['cuts']:
    if x['action']=='merge':
     actual=bs(q,q.u32(int(x['before'],16)+0x14));expected=set(x['members_before'])|set(x['forced'])
     check('adopt-merge',actual==expected,event=e,actual=sorted(actual),expected=sorted(expected))
   st['adopt']=None
  elif what['type']=='remat':
   e=what['event'];model=e['predicted']
   actual=[dict(before=x['tuple'],forced=x['prediction']['forced']) for x in e['native_cuts']]
   expected=[dict(before=x['before'],forced=x['forced']) for x in model['cuts']]
   check('remat-cuts',actual==expected,event=e)
   st['remat']=None
  elif what['type']=='cut':
   e=what['event'];actual=sorted(bs(q,q.u32(c.Eax+0x14))) if c.Eax else []
   e.update(actual=actual);check('cut-members',actual==e['prediction']['expected'],event=e)
  elif what['type']=='masks':
   for b,fields in what['data'].items():
    for off,expected in fields.items():check('mask-field',q.u32(b+off)==expected,block=st['bi'].get(b),offset=hex(off),expected=expected,actual=q.u32(b+off))
   check('mask-extra',c.Eax==what['extra'],expected=what['extra'],actual=c.Eax)
  return
 if a==0x41a985:
  st['selection']=lr(q,c.Edx);st['blocks']=ct.blocks(q,st['root']);st['bi']={b:i for i,b in enumerate(st['blocks'])}
  n=st['selection'];st['detail_pressure']=(n['source_id']==9 or n['value']==0 and n['refs'] in (18,21,22,23) or n['value']==1525 and n['refs'] in (8,9))
  if st['detail_pressure']:p.arm(0x41b06a)
  else:disarm(p,0x41b06a)
  st['tm']={}
  for i,b in enumerate(st['blocks']):
   for t in tuples(q,b):st['tm'][t]=i
   st['tm'][q.u32(b+0x20)]=i
  events.append(dict(stage='selection',cost=st['cost'],node=st['selection']))
 elif a==0x439385:
  events.append(dict(stage='region-start',cost=st['cost'],node=lr(q,c.Ecx),available=sorted(bs(q,c.Edx))))
 elif a==0x439619:
  s,b=c.Ecx,c.Edx;avail=q.u32(c.Esp+4);mode=q.u32(c.Esp+12);universe=bs(q,q.u32(c.Esp+16));singleton=bs(q,q.u32(c.Esp+8))
  e=dict(stage='adopt',cost=st['cost'],node=lr(q,s),block=st['bi'].get(b),mode=mode,
   available_before=sorted(bs(q,avail)),entry_available=sorted(bs(q,q.u32(b+0x24))),universe=sorted(universe),singleton=sorted(singleton),
   predicted=adopt(q,s,b,bs(q,avail),mode,universe,singleton),native_cuts=[],_available_ptr=avail)
  events.append(e);st['adopt']=e;ret(p,c,dict(type='adopt',event=e))
  if st['bi'].get(b)==176 and e['node']['value']==1 and e['node']['refs']==13 and mode:
   e['debug']=[];p.arm(0x439804)
 elif a==0x437d7c:
  e=dict(stage='cut',cost=st['cost'],selection=st['selection'],caller=hex(q.u32(c.Esp)),
   block=st.get('tm',{}).get(c.Ecx),**tup(q,c.Ecx),prediction=ct.cut_prediction(q,c.Ecx,c.Edx,q.u32(c.Esp+4),q.u32(c.Esp+8)))
  events.append(e)
  if st['adopt']:st['adopt']['native_cuts'].append(e)
  if st['remat']:st['remat']['native_cuts'].append(e)
  ret(p,c,dict(type='cut',event=e))
 elif a==0x45a444:
  b,s=c.Ecx,c.Edx;universe=bs(q,q.u32(c.Esp+4));singleton={q.u32(s+0x1c)}
  e=dict(stage='rematerialize',cost=st['cost'],node=lr(q,s),block=st['bi'].get(b),universe=sorted(universe),entry_available=sorted(bs(q,q.u32(b+0x24))),
   predicted=adopt(q,s,b,set(),1,universe,singleton,True),native_cuts=[])
  events.append(e);st['remat']=e;ret(p,c,dict(type='remat',event=e))
 elif a==0x45a4a7:
  e=st['remat'];actual=dict(first=sorted(bs(q,c.Ebx)),remaining=sorted(bs(q,c.Ebp)))
  e['actual']=actual
  for key,value in actual.items():check('remat-'+key,value==e['predicted'][key],event=e)
 elif a==0x4380cf:
  st['blocks']=ct.blocks(q,st['root']);st['bi']={b:i for i,b in enumerate(st['blocks'])}
  data,extra=masks(q,c.Ecx,bs(q,c.Edx));ret(p,c,dict(type='masks',data=data,extra=extra))
 elif a==0x40ee1d:
  st['cost']+=1
 elif a==0x41b06a:
  base=q.u32(c.Esp+0x70);defs=q.u32(c.Esp+0x18);reads=q.u32(c.Esp+0x28)
  expected=base+max(defs,reads);check('instruction-pressure',c.Edi==expected,actual=c.Edi,predicted=expected)
  events.append(dict(stage='instruction-pressure',cost=st['cost'],selection=st['selection'],block=st['bi'].get(q.u32(c.Esp+0x40)),**tup(q,c.Ebx),base=base,definitions=defs,reads=reads,pressure=c.Edi,capacity=c.Eax,
   physical=sorted(bs(q,q.u32(c.Esp+0x44))),pending=sorted(bs(q,q.u32(c.Esp+0x20))),already_cut=bool(q.u8(c.Ebx+0x12)&1),
   read_operands=[operand(q,u) for u in operands(q,c.Ebx,0x18)],write_operands=[operand(q,u) for u in operands(q,c.Ebx,0x1c)]))
 elif a==0x439804:
  st['adopt']['debug'].append(dict(**tup(q,c.Ebp),first=sorted(bs(q,q.u32(c.Esp+0x10))),remaining=sorted(bs(q,c.Esi))))

obj=ct.owned_run(dest,[0x414a64,0x42a8aa],hit)
def payload(obj):
 cf=cmpfunc.Coff(obj)
 return {f.name:(f.data,[(r.offset-f.start,cf.sym_by_index[r.symbol_index].name,r.typ) for r in f.relocs]) for f in cf.functions()}
native=OUT/(name+'-cut-native');native.mkdir(exist_ok=True)
if not (native/'trace.obj').exists():
 for f in cap.iterdir():
  if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,native/f.name)
 native_obj=ct.owned_run(native,[],lambda *args:None)
else:native_obj=native/'trace.obj'
eq=payload(obj)==payload(native_obj)
control_equal=None
if diagnostic:
 control=OUT/(label+'-control');control.mkdir(exist_ok=True)
 for f in cap.iterdir():
  if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,control/f.name)
 force=dict(active=False,done=False)
 def force_only(p,c,a):
  if a==0x414a64:
   root=p.u32(0x48fb48);sy=p.u32(root);fn=p.rd(p.u32(sy+24),180).split(b'\0')[0]
   force['active']=b'ChangePhase@' in fn
   if force['active']:p.arm(0x41b6fa)
  elif a==0x42a8aa:disarm(p,0x41b6fa);force['active']=False
  elif a==0x41b6fa and force['active']:
   n=lr(Snap(p),c.Ecx)
   if (n['value'],n['refs'])==(0,22) and not force['done']:
    p.wr(c.Ecx+0xc,(-79&0xffffffff).to_bytes(4,'little'));force['done']=True
 control_obj=ct.owned_run(control,[0x414a64,0x42a8aa],force_only)
 control_equal=payload(obj)==payload(control_obj)
result=dict(events=events,checks=checks,errors=errors,payload_equal=eq,diagnostic_only=diagnostic,control_payload_equal=control_equal,elapsed=time.monotonic()-started)
(OUT/(label+'-cut.json')).write_text(json.dumps(result,indent=1))
print(label,'checks',checks,'errors',len(errors),'payload',eq,'control',control_equal,'seconds',round(result['elapsed'],2))
for e in errors[:5]:print(str(e)[:1400])
