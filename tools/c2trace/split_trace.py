"""Verify SP3 liveness masks and connected-component splitting independently.

Usage (capture names live under build/c2trace/cap):
  .pyenv\\python.exe tools/c2trace/split_trace.py CAPTURE FUNCTION [--target GAME/EDITION/LANGUAGE] [--prioflip]

The observer checks the 0x43837a fixed-point masks and compares endpoint
partitions predicted from 0x438544 inputs with native component membership.
It also compares function bytes and named relocations with an unhooked replay
of the same captured IL. SP3 is pinned through profiles.py. Outputs go under
build/c2trace/alloc-observers/.
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
 parser.add_argument('--prioflip',action='store_true',help='diagnostic: force zero22 priority to -79 and compare with cut_trace.py control replay')
 profiles.add_arguments(parser)
 args=parser.parse_args()
 if args.compiler not in (None,'sp3'):
  parser.error('component splitting is calibrated only for VC5 SP3')
 profiles.configure('sp3',args.target)
 profiles.require_sp3('component splitting')
 cap=profiles.get().work/'cap'/args.capture
 if Path(args.capture).name!=args.capture or not cap.is_dir():
  parser.error('capture must be an existing name under build/c2trace/cap')
 profiles.check_capture(cap)
 return args,cap

args,cap=arguments()
name,needle,diagnostic=args.capture,args.function,args.prioflip
label=name+('-prioflip' if diagnostic else '')
OUT.mkdir(parents=True,exist_ok=True)
import colour_trace as ct, cmpfunc
# Keep the small cache implementation independent of the command-line pool observer.
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
 v=p.u32(a);kind=p.rd(v+4,1)[0]; val=None
 if kind==13:
  op=p.u32(v+0x28)
  if p.rd(op+8,1)[0]==7:val=ct.signed(p.u32(op+0x18))
 return dict(ptr=hex(a),lr=p.u32(a+0x1c),kind=kind,value=val)

def graph(p,root,parent):
 a=int(parent['ptr'],16);mask=p.u32(a+0x10)
 blocks=ct.blocks(p,root); ids={b:i for i,b in enumerate(blocks)}
 first,last=p.u32(a+0x28),p.u32(a+0x30)
 if not mask or first not in ids or last not in ids:return None
 bs=blocks[ids[first]:ids[last]+1]
 rows=[];uf={}
 def find(x):
  uf.setdefault(x,x)
  while uf[x]!=x:x=uf[x]
  return x
 def union(a,b):uf[find(a)]=find(b)
 for b in bs:
  if not p.u32(b+0x18)&1:continue
  n=ids[b];i=bool(p.u32(b+0x24)&mask);o=bool(p.u32(b+0x28)&mask);cut=bool(p.u32(b+0x34)&mask)
  if i:find((n,'i'))
  if o:find((n,'o'))
  if i and o and not cut:union((n,'i'),(n,'o'))
  rows.append(dict(block=n,ptr=hex(b),incoming=i,outgoing=o,cut=cut,
   pred=[ids.get(p.u32(e+8),-1) for e in ct.chain(p,p.u32(b+8),4)],
   succ=[ids.get(p.u32(e+12),-1) for e in ct.chain(p,p.u32(b+12),0)]))
 for row in rows:
  n=row['block']
  if row['incoming']:
   for pred in row['pred']:
    if (pred,'o') in uf:union((n,'i'),(pred,'o'))
  if row['outgoing']:
   for succ in row['succ']:
    if (succ,'i') in uf:union((n,'o'),(succ,'i'))
 groups={}
 for endpoint in uf:groups.setdefault(find(endpoint),[]).append(endpoint)
 return dict(parent=parent,mask=mask,rows=rows,predicted_groups=sorted([sorted(x) for x in groups.values()]))

def flow(p,head,root,extra):
 """Independent fixed-point equations from 43837a, including the optional pass."""
 active=[p.u32(x+8) for x in ct.chain(p,head)]
 bs=ct.blocks(p,root); data={}
 for b in bs:
  data[b]={off:p.u32(b+off) for off in (0x24,0x28,0x2c,0x30,0x34,0x38,0x3c,0x50,0x54)}
 pred={b:[p.u32(x+8) for x in ct.chain(p,p.u32(b+8),4)] for b in active}
 succ={b:[p.u32(x+12) for x in ct.chain(p,p.u32(b+12))] for b in active}
 def joined(neighbours,off):
  r=0
  for b in neighbours:r|=data.get(b,{off:0})[off]
  return r
 def fixed(backward,aux=False):
  for iteration in range(10000):
   change=False
   for b in (reversed(active) if backward else active):
    d=data[b]
    if backward:
     out=joined(succ[b],0x38); new=(out&~d[0x54 if aux else 0x34])|d[0x50 if aux else 0x30]
     if out!=d[0x3c]:
      d[0x3c]=out;change|=new!=d[0x38];d[0x38]=new
    else:
     inside=joined(pred[b],0x3c)&d[0x24];new=((inside&~d[0x34])|d[0x2c])&d[0x28]
     if inside!=d[0x38]:
      d[0x38]=inside;change|=new!=d[0x3c];d[0x3c]=new
   if not change:return iteration+1
  raise RuntimeError('nonconverging model')
 rounds=[fixed(True)]
 for b in active:
  d=data[b]; d[0x24]=d[0x38];d[0x28]=d[0x3c];d[0x38]=0;d[0x3c]=d[0x2c]&d[0x3c]
 rounds.append(fixed(False))
 for b in active:
  d=data[b];d[0x24]=d[0x38];d[0x28]=d[0x3c];d[0x38]=d[0x50] if extra else 0;d[0x3c]=0
  if not extra:d[0x50]=0
 if extra:
  rounds.append(fixed(True,True))
  for b in active:
   d=data[b];d[0x50]=d[0x3c];d[0x38]=d[0x3c]=0
 return {b:{off:data[b][off] for off in (0x24,0x28,0x50)} for b in active},rounds

dest=OUT/(label+'-split-replay');dest.mkdir(exist_ok=True)
for f in cap.iterdir():
 if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,dest/f.name)
st=dict(active=False,fn='',pending=None);events=[];checks=0;errors=[];flow_checks=0;flow_errors=[];started=time.monotonic()
def disarm(p,a):
 if a in p.orig:p.wr(a,p.orig.pop(a))
def hit(p,c,a):
 global checks,flow_checks
 if a==0x414a64:
  root=p.u32(0x48fb48);sy=p.u32(root);fn=p.rd(p.u32(sy+24),180).split(b'\0')[0].decode('ascii','replace')
  st.update(active=needle=='*' or needle in fn,fn=fn,root=root)
  if st['active']:
   for bp in (0x43837a,0x438544,0x437f2e):p.arm(bp)
   if diagnostic:p.arm(0x41b6fa)
 elif a==0x42a8aa:
  for bp in (0x43837a,0x438544,0x437f2e):disarm(p,bp)
  if diagnostic:disarm(p,0x41b6fa)
  st['active']=False
 elif a==0x41b6fa and diagnostic and st['active']:
  q=Snap(p);d=desc(q,c.Ecx)
  if (d['value'],q.u32(c.Ecx+0x24))==(0,22) and not st.get('forced'):
   p.wr(c.Ecx+0xc,(-79&0xffffffff).to_bytes(4,'little'));st['forced']=True
 elif not st['active']:return
 elif a==0x43837a:
  q=Snap(p);st['flow'],st['rounds']=flow(q,c.Ecx,st['root'],q.u32(c.Esp+4)!=0)
 elif a==0x438544:
  q=Snap(p);rows=[]
  for b,fields in st.get('flow',{}).items():
   for off,expected in fields.items():
    actual=q.u32(b+off);flow_checks+=1
    if actual!=expected:flow_errors.append(dict(block=hex(b),off=hex(off),expected=expected,actual=actual))
  for parent in ct.chain(q,c.Ecx,0x14,100):
   d=desc(q,parent)
   if needle!='*' and d['value'] not in (0,1,10,1525):continue
   g=graph(q,st['root'],d)
   if g:rows.append(g)
  st['pending']=rows
 elif a==0x437f2e:
  q=Snap(p)
  for g in st['pending'] or []:
   parent=int(g['parent']['ptr'],16); children=set(ct.bitset(q,q.u32(parent+0x20))); groups={}
   for row in g['rows']:
    b=int(row['ptr'],16)
    for suffix,off in [('i',0x40),('o',0x44)]:
     for child in children.intersection(ct.bitset(q,q.u32(b+off))):groups.setdefault(child,[]).append((row['block'],suffix))
   observed=sorted([sorted(x) for x in groups.values()]);ok=g['predicted_groups']==observed
   checks+=1
   g.update(fn=st['fn'],children=sorted(children),actual_groups={str(k):v for k,v in groups.items()},ok=ok)
   if not ok:errors.append(dict(fn=st['fn'],parent=g['parent'],expected=g['predicted_groups'],actual=observed))
   events.append(g)
obj=ct.owned_run(dest,[0x414a64,0x42a8aa],hit)
def payload(obj):
 cf=cmpfunc.Coff(obj)
 return {f.name:(f.data,[(r.offset-f.start,cf.sym_by_index[r.symbol_index].name,r.typ) for r in f.relocs]) for f in cf.functions()}
native=OUT/(name+'-split-native');native.mkdir(exist_ok=True)
if not (native/'trace.obj').exists():
 for f in cap.iterdir():
  if f.is_file() and f.name not in ('trace.obj','index.json'):shutil.copyfile(f,native/f.name)
 native_obj=ct.owned_run(native,[],lambda *args:None)
else:native_obj=native/'trace.obj'
eq=payload(obj)==payload(native_obj)
control_equal=None
if diagnostic:
 control_obj=OUT/(label+'-control')/'trace.obj'
 control_equal=control_obj.is_file() and payload(obj)==payload(control_obj)
(OUT/(label+'-split.json')).write_text(json.dumps(dict(events=events,checks=checks,errors=errors,flow_checks=flow_checks,flow_errors=flow_errors,payload_equal=eq,diagnostic_only=diagnostic,control_payload_equal=control_equal,elapsed=time.monotonic()-started),indent=1))
print(label,'split checks',checks-len(errors),'/',checks,'flow',flow_checks-len(flow_errors),'/',flow_checks,'errors',len(errors)+len(flow_errors),'payload',eq,'control',control_equal,'seconds',round(time.monotonic()-started,2))
for e in errors[:3]:print(e)
for e in flow_errors[:3]:print('flow',e)
