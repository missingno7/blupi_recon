# VC5 SP3 c2 global register allocator

A bounded, executable model of how c2 (VC5 SP3, SHA-256 e75aecaf...0515fae) assigns callee-saved registers and stack
homes. Worker sol-alloc, 2026-10-18; full model (615 lines), 14 micros with registered predictions, application
captures and audits: scratch/sol-alloc/ (ignored). Tools: `tools/c2trace/colour_trace.py`, `tools/c2trace/alloc_micros.py` (micro sources and registered predictions in
msvc5kb/micros/alloc). KB rules BLUPI-ALLOC-01..07.
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
