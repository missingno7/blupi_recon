# Residual causes of the four open functions (2026-10-19)

Strict acceptance is unchanged: a function counts only when STRICT. This page groups the remaining machine-code
differences by independent root cause. Counts are aligned-instruction differences (tools/residues.py), not positional
bytes; Build is 38 bytes longer than the oracle, so positional byte counts shift past its first difference.

| Function | Aligned / oracle instructions | Differing instructions |
|---|---|---|
| TreatEventBase | 1886 / 1894 | 8 |
| ChangePhase | 2021 / 2029 | 8 |
| DrawButtons | 2304 / 2304 | 0: STRICT (2026-10-19) |
| CDecor::Build | 1237 / 1237 | 0: STRICT (2026-10-19, B-r solved; section at the end) |
| Total | 3907 / 3923 (two open functions) | 16 |

## Causal map

Each cause below has an in-memory compiler diagnostic that removes its whole group. These are diagnostics, not source,
and not proof that the oracle's internal state was identical. "Demonstrated in C++" means an ordinary source change
reproduces the effect.

| # | Cause (first divergent decision) | Stage | Downstream effect | Diagnostic result | Demonstrated in C++ |
|---|---|---|---|---|---|
| B-a | No singleton cut of the pooled constant 1 at the first reverse-moving type load; decided by batch region adoption (BB85 adopts region 1 with four compatible registers), not by the 7/7 pressure test | global allocation 0x416e6a / 0x41b2fa..0x4399cc | moving B55-B74 and lift B101-B126 register roles, saved-register choices: about 50 of Build's 60 instructions | 1177 -> 1227 aligned alone (scratch/sol-build4/round5, sol-cbuild) | No (CButton::Draw shows the mechanism, BLUPI-ALLOC-13) |
| B-b | Cloud-Y offset hoisted with cloud-X by per-loop LICM 0x43ada6 (both pass 0x43af4b availability); oracle keeps Y in the loop | early C2 0x42127d | cloud B184-B186 | needs B-c for exact cloud | No (varying-Y forms leave extra code) |
| B-c | Normal-player initial-Y result copy (142 temp = load - camera; 131 field = temp) not contracted; adds an 18,000-point camera interference penalty (normal Y EBP vs EDI) | C2 reader / value reconstruction 0x409c5c | B47 original-Y save, player registers | with B-b: cloud and B47 exact (bc 1179; abc 1229, 8 bytes) | No (split/RMW spellings are reconstructed) |
| B-r | Remaining 8 bytes after a+b+c: stack home / reload order | stack packing | 8 masked bytes | unexplained | [OPEN] may be a fourth cause or a consequence of the exact form of a-c |
| C | Pooled zero takes EBX before PLAY (zero -65 vs PLAY -78 with the literal oracle test order); EDI is held by the index range | global allocation (selection event ~1600) | all ChangePhase register roles (canonical currently uses a reordered guard to approximate) | zero-only cut at the first late zero consumer: 0 masked bytes, 2029/2029 | No |
| D | The two split-title x = 330 definitions are placed early (C1 emits font/text first; 0x42127d moves x first); eax unavailable at the first text address | early C2 0x42127d | the one 16-instruction block (order + registers) | both definitions moved before their pushes: 0 masked bytes | No (PB DrawTextCenter shows definitions are never moved later) |
| T | One missing temp-rotor assignment at the terminator-byte test (compare-embedded scratch DL, no assign call) or, equivalently, an old-flag split in the rank-0 toggle | temp assignment 0x42a8aa | the rank-0 toggle registers and the PostMessage HWND regions (3 regions) | split terminator load: 0 masked bytes (scratch/sol-teb/round3) | No |

So 92 differing instructions are removed by **six sufficient compiler-state interventions** (B-a, B-b, B-c, C, D, T),
which is not the same as six independent historical source causes: one upstream source difference may produce several
of them (the B-a/B-b/B-c subset tests show the interventions are independent of each other, not that their source
preconditions are). Build's 8-byte residue
(B-r) is not classified until natural source solutions for B-a..B-c have been tested. None of the six is yet produced
by ordinary C++. Goal: the natural precondition of each decision (region topology, live-range boundaries, def/use
relationships, alias information, expression structure) that the unmodified compiler would follow, e.g. for B-a what
changes BB85's region adoption, rather than the cut itself.

## Shared mechanisms

- B-a and C are the same allocator mechanism: whether a pooled constant (1 in Build, 0 in ChangePhase) is cut at a
  call/loop boundary. In a STRICT function the stock compiler cuts it under call pressure (CButton::Draw, ALLOC-13).
  The source lever is therefore the set of values live across that boundary, not the constant's own spelling.
- B-b, B-c and D are all early-C2 normalizations (0x42127d LICM / value reconstruction) that erase a distinction a
  source spelling would otherwise carry (BLUPI-C2-01). A solution must survive those passes without extra native work.
- T is local to one temp assignment.

## Highest-value experiment

B-a alone moves Build from 1177 to 1227 aligned instructions (50 of 92 total). The experiment: find which value live
across the first reverse-moving call (or present in the predecessor region) changes BB85's region adoption so the
singleton-1 cut appears, using only source the oracle's loop accesses allow (no extra load, same cursor anchor).
The same question, asked of ChangePhase's index/mouse boundary, targets cause C.

Sources: docs/COMPILER_STUDY.md (latest sections), scratch/sol-build4/round5, scratch/sol-cbuild (r1-r2),
scratch/sol-ccp, scratch/sol-cdb, scratch/sol-cteb, scratch/sol-teb/round3, scratch/sol-sched/round2.

## Update 2026-10-19 (sol-cutcp, sol-cbuild r3, sol-cteb r2)

