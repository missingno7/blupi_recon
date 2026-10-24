"""Compile exact ancestry/history TUs serially for real COMDAT keys."""
import os,sys
sys.dont_write_bytecode=True
os.environ['PYTHONDONTWRITEBYTECODE']='1'
import json,subprocess,shutil,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
OUT=HERE/'solver-sources'
BUILD=ROOT/'build/w-sb1icf-source-keys'
BUILD.mkdir(exist_ok=True)
sys.path.insert(0,str(ROOT/'tools'))
from cmpfunc import Coff
manifest=json.loads((ROOT/'recon.json').read_text())
def git(*args,cwd=ROOT):return subprocess.check_output(['git',*args],cwd=cwd)
def tree(commit,dest,repo=ROOT,prefix='src/'):
    dest.mkdir(exist_ok=True)
    paths=git('ls-tree','-r','--name-only',commit,cwd=repo).decode().splitlines()
    for p in paths:
        if p.startswith(prefix) and '/' not in p[len(prefix):] and Path(p).suffix.lower() in ['.cpp','.h','.inc']:
            (dest/Path(p).name).write_bytes(git('show',commit+':'+p,cwd=repo))
    return dest
pb=tree('b0270ab',OUT/'pb-tree',ROOT/'external/planetblupi',prefix='')
hist=tree('60c7fab^',OUT/'history-60c7fab-parent')
hist2=tree('f47a4e6^',OUT/'history-f47a4e6-parent')
text=tree('699e45d^',OUT/'history-699e45d-parent')
wave=OUT/'wave';wave.mkdir(exist_ok=True)
for name in ['wave.cpp','wave.h']:shutil.copyfile(ROOT/'external/sb2-decomp/src'/name,wave/name)
sources=[('pb-fifo',pb/'fifo.cpp'),('pb-arrange',pb/'arrange.cpp'),('pb-obstacle',pb/'obstacle.cpp'),
    ('pb-action',pb/'action.cpp'),('sb2-wave',wave/'wave.cpp'),
    ('history-decmove-60',hist/'decmove.cpp'),('history-decmove-f47',hist2/'decmove.cpp'),
    ('history-text',text/'text.cpp')]
rows=[]
for label,source in sources:
    obj=BUILD/(label+'.obj')
    cmd=[sys.executable,'tools/msvc.py',str(source),'--tree','sp3','--out',str(obj),
        '--log-dir',str(BUILD/'logs'),'--include',str(source.parent),'--include',r'C:\tools\dx5sdk\INCLUDE']
    for define in manifest['toolchain']['defines']:cmd+=['--define',define]
    for flag in manifest['toolchain']['flags']:cmd+=['--flag',flag]
    start=time.time()
    run=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,encoding='utf-8',errors='replace')
    (BUILD/(label+'.log')).write_text(run.stdout+run.stderr)
    row=dict(label=label,source=str(source),object=str(obj),command=cmd,exit=run.returncode,seconds=time.time()-start)
    if run.returncode:
        print(label,'compile failed:',(run.stdout+run.stderr)[-1700:],flush=True)
    else:
        c=Coff(obj)
        syms={}
        for s in c.symbols:
            if s.section>0 and s.name not in ['.text','.text$x'] and (s.is_function or s.storage==2):syms.setdefault(s.section,s.name)
        row['comdats']=[dict(name=syms.get(s.index,''),section=s.index,size=s.raw_size,relocs=len(s.relocations),
            key=[s.raw_size,len(s.relocations)]) for s in c.sections if s.characteristics&0x20000000 and s.characteristics&0x1000 and s.raw_size]
        print(label,'compiled',len(row['comdats']),'executable COMDATs',flush=True)
    rows.append(row)
    (HERE/'solver-source-keys.json').write_text(json.dumps(rows,indent=2))
