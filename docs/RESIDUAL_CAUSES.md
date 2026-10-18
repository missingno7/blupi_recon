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
