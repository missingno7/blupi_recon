"""Run registered micro predictions, serially, through the repo compiler wrapper."""
import json
from pathlib import Path
import sys
import lab,tracer,model
W=lab.W
PREFIX='''struct C { int *pix; int index; char names[100]; };
extern void Copy(char*,char*);
extern void Draw(int*,char*,int);
'''
SOURCES={
 'stack_array':PREFIX+'''void Test(C* p,int i) {
 char text[100]; Copy(text,p->names); text[i]=0;
 Draw(p->pix,text,p->index==i?2:0);
}
''',
 'unknown_pointer':PREFIX+'''void Test(C* p,int i,char* text) {
 text[i]=0;
 Draw(p->pix,text,p->index==i?2:0);
}
''',
 'member_array':PREFIX+'''void Test(C* p,int i) {
 p->names[i]=0;
 Draw(p->pix,p->names,p->index==i?2:0);
}
''',
 'named_first':PREFIX+'''void Test(C* p,int i) {
 char text[100]; Copy(text,p->names); text[i]=0;
 char* first=text;
 Draw(p->pix,first,p->index==i?2:0);
}
''',
 'shared_first':PREFIX+'''void Test(C* p,int i) {
 char text[100]; Copy(text,p->names);
 char* first=text; first[i]=0;
 Draw(p->pix,first,p->index==i?2:0);
}
''',
 'reference_cut':PREFIX+'''void Test(C* p,int i) {
 char text[100]; Copy(text,p->names);
 char& cut=text[i]; cut=0;
 Draw(p->pix,text,p->index==i?2:0);
}
''',
 'tie_loads':'''extern void Draw(int,int,int);
void Test(int* p) { Draw(p[0],p[1],p[2]); }
''',
}
PREDICTIONS={
 'stack_array':'The stack byte store and member loads need no memory-alias edge. Register WAR/WAW edges can still order them. P5 priorities and full schedule predicted from DAG.',
 'unknown_pointer':'The char store through an unknown pointer must precede both member reads (C++ char aliasing); observe memory edges, regardless of the assigned registers.',
 'member_array':'Variable member-array indexing is not assumed bounded: the store can alias adjacent pointer/index fields, so inspect alias edges before claiming disjointness.',
 'named_first':'A single-use copy of the local array address is expected to fold, leaving the same dependency graph/register pattern as stack_array.',
 'shared_first':'The reused first pointer may coalesce into an address calculation, or acquire a saved register; the model predicts placement only after seeing its lowered DAG.',
 'reference_cut':'Taking a reference to the exact byte may fold; changing to a reference alone does not guarantee a new store/member-load alias edge.',
 'tie_loads':'Equal-priority available loads are selected in ascending original tuple ordinal, subject to the P5 pairing/latency checks.',
}
SOURCES['split_plain']=(lab.ROOT/'scratch/sol-evanc/bt/split_plain.cpp').read_text()
SOURCES['split_early_advance']=SOURCES['split_plain'].replace(
    'Draw(pix,pos,text,fileIndex==i?2:0);\n   pos.y+=16;',
    'Draw(pix,pos,text,fileIndex==i?2:0);\n   ++line2;\n   pos.y+=16;').replace('line2+1,','line2,')
SOURCES['split_shadow_buffer']=SOURCES['split_plain'].replace('else {\n   strcpy(text,names[i]);',
    'else {\n   char text[100];\n   strcpy(text,names[i]);')
PREDICTIONS['split_plain']='Control: complete P5 DAG schedule is predicted from the captured input graph. Compare pointer/font register anti-dependencies with the real DrawButtons window.'
PREDICTIONS['split_early_advance']='Consume the newline as a cursor step immediately after the first draw, before coordinate changes. This may make the second pointer live before the font; if it folds back to the old binding, the model predicts no schedule repair.'
PREDICTIONS['split_shadow_buffer']='Give the first line its own scoped buffer. Stack coloring may reuse a prior array home, but an extra frame slot rejects this source for the target even if the pointer/font anti-dependence improves.'

def run(name):
    src=lab.ROOT/'build/c2trace/sched'/'micros'/(name+'.cpp');src.parent.mkdir(parents=True, exist_ok=True)
    src.write_text(SOURCES[name],encoding='ascii')
    # This file is written before the compiler or debugger runs.
    prediction=lab.ROOT/'build/c2trace/sched'/'micros'/(name+'.prediction.json')
    lab.write(prediction,dict(name=name,prediction=PREDICTIONS[name],source=src.read_text(),
        model='P5 hard-edge complete DAG simulator, plus independent priorities/insertion/pipe checks'))
    cap=lab.capture('micro-'+name,src,True)
    out=lab.ROOT/'build/c2trace/sched'/'micros'/(name+'.sched.json')
    tracer.trace(cap,'Test',out)
    result=model.validate(out);lab.write(str(out)+'.validation.json',result)
    obj=Path(json.loads((cap/'capture.json').read_text())['object'])
    coff=lab.cmpfunc.Coff(obj);md=lab.capstone.Cs(lab.capstone.CS_ARCH_X86,lab.capstone.CS_MODE_32)
    lab.write(lab.ROOT/'build/c2trace/sched'/'micros'/(name+'.asm.json'),[dict(name=f.name,instructions=[
        dict(offset=i.address,asm=i.mnemonic+' '+i.op_str) for i in md.disasm(f.data,0)]) for f in coff.functions()])
    lab.write(lab.ROOT/'build/c2trace/sched'/'micros'/(name+'.result.json'),dict(capture=str(cap),trace=str(out),
        failures=result['failures'],checks=result['checks'],
        full=[dict(window=x['window'],match=x.get('match'),excluded=x.get('excluded'),choices=x.get('choices'))
              for x in result['full_schedules']]))
    if result['failures']:raise RuntimeError(name+' model failed')

if __name__=='__main__':
    for name in sys.argv[1:] or SOURCES:run(name)
