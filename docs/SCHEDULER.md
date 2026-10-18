# VC5 SP3 P5 scheduler model

[EVIDENCE] Pinned c2 SHA-256:
`e75aecaf4073b0817ffb638cae5fff636b2e2d1a090daabf8f68dbc954515fae`.
The captured flags contain `-G5`; the manifest passes `/O2 /GX /ML /Gd /Gy`.
Static source: `build/c2/out/all.c`; instruction boundaries checked against
the pinned executable. Only the compiler was executed/debugged.

## Entry points and inputs

[EVIDENCE] These are image VAs, not game RVAs:

| Address | Role |
|---|---|
| `0040d9be` | Scheduling driver; second fastcall argument is phase, 1 before temporary assignment, 0 after |
| `0040db1c` | Phase eligibility gate |
| `0042fd30` | Processor model, priority weights, issue resources and opcode-table setup |
| `00431ec0` | Build a window's dependence graph from lowered tuples |
| `004305ba`, `0043071a` | Input/output dependence construction |
| `0043267a`, `00432736` | Search prior indirect memory writes/reads for dependence edges |
| `00407fa0`, `00407ef0` | Operand overlap and alias-class overlap tests |
| `00430b4c` | Reverse graph heights, critical-path flags, base priorities |
| `004315f1` | Schedule a graph/window |
| `0043170c` | Insert ready node |
| `0043179a` | Select up to the issue width for one cycle |
| `00431985`, `004319f6` | Adjust priorities and re-sort ready queue |
| `00431a3b` | P5 resource/pairing selector; Ghidra exports this as a label |
| `00431c72` | U/V pairing compatibility |
| `00431be3`, `00431c31` | Remove chosen node / discharge dependence edge |
| `00431d74`, `00431da0`, `00431e31` | Re-link selected tuples, reserve resources, advance cycle |

[EVIDENCE] Global state: ready head/tail `493134/4930c4`, count `493124`,
cycle `4930ac`, issue width `493078`, selected nodes `4930b0`, previous
selection `493144`, processor model `49309c`, opcode table `4930a4`,
resource selector `493074`, phase `49321c`, current tuple range
`493224..493228`. P5 table is `4922c8`, selector is `431a3b`, model index 2.
Windows can be partitioned at control boundaries or the driver's size limit;
they are not necessarily identical to the original source basic blocks.

[EVIDENCE] Both phase entries occur for DrawButtons, but only phase 0 builds
scheduling windows. Its relevant graph therefore already contains physical
register bindings. Scheduling does not choose those registers.

[EVIDENCE] Tuple next/previous pointers are +0/+0c, opcode +4, input/output
lists +18/+1c, relative source line +10 (low word). An operand's +14 is its
original descriptor; +18 is the allocated descriptor. Reading only +14
mistakes assigned temporaries for unallocated virtual names. `tracer.py`
prints both, including `assigned_register`.

## Graph representation and priority

[EVIDENCE] Node offsets: tuple +1c, incoming/outgoing/nonblocking incoming
counts +20/+22/+24 (words), base/current priority +28/+2c, earliest cycle
+30, height/ordinal +34/+36 (words), latency +38 (byte), flags +39.
Ready next/previous are +10/+14. Incoming/outgoing edge lists are +8/+c.
An edge has source/destination +8/+c, dependence flags +10, latency +14
(word), and address-rewrite category +16 (low five bits).

[EVIDENCE] Dependence flags combine:

| Flags | Register/descriptor dependence | Memory dependence |
|---|---|---|
| RAW | `01` | `20` |
| WAR (anti-dependence) | `02` | `40` |
| WAW (output dependence) | `04` | `80` |

[EVIDENCE] `80000` supplies a barrier/control edge. Source tuple order feeds
the original ordinal and the construction of these edges. Indirect operands
also carry alias-class information; `407fa0` dispatches by operand kind and
`407ef0` compares alias-class sets. A stack-derived address can retain
different alias information after a pointer/reference source rewrite even
when its eventual machine addressing mode is unchanged.

