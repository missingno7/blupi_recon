# VC5 knowledge transfer to Blood II (handoff, 2026-10-19)

For the blood2_recon supervisor. Self-contained; nothing here was written to blood2_recon. Source rules:
`msvc5kb/catalog.json` (read with `.pyenv\python.exe msvc5kb\tools\kb.py rule ID`). Blupi state: branch
`claude/final-blockers`. Blood II state surveyed read-only at `d676dee4` (`recon/knowledge/compiler/vc5.json`,
93 rules; `scripts/c2trace` ported from Blupi `bcbe197`). Compiler: the same SP3 c2.exe 11.00.7303,
SHA-256 `E75AECAF4073B0817FFB638CAE5FFF636B2E2D1A090DAABF8F68DBC954515FAE`, and c1xx `E27DF3BB...11111D3D9` on
both sides, so internal addresses carry over. Blupi flags: `/W3 /GX /O2 /ML /Gd /Gy`. **Nothing below is validated
for Blood II.** A rule becomes validated there only through the measurements in "Validation protocol".

## 1. New transferable rules (absent from Blood II's vc5.json)

| Blupi rule | Recognize | Mechanism (c2 SP3) | Source lever | Blupi evidence |
|---|---|---|---|---|
| BLUPI-ALLOC-17 | Which pooled constant or value keeps a saved register differs; a priority count from the final code does not reproduce the traced priorities | Cost pass 0x40ee1d: at each block end P = W x n_b (n_b = ranges referenced in the block, 0x40eda6; W = loop-region weight). Referenced ranges gain P x operand benefit, live but unreferenced ranges lose P. Benefit 0x40fada: non-constant 2; constant: zero compare 0, memory base/index or frame operand 1, register variable or temp 0. Late peepholes run after costing. One range per pooled value (web builder 0x416a17..0x416a6f; interner 0x417fb7 compares tag + low32). | Block membership and crowding of the competing references; memory vs register operand at the constant's use | ChangePhase priorities reproduced exactly (-65/-78); no source route found (32 bytes open) |
| BLUPI-ALLOC-16 | Callee-saved roles or equal-size homes swapped; code otherwise identical | Queue 0x41b6fa: signed priority, then rank +0x40, exact ties newest first. A placement inside an equal-priority group is identity-decided (fixable by code-neutral rewrites); a placement between different priorities needs a cost change | Tie class: which decisive expression is interned first. Quantity class: references and operand forms | Build fix recovered blind (1 of 10 generic rewrites is STRICT); ChangePhase classified quantity (208 flips, 0 oracle) |
| BLUPI-ALLOC-15 | Two equal-weight stack homes swapped and their boundary reloads emitted in the other order | Fragment IDs pop from LIFO free list 0x493230 in split order; reloads in ascending fragment ID (0x40fbfa); web order follows early-C2 expression interning (0x42127d) | Write the value's plain expression as its own statement where it first appears | Build 8 -> 0 bytes, full gate clean (canonical `1c5c07d`) |
| BLUPI-REG-11 | setcc result register off by one rotor step, all later eax/ecx/edx temps shifted (period 3) | Three routes at 0x435c37: preference via copy into a register-held range (0x42b051), rotor, rotor with busy compare inputs skipped. A store-only boolean has its preference cleared (0x42afe5) | Give the boolean a register consumer (return, pushed argument, byte-capable register local) | Census of 50 setcc temps over 18 TUs: 15 / 28 / 7 by route |
| BLUPI-NEG-04 | (negative) | Dead local stores and `do{}while(0)` / `for(;;){break;}` wrappers are gone before costing; line layout, /Ox /G5 /GB /Op /Ob0 and seven compiler trees do not move allocation | Do not use these as allocation perturbations | ChangePhase priorities unchanged in all variants |

Earlier Blupi rules that the survey did not find in Blood II's imports (it has ALLOC-01..05/07..09, REG-01..05/07..10,
CTX-01..04): BLUPI-ALLOC-06 (cloning a constant definition when its only colour is exhausted), ALLOC-10..14,
COPY-01, C2-01 and SCHED-01/02. Check each against vc5.json before importing; this list is from the survey, not a
diff.

## 2. Existing Blood II rules strengthened by independent Blupi evidence

- **VC5-RA-03** (preference for a copied temp, MICRO). Blupi corpus: all 15 preference-assigned setcc temps use the
  copy-into rule 0x42b051, and none uses 0x42acf4. A store-only result never gets a preference (BLUPI-REG-11).
- **VC5-RA-01** (rotor). Rotor-shift diagnostic: +1 and +2 shifts give the oracle with all later assignments
  unchanged. This identifies a rotor-neutral original block, not a missing pick (BLUPI-REG-06 evidence).
- **VC5-RA-09 / VC5-EXP-09** (pooled zero). One range per pooled value: source cannot create a separate constant
  web. The score is the per-block crowding sum of ALLOC-17, not only +1 per store and -1 per materialisation.
- **VC5-RA-07** (colouring queue). Tie-order vs quantity classification (ALLOC-16) applies to its priority queue.
- **Flags.** Blood II found /G6, /GB and /Ox byte-identical to /G5 /O2. Blupi independently found /Ox, /G5, /GB,
  /Op and /Ob0 inert on ChangePhase under `/GX /O2`.

## 3. Rules that may not fit Blood II's settings (test before use)

