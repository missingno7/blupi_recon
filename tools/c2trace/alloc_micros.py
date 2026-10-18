"""Allocator micro controls for docs/ALLOCATOR.md (KB BLUPI-ALLOC-*): compile, capture and trace each micro sequentially.

  .pyenv\python.exe tools/c2trace/alloc_micros.py [name ...] [--replay-only]

Sources and registered predictions are committed in msvc5kb/micros/alloc; outputs go to build/c2trace/alloc-micros."""
import json
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path[:0] = [str(HERE), str(ROOT / "tools")]
import colour_trace
import cmpfunc
import capstone

M = ROOT / "build/c2trace/alloc-micros"
M.mkdir(parents=True, exist_ok=True)
REPLAY_ONLY = "--replay-only" in sys.argv
cases = {}


def case(name, src, prediction):
    cases[name] = dict(source=src, prediction=prediction)


stores = "\n".join(f"    p[{2*i}]=0x12345678; p[{2*i+1}]=0x23456789;" for i in range(4))
case("constants_equal", "extern void Tick();\nvoid Test(volatile int *p) {\n" + stores[:stores.find('\n', 1)] + "\n    Tick();\n" + stores[stores.find('\n', 1)+1:] + "\n}\n",
     "Both constant webs have four stores, equal benefit; exact queue-key ties put the later insertion first. At every colour decision choose first eligible minimum-penalty register.")
case("constants_plus_one", cases["constants_equal"]["source"].replace("\n}", "\n    p[8]=0x12345678;\n}"),
     "Adding one memory store raises the first constant web's benefit by one if boundaries remain equal; its refs rise by one. No claim that raw refs alone determine order.")
case("loop_flat", "void Test(volatile int *p) { p[0]=0x12345678; p[1]=0x12345678; p[2]=0x12345678; }\n",
     "Three eligible stores pool one constant; ordinary nonzero constant has benefit 3 minus materialization costs. It survives only with positive final benefit.")
case("loop_depth", "void Test(volatile int *p, int n) { for (int i=0; i<n; ++i) { p[3*i]=0x12345678; p[3*i+1]=0x12345678; p[3*i+2]=0x12345678; } }\n",
     "The loop-body per-use weight is 2^(loop_depth+1), not a constant times raw refs; benefit exceeds loop_flat's benefit with the same three stores.")
case("call_live", "extern void Tick();\nint Test(int *p) { int x=p[0]; Tick(); return x+p[1]; }\n",
     "The non-rematerializable x survives the call in a callee-saved register (4,6,7,8) or a stack home; no eax/ecx/edx binding may span Tick.")
case("call_dead", "extern void Tick();\nint Test(int *p) { int x=p[0]; p[1]=x+3; Tick(); return p[1]; }\n",
     "Moving x's last use before Tick removes its call-crossing requirement. x needs no callee-saved binding across Tick; pointer p still crosses the call.")
body = "extern void Fill(int*,int*);\nextern void Use(int);\nvoid Test() { DECL Fill(&a,&b); Use(a); Use(b); Use(a); Use(b); }\n"
case("slots_ab", body.replace("DECL", "int a; int b;"),
     "Escaping a,b have stack homes. Equal size/weight insertion uses first reference, not lexical declaration; exact weight ties do not cross on updates.")
case("slots_decl_ba", body.replace("DECL", "int b; int a;"),
     "Reversing only declarations leaves use order, reference-history stack order and a/b offsets identical to slots_ab.")
case("slots_b_extra", body.replace("DECL", "int a; int b;").replace("Use(a); Use(b); }", "Use(a); Use(b); Use(b); }"),
     "One extra late reference moves b ahead of a within equal-size stack group only if its accumulated weight becomes strictly greater; stack size remains two int homes.")
case("slots_tail_ba", body.replace("DECL", "int a; int b;").replace("Use(a); Use(b); Use(a); Use(b);", "Use(a); Use(b); Use(b); Use(a);"),
     "Same declarations, first references, sizes and final counts 3/3 as slots_ab. Only final reads reverse. Predict b reaches three first, retains the tie, and gets the first home: b=-8, a=-4. This tests history independently of final weight.")
case("pressure_call", "extern void Tick();\nextern int Mix(int,int,int,int,int,int);\nint Test(int *p) { int a=p[0],b=p[1],c=p[2],d=p[3],e=p[4],f=p[5]; Tick(); return Mix(a,b,c,d,e,f)+p[6]; }\n",
     "Seven independent call-crossing values exceed four callee-saved colours; at least one real spill/split is required. Selector penalties must protect a profitable singleton's sole register, and predict every choice from the available set.")
displaced = "\n".join(f"    p[{2*i+1}]=0x12345678; p[{2*i+2}]=0x23456789;" for i in range(4))
case("constants_equal_displaced", "void Test(volatile int *p) {\n" + displaced + "\n}\n",
     "Every store now has a nonzero displacement. Predict both constants: four refs, benefit 4-1=3, priority 12, rank zero. Exact key tie inserts constant 2 ahead of constant 1. Addresses, not Tick, caused the earlier unequal benefits; no nocall-only trial was compiled.")
