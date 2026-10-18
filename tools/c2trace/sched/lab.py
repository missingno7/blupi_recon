"""Private captures, serial compilation and fully reaped c2 replay."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
import uuid
ROOT = Path(__file__).resolve().parents[3]
W = Path(__file__).resolve().parent
sys.path[:0] = [str(ROOT/'tools'), str(ROOT/'tools/c2trace')]
import c2dbg, colour_trace, cmpfunc, fnprobe, keyforce, capstone
PIN = 'e75aecaf4073b0817ffb638cae5fff636b2e2d1a090daabf8f68dbc954515fae'

def write(p, x):
    Path(p).write_text(json.dumps(x,indent=2)+'\n',encoding='utf-8')

def capture(name, source, micro=False):
    assert hashlib.sha256(Path(c2dbg.C2).read_bytes()).hexdigest() == PIN
    cap = ROOT/'build/c2trace/sched'/(name+'-'+uuid.uuid4().hex[:12])
    cap.mkdir(parents=True)
    shutil.copyfile(ROOT/'build/c2trace/stub/c2stub.exe',cap/'c2stub.exe')
    (cap/'il').mkdir()
    if micro:
        obj=cap/'micro.obj'
        cmd=[sys.executable,str(ROOT/'tools/msvc.py'),str(source),'--tree','sp3',
             '--out',str(obj),'--log-dir',str(cap/'logs')]
        for flag in ['/W3','/GX','/O2','/ML','/Gd','/Gy','/B2'+str(cap/'c2stub.exe')]:
            cmd+=['--flag',flag]
        r=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        (cap/'compile.log').write_text(r.stdout,encoding='utf-8')
        if r.returncode: raise RuntimeError(r.stdout)
    else:
        obj=fnprobe.compile_unit('src/event.cpp',source,cap,['/B2'+str(cap/'c2stub.exe')])
    for f in (cap/'il').iterdir():
        if f.is_file(): shutil.copyfile(f,cap/f.name)
    write(cap/'capture.json',dict(cap=str(cap),object=str(obj),source=str(source),
          sha256=hashlib.sha256(source.read_bytes()).hexdigest(),c2_sha256=PIN,micro=micro))
    return cap

def replay(cap, points, callback):
    cap=Path(cap)
    dest=ROOT/'build/c2trace/sched'/'replays'/('sched-'+uuid.uuid4().hex[:12]); dest.mkdir(parents=True)
    for f in cap.iterdir():
        if f.is_file() and f.suffix not in ['.exe','.obj']:
            shutil.copyfile(f,dest/f.name)
    obj=colour_trace.owned_run(dest,points,callback)
    native=Path(json.loads((cap/'capture.json').read_text())['object'])
    a={f.name:f.data for f in cmpfunc.Coff(native).functions()}
    b={f.name:f.data for f in cmpfunc.Coff(obj).functions()}
    if a!=b: raise RuntimeError('Tracing changed compiler output')
    return obj

if __name__=='__main__':
    print(capture(sys.argv[1],Path(sys.argv[2]).resolve(),len(sys.argv)>3))
