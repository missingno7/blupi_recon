# Predictions registered before validation compilation

[EVIDENCE] Code equality below excludes relocation fields. It is diagnostic, never STRICT acceptance.
[EVIDENCE] `predictions.json` was written before the 35 held-out variant compiles (56 function checks).

| Input state | N | Function | Predicted equal | Observed equal | Fixed-byte differences | Hit |
|---|---:|---|---|---|---:|---|
| mos | 364 | MoveObjectSort | False | False | 8 | True |
| mos | 364 | MockeryDetect | False | False | 436 | True |
| mos | 365 | MoveObjectSort | True | True | 0 | True |
| mos | 365 | MockeryDetect | False | False | 436 | True |
| mos | 491 | MoveObjectSort | True | True | 0 | True |
| mos | 491 | MockeryDetect | True | True | 0 | True |
| mos | 493 | MoveObjectSort | False | False | 8 | True |
| mos | 493 | MockeryDetect | True | True | 0 | True |
| mos | 397 | MoveObjectSort | True | True | 0 | True |
| mos | 397 | MockeryDetect | False | False | 436 | True |
| mos | 398 | MoveObjectSort | True | True | 0 | True |
| mos | 398 | MockeryDetect | True | True | 0 | True |
| macro | 1 | IsDeepWater | True | True | 0 | True |
| macro | 1 | IsVentillo | True | True | 0 | True |
| macro | 21 | IsDeepWater | True | True | 0 | True |
| macro | 21 | IsVentillo | True | True | 0 | True |
| macro | 22 | IsDeepWater | False | False | 66 | True |
| macro | 22 | IsVentillo | True | True | 0 | True |
| macro | 85 | IsDeepWater | False | False | 66 | True |
| macro | 85 | IsVentillo | True | True | 0 | True |
| macro | 86 | IsDeepWater | True | True | 0 | True |
| macro | 86 | IsVentillo | True | True | 0 | True |
| macro | 105 | IsDeepWater | True | True | 0 | True |
| macro | 105 | IsVentillo | True | True | 0 | True |
| macro | 106 | IsDeepWater | True | True | 0 | True |
| macro | 106 | IsVentillo | False | False | 367 | True |
| macro44 | 1 | IsDeepWater | False | False | 66 | True |
| macro44 | 1 | IsVentillo | True | True | 0 | True |
| macro44 | 2 | IsDeepWater | True | True | 0 | True |
| macro44 | 2 | IsVentillo | True | True | 0 | True |
| macro44 | 65 | IsDeepWater | True | True | 0 | True |
| macro44 | 65 | IsVentillo | True | True | 0 | True |
| macro44 | 66 | IsDeepWater | False | False | 66 | True |
| macro44 | 66 | IsVentillo | True | True | 0 | True |
| macro44 | 129 | IsDeepWater | False | False | 66 | True |
| macro44 | 129 | IsVentillo | True | True | 0 | True |
| macro44 | 130 | IsDeepWater | True | True | 0 | True |
| macro44 | 130 | IsVentillo | True | True | 0 | True |
| macro44 | 149 | IsDeepWater | True | True | 0 | True |
| macro44 | 149 | IsVentillo | True | True | 0 | True |
| macro44 | 150 | IsDeepWater | True | True | 0 | True |
| macro44 | 150 | IsVentillo | False | False | 367 | True |
| grid | 13 | TargetGrid | True | True | 0 | True |
| grid | 14 | TargetGrid | False | False | 56 | True |
| grid | 77 | TargetGrid | False | False | 56 | True |
| grid | 78 | TargetGrid | True | True | 0 | True |
| calls | 82 | TargetCalls | True | True | 0 | True |
| calls | 83 | TargetCalls | False | False | 25 | True |
| calls | 84 | TargetCalls | True | True | 0 | True |
| calls | 85 | TargetCalls | False | False | 25 | True |
| calls | 86 | TargetCalls | False | False | 25 | True |
| bfbb | 0 | IsDeepWater | False | False | 66 | True |
| bfbb | 4 | IsDeepWater | False | False | 66 | True |
| bfbb | 5 | IsDeepWater | True | True | 0 | True |
| bfbb | 6 | IsDeepWater | True | True | 0 | True |
| bfbb | 7 | IsDeepWater | True | True | 0 | True |

[EVIDENCE] Final decision model: **56/56**. Suffix arithmetic: **4/4**.

| Header state | Predicted suffix | Observed suffix | Next front-end ID |
|---|---:|---:|---:|
| original | 34693 | 34693 | 34838 |
| enum-only | 34250 | 34250 | 34395 |
| reference | 34188 | 34188 | 34333 |
| tree | 34176 | 34176 | 34321 |

[EVIDENCE] Initial transferred cost calibration: **12/15**, including the zero-cost control. Three failures are retained.

| Construct | Predicted cost | Observed cost | Hit |
|---|---:|---:|---|
| base | 0 | 0 | True |
| extern | 1 | 1 | True |
| global | 1 | 1 | True |
| proto0 | 1 | 1 | True |
| proto2named | 3 | 3 | True |
| proto2unnamed | 1 | 3 | False |
| typedef | 1 | 1 | True |
| enum3 | 4 | 4 | True |
| struct0 | 7 | 7 | True |
| struct2 | 7 | 7 | True |
| inline0 | 4 | 2 | False |
| inline1 | 5 | 3 | False |
| empty-header | 1 | 1 | True |
| guard-repeat | 1 | 1 | True |
| local | 1 | 1 | True |

[EVIDENCE] The discarded first-pass-only MOS model scored **13/16** (`predictions-initial.json`, `validation-first.json`):
it falsely predicted output changes at 114, 115 and 116. The later comparison repairs all three. These are excluded from the final held-out 56/56, not hidden.