- C and B-a share the allocator route: both go through the large-group batch region adoption at 0x438f79/0x43918b.
  Build's header adopts with four free compatible registers; ChangePhase's earlier zero groups adopt a common region
  (3/2/1 free saved colours) so zero is already protected at the mouse calls. In both functions the shaping group
  traces to an induction cursor from an earlier loop (Build: source id 2058 cursor; ChangePhase: the m_bNamesExist
  loop cursor). [HYPOTHESIS] one source property class (how those earlier loops are written) may control both.
  Best ChangePhase partial: release 2025/2029 with an early load and PLAY cut to 8 reads (rejected; eGames 1914/2034).
- B-b: whole-expression LEA recreation exists in the stock compiler (STRICT BlupiAddFifo, induction RMW); both cloud
  LEAs qualify, but argument copies reject substitution before costing.
- T: blockers named (old+1 folded into the address early; 0x41dd18 keeps the original operand; 0x435d65 preference
  and 0x42dcb5 LEA->INC; the definition-less byte is globally eligible). TextInputDelete is a STRICT anonymous-byte
  rotor assignment but with a different address/order shape.
- Now: C2 decision reverse engineering of 0x438f79 adoption (sol-cbuild r4) and of the TreatEventBase predicates
  (sol-cteb r3).

## Update: TreatEventBase predicates (sol-cteb r3, 2026-10-19)

Chain (EVIDENCE unless marked): `m_posCheat++;` then `cheat_code[m_rankCheat][m_posCheat] == 0` -> C1 int increment record,
statement end, byte read + zero + equality (no byte assignment record) -> C2 compare op2f with kind-6 memory input ->
legality dispatch 0x40e1c0 (not taken), 0x40e3c3 / 0x40e4b2 taken -> materialization mask 0 at 0x40dfe0, no extraction;
0x41dd4c finds an adjacent compare, no preservation temp -> LEA kept because 0x42dc02 sees unequal base/output bindings
-> oracle-identical terminator instructions. The first unexplained difference stays at +0x41b (rank-0 toggle).
STRICT TextInputDelete's rotor byte has a real defining tuple from lowering (non-null kind-3 definition, excluded at
0x414e4f); the distinguishing source property there is the consumer opcode/destination (a byte copy, not a compare).
[HYPOTHESIS] The oracle's extra rotor step need not be a terminator byte; the old-flag alternative stays open. No source
form is predicted by the recovered predicates; next is a C1-reader/CFG slice of a real compare-result across a state
store (before 0x412d5c). Pseudocode: scratch/sol-cteb/r3/PSEUDOCODE.md.

## Update: B-a decision predicates (sol-cbuild r4, 2026-10-19)

Pseudocode: scratch/sol-cbuild/r4/PSEUDOCODE.md (dispatch 0x438f79, caller 0x41ba2b, adoption 0x43905c/0x439060,
scalar 0x439385/0x439619, cut constructor 0x437d7c, component rebuild 0x437e67/0x438544). Exact predicates [EVIDENCE]:
- 0x438f98: exhausted group G < 3 -> scalar scanning, which can emit singleton cuts;
  CButton::Draw (G = 3) instead reaches the scalar route through the block-count test below.
- 0x438fa1: per-function CFG block count >= 100 -> batch route (counter 0x4910e8, reset 0x416eab; Button 8, Build 205).
- 0x439060: in the batch route the header adopts the predecessor region unless (A - O) - U is empty; Build keeps
  {EAX,ECX,EBX,EDI}, so it adopts and mixed edges get full-group cuts, not the singleton.
- Build's G has 12 members because the cursor RMW unions its backward live set (0x41ac8e); costs leave 1 with {EBP,EDI},
  moving X takes EDI, the cursor takes EBP, exhausting all 12 together.
Natural preconditions for the singleton: a smaller exhausted group (G < 3, source-controllable: icon-strip witness
produced singleton-1 cuts natively, but homed Y to the stack) or an empty compatible header set; both need different
real cursor/lifetime relations. Block count is a whole-function property (205), not a lever.
Next: the lift cursor LR128 and reverse cursor LR127 share source variable 449; test whether separate traversal
variables change the late-one web's EBX exclusion or its group, keeping both native cursor LEAs and all loads.
- [EVIDENCE, sol-cbuild r5] Eliminated: shared versus separate traversal variables for the lift and reverse-moving
  loops. 0x41615d partitions definitions into the same two cursor webs either way (identical 592-byte payloads,
  G stays 11/4). ChangePhase's shaping cursor is compiler-generated, so the same normalization applies.
- [EVIDENCE, sol-cbuild r6] Oracle member audit at the reverse-moving header: the exhausted group's membership is
  oracle-consistent (literals immediate, counter homed at [esp+58], posDecor.x/.y homed). Lifetimes that differ (reverse
  X held in EDI once, lift camera X held in EBP across the lift loop, lift Y read from its home) are consequences of a
  different connected partition and its definition/reload costs (0x40ee1d, 0x40fada, 0x40ed6c), not of an extra or
  missing source value. A pre-lift cache micro normalizes away (identical 592 bytes). B-a is an exhaustion-order /
  partition-cost question; the oracle cannot reveal its internal priorities. Next possible step: slice the first
  differing component rebuild (reverse X two-bound child, lift X 8-read child) baseline vs sufficient control.
- [EVIDENCE, sol-cutcp2] ChangePhase under the B-a predicates: 345 blocks (batch route); earlier zero groups keep
  3/2/1 compatible registers; the m_bNamesExist cursor is created by C2 (0x43de8b), so loop spelling does not control it.
  Minimal input changes holding the rest fixed: group 2 shrinks from 8 to <= 2 members, or loses both EBX and EDI from
  (A-O)-U before Search, or a pre-existing zero boundary separates the late reads. No ordinary source property is
  predicted; next: trace the first loss of each saved colour for zero and the seven neighbours (0x41759e/0x41be92).
- [EVIDENCE, sol-cbuild r7] B-a downstream costs are consequences of the cut itself: the injected singleton changes
  the first partition before remote-cursor rebuilding (explanation A); reverse X benefit 6 and lift X 62 (baseline 14)
  follow from it (1,135 native benefit updates verified). The controlled difference implies no upstream source property;
  the natural cause of the cut remains [OPEN]. Further slicing of this counterfactual cannot reveal it.

