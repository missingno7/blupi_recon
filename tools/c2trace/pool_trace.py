"""Observe and independently predict the whole SP3 literal-pooling predicate.

Usage (capture names live under build/c2trace/cap):
  .pyenv\\python.exe tools/c2trace/pool_trace.py CAPTURE FUNCTION [--target GAME/EDITION/LANGUAGE]

The model is pinned to VC5 SP3 through profiles.py. It checks every native
eligibility return, then compares the hooked object payload and named
relocations with an unhooked replay of the same captured IL. Replay receipts
are written under build/c2trace/alloc-observers/.
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
 profiles.add_arguments(parser)
 args=parser.parse_args()
 if args.compiler not in (None,'sp3'):
  parser.error('pooling eligibility is calibrated only for VC5 SP3')
 profiles.configure('sp3',args.target)
 profiles.require_sp3('pooling eligibility')
 cap=profiles.get().work/'cap'/args.capture
 if Path(args.capture).name!=args.capture or not cap.is_dir():
  parser.error('capture must be an existing name under build/c2trace/cap')
 profiles.check_capture(cap)
 return args,cap

args,cap=arguments()
name,needle=args.capture,args.function
OUT.mkdir(parents=True,exist_ok=True)
import colour_trace as ct, cmpfunc

class Snap:
 def __init__(self,p):self.p=p; self.pages={}; self.alloc_cache={}
 def rd(self,a,n):
  b=bytearray()
  while n:
   page,off=a&~4095,a&4095
   if page not in self.pages:self.pages[page]=self.p.rd(page,4096)
   k=min(n,4096-off); b+=self.pages[page][off:off+k]; a+=k;n-=k
  return bytes(b)
 def u32(self,a):return int.from_bytes(self.rd(a,4),'little')

def operand(p,u):
 tag=p.rd(u+8,1)[0]; v=p.u32(u+0x18)
 d=dict(tag=tag,value=v,type=p.u32(u+8)>>16,flags=p.u32(u+0x10),ptr=hex(u))
 if tag==1:d.update(kind=p.rd(v+4,1)[0],desc=hex(v),backlink=hex(p.u32(v+0x14)))
 return d

def tup(p,t):
 return dict(ptr=hex(t),op=p.u32(t+4),flags=hex(p.u32(t+8)),line=p.u32(t+0x10)&65535,
  reads=[operand(p,u) for u in ct.chain(p,p.u32(t+0x18),limit=100)],
  writes=[operand(p,u) for u in ct.chain(p,p.u32(t+0x1c),limit=100)])

REJECT={4,6,8,10,11,*range(0x25,0x2c),0xa3,0xc1,0xca,0xcb,0x109,0x13c,0x14c}
def predict(t):
 op=t['op']; r=t['reads']; w=t['writes']
 if op in REJECT:return 0,'opcode'
 if op==1 and r[0]['tag']==7 and r[0]['value']==0 and w[0]['tag']==1 and w[0]['kind']!=2:return 0,'zero-copy-virtual'
 if op==0x2f and len(r)>1 and r[1]['tag']==7 and r[1]['value']==0 and r[0]['tag']==1 and r[0]['kind']!=2:return 0,'zero-rhs-virtual'
 if op in (0x2d,0x32) and w and w[0]['tag']==1 and w[0]['value'] in (0x494938,0x4948e8):return 0,'fixed-result'
 return 1,'default'

dest=OUT/(name+'-pool-replay')
dest.mkdir(parents=True,exist_ok=True)
for f in cap.iterdir():
 if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,dest/f.name)
st=dict(active=False,fn='',pending=None,intern=None,cost=False); events=[]; errors=[]; started=time.monotonic()
def disarm(p,a):
 if a in p.orig:p.wr(a,p.orig.pop(a))
def hit(p,c,a):
 if a==0x414a64:
  root=p.u32(0x48fb48); sy=p.u32(root); fn=p.rd(p.u32(sy+24),180).split(b'\0')[0].decode('ascii','replace')
  st.update(active=needle=='*' or needle in fn,fn=fn,root=root,cost=False)
  if st['active']:
   for bp in (0x4156b6,0x414ddb,0x414dff,0x417fb7,0x40ee1d):p.arm(bp)
 elif a==0x42a8aa:
  for bp in (0x4156b6,0x414ddb,0x414dff,0x417fb7,0x40ee1d):disarm(p,bp)
  st['active']=False
 elif not st['active']:return
 elif a==0x4156b6:
  q=Snap(p); t=tup(q,c.Ecx); pred,reason=predict(t)
  st['pending']=dict(stage='eligibility',fn=st['fn'],tuple=t,predicted=pred,reason=reason)
 elif a in (0x414ddb,0x414dff):
  e=st['pending']; e['actual']=c.Eax; e['ok']=c.Eax==e['predicted']; events.append(e)
  if not e['ok']:errors.append(e)
 elif a==0x417fb7:
  q=Snap(p);events.append(dict(stage='intern',fn=st['fn'],tuple=tup(q,c.Ecx),operand=operand(q,c.Edx)))
 elif a==0x40ee1d and not st['cost']:
  st['cost']=True;q=Snap(p)
  for r in ct.ranges(q).values():
   if q.rd(q.u32(r)+4,1)[0]==13:events.append(dict(stage='initial-web',fn=st['fn'],node=ct.describe(q,st['root'],r,True)))
obj=ct.owned_run(dest,[0x414a64,0x42a8aa],hit)
def payload(obj):
 cf=cmpfunc.Coff(obj)
 return {f.name:(f.data,[(r.offset-f.start,cf.sym_by_index[r.symbol_index].name,r.typ) for r in f.relocs]) for f in cf.functions()}
# A native replay with no hooks gives the independent unmodified control.
native=OUT/(name+'-pool-native')
native.mkdir(exist_ok=True)
for f in cap.iterdir():
 if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,native/f.name)
native_obj=ct.owned_run(native,[],lambda *args:None)
equal=payload(obj)==payload(native_obj)
(OUT/(name+'-pool.json')).write_text(json.dumps(dict(events=events,errors=errors,payload_equal=equal,elapsed=time.monotonic()-started),indent=1))
checks=[e for e in events if e['stage']=='eligibility']
print(name,'checks',sum(e['ok'] for e in checks),'/',len(checks),'payload',equal,'errors',len(errors),'seconds',round(time.monotonic()-started,2))
for e in checks:
 if needle!='*' and e['tuple']['line'] in (194,199,213,219):print(e['fn'],e['tuple']['line'],hex(e['tuple']['op']),e['actual'],e['reason'],e['tuple']['reads'])