- **Server.dll `/O1`** (and FullIntersectLine.cpp). Every Blupi allocator measurement is `/O2`. Blupi's `/O1` builds
  change whole functions (VC5-ORD-02, INL-05). Weights, benefits and queue order must be re-derived on one STRICT
  Server function before ALLOC-15/16/17 are used there.
- **CShell `/Ot /Oi /Oy /Ob1` without /Og.** [OPEN] Whether global colouring and the optimize flag 0x48fb60 (rotor
  path) are active without /Og is untested. Check for queue insertions at 0x41b6fa and rotor moves in one CShell
  capture first. If neither occurs, REG-11 and ALLOC-15..17 do not apply to CShell.
- **No /GX in the engine.** The Blupi evidence comes from EH-framed functions. Frame operands (benefit 1 for
  constants) may be shaped differently without EH state; this is untested.
- **/YX shared PCH in game modules.** TU context (CTX rules) interacts with interning order (ALLOC-15). Keep the
  PCH mode fixed while testing identity rewrites.

## 4. Blood II mismatch signatures worth testing

These rows come from the survey of `analysis/engine_recon_status.json` and `candidates/codex-residues`. The residue
list is partly stale, so recheck each row.

| Function (module, bytes/size) | Signature | Rule to test | Predicted first step |
|---|---|---|---|
| `HNode::Load` Huffman.cpp (Server, ~32/133) | open zero-register CSE (VC5-EXP-09) | ALLOC-17 + RA-09 | Per-block priority sum of the zero range; predict the reference change that flips its sign. /O1 caveat |
| `cis_GetSurfaceMemory` (Client, 4/64) | callee-saved allocation | ALLOC-16 | `tiesweep enum`: tie or quantity class |
| `pal2_ArePalettesEqual` (Client, 13/64) | callee-saved allocation | ALLOC-16, then ALLOC-17 | Same; small enough to decompose all costs |
| `FitIntoPushawayPlanes` collision.cpp (Server, 5/440) | stack-slot swap `[ebp-0x74]`/`[ebp-0x78]` + count window | ALLOC-15 (+ ALLOC-03) | Check whether the two homes have equal weight; trace split children order |
| `Rotate_Update` effects.cpp (Client, 37/560) | frame-slot | ALLOC-15 / ALLOC-03 | Stack-home weights and overtake history |
| `CheckIntersectOnMovement` MoveObject.cpp (Client, 6/3936) | load order / spill-reload, count-unreachable | ALLOC-15 | Are the reordered reloads boundary reloads of one split? If yes, the interning order of the two values decides them |
| `IntersectLine` intersect_line.cpp (Server, 39/577) | frame-slot + callee-saved | ALLOC-16 first | Separate the queue class from the home class |

Rows dominated by x87 order or hashed-ID SIB order (PopSmallest, SSMemStream::Write, MatVMul) are outside these rules.
Rows closed only by count windows belong to CTX-10/11.

Before testing a count-window or declaration-context explanation on any row, screen the region with
`keyforce.py list NAME FUNC --equal-cost --events A-B` (Blupi BLUPI-CTX-05): only comparisons with equal cost halves
can depend on symbol IDs. In Blupi's ChangePhase this closed the hypothesis without a single forced replay.

## 5. Minimal positive controls (before any unmatched function)

Run each on a function that is already STRICT, in the module and flag set where the rule will be used:

- **P1, queue reader.** Run `tiesweep enum` on a STRICT Client `/O2` function with two or more callee-saved ranges.
  The baseline must be 0 masked bytes, and at least one flip must change code. This shows that the node layout
  (+0x0c, +0x40, link +0x14) and the identity groups are read correctly.
- **P2, cost model.** Take a STRICT function where `xor r,r` stays in a register across blocks. The per-block sum
  (breakpoint 0x40eda6) must equal the traced priority of the zero range. Repeat once under Server `/O1`. A
  mismatch falsifies ALLOC-17 for that flag set.
- **P3, boolean routes.** Run `booltemps` on one Client TU and confirm the pref/rotor/rotor-skip classification
  against the emitted registers. Under CShell flags, first check that any rotor path exists at all.
- **P4, fragment order.** Pick a STRICT function with a paired boundary reload. Forcing the drop of a range (the
  freelist `force` diagnostic) must flip the reload order as ALLOC-15 predicts.

## 6. Validation protocol (sample of 5-7 functions from section 4)

1. For each function, register before compiling: the class (tie or quantity), the predicted source change, and the
   predicted instruction effect.
2. Compile candidates only through Blood II's own pipeline (`scripts/agent/search.py`, `engverify.py`), one at a time.
3. Record:
   - STRICT functions gained;
   - masked-byte and aligned-instruction change;
   - module-gate regressions;
   - experiments whose outcome contradicts the prediction.
   A contradiction falsifies transfer for that flag set, even if a different candidate happens to match.
4. Promote a rule in vc5.json only from these Blood II measurements, citing this document and the Blupi rule as
   provenance. "Proven in Blupi" alone stays a hypothesis there.

## 7. Porting notes

- Tools: `tools/c2trace/tiesweep.py`, `booltemps.py` and `freelist.py` are written against Blupi's
  `c2dbg.py`/`colour_trace.py` and compare against BLUPI.EXE (`freelist.oracle_diff`). Blood II has
  `c2dbg.py`/`colour.py` but not freelist, tiesweep or booltemps.
- To port: replace capture compilation (`fnprobe.compile_unit`) and the oracle diff with Blood II's
  capture/engverify paths. The breakpoint addresses and node offsets are compiler facts and need no change.
- Each replay is bounded to 300 s. Keep the one-command-at-a-time rule.
