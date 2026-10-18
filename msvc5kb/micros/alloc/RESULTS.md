# Predicted / observed controls

[EVIDENCE] Predictions in `PREDICTIONS.json` were written before the first
nine compiles. Sources, captures, native objects, disassembly, normal compiler
logs and unperturbed debugger JSON are retained here. All compilation uses
`tools/msvc.py` with `/W3 /GX /O2 /ML /Gd /Gy`; default compile-slot limits.
Micro programs are compiler input only and are never executed.

| Micro | Prediction made before compile | Observation | Verdict |
|---|---|---|---|
| constants_equal | Both four-store webs have equal benefit; equal keys use newest insertion first | Refs 4/4, but benefits **2/3**, priorities **9/12**, rank 0/0. EBX/EDI selected; EBX later removed | **FAIL**: equal refs insufficient |
| constants_plus_one | One additional displaced store adds one ref and one benefit, with unchanged boundaries | First refs 4->5, benefit 2->3, priority 9->12. Keys tie 12/0, second constant stays ahead. EBX now survives in assembly | held |
| loop_flat | Three stores give benefit 3 minus materialization cost; profitable constant survives | Only **two** stores earn benefit; one 139 definition, final benefit **1**, priority 4, ECX | **FAIL**: the explicit three-benefit assumption was wrong; pooling/positive survival held |
| loop_depth | Actual loop depth multiplies benefits; higher score than flat with three stores | Final benefit **7** versus 1; priority 32; EDX | held |
| call_live | Non-rematerializable scalar crossing Tick requires a saved colour or home | Pointer ESI, cached scalar EDI; two global ranges | held |
| call_dead | Moving scalar last use before Tick removes its saved-colour requirement | Scalar disappears from global selection; pointer ESI remains | held |
| slots_ab | Homes follow reference history with strict overtakes | Final equal weights 3/3, a before b, frame offsets a=-8/b=-4 | held |
| slots_decl_ba | Only declaration reversal leaves home order/offsets unchanged | Same reference stream and disassembly as slots_ab | held |
| slots_b_extra | One extra late b reference strictly overtakes a | b=4/a=3, b before a; offsets b=-8/a=-4 | held |

[EVIDENCE] Initial source-prediction result: **7/9**, retaining both failures.
The independent **post-IL** insertion/choice checks are **30/30** (15 sorts,
15 register choices); all 15 penalty vectors agree, although these first
controls have no nonzero interference penalty. Every traced object function
payload equals its normal compile payload. These are different measurements;
the latter is not a 100% C++-to-assembly prediction claim.

[EVIDENCE] Both `constants_equal` values have **one** synthetic materialization.
The asymmetry is the base-only `p[0]` store, whose benefit is zero. It is not
an extra materialization around Tick. The same addressing caveat explains
the `loop_flat` numeric failure. The original failed predictions remain
unchanged. The revised equality control uses only displaced addresses.

[EVIDENCE] Subsequent predictions were recorded in numbered ledgers before
their compiles. The first nine sources and failed predictions were preserved.

| Added micro | Prediction before compile | Observation | Verdict |
|---|---|---|---|
| pressure_call | Seven independent call-crossing values exceed four saved colours; real splitting/spilling is required | Four bindings ESI/EDI/EBX/EBP; three exhausted ranges split into memory homes; singleton penalties predicted | held |
| constants_equal_displaced | Both have refs 4, benefit 3, priority 12, rank 0; second inserted first | All four numbers match; second ECX before first EDX | held |
| dependent_web | Reused y=x+3 with a live x input enters dependency requeue/substitution | No successful 45aaf9 entry; y binds normally | **FAIL** of proposed C++ trigger; formula not dynamically validated |
| slots_tail_ba | Same first references and final counts 3/3; reversing only final reads makes b first | b=-8/a=-4, versus a=-8/b=-4 in slots_ab | held |
| background_clone | Background-only reduction reproduces cloning of bottom=480 after an interfering binding exhausts availability | Bottom source 20 / LR17 has refs 2, benefit 18, empty set; mode-1 eligibility succeeds, 451ac3 clones it | held |

[EVIDENCE] Final source-prediction result: **11/14 (78.6%)**, three failures
retained. There were **24 native micro compiles**, including controlled
repeats and one compile followed by a callback-formatting error. That
instrumentation error is archived in `instrumentation-error.json/.log`;
the compiler finished normally and its payload equaled the native compile.
The corrected tracer was revalidated on all 14 captures without another
front-end compile (`run_micros.py --replay-only`).

[EVIDENCE] All final trace callbacks are error-free and every micro's raw
function payload equals its normal compiler output. Across these micros
plus four application captures, `SUMMARY.json` records **2124/2124** checked
queue/colour decisions, **593/593** penalty vectors, **13/13** late cleanup
lists, **9/9** home-history orders, and **7/7** packing / **7/7** logical-offset
predictions. These post-IL totals are deliberately separate from 11/14
source predictions. Build contributes a full 11-group / 36-home prediction.

[EVIDENCE] No positive dependency-priority rewrite was observed. The formula
min(input priorities)-1 remains static evidence. Large-frame/double-alignment
branches and special interference expansion also remain bounded by MODEL.md.
Micros were never run as programs; only the pinned compiler was executed.

[EVIDENCE] A later optional post-IL helper ledger, `../CUT-PREDICTIONS.json`,
was recorded before its debugger replays. `check_cuts.py` preserves 5/5
cut-set predictions in pressure_call and 29/29 in background_clone, both
including an actual class-capacity threshold. Together with Build and
ChangePhase this is **1236/1236** helper sets, kept separate from the main
queue/colour total. No local-temporary eviction occurs in these captures;
that helper's scan is statically modeled but has zero dynamic checks.