## Update: DrawButtons cause D reduced (sol-cdb r2, canonical 2026-10-19)

Source change: title `pos.x = 330` set once before the save-row loop; the number labels use their own `POINT numberPos`
(x 296, y = pos.y); the three per-iteration title x resets are removed. Chain [EVIDENCE]: one loop-invariant title x ->
C1 early x = 330 with whole POINT arguments -> C2 reader: one 4-byte field descriptor, no partial-cover substitution
(0x40b803/0x40b811), x stays early -> lowered loop-spanning source-field range (LR149, refs 4, benefit 26) with no
available register -> 0x41ba2b / 0x44ca36 -> 0x451ac3 validated-site cloning -> literal 330 cloned immediately before
each x push; the early definition dies. Positive control: STRICT DrawInfo clones a field definition the same way.
Result: 63 -> 30 differing bytes, 2288 -> 2300 / 2304 aligned; zero accepted regressions. The late definitions are
produced by the stock allocator, so the earlier 'definitions are written late' inference was wrong: they are cloned.
Remaining: 4 instructions, a subtraction ordering decided before C2 stage 0x412d5c (sol-cdb r2 FINDINGS).
- [EVIDENCE, sol-cbuild r8] The DrawButtons clone mechanism (0x41ba2b -> 0x44ca36 -> 0x451ac3) already operates in
  Build: whole-LEA cloning in the network-player cloud loop. The target cloud-Y range still has available registers when
  selected, so it never reaches 0x451ac3; the initial-Y range is blocked by flag 0x08. [HYPOTHESIS] B-b is the same
  clone, reached only if cloud Y finds no register; that depends on the selection order of original Y, derived Y and
  shifted X (next trace). One micro failed (no clones, extra coordinate store).

## DrawButtons STRICT (sol-cdb r3, 2026-10-19)

The last 4 instructions: naming the truncation length, `int cutLength = line2 - m_filenameBuffer[i]; text[cutLength] = ' ';`,
gives the predicted C2 operand-key order (symbol-ID key at 0x408d58; scratch/sol-cdb/r3/FINDINGS.md, PSEUDOCODE.md).
DrawButtons: 0 differing bytes, 2304/2304, all 234 relocations verified; full gate accepted it STRICT with zero regressions
(coverage 116,160 -> 123,584). Cause D is closed by two natural source properties: a loop-invariant title x with a separate
number-label POINT (allocator cloning), and a named cut length (operand key order).
- [EVIDENCE, sol-cbuild r9] Network cloud vs target cloud: the network original-Y range wins at priority 320; the
  target original Y ties at 76 with derived Y and shifted X and loses, so cloud Y keeps a register and is not cloned.
  A micro's +16/-16 credits cancelled (original Y 55 < derived Y 59, no clone). Next: a STRICT asymmetric reg+constant
  clone positive control (one axis held, the other cloned from its held base).
- [EVIDENCE, sol-cbuild r10] No STRICT asymmetric clone control exists: across 409 STRICT functions (19 audited
  replays) four held-base address clones occur, none with Build's three-range held-axis / cloned-derived pattern. The
  matched corpus does not demonstrate B-b's precondition; its source cause remains [OPEN].
- [EVIDENCE, sol-cteb r4] Clone hypothesis for T eliminated: TreatEventBase has 29 exhausted ranges but zero
  clone-safety calls or clones, and no current range supplies the diagnostic's extra old-flag load. T stays open.
- [EVIDENCE, sol-cteb r5] Front-end trace of the rank-0 toggle: C1 emits field read, branch, two stores, then
  bEnable = field. Early C2 backward reconstruction (0x40cbc8 / 0x40cd08) creates a short old-flag definition, and
  0x41ddfe folds it into the adjacent compare before if/else lowering, so it never reaches temp assignment. The earliest
  ownership-dependent branch is 0x40cc03: a named (kind-4) local is excluded from anonymous backward splitting, the
  anonymous field value (kind-3) passes. Required for either explanation: a short definition that survives 0x41ddfe
  because a real intervening definition/use prevents substitution, with the same emitted MOV/XOR/TEST/SETcc. Next: find
  such a survival in stock/STRICT code. A POINT micro produced neither shape.
- [EVIDENCE, sol-cutcp2 r3] ChangePhase before allocation: zero's synthetic definitions share one initial pool
  identity fixed at 0x414ba6, before web partition 0x41615d; later fragments and groups follow from that web and ordinary
  lifetimes. Named copies collapse early (copy propagation). A real counter-definition epoch keeps a snapshot through
  early C2 and changes 0x43de8b's induction representation but adds 18 body bytes (fails the instruction gate). Needed IR:
  a separate late-zero component (index sign test, HideMouse argument, pre-Write flag) with PLAY connected through
  WriteInfo. Next: backward slice of reader finalization 0x4262d8 and 0x425f81/0x44422c preservation on existing captures.

## Build B-b/B-c reproduced by source (sol-cbuild r11, canonical 2026-10-19)

Source change in the normal-player branch: `long screenY = m_blupiPos.y - posDecor.y; tinyPoint.y = screenY;
pos.y = screenY - 34;` with the cloud loop's per-iteration `pos.y` assignment removed (pos.y is reassigned before every
other read). Chain [EVIDENCE]: C1 gets a real LONG with two consumers (4916 -> 4921 records) -> distinct definitions
survive 0x42127d, 0x409c5c, 0x414a64 and 0x41615d -> early available-copy forwarding 0x40b7b9 substitutes screenY for
the 4-byte shield read (0x40b811 width check, as in DrawButtons) -> stock cloning and the oracle's cloud and initial-Y
instruction sequences. Result equals the earlier bc compiler-state diagnostic: 2328 -> 2212 masked bytes, 1177 -> 1179
aligned, body 3904 -> 3896 (oracle 3866), 52 -> 48 regions; zero accepted regressions. A name alone is not the mechanism:
the second real consumer keeps the fork (the earlier one-consumer named baseY collapsed). B-a (singleton-1 cut) remains;
with abc the diagnostic reached 8 bytes, so B-a is now the main Build residue.
- [EVIDENCE, sol-cutcp2 r4] ChangePhase index preservation: keeping the Search index past the mouse calls is
  forced by the volatile EAX return. The sign test's use of that source snapshot follows the ordinary C1 assignment/read
  (reader tag 2; 0x413265 / 0x413245 global marking), not a special preservation predicate. Renamed, scoped or copied
  results are substituted back by early forwarding 0x40b7b9 (measured r3). No source operation qualifies; gate closed.
