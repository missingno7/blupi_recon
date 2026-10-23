"""Observe SP3 pressure-weighted priority updates and independent read counts.

Usage (capture names live under build/c2trace/cap):
  .pyenv\\python.exe tools/c2trace/cost_trace.py CAPTURE FUNCTION [--target GAME/EDITION/LANGUAGE] [--passes N[,N]|all]

The observer checks the native pressure multiplier and each priority delta,
counts IR reads independently of descriptor reference counts, and compares
function bytes plus named relocations with an unhooked replay of the same IL.
SP3 is pinned through profiles.py. Outputs go under build/c2trace/alloc-observers/.
Without --passes, micros are traced at every cost pass, ChangePhase at passes
10 and 14, and BlupiStep at every pass.
"""
import argparse
from pathlib import Path
import sys,json,shutil,time
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'build/c2trace/alloc-observers'
sys.path[:0]=[str(ROOT/'tools/c2trace'),str(ROOT/'tools')]
import profiles

def arguments():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('capture',help='capture name under build/c2trace/cap')
 parser.add_argument('function',help='decorated-name substring, or * for every function')
 parser.add_argument('--passes',help='comma-separated cost-pass numbers, or all')
 profiles.add_arguments(parser)
 args=parser.parse_args()
 if args.compiler not in (None,'sp3'):
  parser.error('cost tracing is calibrated only for VC5 SP3')
 if args.passes and args.passes.lower()!='all':
  try:
   parsed={int(x) for x in args.passes.split(',')}
   if not parsed or min(parsed)<1:raise ValueError
  except ValueError:
   parser.error('--passes must be all or a comma-separated list of positive integers')
 profiles.configure('sp3',args.target)
 profiles.require_sp3('pressure-weighted cost tracing')
 cap=profiles.get().work/'cap'/args.capture
 if Path(args.capture).name!=args.capture or not cap.is_dir():
  parser.error('capture must be an existing name under build/c2trace/cap')
 profiles.check_capture(cap)
 return args,cap

args,cap=arguments()
name,needle=args.capture,args.function
cost_filter=None if not args.passes or args.passes.lower()=='all' else {int(x) for x in args.passes.split(',')}
OUT.mkdir(parents=True,exist_ok=True)
import colour_trace as ct,cmpfunc
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
def desc(p,a):
 v=p.u32(a);kind=p.rd(v+4,1)[0];val=None
 if kind==13:
  u=p.u32(v+0x28)
  if p.rd(u+8,1)[0]==7:val=ct.signed(p.u32(u+0x18))
 return dict(ptr=hex(a),lr=p.u32(a+0x1c),kind=kind,value=val,refs=p.u32(a+0x24),
  priority=ct.signed(p.u32(a+0xc)),benefit=ct.signed(p.u32(a+0x3c)))
dest=OUT/(name+'-cost-replay');dest.mkdir(exist_ok=True)
for f in cap.iterdir():
 if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,dest/f.name)
st=dict(active=False,fn='',cost=0,ret=None);events=[];errors=[];pressure_checks=0;formula_checks=0;read_checks=0;started=time.monotonic()
DETAIL=(0x40f5c7,0x40f5f5,0x40f71e,0x40f72c,0x40f745,0x40f75a)
def disarm(p,a):
 if a in p.orig:p.wr(a,p.orig.pop(a))
