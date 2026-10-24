"""Bounded heap sensitivity experiment, not a historical heap-state oracle.

Independent implementation reference for scene splitting: historical Build.
The constructor wrapper also checks explicit historical constructor constants.
Poison differences are research results saved in results.json, not blessed goldens.
"""
from pathlib import Path
import argparse, subprocess, json, shutil, os, hashlib

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--exe',required=True); p.add_argument('--data',required=True)
    p.add_argument('--game',choices=['sb1','sb2'],required=True)
    p.add_argument('--work',required=True); p.add_argument('--scenario',choices=['attract','tour','editor','save'],required=True)
    p.add_argument('--members',nargs='*',default=[])
    p.add_argument('--frames',type=int)
    a=p.parse_args(); work=Path(a.work).resolve(); work.mkdir(parents=True,exist_ok=True)
    tests=Path(__file__).resolve().parent; sb2=a.game=='sb2'
    frames={'attract':8000 if sb2 else 7500,'tour':260 if sb2 else 800,'editor':260 if sb2 else 360,'save':200}[a.scenario]
    args=['--color','16','--frames',str(a.frames or frames)]
    if a.scenario!='attract':
        # The same F3/F4 keys/dialog layout is present in both phase tables.
        script=('sb2_' if sb2 or a.scenario=='save' else '')+a.scenario+'.input'
        args+=['--input',str(tests/'golden'/script)]
    env=dict(os.environ); env.update(SDL_AUDIODRIVER='dummy')
    def run(name,cls='none',member='all',pattern='0xa5',reference=False):
        user=work/name
        if user.exists(): shutil.rmtree(user)
        for suffix in ['trace','dump','alloc','log']:
            artifact=work/(name+'.'+suffix)
            if artifact.exists(): artifact.unlink()
        cmd=[a.exe,str(work/(name+'.alloc')),cls,member,pattern,'--headless','--data',a.data,'--user',str(user),*args,
             '--trace',str(work/(name+'.trace')),'--state-dump','1:'+str(work/(name+'.dump'))]
        if reference: cmd+=['--reference']
        else: cmd+=['--check-commands']
        try:
            result=subprocess.run(cmd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,env=env,timeout=180)
            rc=result.returncode; log=result.stdout.decode(errors='replace')
        except subprocess.TimeoutExpired as exc:
            rc='timeout'; log=(exc.stdout or b'').decode(errors='replace')
        (work/(name+'.log')).write_text(log)
        trace=work/(name+'.trace')
        lines=trace.read_text().splitlines() if trace.exists() else []
        alloc=(work/(name+'.alloc')).read_text() if (work/(name+'.alloc')).exists() else ''
        if rc==0:
            for c in ['CDecor','CEvent','CPixmap','CSound','CMovie','CNetwork']:
                if alloc.count('construct '+c+' ')!=1: raise AssertionError('missing/duplicate wrapper '+c)
            if not lines: raise AssertionError('empty trace')
        if rc==9: raise AssertionError('historical constructor constant failed: '+name)
        saves={str(f.relative_to(user)):hashlib.sha256(f.read_bytes()).hexdigest() for f in user.rglob('*.blp')
            if f.name.lower().startswith(('s00','u00'))}
        return {'rc':rc,'lines':lines,'alloc':alloc,'saves':saves}
    def compare(base,other):
        delta={'rc':other['rc'],'equal':base['rc']==other['rc'] and base['lines']==other['lines']}
        for i,(x,y) in enumerate(zip(base['lines'],other['lines'])):
            if x!=y:
                delta.update(first_line=i,zero=x,poison=y); break
        delta['phase_screen_differences']=sum((x.split()[1],x.split()[3])!=(y.split()[1],y.split()[3])
            for x,y in zip(base['lines'],other['lines']))
        delta['state_differences']=sum(x.split()[2]!=y.split()[2] for x,y in zip(base['lines'],other['lines']))
        delta['trace_lengths']=[len(base['lines']),len(other['lines'])]
        delta['save_bytes_equal']=base['saves']==other['saves']
        return delta
    zero=run('zero'); repeat=run('repeat'); reference=run('reference',reference=True)
    assert zero['rc']==0,zero['rc']
    if a.scenario=='save' and not a.frames:
        payload=work/'zero/data/s001-000.blp'
        assert payload.exists() and payload.stat().st_size==(57008 if sb2 else 54416),'save scenario did not create the historical-size save'
    assert compare(zero,repeat)['equal'],'zero replay is nondeterministic'
    assert compare(zero,reference)['equal'],'native/reference zero traces differ'
    assert compare(zero,repeat)['save_bytes_equal'],'zero saved payload is nondeterministic'
    assert compare(zero,reference)['save_bytes_equal'],'native/reference zero saved payloads differ'
    results={'game':a.game,'scenario':a.scenario,'frames':a.frames or frames,'zero_reference_equal':True,'experiments':{}}
    selections=[(x.split(':')[0],x.split(':')[1]) for x in a.members] if a.members else [(c,'all') for c in ['CDecor','CEvent','CPixmap','CSound','CMovie','CNetwork']]+[('all','all')]
    for cls,member in selections:
        name=cls+'_'+member; other=run(name,cls,member)
        results['experiments'][name]=compare(zero,other)
        print(name,json.dumps(results['experiments'][name]),flush=True)
    if a.scenario=='save' and not a.frames and not a.members:
        # Historical CurrentWrite reads this field unconditionally. There is
        # no historical writer except CurrentRead. Validate that the poison
        # survived to this exact source-established serialized field, without
        # asserting that the original heap was zero.
        fields=work/'savefields.txt'
        subprocess.run([a.exe,str(fields),'savefields','all','0'],check=True,env=env,timeout=10)
        offset,size=next((int(o),int(s)) for n,o,s in (line.split() for line in fields.read_text().splitlines()) if n=='scrollFollow')
        for mode,expected in [('zero',b'\0'),('CDecor_all',b'\xa5'),('all_all',b'\xa5')]:
            raw=(work/mode/'data/s001-000.blp').read_bytes()
            assert raw[offset:offset+size]==expected*size,'serialized unwritten scrollFollow did not retain allocation bytes'
    (work/'results.json').write_text(json.dumps(results,indent=2))
    print('PASS: constructor contracts, wrapper coverage, zero determinism and historical Build reference; poison outcomes recorded separately')

if __name__=='__main__': main()