- [EVIDENCE, sol-cbuild r12] On the canonical screenY base, the a-cut diagnostic alone gives 8 masked bytes,
  1229/1237, body 3866 (the oracle size), identical to the old abc result: B-b and B-c are fully absorbed by source.
  Build = B-a + the 8-byte residue B-r. All 27 eligible literal-1 uses share one initial pool identity; a genuine
  Boolean fork survives early C2 but adds instructions. B-a's source cause remains [OPEN].
- [EVIDENCE, sol-cteb r6] No STRICT fold-survival control for T: across 409 STRICT functions, 78 nonadjacent
  backward-reconstructed copies occur; 77 are later substituted, and the one retained (a COM vtable copy) is not the
  Boolean sequence. Both T explanations remain open; the matched corpus does not demonstrate the precondition.
- [EVIDENCE, sol-cbuild r13] STRICT large-function witness: CDecor::BlupiStep (0x9280, 1516 C2 blocks, G 33)
  reaches the singleton-one cut because A - O is empty at 0x4390b6 (pooled zero EBX, this ESI, two POINT fields spanning
  ObjectDelete in EDI/EBP, call clobbers EAX/ECX/EDX; the local scanner cuts 1 at its second use). Build's shaping header
  already has X/EDI, Y/EBX, cursor/EBP, this/ESI across its first draw, but its coordinates start after the header
  guards, so the header keeps four free colours; the oracle's instructions do not support a direct transfer.
- [EVIDENCE, sol-cbuild r14] Falsified: early coordinate lifetimes exhausting Build's header. An early working POINT
  changes C1, components and occupancy but leaves two compatible colours (region 1 still adopted) and moves object-
  position loads before the zero-type branch, contrary to the oracle (1150/1237, 2256 bytes; rejected). The missing
  input is a stock definition/cut boundary giving camera X retained for the reverse bounds but read from home for the
  drawing SUB, and a long lift-X component; full header occupancy and the exact singleton are not shown necessary.
- [EVIDENCE, sol-cbuild r15] Second STRICT large-function witness in BlupiStep: a RECT top taken from end.y alone gives
  an exhausted group G = 1, which enters the scalar path and cuts with three colours free. Its POINT/RECT consumer
  relationship has no demonstrated equivalent at Build's reverse-moving header, so no transfer is predicted. Both
  stock routes to a singleton cut are now demonstrated in STRICT code (empty A - O; G < 3); neither maps onto Build's
  oracle-constrained source as written.
- [EVIDENCE, sol-cteb r7] Bounded experiment for T: reusing the matcher's search index as the old-flag holder keeps
  source ownership through 0x40cc03 but gives old flag EAX / result EDX and duplicates the Boolean computation (+11 body
  bytes) in the micro; prediction failed, no target transfer. Eliminates index-variable reuse as the old-flag split.
- [EVIDENCE, sol-cutcp2 r5] Bounded experiment for C: a shared two-consumer value (iconDim, the screenY pattern)
  survives early C2 and triggers cloning in both editions, but does not give the separate late-zero / connected-PLAY
  structure and adds 7 bytes and one instruction per micro; prediction failed, no transfer.

## Build B-a: the continue form (sol-cbuild r16, 2026-10-19)

Writing the reverse-moving loop as `for (i = MAXMOVEOBJECT; i != 0; pMoveObject--, i--)` with the rejections as
`if (...) continue;` statements gives Build **8 masked bytes, 1229/1237, body 3866** from ordinary source, the same as
the a-cut diagnostic. Cause [EVIDENCE]: the changed predecessor order selects a different latch region (the header
decision is unchanged); nested guards alone are inert. Not integrated: it regresses STRICT GetBlupiInfo (0x57b0) by a
two-byte commutative OR input order, with identical C1 after alpha renaming, i.e. the TU-wide symbol-ID key effect.
Next (sol-cbuild r17): pick the equivalent continue spelling whose ID cost keeps GetBlupiInfo in its window.

## Build B-a reproduced by source (sol-cbuild r17, canonical 2026-10-19)

Reverse-moving loop: cursor advanced in the for step (`pMoveObject--, i--`) and the rejections written as three
`if (...) continue;` guards (type==0 or out of bounds; type 8..11; type 1). Equivalent to the old single && guard with an
unconditional `pMoveObject--`. [EVIDENCE] The changed predecessor order selects a different latch region; Build reaches
8 masked bytes, 1229/1237, span 3872 (= oracle), i.e. the a-cut diagnostic result, from ordinary source. The three-guard
spelling (front-end ID delta +2) keeps GetBlupiInfo in its symbol-ID window; the four-guard spelling regressed it.
Full strict gate: zero accepted regressions, 25 decblock peers STRICT. Remaining in Build: the 8-byte residue B-r.

## Build B-r, the final 8 bytes (sol-cbuild r18 + supervisor, 2026-10-19)