[EVIDENCE] Height is zero for the terminal node; otherwise it is
`max(edge latency + successor height + 1)`. Critical-path flag 08 propagates
from the synthetic start along edges attaining that maximum.
P5 weights, as signed shorts, are `[-1,13,-5,-1,-2,16,16,0]`.
In the measured integer windows the priority reduces to
`(height << 13) + (outgoing_count >> 5) + (is_memory_read << 16)`.
The general implementation also has boolean flag terms and a floating-point
memory-write term. Negative shifts mean right shifts, not negative values.

[EVIDENCE] Ready insertion uses descending unsigned priority, then ascending
original ordinal. A new node with the exact same priority and ordinal follows
an existing one. Thus it is not a symbol-key or pointer-address tie-break.
The P5 pair-preference priority increment is `1 >> 2 == 0` here.

## Issue and latency

[EVIDENCE] Opcode records are three dwords: base latency, resource occupancy,
pair/resource class. For the GP corpus the last value is 0 (both pipes),
100 (U), 200 (V), or 300 (neither pairing position). Pairing requires the
first type in {0,100} and second in {0,200}. SETcc has 300 and issues alone.
P5 issue width is 2; resource capacities are [1,1,3,1]. The integer selector
checks both GP resources for the first issue slot, then pair compatibility
for the second. It scans the priority queue for the first eligible node;
latency or resource blocking can skip the head. Successor earliest times
include the current cycle, nonnegative selected-resource delay, and edge
latency. Reservations are charged after the issue group, then decremented
at each cycle. Load/store and particular opcode forms add latency in
`430388`; the model consumes the completed graph's edge latencies.

[EVIDENCE] `model.py` simulates complete all-hard-edge P5 windows from the
input DAG, before observing selections. It also independently checks heights,
critical flags, priorities, insertion results, and each pipe-selector result.
`tracer.py` flushes predictions to `*.predictions.jsonl` at window entry,
before allowing c2 to make a scheduling decision. It uses breakpoints only;
all replay function payloads must equal the normal captured compilation.
`colour_trace.owned_run` retains the process handle, awaits exit and reaps
the process, including on errors. Captures/replays have unique private names.

## Validation and limits

[EVIDENCE] `validation-summary.json`: 10 micro sources and the target trace;
25 complete windows / 599 selections exactly predicted; 5,735 local checks
with no failure. The target's complete 64-selection window is predicted
before execution in `split-final.sched.json.predictions.jsonl`. Three simple
micro replays also retain pre-execution prediction files. Source-level micro
hypotheses were written before compilation in `micros/*.prediction.json`.

[EVIDENCE] One scoped-buffer micro window has nonzero address-rewrite edge
categories and is explicitly excluded from complete simulation. Its local
checks still pass. Complete simulation excludes other CPUs, phase 1,
nonblocking/address-rewrite dependencies and nonzero priority-boost modes.
The measured corpus is integer code; this is not a validated x87 scheduler
model. `selected_first_eligible` in the JSON counts is only a last-trial
consistency check; complete-DAG simulation supplies the independent winner
prediction. The model takes lowered tuples and graph edges as inputs; it
does not predict front-end lowering, allocation or alias analysis from C++.

## Reproduce

```powershell
.pyenv\python.exe scratch/sol-sched/tracer.py scratch/sol-sched/captures/baseline-c2c916d61112 DrawButtons scratch/sol-sched/recheck.json --line-min 636 --line-max 643
.pyenv\python.exe scratch/sol-sched/model.py scratch/sol-sched/recheck.json
.pyenv\python.exe scratch/sol-sched/report.py scratch/sol-sched/recheck.json 130
.pyenv\python.exe scratch/sol-sched/micros.py tie_loads unknown_pointer stack_array
```

[EVIDENCE] All commands are serial. Replays create a fresh directory; do not
run them concurrently. `report.py` prints the ready list and each chosen
tuple; the full trace also records unsuccessful pipe attempts.
