# Residual causes of the four open functions (2026-10-19)

Strict acceptance is unchanged: a function counts only when STRICT. This page groups the remaining machine-code
differences by independent root cause. Counts are aligned-instruction differences (tools/residues.py), not positional
bytes; Build is 38 bytes longer than the oracle, so positional byte counts shift past its first difference.

| Function | Aligned / oracle instructions | Differing instructions |
|---|---|---|
| TreatEventBase | 1886 / 1894 | 8 |
| ChangePhase | 2021 / 2029 | 8 |
| DrawButtons | 2288 / 2304 | 16 |
| CDecor::Build | 1177 / 1237 | 60 |
| Total | 7372 / 7464 | 92 |

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
