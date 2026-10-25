# Source-build baseline and behavioral exceptions (2026-10-19)

The milestone is consolidated; see [RECOVERY_MILESTONE.md](RECOVERY_MILESTONE.md)
for retained artifacts and restoration of historical experiment paths.

[EVIDENCE] All 19 translation units and checked-in resources compile and link
without the original executable, asset directory, oracle maps or reference
repositories. Historical VC5 SP3, DirectX 5 SDK and Python with pefile remain
build dependencies. Capstone is needed for verification.

```powershell
.\.pyenv\python.exe tools/build.py
```

[UPDATED 2026-10-25] The ordinary build now covers all 14 targets, and each writes to its own
`build/standalone/<target-slug>/`. The current commands, requirements, outputs and reproducibility
results are in [BUILDING.md](BUILDING.md). The original 2026-10-19 baseline follows; the output
paths in it have since changed.

Output (2026-10-19): `build/standalone/BLUPI.EXE`, map, logs, `build.json` and `image.json`.
Explicit link settings are recorded in `recon.json:standalone_link`. The build reads no oracle.
The existing sound build-path helper may require running outside the sandbox. Playing would still
require level, graphics and sound assets. No game executable is run by the build.

## Accepted behavioral tier

[EVIDENCE] The user accepted the two remaining mapped game functions as
**EQUIVALENT**, following the separate-tier pattern in
[Icy Tower](https://github.com/missingno7/icytower_rerecon).
Here this means static local behavioral equivalence under the assumptions below.
Icy Tower's runtime replay evidence does not transfer to this project.

| Function | Non-relocation differences | Static evidence |
|---|---:|---|
| `CEvent::ChangePhase`, RVA 0x19310 | 32 bytes | 32 exhaustive guard predicate classes, 3 WriteInfo predicate classes, discarded-register/flags liveness; 231/231 relocation sites verified |
| `CEvent::TreatEventBase`, RVA 0x1d230 | 8 bytes | Toggle register-renaming identity for every 32-bit input, discarded-register liveness, identical PostMessage target and stack arguments; 273/273 relocation sites verified |

All other non-relocation bytes in these function bodies are identical. Guard
classes partition equality predicates rather than sample ordinary game states.
Assumptions: valid ordinary object memory, stable fields during reordered reads,
and declared x86 calling conventions. Timing, intermediate scratch registers,
concurrent mutation and fault ordering are outside this equivalence claim.
Two CRT target identities use unique pinned-library signatures; one folded getter
uses a relocation-free raw-identical body. This is not a whole-program proof.

Certificates in `evidence/equivalence/` bind complete emitted function bytes and
symbolic relocations, the original executable, checker, comparator, Capstone
version and CRT library. Each normal verification reruns the static proof and
checks the certificate. Changed evidence fails acceptance; fresh STRICT/FOLDED
results supersede the exception without needing it.

`tools/verify.py` and normal `tools/relink.py` enforce both tiers. Raw comparator
results remain MISMATCH for these bodies. EQUIVALENT contributes **zero** bytes
to strict coverage: **127,456 / 141,488 (90.1%)**. No canonical game source changed
for this milestone. Whole-image byte identity remains open.

To recheck existing objects, including their CRT evidence objects:

```powershell
.\.pyenv\python.exe tools/equivalence.py --objects-dir build/verify/objects
.\.pyenv\python.exe -m unittest discover -s tests
```

`--write-certificates` explicitly re-certifies after review of new evidence;
do not use it simply to silence a regression.

## Validation and next work

[EVIDENCE] A fresh copy containing only `src/`, `tools/` and `recon.json`, at
`build/standalone-isolation`, compiled all sources and resources successfully.
It has no `assets/`, `external/` or `oracle/` directory. Its exact objects passed
a separate historical relink into `build/standalone-validation/BLUPI.EXE`:
zero accepted regressions, both EQUIVALENT certificates passing, resources exact.
LINK emits its existing LNK4108 alignment warning; successful linking is not
runtime validation. Neither original nor rebuilt game was executed during that build validation.
Subsequently, the user authorized runtime testing with cnc-ddraw: the rebuilt game
reached the first playable level and exited cleanly. See [RUNTIME.md](RUNTIME.md).

[OPEN] Strict function bytes and historical LINK placement/ICF survivor choices
are deferred until new evidence or ideas arise. Preserve `BLOCKER_CHECKS.md`,
`PLANET_ORACLE.md`, `RESIDUAL_CAUSES.md` and the compiler catalog as research leads.
They do not block continuing from this complete source-build baseline. Full-game
runtime equivalence remains unproven; the new runtime smoke test has limited coverage.
