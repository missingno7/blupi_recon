# decdesign completion (sol-decdesign-2, 2026-10-17)

## Result and review package

[EVIDENCE] **PlaceDecor6 is STRICT at RVA `0x7b20`: 2,077 original body
bytes, 2,080 compared bytes, zero fixed differences and 24/24 proved
relocations.** The final verifier exits 0 and preserves every active accepted
entry. This adds **2,077 unique original code bytes**.

[EVIDENCE] Every one of the **42 oracle functions assigned to decdesign.cpp**
is now STRICT in this candidate, covering **14,181 unique original body
bytes**. `scratch/sol-decdesign-2/tu-audit.csv` lists them in tu_map order,
with names, RVAs and oracle sizes. There are no remaining unmatched mapped
decdesign functions to reconstruct. The emitted POINT operator helpers and
SB2-only PlaceDecor8..10 stubs have no assigned oracle bodies and receive no
coverage credit; they were not modified.

[EVIDENCE] Candidate: `scratch/sol-decdesign-2/decdesign.cpp`.
Only **PlaceDecor6's body** differs from the starting canonical source.
`audit.json` verifies that all other source text is identical and that the
canonical source was left unchanged. No header, manifest, compiler catalog,
asset, external reference or toolchain was edited. No commit was made.

```powershell
.pyenv\python.exe tools\verify.py --tu src/decdesign.cpp=scratch/sol-decdesign-2/decdesign.cpp --build-dir sol-decdesign-2
```

[EVIDENCE] Final review artifacts under `scratch/sol-decdesign-2/`:

- `integration.patch`: the three focused source hunks for the supervisor.
- `final/`: exact source/object snapshot, full results.json, verifier transcript,
  exit code, target/candidate disassemblies and score.json.
- `audit.json`, `tu-audit.csv`, `ledger.csv`: scope, placement, coverage and
  iteration evidence.
- `run.py`, `trials.py`: focused sequential verification and source experiments.

[EVIDENCE] All candidate evaluations used tools/verify.py, its tools/msvc.py
wrapper, the full manifest, canonical headers, pinned VC5 SP3 and
`/W3 /GX /O2 /ML /Gd /Gy`. TMP/TEMP remain inside the repository; job/slot
defaults were retained. Shell commands ran one at a time to completion.
No original executable was run and no background process remains.

## Frame and region check

[EVIDENCE] The established frame map from the first pass remains valid:
four allocated local bytes and four saved registers. Let E be entry ESP.
The target reuses E-4 for the tile pointer and later personal-bomb y coordinate;
E+4 holds cell x and then the address-taken tile icon; E+8 is cell y; E+c is
the menu index; E+10 holds flags and then the personal-bomb x coordinate;
E+14 holds bMix and then the selected personal-bomb type. Complete target
access enumeration is already in `scratch/sol-decdesign/stack-7b20.{txt,json}`.
That completed investigation was reused, not repeated.

[EVIDENCE] Baseline and final both have this four-byte frame. All creation
regions, fields, conditions, constants and the 20 direct calls retain their
meaning and order. Four placement-palette data references bring the total
to 24 relocations. Existing canonical data definitions prove all four palettes;
**no data or header delta is needed**.

## Source change that closes the match

[EVIDENCE] The baseline personal-bomb branch explicitly caches two scalar
coordinates, passes them to MoveObjectSearch, and copies them into posStart
after deletion/allocation:

```cpp
posX = cel.x * DIMOBJX + 2;
posY = cel.y * DIMOBJY + 6;
rank = MoveObjectSearch(POINT(posX, posY), -1);
// select type; delete; allocate
m_moveObject[rank].posStart.x = posX;
m_moveObject[rank].posStart.y = posY;
```

[EVIDENCE] The final source removes those two scalar declarations and writes
the coordinate expressions at their actual uses:

```cpp
rank = MoveObjectSearch(POINT(cel.x * DIMOBJX + 2,
                              cel.y * DIMOBJY + 6), -1);
// select type; delete; allocate
m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 6;
```

[EVIDENCE] VC5 still computes the repeated coordinates once and preserves
them across the calls in the target's reused stack slots. The resulting body
matches the target completely. This source change also fixes the much earlier
menu-index register choice: ECX replaces EAX, the first delete gains its target
`mov ecx,esi`, and menu-index reloads move to the target positions. Tile-address
registers and personal-bomb LEA/store scheduling also match.