[EVIDENCE] All 8 bytes are one stack-home swap between A (Y remainder `0 - posDecor.y % 64`, assigned to tinyPoint.y before
each grid's inner loop) and B (quotient `posDecor.y / 64`, the inner loop start / bound): ours puts A at [esp+20] and B at
[esp+24], the oracle the reverse, at the first stores (+0x6aa/+0x6b9), their reads (+0x6e2/+0x6ea), the second spills
(+0x961/+0x96c) and the paired boundary reloads (+0xc4a/+0xc4e). The first divergent decision is the ascending boundary
reload order of the animated-grid children A155 -> B160 before temp assignment, which fixes the home order.
Eliminated: (1) moving tinyPoint.y into the for-init (`for (j = posDecor.y / 64, tinyPoint.y = ...)`), floor, animated or
both grids: byte-identical (normalized before allocation); (2) a named row variable `y = posDecor.y / 64` with a real
second consumer: ownership survives to 0x414a64 but the inner loop changes from DEC/JNE countdown to INC/CMP/JL with an
extra bound reload (fails the native gate).
[OPEN] Competing explanations: the original's grid loops reached a different counted-versus-direct-index loop rewrite
(already different by the pool snapshot), or B was materialized earlier than A. Next discriminator: trace the early
loop-rewrite predicate that selects counted versus direct-index form for these grid loops, and which source form keeps
the countdown while giving B the earlier boundary reload.
- [EVIDENCE, sol-cbuild r19] B-r causal chain confirmed: a diagnostic swapping only the animated-grid boundary reload
  order (B before A) gives 0 masked bytes, 1237/1237; the stack-packing model confirms it causes all eight. The order is
  made at 0x40fbfa (mode 1), which enumerates the required set in ascending fragment-ID order (A155 then B160); those IDs
  are reused LIFO slots (previously pixmap-member and moving-icon address LEAs freed at 0x41ccbc). Both grid loops already
  take the oracle's counted form (selector 0x43e5ed, not 0x43de8b), so the loop-rewrite hypothesis is falsified.
  'Earlier B materialization' is also falsified for current inputs: across all 205 mode-0 boundary calls neither value is
  ever required (98/100 live edges, zero required).
  Handoff / precise blocker [OPEN]: the source must give either (1) B's reload before A through ordinary fragment
  construction with the same three earlier A/B reference pairs and native loops, or (2) a pre-web B availability hole
  (B105 in target+40 minus predecessor+44) whose stock materialization is reused without extra loads or stores.
- [EVIDENCE, sol-tebx] Two fresh hypotheses for T eliminated in micros: (H1) rank-1 test as an else-if after rank 0
  changes the reader CFG as predicted, but 0x41ddfe still folds the old flag (adds a JMP/copy; result EAX, HWND ECX);
  (H2) a single shared bEnable feedback after the rank chain is identical to the control and needs extra reloads/tests.
  Neither yields an extra rotor definition or the old-flag interval.
- [EVIDENCE, sol-cpx] Two fresh hypotheses for C eliminated in both edition micros: (H1) the index sign test as
  `index & 0x80000000UL` removes only the index's zero read and emits a literal-mask TEST, with no split of the other late
  zero reads; (H2) both FALSE exits through one `goto invalid_phase` failure tail keeps a shared tail into early C2 but
  changes neither zero's partition nor its costs, and shares one native epilog where the oracle has two.

## Build STRICT: B-r solved by the big-decor Y start (2026-10-19)

Source change (the only one): in the big-decor loop the Y start is two statements,
`tinyPoint.y = 0 - posDecor.y % DIMOBJY; tinyPoint.y += 2 - DIMOBJY;` instead of
`tinyPoint.y = 0 - posDecor.y % DIMOBJY + 2 - DIMOBJY;`. The first statement is textually the floor/animated grid
reset. Build: 0 masked bytes, 1237/1237, 61/61 relocations STRICT; full gate exit 0, zero regressions, coverage
123,584 -> 127,456. Tools: `tools/c2trace/freelist.py` (trace, forced-drop diagnostic).

Causal chain [EVIDENCE]:
1. The A/B home order is the order of the animated-boundary reloads, which 0x40fbfa (mode 1) emits in ascending
   fragment ID (r19). The fragments come from the first group split (0x437e67/0x438544, triggered by committing the
   animated-grid `tinyPoint.x` web LR87 to EBP; G = the 33 ranges whose last register was EBP). Members are walked in
   ascending LR order and each pops one ID from the LIFO free list 0x493230 (head), link +0x2c.
2. The free list at that split is batched: top = 0x41a6f8 drop pass (25 constants with < 2 references, freed through
   0x4375fe, plus the `m_time` reload copy LR165 freed at 0x41a86c), then the 0x41ccbc LEA batch (174, 160, 156, 155,
   ...). Each batch pops in descending ID (both passes walk ascending IDs). Canonical: B is member 28, the bound
   member 29, A member 30; the drop batch holds 26 entries, so B, bound and A all pop from the LEA batch in descending
   order (B160 > A155): A's reload first.
3. Counting model, confirmed by forced-drop diagnostics (redirect 0x41a775 -> 0x41a73e, never source): forcing any one
   of 18 group constants before B to drop gives 0 bytes / 1237 (B pops the last drop-batch entry 13, A pops 160);
   forcing one non-member drop leaves 8 bytes (B174 > A156); two non-member drops give 0 bytes; only the pooled zero
   (35 refs, kept in a register) breaks other code. The residue needed F - P_B in {0, 1}; canonical had -2.
4. The counts are robust: 36 natural spellings (increment forms, block locals, aggregate vs field copies, loop forms,
   literal sign/shift forms, term order, `IsValidCel` inlining, a floor `switch`; the switch changes code) and 39
   declaration counts 1-128 before Build (symbol-ID context) all keep F = 26, P_B = 28. Literals are interned by value only (0x417fb7); eligibility is per
   opcode (0x4156b6); C2 canonicalizes `+= -20`, `<< 3`, term order before pooling; literals whose reads are all
   outside loops (25, 0xED, 0xF5, 32768, -64, -62, -51664) get no web and are freed/recycled at 0x4169a7.
5. The fix changes which value is walked first, not the counts. Expression descriptors are numbered on interning in
   early C2 (0x42127d) and webs get LR IDs in descriptor order (0x416a40). Canonical: the big-decor Y start folds to
   `-62 - y%64` (descriptor 1182), the quotient B is interned there (1183), and A = NEG(y%64) first appears at the
   floor grid (1412). With the split, NEG(y%64) is interned in the big-decor loop (1182) before the quotient (1184),
   so A's web LR145 precedes B's LR148 in the split walk: A pops 161, B 157, the bound 156; mode 1 emits B's reload
   first; stack history ABABABBA puts B at [esp+20], A at [esp+24] (the oracle).
6. Boundary of the mechanism: it works only when the first statement assigns exactly the reset expression
   (`0 - y % 64` or `-(y % 64)`) and the -62 follows as any separate statement (`+= 2 - DIMOBJY`, `-= DIMOBJY - 2`,
   `+= 2; -= DIMOBJY`, `= tinyPoint.y + 2 - DIMOBJY`, `-= 62`): all give identical objects. Folding `- DIMOBJY` or
   `+ 2` into the first statement keeps 8 bytes; splitting the X start the same way changes 3,175 bytes. [HYPOTHESIS]
   The author wrote the big-decor start from the common grid template plus the layer's 2-pixel/one-cell offset, the
   style also seen in the start-position loop (`tinyPoint.x += 10; tinyPoint.x += i * 8 - 12;`, both kept by the
   oracle). Which of the equivalent adjustment spellings the original used is not recoverable from the binary.

## ChangePhase: the residue is one queue order (2026-10-19, tools: scratch-only probes)

[EVIDENCE] With the oracle's guard order (`!m_bDemoPlay && (phase != PLAY || m_phase == PLAY) && ...`) and WriteInfo
test order (`phase == GAMER || phase == PLAY`), our build and the canonical reordered-guard build are identical in the
allocator until the 12th cost pass, which follows the batch dispatch triggered by `oldPhase`'s commit to EBX (LR78,
BB 100..152). That dispatch's region 1 spans blocks 3..99 (EBX is free in every block until block 100), so it leaves an
early zero fragment (21 reads, BB 3..99, overlapping `index` = EDI in BB 89..100) and an early PLAY fragment (9 reads,
BB 8..92). Whichever is coloured first takes EBX:
- oracle order: zero/21 is queued at -65, PLAY/9 at -78; zero takes EBX (its only colour), PLAY ends in EDI (4,668 bytes);
- canonical order: PLAY/9 -69 beats zero/21 -71; PLAY takes EBX, its pressure scan rematerializes zero's three late
  reads (lines 184/189/203), zero shrinks to BB 3..48 and takes EDI.