case("dependent_web", "extern void Tick();\nextern int Mix(int,int,int,int);\nint Test(int *p) { int x=p[0]; int y=x+3; Tick(); return Mix(y,y,y,x); }\n",
     "Hypothesis: the multiply-used y definition with a still-live x input enters dependency requeue or substitution in 45aaf9. Any surviving requeued y must receive min(input.priority)-1. If no such entry succeeds, mark the proposed C++ trigger as a failed prediction; do not claim formula validation from a vacuous condition.")
case("background_clone", """struct Point { int x,y; };
struct Rect { int left,top,right,bottom; };
class Pix { public: int DrawPart(int,int,Point,Rect,int,int); };
class Scene { public: Pix *pix; Point Next(); void Test(Rect,Point); };
void Scene::Test(Rect rect, Point mouse)
{
    int i,j;
    Point tiny,pos,decor;
    decor=Next();
    pos.x=decor.x*2/3;
    pos.y=decor.y*2/3;
    int x=pos.x%640;
    rect.left=x;
    int y=pos.y%480;
    tiny.x=0;
    for(i=0;i<2;i++) {
        tiny.y=0;
        rect.top=y;
        rect.bottom=480;
        for(j=0;j<2;j++) {
            rect.right=640;
            pix->DrawPart(-1,4,tiny,rect,1,0);
            tiny.y+=480-rect.top;
            rect.top=0;
        }
        tiny.x+=640-rect.left;
        rect.left=0;
    }
}
""", "Hypothesis: the background-only reduction still reproduces 451ac3 cloning of the bottom=480 field definition after a competing saved-register binding exhausts availability. If the clone does not occur, record a failed reduction: later Build context is necessary, rather than adding dummy locals/uses to force it.")

for name, c in cases.items():
    (M / (name + ".cpp")).write_text(c["source"])


def run(name):
    cap = M / (name + "-capture")
    cap.mkdir(exist_ok=True)
    stubdir = ROOT / "build/c2trace/alloc-stub"
    stubdir.mkdir(parents=True, exist_ok=True)
    stub = stubdir / "c2stub.exe"
    if not stub.exists():
        shutil.copyfile(ROOT / "build/c2trace/stub/c2stub.exe", stub)
    il = stubdir / "il"
    il.mkdir(exist_ok=True)
    # Only files beneath our dedicated stub/il; never shared capture scratch.
    if not REPLAY_ONLY:
        for old in il.iterdir():
            if old.is_file():
                old.unlink()
    obj = M / (name + ".obj")
    cmd = [sys.executable, str(ROOT / "tools/msvc.py"), str(M / (name + ".cpp")),
           "--tree", "sp3", "--out", str(obj), "--log-dir", str(M / (name + "-logs"))]
    for f in ["/W3", "/GX", "/O2", "/ML", "/Gd", "/Gy", "/B2" + str(stub)]:
        cmd += ["--flag", f]
    if not REPLAY_ONLY:
        r = subprocess.run(cmd, capture_output=True, text=True)
        (M / (name + "-compile.log")).write_text(r.stdout + r.stderr)
        if r.returncode:
            raise RuntimeError(r.stdout + r.stderr)
        for f in il.iterdir():
            if f.is_file():
                shutil.copyfile(f, cap / f.name)
    result = colour_trace.trace(cap, 0, True, include_costs=True)
    base = {f.name: f.data for f in cmpfunc.Coff(obj).functions()}
    observed = {f.name: f.data for f in cmpfunc.Coff(Path(result["object"])).functions()}
    result["unmodified_payload_equality"] = base == observed
    (M / (name + ".json")).write_text(json.dumps(result, indent=1))
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    (M / (name + "-asm.txt")).write_text("\n".join(f"{i.address:04x} {i.mnemonic} {i.op_str}" for f in cmpfunc.Coff(obj).functions() for i in md.disasm(f.data, 0)))
    es = result["events"]
    checks = [e for e in es if "prediction_ok" in e]
    print(name, "checks", sum(e["prediction_ok"] for e in checks), "/", len(checks), "payload", base == observed, "errors", result["errors"], flush=True)
    for e in es:
        n = e.get("node", {})
        if e["stage"] in ("sort", "select", "spill", "split", "defer"):
            print(" ", e["stage"], n.get("lr"), "v", n.get("value"), "ref", n.get("refs"), "priority", n.get("priority"), "benefit", n.get("benefit"), "tie", n.get("tie"), "reg", e.get("chosen"), flush=True)
        if e["stage"] in ("stack-ordered", "stack-assigned"):
            print(" ", e["stage"], [(r["id"], r["weight"], r["offset"]) for r in e["order"]], flush=True)
    if result["errors"] or base != observed:
        raise RuntimeError("trace validity failed")


if __name__ == "__main__":
    for name in [s for s in sys.argv[1:] if s != "--replay-only"] or cases:
        run(name)
