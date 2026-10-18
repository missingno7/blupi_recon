# VC5 SP3 c2 global register allocator

A bounded, executable model of how c2 (VC5 SP3, SHA-256 e75aecaf...0515fae) assigns callee-saved registers and stack
homes. Worker sol-alloc, 2026-10-18; full model (615 lines), 14 micros with registered predictions, application
captures and audits: scratch/sol-alloc/ (ignored). Tools: `tools/c2trace/colour_trace.py`, `tools/c2trace/alloc_micros.py` (micro sources and registered predictions in
msvc5kb/micros/alloc). KB rules BLUPI-ALLOC-01..12.
Late temp assignment (rotor, scratch picker, preference pass) is a separate, later stage: BLUPI-REG-06..08.

## Pipeline ([EVIDENCE])

1. Eligible literal reads become class-13 virtual constants (pool scan 0x414a64, eligibility 0x4156b6, intern
   0x417fb7). Symbol webs exist before allocation.
2. 0x416a8d turns webs into range descriptors (0x44 bytes, allocated at 0x40ebb6) and records a definition traversal
   rank (+40). 0x416e6a drives allocation: copy simplification (0x41c72e, 0x41ca9b), block sets (0x41cdea), register
   availability and pressure (0x41759e, 0x41be92, 0x41beb8).
3. Cost pass 0x40ee1d: block weight `W = 1 << ((uint16(block+86) + 1) & 31)` when optimizing; benefit +=
   `W * operand_benefit` (0x40fada: 2 for ordinary operands, usually 1 for constant memory operands, 0 for a zero test or
   a usable immediate); materialization/reload/store costs subtracted (0x40ed6c: 1 for class 13, else 2). Results:
   +24 reference count, +3c weighted profitability, +0c signed queue priority. 0x41a6f8 drops low-use ranges.