[HYPOTHESIS] Compiler common-expression identities and register allocation,
rather than actual repeated runtime arithmetic, explain the broad effects.
The identical output is observed; VC5's internal optimizer representation is
not exposed by this experiment. No artificial declarations or forced bytes
are needed.

## Compiler findings

| Trial | Body / extent | Frame | Isolated score | Fixed prefix | Full verifier |
|---|---:|---:|---:|---:|---|
| Canonical baseline | 2,075 / 2,080 | 4 | 471 | 27 | MISMATCH; accepted gate passes |
| Shared explicit short tile pointer | 2,075 / 2,080 | 4 | 471 | 27 | MISMATCH; gate passes |
| Separate block-local tile pointers | 2,075 / 2,080 | 4 | 471 | 27 | MISMATCH; gate passes |
| Block-local staged icon integers | 2,075 / 2,080 | 4 | 559 | 27 | MISMATCH; gate passes |
| Shared staged icon / personal type | 2,083 / 2,096 | 4 | 499 | 27 | MISMATCH; gate passes |
| SB2-style personal-channel switch | 2,073 / 2,080 | 4 | 509 | 27 | MISMATCH; gate passes |
| Repeated coordinate expressions, component stores | **2,077 / 2,080** | **4** | **1** | **2,080** | **STRICT, 24/24; gate passes** |
| Repeated expressions, aggregate posStart assignment | 2,045 / 2,048 | 4 | 507 | 27 | MISMATCH; gate passes |

[EVIDENCE] Isolated scores come from tools/asmscore.py. Its one-object scoring
run lacks the owning data object and therefore labels the winning body
RELOC-UNVERIFIED with score 1. The **full verifier** includes every object and
reports STRICT with all 24 relocations proved. The final score.json records
both statuses explicitly. Extents include alignment NOPs; only the 2,077 body
bytes count as new coverage.

- [EVIDENCE; `BLUPI-REG-01`, `BLUPI-REG-02`, `VC5-ORD-01`;
  **blupi-specific**] Trigger: correct frame and behavior, but EAX-versus-ECX
  menu-index choices, displaced reloads, and two missing body bytes. Replacing
  the genuine posX/posY caches with repeated coordinate expressions in the
  personal-bomb branch changes body size 2,075 -> 2,077 and isolated score
  471 -> 1; full-run result becomes STRICT, 24/24 proved relocations. Frame
  remains four bytes and every accepted peer stays strict. This confirms a
  source-expression identity effect within this function, not a general rule
  to expand arbitrary cached expressions.
- [EVIDENCE; `BLUPI-REG-02`, `VC5-LOC-03`; **negative**] Explicit shared and
  scoped short tile pointers are byte-inert in the complete canonical TU.
  Scoping the four staged icon integers changes instruction scoring to 559
  without fixing the prefix or body length. Correctly sharing the icon/type
  scalar grows the body to 2,083 bytes and remains MISMATCH. These are new,
  valid full-body tests; the malformed shared-variable trial noted in the
  previous pass was not reused as evidence.
- [EVIDENCE; `VC5-CTL-03`, `VC5-ORD-01`; **negative**] Restoring the ancestor's
  personal-channel switch produces 2,073 bytes and score 509; it does not
  reproduce the target's independent conditional stores. Keeping repeated
  coordinate expressions but assigning posStart as one POINT shortens the
  body to 2,045 bytes, score 507. Component stores are required at this
  checkpoint. All these runs retain the same compiler, flags, header set and
  accepted-function gate.

[EVIDENCE] Earlier before-definition count probes and POINT/scalar scope
campaigns are recorded in SENIOR_SOL_DECDESIGN.md and were not rerun. The
new source-supported experiment closed the match before a wider context
campaign was necessary. No context-padding declaration is present in the
delivered source.

## Integration

[OPEN] The supervisor can apply `scratch/sol-decdesign-2/integration.patch`
or copy the audited PlaceDecor6 body, then use the normal canonical acceptance
workflow. No header or data work is pending for this TU. The pre-existing
withdrawn IsDeepWater entry in decblupi.cpp is outside this worker's ownership
and was not changed or counted as a new regression.
