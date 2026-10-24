# SB1 historical-link candidate (experimental, not canonical)

Branch `sb1-histlink`, based on `sb2-matching` `aa3dfd6`. Kept apart from the canonical reconstruction until its
evidence is understood (user direction, 2026-10-24). Worker report: [WORKER_REPORT.md](WORKER_REPORT.md)
(sections "SB1 ICF with external network callbacks" and "Solver").

## Source change (`solver-tail8.diff`, applied on this branch)

- `src/network.h`, `src/network.cpp`: `EnumProvidersCallback` / `EnumSessionsCallback` without `static`
  (msvc5kb BLUPI-LAY-04: C2 emits file-static functions only after a compiled reference; the original places each
  callback before its caller).
- `src/decmove.cpp`: eight CDecor methods restored at their pre-`60c7fab` definition positions, never called:
  FlushBalleTraj, SetBalleTraj, IsBalleTraj, FlushMoveTraj, SetMoveTraj, IsMoveTraj, AscenseurShift,
  MoveAscenseurDetect (bodies from the repository's earlier SB2-derived reconstruction).
- `src/decor.h`: their declarations and a [HYPOTHESIS] view of the proven 100,044-byte CDecor tail:
  `m_blupiTimeNoAsc` and two `MAXCELX * MAXCELY / 8 + 1` (1,251-byte) bitmaps, rest opaque. SB2 binary evidence has
  1,300-byte (`TRAJBYTEX * MAXCELY`) bitmaps elsewhere in its class; with those bodies `_strncnt` survives in
  aw_cmp instead of aw_map.

## Result (historical host, attested historical sound object)

| | Canonical guard | This candidate |
|---|---:|---:|
| Image SHA-256 | `557ca92e…3533` | `882bf008…bebbec` |
| Functions at original RVA (412 mapped) | 131 | 412 |
| Same-offset `.text` bytes / 176,019 | 54,919 | 175,718 |
| Whole-file differing bytes / 535,552 | 121,401 | 596 |
| Entry point | `0x24d20` | `0x24d40` (original) |

Original `BLUPI.EXE` SHA-256 `be6de5a206bc636346dde1b1a4dd11ddc3bda30c6efe64eebe96513f9c8290ac`.
Remaining 596 bytes: `.text` 301 (261 import references, ChangePhase 32, TreatEventBase 8), `.rdata` 294 (INT/IAT
order), header 1 (Rich stamp-less count 268 vs 270). `.data` and `.rsrc` byte-exact.

All eight additions are REF-dead in LINK's trace (801 contributions); the 460 existing function payloads are
unchanged in all nine SB1 targets. Verifiers exit 1: the added declarations shift C1-generated symbol names
(`$S33911 -> $S33925`, ...), so the pinned ChangePhase/TreatEventBase certificates (payload hash binds symbolic
names) reject; the six Eggbert 1 targets also hit the GetHide/GetNbSessions fold-owner cycle
(`solver-eggbert-inventory.diff`, a separate inventory correction).

## Toolchain and link

SP3 LINK 5.10 SHA-256 `04c892989f4cc8076ef45e033215eb7aefbdf1459cbee9b13a81f8ddb718b579`, historical MSVCRT
`e390b0529fb9ee1214bb68302a4658b578134d92ab76bc5eb48f3badd843d210`, run under `tools/histlink.py --trace-rich`
(oracle clock); response file and object hashes in `link/link.rsp`, `link/build.json`; map `link/BLUPI.map`;
layout comparison `link/layout.json`. Local copies of the objects and image (not committed):
`build/sb1-histlink-ref/` in the main checkout.

## LINK model

ICF (`0x431fb0`): MSVCRT qsort (unstable, middle pivot, shortsort <= 8) over executable COMDAT contributions in
load order, key (effective contribution size, 16-bit relocation count); the first eligible equivalent in each
bucket survives. The replay (`scripts/solver.py`, `analyze.py`) reproduces all 793/801 sorted positions of the real
links. `solver-constraints.json` and `solver-{insert,move,remove2}.json`: exhaustive one-edit families;
`solver-corrected-family.json`: the 42 working subsets of the twelve historical helpers, one closed under CDecor
dependencies (this candidate).

## Status of the claims

- [EVIDENCE] The output layout: reproduced by a normal compile and link, predicted by the replayed LINK model.
- [EVIDENCE] External callback linkage in the original (BLUPI-LAY-04 plus the original function order).
- [HYPOTHESIS] That the original SB1 source contained these eight methods (dead), their bodies and the tail view.
  Only their contribution keys (size, relocation count) and positions enter ICF.
- [OPEN] Import order, Rich count, ChangePhase, TreatEventBase.