4. Queue: 0x41bdd7 walks 1024 buckets (`LR & 1023`) and inserts with 0x41b6fa, a doubly linked insertion sort on
   `(signed +0c priority, unsigned +40 rank)` descending; exact ties go **newest first**. No LR-ID, symbol-ID or
   reference-count tie-break. Queue order is not selection order: 0x437c69 defers class-13 constants to the tail and
   overwrites their priority with the tail's; 0x45aaf9 can requeue a substitutable definition at `min(input
   priority) - 1`.
5. Pressure and splitting: 0x41a985 scans the chosen range against pressure, may add op-17c cuts that protect
   already committed ranges (0x437d7c), and rematerializes constants; 0x437e67 splits groups into connected fragments
   (0x438544), rewrites tuples and requeues them (0x438f32). [OPEN] Not every pressure/cut predicate is reimplemented.
6. Register choice, 0x41b785: nine signed penalties start at 0. For each interfering range v with benefit > 0, add v's
   copy-preference counts to registers still available to v, and `100 * v.benefit` to r if r is v's only available
   register. Subtract the chosen range's own preferences. Take the first eligible register with the **strictly lowest**
   penalty in the class order `eax, ecx, edx, esi, edi, ebx, ebp` (so saved registers tie-break esi, edi, ebx, ebp). Binding
   at 0x41b90e; 0x41ba2b removes the register from neighbours and handles exhausted availability (constant-definition
   cloning via 0x44ca36/0x451ac3).
7. Late demotion: 0x42ac33 recomputes each physical register's aggregate constant profitability (descriptor +34;
   ordinary nonconstant uses protect it with 0x1000000); 0x42bf00 removes a saved-register binding scoring **below 3**
   and 0x443d3f restores immediates. Score > 0 is enough for global allocation, not for keeping a callee-saved register.
8. Stack homes: 0x4367f0 keeps nodes grouped by ascending size; each memory reference adds weight 1 (no loop weight)
   and moves a node backward only past **strictly lighter** same-size predecessors, so equal weights keep the history
   of earlier overtakes. 0x43f93b/0x440cbd pack slots by a backward scan of existing slots (first non-interfering slot
   at least half the size), then assign offsets by frame mode; frames above 128 bytes add a density quicksort (0x459eb7).

## Validation ([EVIDENCE])

From actual IL inputs: queue insertions plus colour choices 2124/2124, full penalty vectors 593/593, late demotion lists
13/13, stack reference histories 9/9, packed groups 7/7, logical frame offsets 7/7 (14 micros, 4 game captures; tracer
payloads equal native compiles). Source-level micro predictions registered before compiling: 11/14 (misses: equal
refs are not equal benefit; a zero-benefit base-only store; a dependency requeue not triggered). A fresh supervisor
capture of CDecor::Build passes all 658 model checks.

## What it says about the open SB1 functions

- **ChangePhase:** no tie-break puts zero before WM_PHASE_PLAY. The sorted sequence is PLAY/9 (-78), ZERO/21 (-65)
  selected, then PLAY/8 (-63), which did not exist yet when zero was chosen. Zero's penalty vector is EBX +100 and
  everything else 0, so its only available register is EBX, reserved by the PLAY/9 singleton. If zero could use EDI it
  would take EDI and leave EBX for PLAY, the oracle's roles, without any sort change. The canonical index in EDI overlaps
  zero's late consumers (index test, HideMouse argument, Write flag), so the source condition is a different overlap or
  split: zero separated before that index, its late uses rematerialized. Adding PLAY references does not help.
- **Build:** the pooled zero (9 refs) survives in EAX; `rect.bottom`'s 0x1e0 hold is constant-definition cloning
  after rect.left binds EBP (exhausted availability); the swapped [esp+0x20]/[esp+0x24] homes come from the strict
  overtake history of two equal-weight slots (reversing the final read pair reverses the homes).
- No new STRICT result; the bounded source applications are listed in scratch/sol-alloc/MODEL.md.

## Use

- `tools/c2trace/c2trace.py capture NAME src/X.cpp`, then
  `tools/c2trace/colour_trace.py NAME --function N --out build/c2trace/colour/NAME.json [--uses] [--costs] [--cuts]`
  (N = zero-based compile order; ChangePhase 51 in event.cpp, Build 15 in decblock.cpp). It prints every sort
  insertion, selection with its penalty vector, rematerialization, demotion and stack-home step, and checks each
  against the model.
- Read a register residue as: which range wanted which register, what made it unavailable, and which penalty or
  overlap decided it. Then look for the source condition (a value's lifetime, a join, a use before or after a call)
  that changes that input. Use counts and declaration order are rarely the lever.
- Portable to other VC5 SP3 projects; other c2 builds need the addresses re-verified (the tracer refuses other images).

## Pressure cuts and saved colours, round 2 ([EVIDENCE])

Workers sol-alloc (ChangePhase) and sol-build3 (Build) applied the model; neither function became STRICT.
- Cut pressure (0x41a985, BLUPI-ALLOC-08): after the definition list, `P = physical/chosen + local temporaries +
  max(definition additions, read additions)`; other unbound globals are not counted one each. `P > capacity` evicts
  temporaries; `P == capacity` requests a cut unless tuple+12 flag 01 is set; the chosen LR is excluded from its own cuts;
  a call contributes three definitions. 945/945 tested IL steps; 6/9 registered source micros.
- Saved colours (BLUPI-ALLOC-09): literal pooling is function-wide, so a later real zero read keeps the constant alive
  after an earlier store's scope ends; a lexical scope boundary does not free a saved colour, a real last use does.
- ChangePhase: the index reaches only 5-6/7 pressure, while zero reaches 7/7 after it is committed, so the cuts protect zero
  and split PLAY. The oracle needs zero's three late uses separated while the index keeps its long EDI lifetime. An
  early-store control produced the oracle's register roles but contradicted the oracle's instruction order (58 regions);
  discarded. [OPEN] the edge cuts at 0x439619 that would separate zero before the index's late-use region.
- Build: a height form removes the rect.bottom cloning but changes the Y/left register roles; a slot-order form reaches
  43 regions but regresses accepted GetBlupiInfo; the zero stays profitable in EAX (9 refs, +1). [OPEN] splitting the
  camera lifetimes before background Y is allocated. Details: scratch/sol-alloc/changephase/FINDINGS.md,
  scratch/sol-build3/FINDINGS.md "Round 4".

## byte-tactics mechanisms confirmed ([EVIDENCE])

HectorBailey/byte-tactics (Total Annihilation decomp) uses the identical c2.exe. Workers sol-alloc, sol-evanc and sol-build3
re-checked its claims on our /O2 /GX builds:
- **Confirmed (BLUPI-ALLOC-10):** range descriptors reuse the last-freed ID (0x40ecd2 pushes onto 0x493230; 0x40ebb6 pops
  before allocating fresh IDs), and the first queue fill walks buckets id&1023 in order. Exact ties therefore depend on
  allocation history.
- **Confirmed (BLUPI-ALLOC-11):** the density sort (0x459eb7) runs only when packed ordinary bytes exceed 128; it is unstable.
  Reusing a fixed caller home can restart packing.
- **Confirmed (BLUPI-ALLOC-12):** a source increment of a stack local counts its read and write as two stack-home references.
- **Refuted or unvalidated on our flags:** a no-code IL node shifting the rotor (TreatEventBase, six forms); source-path
  length moving IDs without /Gi; a general "split local +1" reference bonus.
- Applied without a strict gain: DrawButtons (63 bytes; stack homes already match), ChangePhase (zero/PLAY keys unequal),
  Build (slot swap traced to `lastClip` scalarization, recycled IDs and reload order).