Diagnostic (0x41b6fa, never source): with the oracle order, inserting PLAY/9 with any priority above -65 gives
ChangePhase 0 masked bytes, 2029/2029. So "a separate late-zero component" is a consequence of PLAY being coloured
first, not an independent precondition.

[EVIDENCE] Priority model of that cost pass (0x40ee1d, all block weights W = 1 here): at each block end, P = W x n_b with
n_b the popcount of ranges referenced in the block (0x40eda6); a range referenced in the block gains P x (operand
benefit accumulated in the block), a range live but unreferenced loses P. Per-block deltas reproduce both priorities
exactly (scratch probe). The 13-point gap: block 8 (the guard's first test, n = 9 from seven constant materializations)
gives zero a free reference in the oracle order and PLAY's benefit-1 `m_phase == PLAY` compare in the canonical order
(+8); zero gains +12 in block 18 (three `= 0` stores at n = 4) and pays -18 in blocks 92..98; PLAY pays -9 in the QUITPLAY
path (blocks 40..45) and -9 in blocks 89/91 before its WriteInfo compare.
[OPEN] No natural source property found that moves the gap by 14 with identical code: guard nesting, `== FALSE`, the
multi-player guard nesting, Write-flag spellings, early-return shapes, top-declared `index`/`oldPhase`, store and
tooltip orders, QUITPLAYTEST/mouse/DemoRec forms are all inert (gap stays -65/-78) or change code. Next discriminator:
which other ranges are counted in n_b of the asymmetric blocks (8, 18, 40..46, 89, 91, 92..98), and whether an
original commit order or value identity reduces or raises one of them.

## TreatEventBase: the old flag is re-folded twice (2026-10-19)

[EVIDENCE] Blocking 0x41ddfe for the rank-0 old-flag copy (or the terminator copy) keeps a separate `old = [flag];
cmp old, 0` through the fold pass, but 0x409c5c folds it back before select formation (0x412412), so lowering sees the
memory compare again: 8 bytes either way. Stopping 0x40cbc8's short definitions at L143 is inert too. If the load did
survive to lowering, the inserted result zero (`R = 0`) would separate it from its compare and the drop-pass copy rule
(refs == 2, use in the next tuple, 0x41a6f8) would no longer delete it; a surviving old flag in EDX (temp or global)
makes the result's rotor scan skip EDX and EAX and take ECX, leaving the rotor where the demo build leaves it, which is
the oracle. [OPEN] No source form keeps the old-flag load alive through 0x409c5c with the oracle's instructions.

## Model closure tests for the two open functions (2026-10-19, round 2; scratch/claude probes)

ChangePhase [EVIDENCE]:
- Size of the missing effect. Decisive pair at the oldPhase dispatch (zero/21 vs PLAY/9): oracle guard and WriteInfo
  orders -65/-78 (gap -13); canonical WriteInfo order alone -65/-73 (-8); canonical guard order alone -71/-74 (-3);
  both canonical orders -71/-69 (+2, PLAY first). With the oracle's orders the source must supply +14.
- The gap is local to blocks 3..99. Code-changing diagnostics that empty the later GAMER loop or MULTI block shift both
  priorities by the same amount (-64/-77, -65/-78). Line layout (guard on one line, the three stores on one line, blank
  lines) is inert.
- Not a compiler-build effect: SP3, SP2, RTM, NT5-1592, NT5-1691, SP3-deu and SP3-jpn all give the oracle-order source
  the same 1995/2029 aligned instructions.
- Third witness: the June 1998 demo (earlier source revision) has the oracle's guard and WriteInfo orders and the same
  roles (zero EDI early, PLAY EBX through WriteInfo). Its out-of-line DOQUIT block is only tail-merged with a different
  TESTCD failure path. Demo, release and eGames all colour PLAY first.
- Cost inputs are fixed by the machine code. Operand benefits (zero: seven member stores; PLAY: `cmp [m_phase]` and
  `mov [m_phase]`), reference blocks, temps and pooled-constant materialization blocks (the dominator LCA of all reads
  of the constant in the function) all follow from the oracle's instructions. All block weights are 1. Ranges that end
  as immediates contribute +3 net (zero-only blocks 6, 7, 30, 94..98: +15; block 18: -8; PLAY-only blocks 40, 45, 91:
  -4), so even removing every such fragment leaves PLAY behind. No late demotion is possible: every saved register is
  protected by nonconstant uses (0x42ac33 profitability 0x1000000).
- Region adoption (0x438f79): a block joins a predecessor's region while T = A - (O(block) U U(region)) is non-empty,
  and U(region) accumulates the occupied sets. T is {EBX, EDI} over blocks 3..86, {EBX} from block 89 (index in EDI),
  and empty only at block 100 (oldPhase in EBX). Splitting zero's late reads into their own region would need EBX
  occupied inside the region or a spine block where no group member is live; neither occurs with this code.
- Pool eligibility (0x4156b6, logged): `index` and `this` are kind 2 at pool time, so `index < 0`, the HideMouse(FALSE)
  push (always eligible) and the `!m_bBuildOfficialMissions` memory compare are genuinely pooled. Forcing exactly
  these three ineligible gives 0 masked bytes, 2029/2029; any two of them do not.
- Commit order is not the lever: queueing oldPhase above index (diagnostic priority 62..400 at 0x41b6fa) would split
  zero at index's commit instead, but gives 4,513..4,637 bytes with the first difference in the index block.
- Dead local stores (`bEnable = FALSE;`, `bHide = FALSE;`, `num = 0;` at three places in blocks 3..99) are removed
  before allocation: code and gap unchanged (-65/-78).
- [OPEN] Within the recovered model the oracle-order IL cannot colour PLAY first, so the original IL differs in a
  property the machine code does not show (a range or web identity live in blocks 3..99, or an unmodelled cost input).
  Next discriminator: an IL-record-level search (C1 records round-trip) over code-neutral edits in blocks 3..99,
  scored by this gap.

TreatEventBase [EVIDENCE]:
- Temp-assignment trace (c2dbg): assignment 20 is the TreatTextInput lParam load (line +115, rotor ECX -> ECX) and
  assignment 21 the rank-0 result (line +143, rotor EDX -> EDX); nothing between them passes the rotor. Shifting the
  rotor pointer by +1 or +2 slots just before assignment 21 gives 0 masked bytes, 1894/1894, and assignments 22..27
  keep their registers. So the oracle needs only "result in ECX, rotor left at EDX", i.e. a rank-0 block that is
  rotor-neutral, as the June 1998 demo (no rank-0 block) implies. Routes: an extra rotor pick landing in EDX between 20
  and 21 (only the terminator byte DL or the old flag EDX qualify in the oracle's instructions), the old flag occupying
  EDX when 21 is assigned, or 20 or 21 taking ECX by preference without moving the pointer.
- Preference rules (c2 0x42ac33): a temp prefers a register when its tuple copies from a register-held range
  (0x42acf4) or into one (0x42b051); 0x435d84 then uses it without moving the pointer. The flag 0x48fb60 is written
  once (0x428733), so every other temp takes the rotor path.
- Why rank 1 differs: its new value is forwarded to the SetAllMissions push, so the IL has `EAX = R1148` and the setcc
  temp prefers EAX. Rank 0's if/else stores in the arms, so `bEnable = m_bBuildOfficialMissions` stays a memory reload
  until a late peephole emits `mov esi, ecx`; the select result's only use is the store, its preference is cleared at
  0x42afe5, and it takes the rotor.
- Forms that forward the new value (`m_b = !m_b; bEnable = m_b;`, named results `bNew`/`i`, an old value used twice)
  coalesce it with bEnable and emit `mov esi, r; mov [m_b], esi` (14 bytes). A named old value used twice survives but
  is globally allocated to ECX (first free), so the result stays EDX (4,619 bytes).
- No positive control in the game: the SETcc-to-member-store sequence occurs twice in BLUPI.EXE, this toggle and one CRT
  site.
- SB2 and Eggbert decompilations call `m_pDecor->SetBuildOfficialMissions(m_bBuildOfficialMissions)` in rank 0, so they
  do not constrain SB1's inline toggle.

ChangePhase, further negatives [EVIDENCE]:
- Speedy Blupi 2's BLUPI.EXE (2001, linker 5.00) has the same if-chains and the same roles in blocks 3..99: four
  compiles (demo, release, eGames, SB2) colour PLAY first.
- Extra flags on the oracle-order source: /Ox, /G5, /GB, /Op and /Ob0 give the same 4,668 bytes; /Oa, /Ow, /Os, /G6,
  /Oi- and /Og- change the whole function.
- `do { } while (0)` and `for (;;) { ... break; }` wrappers around blocks 89..99 are removed before costing (inert).

## Methodology test: hidden compiler inputs (2026-10-19, round 3; scratch/claude probes)

Positive control [EVIDENCE]. Starting from the pre-fix Build source (8 bytes), the decisive decision (group-split
child order, found by the forced-drop diagnostic) depends on code-invisible identities: expression interning order ->
web LR order -> fragment IDs. A generic generator (scratch/claude/hiaudit.py) that rewrites only the statements that
mention the two decisive subexpressions (`y % DIMOBJY`, `y / DIMOBJY`), using additive-chain splits, constant
reassociation and operand commutation, produces 10 candidates; exactly one is STRICT-sized and 0 bytes, 1237/1237:
the known fix. Its neighbours explain the mechanism: `= 0 - y%64 + 2; -= DIMOBJY` stays at 8 bytes because it folds to
`2 - y%64`, not the pure negation. So "identify the identity input, then rewrite only its source carriers" recovers
this class without being told the transformation.

ChangePhase is not in that class [EVIDENCE]:
- 104 queue insertions before zero/21 are placed by identity (equal priority: rank +0x40 or newest-first). Moving
  each one to the other end of its equal-priority group (208 diagnostic runs, scratch/claude/cptie.py) never gives
  the oracle: 196 unchanged (4,668 bytes), 12 slightly worse. The zero-first decision is quantity-driven.
- Operand benefit (0x40fada): a non-constant operand scores 2; for a pooled constant a zero compare scores 0,
  otherwise the tuple's first operand decides: memory with a base/index or a frame operand (tag 2) scores 1, a named
  register variable or a temp scores 0. Final code does not always show the pass-12 operand form (late load
  splitting, load CSE and store forwarding exist), but every such late normalization in blocks 3..99 (m_bDemoPlay,
  m_bPrivate and m_bMulti loads, the m_bMulti CSE) would cost PLAY, not zero.
- Block weight is an argument of the cost pass per loop region (0x40ee1d edx); blocks 3..99 are all weight 1.
- Interning (0x417fb7) compares only the literal tag and low 32 bits; the per-web range builder (0x416a17..0x416a6f)
  partitions variables and expressions only, so a pooled constant is one range: a separate late-zero web has no
  source route.

TreatEventBase positive controls [EVIDENCE] (tools/c2trace/booltemps.py over 18 captured TUs; sound.cpp omitted):
- 50 SETcc result temps. 15 get their register by preference, always the copy-into-register rule (0x42b051):
  rank 1 (forwarded to a push, EAX), returns copied into EAX (IsRightBorder x8, DrawIcon, DrawIconDemi, DrawIconPart,
  DrawPart), and CPixmap::BltFast's `if (mode == 0) dwTrans = 1; else dwTrans = 0;`, where the setcc temp is copied
  into the named local dwTrans, a global range in ECX (EBX in the other overload): a rotor-neutral boolean exactly as
  rank 0 needs.
- 7 rotor-path temps skip busy registers (IsLave, IsRessort, playMovie, DrawMap, MouseQuickDraw, Save/RestorePalette):
  the result is zeroed before its compare while the compare's register inputs (address temps, `this`) are live; most
  first had a rejected EAX return preference.
- Rank 0 can use neither: its compare input is a frame member ([ebp+0x6d40]) and its result's only consumers are a
  memory store and bEnable (ESI, not byte-capable). Copy-before-store named forms (`bNew`/`i`, four variants) are
  still coalesced with bEnable by early C2 and store from ESI (14 bytes).
- The June 1998 demo already contains the `c++` letter-shift line; only the rank-0 action block was added later, so the
  added block itself must be rotor-neutral.
- [OPEN] No stock-compiler witness in the corpus gives a select result a preference or a busy-input skip without a live
  consumer or register input; the oracle's rank-0 IL shape is therefore absent from all STRICT functions.

## Final targeted experiments: symbol order and inline helpers (2026-10-19, round 4)

ChangePhase, symbol-ID operand order in blocks 3..99 [EVIDENCE]:
- Hypothesis tested: TU-wide symbol IDs -> expression keys (0x405301) -> operand order (0x408d58) -> operand benefit
  (0x40fada) -> zero/21 vs PLAY/9 priority, with identical final code.
- Audit. sol-keyforce traced 3,040 comparisons on the oracle-order source and forced 64; 41 of them lie in blocks 3..99,
  and all 64 objects were byte-identical. sol-ilsearch's 155 C1-record commutes (operands and relation reversed) kept
  -65/-78; that tests source operand order, not key order.
- Screen. A key is (cost << 16) | hash, so a pair with unequal cost halves is decided before any hash bit and no ID
  assignment can flip it. Fresh trace on the current tree (`keyforce.py trace` on the oracle-order candidate): blocks
  3..99 are events 0-279 (pass 1) and 2003-2202 (pass 2), 480 comparisons, **none with equal cost halves** (gap 1 in
  419). The 12 PLAY-literal and 40 zero-literal comparisons there pair a cost-0 literal with a cost >= 1 operand. All 82
  equal-cost comparisons of the function are in later code (SetState loops, file-page guards, music/region, joystick).
- Result: the largest change any header, declaration order, earlier function or inline context can make to these
  operand orders is zero, so no forced replay was justified. Closed. Reproduce with
  `keyforce.py list NAME '?ChangePhase@CEvent@@' --equal-cost --events 0-279,2003-2202`.

TreatEventBase, inline-helper preference [EVIDENCE]:
- Oracle and ours differ only in the register pair: oracle `mov edx,[ebp+0x6d40]; xor ecx,ecx; test edx,edx; sete cl;
  mov [ebp+0x6d40],ecx; mov esi,ecx`, ours with ECX and EDX swapped. The old-flag load is a compare-only scratch pick,
  so it follows the result: with the result in ECX the picker skips EAX (m_rankCheat) and ECX and takes EDX.
- BltFast control (IL at 0x42ac33): its select ends with `ECX = R`, a copy into dwTrans, already coloured ECX, so
  0x42b051 records the preference. Rank 0 ends with `[ebp+0x6d40] = R` and bEnable is a reload (cleared preference).
- Five inline CEvent members defined in the class body (per-candidate event.h copy, scratch/claude/t6.py), predictions
  written first: toggle body (control) 8 bytes; toggle returning the member 8; getter for the old flag 8; getter for
  both reads 8; `SetBuildOfficialMissions(!m_bBuildOfficialMissions)` 14 (`sete dl; mov esi,edx; mov [m],esi`, the
  named-copy coalescing). No other event.cpp function changed. The getter capture's rank-0 IL at 0x42ac33 equals the
  direct form (memory compare, `[ebp+0x6d40] = R`): inlined returns and parameters are forwarded before select
  formation, so no inline placement gives the select a register destination. Closed.
