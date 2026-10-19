# Recovery milestone and cleanup (2026-10-19)

## Accepted result

[EVIDENCE] The complete game builds from 19 canonical translation units and
checked-in resources using VC5 SP3 and the DirectX 5 SDK. Building does not need
the original executable. Strict verification covers 127,456 / 141,488 game-code
bytes (90.1%); two remaining mapped functions have pinned static EQUIVALENT
certificates. The user accepted the playable reconstruction after manual testing.
The cnc-ddraw smoke test reached the first level and exited normally.

| Question | Canonical record |
|---|---|
| Build without the original executable | [STANDALONE.md](STANDALONE.md), `tools/build.py`, `recon.json` |
| Byte-exact acceptance and exceptions | [WORKFLOW.md](WORKFLOW.md), `evidence/equivalence/`, `tools/verify.py` |
| Runtime setup and color depth | [RUNTIME.md](RUNTIME.md), `tools/runtime_smoke.ps1` |
| Remaining 40 differing code bytes | [RESIDUAL_CAUSES.md](RESIDUAL_CAUSES.md), [STATUS.md](STATUS.md) |
| Historical compiler runtime controls | [BLOCKER_CHECKS.md](BLOCKER_CHECKS.md) |
| Independent Planet compiler evidence | [PLANET_ORACLE.md](PLANET_ORACLE.md), `tools/planet_oracle.py` |
| Compiler findings and negative experiments | `msvc5kb/catalog.json`, [STEERING.md](STEERING.md) |

[OPEN] Neither full-image byte identity nor exhaustive gameplay equivalence is
claimed. The two strict body matches and original linker placement remain parked
until new evidence appears. Source-steering diagnostics are not canonical code.

## Retained local working set

- `assets/`, `external/`, `.pyenv/`, `.git/` and all tracked project files remain.
- `build/verify/`: current objects, compiler logs, comparison and equivalence results.
- `build/standalone-validation/`: final historical relink, EXE/map and gate receipts.
  Its inputs now reference retained `build/verify/objects`, not the old isolation tree.
- `build/runtime-20261019-230611/`: the user's working game folder, copied assets,
  cnc-ddraw, screenshot and runtime receipt. Its configuration and saves are preserved.
- `build/recovery/`: deduplicated research archive, inventory, archive verification,
  move receipt and the one-off consolidation recipes.

`scratch/` is reset to an empty workspace. No canonical source, original asset,
reference repository or historical toolchain is removed or rewritten by cleanup.

## Research archive and historical paths

Old `build/...` and `scratch/...` paths in research documents identify the original
experiment locations. They are no longer promises that every raw intermediate is
in the live workspace. Conclusions and reproducible canonical inputs remain in
Git; a compact local archive retains research scripts/source variants, small
reports and selected cited evidence. It also retains the authenticated extracted
Planet executables for static analysis, so the downloaded ISO copies are redundant.

`build/recovery/research.zip` stores payloads by SHA-256 plus `index.json`, mapping
each original path to its payload, size and retention reason. Identical files are
stored once. Every archived payload was hash-verified before moving originals.
The archive is local and ignored by Git; copy `build/recovery/` separately when
backing up research provenance. A Git clone still contains the complete canonical
source, tools, certificates and written findings.

```powershell
.\.pyenv\python.exe tools/recovery_archive.py list "scratch/blockers-20261019/*.py"
.\.pyenv\python.exe tools/recovery_archive.py restore "scratch/blockers-20261019/*.py"
```

Restoration verifies hashes, writes only under `build/` or `scratch/`, and refuses
to overwrite existing files. Archived scripts may require regenerating their
former binary/intermediate inputs. Large full compiler traces, IL snapshots,
duplicate objects, historical build trees and redundant runtime data copies are
intentionally not retained in the compact archive. Reproduce them only if a new
investigation needs them.

## Quarantine

[EVIDENCE] Final measured footprint (logical file sizes, excluding the four
uninspectable legacy staging entries noted below):

| Location | Size |
|---|---:|
| Active repository, excluding `to_delete/` | **1.127 GiB** |
| Quarantined obsolete work | **24.494 GiB**, 355,640 counted files |
| Deduplicated research ZIP, included in active total | **187.7 MiB** |

The archive retains 177,513 original paths backed by 71,825 distinct payloads
(2,365,251,375 logical bytes). SHA-256:
`4f533f05386c673822b984bc0e8b814d3c99a86664baaf7ddbcdf7fb1e8b627f`.
Selection retained scripts/source variants up to 2 MiB, compact text/results up
to 128 KiB, and selected cited/final receipts and independent oracle inputs above
those limits. Full selection and per-file hashes remain in the archive index.

Obsolete files are moved, not deleted, into
`to_delete/recovery-20261019/{build,scratch}` plus the old root `vc50.pdb`.
`to_delete/` is ignored by Git. The user can delete that entire folder after
reviewing this milestone; it contains no required live build/runtime inputs.
The original directory structure remains available there until deletion.
Four old Planet staging directory entries (`build/match/pb_src/{data,image,movie,sound}`)
return Windows error 6 during metadata inspection. They were not traversed or
archived; the parent tree was moved intact. Their type/contents were not established.

The before inventory and exact archive/move/size receipts are under
`build/recovery/`. Moving on the same drive does not free disk space until the
user deletes `to_delete/`.

## Post-move validation

[EVIDENCE] All 273 pinned retained runtime/build files are unchanged. Five
acceptance tests pass; both static equivalence checks and the exact-peer gate
pass using retained objects. Restoring `scratch/blockers-20261019/declarations.py`
from the archive reproduced the quarantined original byte-for-byte, and a second
restore correctly refused overwrite. The test copy was moved into quarantine.
A fresh full standalone build after cleanup compiled all 19 TUs and linked
successfully under `build/recovery/source-build-check/`, without oracle reads.
No game was launched during cleanup. Nothing was deleted by this consolidation.