def hit(p,c,a):
 global pressure_checks,formula_checks,read_checks
 if a==0x414a64:
  root=p.u32(0x48fb48);sy=p.u32(root);fn=p.rd(p.u32(sy+24),180).split(b'\0')[0].decode('ascii','replace')
  st.update(active=needle=='*' or needle in fn,fn=fn,root=root,cost=0)
  if st['active']:p.arm(0x40ee1d)
 elif a==0x42a8aa:
  for bp in (0x40ee1d,)+DETAIL:disarm(p,bp)
  st['active']=False
 elif not st['active']:return
 elif a==0x40ee1d:
  st['cost']+=1;st['ret']=p.u32(c.Esp);p.arm(st['ret'])
  st['deltas']={};st['initial']={}
  if args.passes is not None:
   st['measure']=args.passes.lower()=='all' or st['cost'] in cost_filter
  else:
   st['measure']=needle=='*' or needle=='BlupiStep' or st['cost'] in (10,14)
  if st['measure']:
   for bp in DETAIL:p.arm(bp)
 elif a==st['ret']:
  disarm(p,a)
  for bp in DETAIL:disarm(p,bp)
  if not st['measure']:return
  q=Snap(p);nodes={a:desc(q,a) for a in ct.ranges(q).values()}
  wanted={a:n for a,n in nodes.items() if n['kind']==13 and (needle=='*' or n['value'] in (0,1,10,1525))}
  # Verify counts from the IR, with no use of the descriptor's count field.
  verify={a:n for a,n in wanted.items() if needle=='*' or (n['value'],n['refs']) in ((0,22),(0,18),(1525,9),(1525,8),(1,49),(1,46),(1,338),(10,71),(10,11),(10,9))}
  counted={a:[] for a in verify}
  if verify:
   for bi,bb in enumerate(ct.blocks(q,st['root'])):
    end=q.u32(bb+0x20)
    for t in ct.chain(q,q.u32(bb+0x1c)):
     for u in ct.chain(q,q.u32(t+0x18)):
      r=q.u32(u+0x18)
      if q.rd(u+8,1)[0]==1 and r in counted:counted[r].append(dict(block=bi,line=q.u32(t+0x10)&65535,op=q.u32(t+4)))
     if t==end:break
  for a,n in wanted.items():
   record=dict(fn=st['fn'],cost=st['cost'],node=n)
   if a in st['deltas']:
    expected=st['initial'][a]+st['deltas'][a];record.update(predicted_priority=expected,priority_ok=n['priority']==expected)
    if not record['priority_ok']:errors.append(record)
   if a in verify:
    read_checks+=1;record.update(reads=counted[a],read_ok=n['refs']==len(counted[a]))
    if not record['read_ok']:errors.append(record)
   events.append(record)
 elif a==0x40f5c7:
  q=Snap(p);temp=len(ct.bitset(q,c.Ebp));defs=len(list(ct.chain(q,c.Ebx,0x14)))
  live=list(ct.chain(q,q.u32(c.Esp+0x10),0x14));used=sum(bool(q.rd(x+6,1)[0]&16) for x in live)
  st['pressure']=temp+defs+used;st['weight']=q.u32(c.Esp+0x28)
 elif a==0x40f5f5:
  pressure_checks+=1
  if (c.Esi,c.Edi)!=(st['pressure'],st['weight']):errors.append(dict(kind='pressure',actual=[c.Esi,c.Edi],expected=[st['pressure'],st['weight']]))
  st['factor']=c.Esi*c.Edi
 elif a==0x40f745:st['block_benefit']=p.u32(c.Eax+0x18)
 elif a in (0x40f71e,0x40f72c,0x40f75a):
  q=Snap(p);old=ct.signed(q.u32(c.Eax+0xc));b=q.u32(c.Eax+0x18)
  if a==0x40f75a:b=st['block_benefit']
  expected=(-st['factor'] if a==0x40f72c else st['factor']*b)
  actual=(-ct.signed(c.Edx) if a==0x40f72c else ct.signed(c.Edi if a==0x40f71e else c.Esi)-old)
  formula_checks+=1
  if actual!=expected:errors.append(dict(kind='delta',site=hex(a),actual=actual,expected=expected))
  st['initial'].setdefault(c.Eax,old);st['deltas'][c.Eax]=st['deltas'].get(c.Eax,0)+expected
obj=ct.owned_run(dest,[0x414a64,0x42a8aa],hit)
def payload(obj):
 cf=cmpfunc.Coff(obj)
 return {f.name:(f.data,[(r.offset-f.start,cf.sym_by_index[r.symbol_index].name,r.typ) for r in f.relocs]) for f in cf.functions()}
native=OUT/(name+'-cost-native');native.mkdir(exist_ok=True)
if not (native/'trace.obj').exists():
 for f in cap.iterdir():
  if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,native/f.name)
 native_obj=ct.owned_run(native,[],lambda *args:None)
else:native_obj=native/'trace.obj'
eq=payload(obj)==payload(native_obj)
suffix='-cost'+('-'+(args.passes.replace(',','-') if cost_filter else 'all') if args.passes is not None else '')+'.json'
(OUT/(name+suffix)).write_text(json.dumps(dict(events=events,pressure_checks=pressure_checks,formula_checks=formula_checks,read_checks=read_checks,errors=errors,payload_equal=eq,elapsed=time.monotonic()-started),indent=1))
print(name,'pressure',pressure_checks,'deltas',formula_checks,'readcounts',read_checks,'errors',len(errors),'payload',eq,'seconds',round(time.monotonic()-started,2))
for e in errors[:5]:print(e)
