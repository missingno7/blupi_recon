# Consolidation plan (Priority 5) — for review

2026-10-25, branch `sb2-matching`, HEAD `5711f29`. Status: **PROPOSAL**. Nothing below has been moved,
renamed or deleted.

[UPDATED 2026-10-25] The user approved the plan, and stage 1 is **done**: commits `5be3032`,
`d0ad80e` and `a4a2352`, with [BUILDING.md](BUILDING.md).

- All 14 targets build without the original executables, from a pruned clean export.
- The verify and relink gates for all 14 targets are unchanged.

Stage 2 (documentation) follows. It is based on three read-only audits, whose reports keep the file-level
evidence (ignored scratch):

- `scratch/w-consolbuild/REPORT.md`: builds, tooling inventory, and a clean-export test.
- `scratch/w-consolport/REPORT.md`: SDL3 port review.
- `scratch/w-consoldocs/REPORT.md`: documentation inventory and fresh per-target coverage.

Constraints carried through every stage:

- Matching tools and STRICT verification stay as they are.
- No experimental linker population is presented as recovered source, and no TU boundary changes for
  an unproven Rich count.
- No gameplay rewrites.
- `port/provenance.json`, `check_fidelity.py` and the fidelity CI stay valid.
- Every accepted function, certificate and independent target is preserved.
- Originals and compilers stay outside the repository.

## 0. Where the project stands (fresh, 2026-10-25)

Coverage figures come from a fresh `tools/target_coverage.py --target <t>` run over the accepted
manifests. Whole-image figures are taken from the documents, not recomputed.

| Executable | STRICT (+FOLDED) records | STRICT bytes / game .text | Target EQUIVALENT | Uncovered |
|---|---:|---:|---:|---:|
| SB1 Blupi us | 407 (+6) | 127,456 / 141,488 (90.1%) | 0 (1) | 14,032 |
| SB1 Blupi de, fr | 407 (+6) | 127,456 / 141,488 | 2 / 14,000 B | 32 |
| Eggbert 1 ×6 languages | 403 (+6) | 123,088 / 141,328 (87.1%) | 3 / 15,360 B | 2,880 |
| SB1 June demo (evidence only) | 372 (+3) | 73,248 / 140,784 (52.0%) | 0 | 67,536 |
| Blupi 2 en, de, fr | 452 (+5) | 122,240 / 173,072 (70.6%) | 9 / 50,800 B | 32 |
| Eggbert 2 | 450 (+12) | 136,448 / 172,992 (78.9%) | 4 / 36,304 B | 240 |

(1) The primary's ChangePhase and TreatEventBase have standalone static-equivalence certificates
(docs/STANDALONE.md). `equivalence_target.py` deliberately does not count them as a target tier. The
status page must state both facts, not one "accepted" line.

Whole image:

- SB1 us: canonical relink `557ca92e…3533`.
  - .rsrc exact.
  - .data differs by 1 byte.
  - .text is the right size; the layout and population are underdetermined (docs/SB1_CLOSURE.md).
- SB2: all four editions keep .data virtual size +32.

Port: 139 provenance edges. The last documented Windows suite ran 217/217 tests.

## 1. Proposed architecture (no large move)

The layout is already close to right. What's missing is clear **boundaries**, not new directories.

```
src/                     SB1 historical source (all 10 SB1 targets)
games/sb2/src/           SB2 historical source (Eggbert 2 + Blupi 2 base)
games/sb2/blupi/         Blupi 2 overlay (4 files; tools/source_tree.py)
src/res/, games/sb2/res/ resource scripts (SB2 RC files reference src/res images)
targets.json, recon.json, games/sb2/recon.json   registry + manifests
tools/                   front doors: build.py (ordinary), verify.py (STRICT/EQUIVALENT), relink.py
  ordinary-build closure: build, relink, msvc, buildpath, source_tree, targets
  verification:          verify, cmpfunc*, equivalence*, datacmp, target_coverage, crossmap --audit, test_targets
  research:              everything else (kept in place, indexed in docs/research/README.md)
oracle/, evidence/       verification inputs (unchanged paths; certificates pin them)
msvc5kb/, steering/      research (unchanged)
port/                    SDL3 port (own rules, own docs)
docs/STATUS.md           single current status page
docs/BUILDING.md         ordinary historical build, per edition
docs/VERIFYING.md        matching/verification workflow (maintainers)
docs/research/README.md  index of archived research docs (docs files stay in place until stage 5)
```

Tools stay where they are. Pinned paths (`tools/cmpfunc.py`, `tools/equivalence.py`, and histlink's
import of `tools/c2trace/c2dbg.py`) make moves costly and gain nothing.

The separation between reconstruction and research is made **by documentation and the import
closure**, not by moving files. `build.py` must never import a research or verification module, and
a test enforces that (stage 1).

## 2. Keep / move / archive / remove inventory

The audits classify each of the 189 tracked files. In summary:

| Group | Proposal | Justification |
|---|---|---|
| `src/`, `games/sb2/{src,blupi,res}`, `src/res`, manifests, targets.json | KEEP | Canonical source and registry. |
| The six ordinary-build tools (§1) | KEEP, fix boundary | `build.py` is the front door; the `relink.py` standalone branch has regressed (§4 P0). |
| Verification tools, `oracle/`, `evidence/`, `tests/` | KEEP unchanged | Pinned by certificates. Moving them would invalidate proofs. |
| Research tools (asmscore, cfgdiff, ctxsweep_*, decomp, derive_target, fnprobe, make_rc, oracle, target_oracle, permute, planet_oracle, recovery_archive, residues, c2trace/, ghidra/, fixtures/), `msvc5kb/`, `steering/` | KEEP in place, INDEX | None is demonstrated superseded. c2trace is a live histlink dependency. |
| `target_oracle.py` reference to nonexistent `tools/sb2_tumap.py`; SP3-only CLI text in HISTLINK.md | FIX text | Stale. Replaced by crossmap.py (`5e15f9b`) and RTM LINK integration (`22129ec`, `47c1d87`). |
| 72 `docs/*.md` | KEEP about 12 as live pages, ARCHIVE about 60 research/worker reports via an index; MERGE build pages | Detail in the docs audit, inventory table. 14 SENIOR_SOL_* and 4 SWEEP_* files are one-off reports. |
| `docs/STATUS.md` | REPLACE contents | Dated 2026-10-19, primary-only. Becomes the all-target status page. |
| README.md, docs/MULTI_GAME.md, docs/EGGBERT.md, docs/SB1_DEMO.md, docs/SB2.md status blocks | FIX stale numbers / label historical | Eleven conflicts listed in §5. |
| Ignored `build/`, `scratch/` (≈120 dirs) | KEEP reports/inputs/negatives; MOVE finished bulky output to `to_delete/` after review | Never by name pattern. `build/tmp/cd1/` holds 4 hash-confirmed original copies, and `build/recovery/research.zip` holds research not available anywhere else (back it up). |
| `__pycache__`, empty `msvc5kb/rules/` | REMOVE (trivial) | Regenerable or empty. |

Nothing tracked is proposed for deletion.

## 3. Minimal historical-build dependencies per edition

This is for an ordinary build: no oracle, no original executable, no inventories. The dependencies
common to every edition are:

- Python 3.11 with pefile.
- DX5 SDK INCLUDE and LIB (dsound, ddraw, dplayx).
- A Windows host.

Every edition uses `/W3 /GX /O2 /ML /Gd /Gy` and the defines `WIN32 NDEBUG _WINDOWS nullptr=0`. The
libraries are kernel32, user32, gdi32, winmm, the DX5 libraries, LIBC and OLDNAMES. dxguid.lib is not
needed.

| Edition | Source | Compile + RC | LINK + libs | Defines | RC | `sound.cpp` path |
|---|---|---|---|---|---|---|
| SB1 Blupi us/de/fr | src (19 TU) | VC5 SP3 | SP3 LINK 5.10 | de: LANGUAGE_DE | src/res/{blupi,sb1-blupi-de,-fr}.rc | `F:\Profil\` |
| Eggbert 1 ×6 | src | VC5 SP3 | RTM LINK 5.00 + RTM libs | _EGAMES (+ language) | src/res/sb1-eggbert-*.rc | `F:\Profil\` |
| SB1 demo | src | VC5 RTM | RTM 5.00 | _DEMO | none (falls back to retail; [OPEN]) | `F:\Profil\` |
| Eggbert 2 | games/sb2/src (19 TU) | VC5 SP3 | SP3 5.10 | none (_EGAMES default) | games/sb2/res/sb2-eggbert-en.rc (+ src/res images) | `D:\Blupi\Profil egames 2\` |
| Blupi 2 en/de/fr | games/sb2/src + blupi overlay | VC5 RTM | RTM 5.00 | _EGAMES=0 | games/sb2/res/sb2-blupi-*.rc | `D:\Blupi\Profil\` |

Component minimum: `CL.EXE, C1XX.DLL, C2.EXE`, `MSPDB50.DLL`, `RC.EXE, RCDLL.DLL` (plus MFC
`afxres.h`), and `LINK.EXE, CVTRES.EXE, MSDIS100.DLL` from the selected tree.

- **Real-path fallback:** without the historical `sound.cpp` path, the build still produces an
  executable, but its embedded data differs. Such a build never counts toward acceptance.
- **Not needed for an ordinary build:**
  - `--historical-host` (original MSVCRT qsort and clock);
  - Capstone;
  - inventories;
  - C1.DLL and the CRT sources (STRICT CRT evidence only).

**Additionally needed for STRICT/EQUIVALENT:**

- the registered original executable and its hash;
- Capstone 5.0.3;
- `oracle/` inventories and address maps;
- the certificates;
- fresh CRT evidence objects.

**Additionally needed for runtime testing:** cnc-ddraw, the game data, a bounded process, and
separate receipts.

## 4. Fragile or unreproducible workflows

| # | Failure | Fix (stage) |
|---|---|---|
| **P0** | **`tools/build.py` (the documented oracle-free build) exits before compiling when the original executable is absent.** `relink.py:299–301` checks and hashes the original before the standalone branch (regression `b63ac6d`, 2026-10-23). It also claims `oracle_read=false`. | Check the original only outside `--standalone`. Prove it in a clean export (pilot, §8). |
| P0 | `targets.load` reads the oracle address map even for ordinary builds. | It is harmless while `oracle/` is tracked; document that the map is a proof input. Stage 1. |
| P1 | SB2 has no `standalone_link` (base, alignment, object order). No ordinary SB2 build exists. | Add explicit settings per manifest. Label the object order as a *source-build order*, not recovered. Stage 1. |
| P1 | Secondary targets' `build.py` writes to the same `build/standalone`. | Default the output to `build/standalone-<slug>`. Stage 1. |
| P1 | The CL banner (11.00.7022) is the same for RTM and SP3. Eggbert 1 mixes an SP3 compile with an RTM link. | Record the selected component hashes in build.json and preflight them. Stage 1. |
| P1 | Hardcoded `C:\tools` roots. | Optional `BLUPI_TOOLS` root override plus a version/hash preflight. Defaults unchanged. Stage 2. |
| P1 | Historical-path unavailability prints INCOMPLETE but exits 0. | build.json records `degraded`, and the doc states it never accepts. Stage 1 (doc), stage 2 (exit code for verify only). |
| P1 | Compile/drive locks live under each checkout's `build/locks`, so they are not shared across worktrees. | Keep the serial policy; document the limitation. A shared lock root comes later if worktrees return. |
| P1 | SB2 RC files reach into `src/res`. | Document it as a declared dependency; no copy. |
| P1 | The demo silently falls back to the retail RC. | Print an explicit "partial resources" notice. The demo stays evidence-only. |
| P1 | `build/tmp/cd1` holds copies of originals that can be confused with rebuilt files. | Document it, and make runtime selection explicit by hash (already true for the oracle tools). |
| P1 | `build/recovery/research.zip` is ignored and is the only copy. | The user backs it up outside the repo. Index it in the research README. |
| P2 | `unittest discover -s tests` skips `tools/test_targets.py`. | Document both commands in VERIFYING.md. |
| P2 | Ghidra, JDK and 7z paths are hardcoded in research tools. | Document them in the research index only. |
| P2 | The sandbox denies worktree metadata. | A real clean-worktree test runs once on the host, not in the sandbox. |

## 5. Documentation cleanup

The docs audit lists eleven concrete stale or contradictory statements, with file:line references.
The main ones:

- **README and MULTI_GAME say SB2 "not started".** Both also carry Eggbert 1 at 123,120 B; it is
  123,088.
- **MULTI_GAME carries stale SB2 totals:** 136,080 and 121,712 B, against fresh 136,448 and 122,240.
- **The primary's two standalone certificates are called "accepted EQUIVALENT".** Fresh tooling
  counts no target EQUIVALENT tier for the primary.
- **README says "411 STRICT".** That is an emission sum; there are 407 unique records.
- **SB1_DEMO says 89,296 B.** That is a candidate result; the accepted manifest gives 73,248 B.
  Reconcile, or integrate the candidate, only on new evidence. The demo stays evidence-only.
- **SB2.md keeps the superseded Eggbert-primary edition policy** and pre-round-18 totals.
- **The port README is dated 2026-10-22,** and SB2_PORT has 100/116/199/210 milestone totals ahead of
  the current 217.

Target structure:

| Page | Audience | Built from |
|---|---|---|
| README.md | user | Short entry point. Links only; no metrics. |
| docs/STATUS.md | everyone | The only current page for all 14 executables plus the port. It has four separate sections: STRICT, EQUIVALENT, whole image, port fidelity. Each number is dated and marked as fresh tool output or doc-only. |
| docs/BUILDING.md | builders | §3 of this plan, plus STANDALONE, TOOLCHAIN (selection part), BUILD_PATH, RESOURCES. |
| docs/VERIFYING.md | maintainers | WORKFLOW, MATCHING, and the target-policy parts of MULTI_GAME. AGENTS.md stays the binding rules file. |
| docs/research/README.md | researchers | Index of every research and worker report, plus SB1_CLOSURE, SB2, SB1_DEMO, msvc5kb and oracle docs. Old rounds and policies are labelled historical, not deleted. |
| port/README.md, port/docs/* | port | Unchanged structure. The current total moves to STATUS; milestone history stays. |

## 6. SDL3 port cleanup, ranked (value vs risk)

All work stays in `port/`. Shared gameplay TUs, the scene builders, EndOfTick, wide-editor dispatch,
state lists and historical main/event bodies stay game-owned: their duplication is required for
provenance.

| Rank | Change | Value | Risk | Gate |
|---|---|---|---|---|
| 1 | `tools/linux_release.sh` copies an incomplete tree, so `fidelity_map` cannot pass. `fidelity_{init,gaps,audio}.cmake` hardcode `.pyenv/python.exe` and asset paths, so tests silently vanish on Linux. | High: release recipe and hidden gates | Low gameplay risk | Fresh Linux configure and test inventory; `check_fidelity --check` on the full tree. |
| 2 | Current-state docs and comments (README "Next steps", ARCHITECTURE §6/7/9, SB2_PORT OPEN lists, `games/sb2/games/sb2` path typos). | Medium/high | None in code; snapshot hashes change | `check_fidelity` snapshot plus base/head impact. |
| 3 | `game/language.cpp` (376 of 394 lines shared) moves into the common language service, with game catalogs as data. | High | Medium: fallback and char-table behaviour | Language/text/resource fidelity tests, all goldens, review of all map edges. |
| 4 | Pixmap Device/Recorder/capture/BMP conversion extracted, one helper at a time. CPixmap policy and clipping stay. | High | Medium/high: pixels and lifetime | Render, scene and wide-view goldens at both depths, the oracle display comparison. |
| 5 | Byte-identical `member_access.h` shared; context-compiled `misc.cpp`. | Medium | Low/medium | Layout/RNG/stdio/cursor fidelity tests, plus GCC and Clang builds. |
| 6 | Movie/network unavailable adapters aligned; the missing SB1 log-once added. | Low/medium | Low | Menu-tour goldens; empty-provider and skip contracts. |
| 7 | PCM load and MIDI poll plumbing shared. CSound policy stays per game. | Medium/high | Medium/high | Audio fidelity tests for all 4 editions. |
| 8 | The Android importer accepts SB2 and malformed trees that native code rejects. | Medium (real bug) | Medium (storage) | New predicate tests; bounded emulator import. |
| 9 | Probe script parser shared; setters stay game-owned. | Low | Medium (trace order) | Exact per-frame state dumps. |
| 10 | CMake context helper; prune the confirmed-unused `PortPendingMessages` and SB2 `PortPixmapPresentBack`. | Low | Low/medium | Effective compile-flag comparison. |

Recommended scope: ranks 1, 2, 5, 6 and 8. Ranks 3, 4 and 7 are worthwhile, but each needs its own
review round. Ranks 9 and 10 are optional.

## 7. Staged migration with gates and rollback

Every stage is one or more ordinary commits on `sb2-matching`. Its gate must pass before the next
stage starts, and the rollback is `git revert` of that stage's commits.

**The global gate, required at every stage:**

- `tools/verify.py` primary report byte-identical, plus relinked image `557ca92e…3533` unchanged;
- `tools/verify.py --target` for all 13 secondaries, with unchanged accept sets;
- `scratch/sb2/samestatus.py`;
- `tools/test_targets.py` and `tests/`;
- `port/tools/check_fidelity.py --check`;
- the port ctest suite, when `port/` is touched.

| Stage | Content | Extra gate | Rollback point |
|---|---|---|---|
| 1. Ordinary build | P0 standalone fix (pilot); per-target output dirs; component hashes in build.json; SB2 `standalone_link` plus an ordinary SB2 build for 4 editions; import-closure test for build.py | Clean `git archive` export without assets: `build.py` for all 14 targets exits 0, `oracle_read=false`. Hashes recorded in BUILDING.md (deterministic modulo timestamp) | Tag `consol-1` |
| 2. Docs | STATUS.md replaced; BUILDING.md and VERIFYING.md created; research index; eleven conflicts fixed; HISTLINK and target_oracle text fixed | Link check; numbers re-derived by `target_coverage.py` in the same commit | `consol-2` |
| 3. Port low-risk | Port ranks 1, 2, 5, 6, 8 | Windows suite (217 or more) plus a Linux check; map base/head impact reviewed | `consol-3` |
| 4. Port services | Port ranks 3, 4, 7, one per commit | All goldens, plus the oracle display comparison for the affected game | Per commit |
| 5. Local hygiene | Reviewed `scratch/` and `build/` output moved to `to_delete/` (never deleted); archive docs moved to `docs/research/`, only if the user wants physical moves | Every certificate, evidence path and doc link still resolves; global gate | `consol-5` |

Stages 1 and 2 are independent and could run in parallel. Stages 3 and 4 touch only `port/`.

Matching research resumes only on new evidence, after stage 2.

## 8. Pilot (non-disruptive): repair the oracle-free build

Change: in `tools/relink.py`, hash the original executable only when `--standalone` is not set.

- The standalone path already sets `oracle=None` and reads base and alignment from
  `recon.json:standalone_link`.
- Verification and relink modes keep the same hash check.
- No matching source, tool pinned by certificates, manifest or port file changes.

Gates:

- `build.py` with the original present: same build as before.
- `build.py` in a `git archive` export without `assets/`: exit 0.
- Global gate unaffected: verify.py does not use this path.

Result: see §9.

## 9. Pilot result

[EVIDENCE] 2026-10-25. The change is applied in `tools/relink.py`: the original-executable
existence and hash check runs only when `--standalone` is absent.

- **Determinism.** Two consecutive `build.py` runs differ in exactly one byte, file offset 0xb8, which
  is part of the PE TimeDateStamp (the current clock). The standalone build is otherwise
  deterministic.
- **Behaviour unchanged.** HEAD and the fix give the same `build/standalone/BLUPI.EXE`, 535,552 bytes
  in both, identical except for the 4 PE timestamp bytes.
- **Oracle-free primary build.** In a `git archive HEAD` export without `assets/`, with only `.pyenv`
  junctioned and the fixed relink.py copied in, `tools/build.py` returns exit 0. build.json records
  `oracle_read=false` and `degraded={}`: the historical `F:\Profil\sound.cpp` path worked. The output
  equals the main checkout's build apart from the timestamp. Before the fix, this command exited 2
  before compiling (consolbuild audit).
- **Oracle-free Eggbert 1.** In the same export,
  `build.py --target sb1/eggbert/en --out-dir build/standalone-eggbert-en` links `egames.exe` with
  exit 0.
- **SB2 still missing.** `build.py --target sb2/eggbert/en` stops at "requires image_base and
  section_alignment". That is the stage 1 SB2 work and is still open.
- **Tests.** `-m unittest discover -s tests` passes 5/5 and `tools/test_targets.py` passes 44/44.
- **Not run:** verify.py and relink for acceptance. That code path is textually unchanged: the same
  check still runs whenever `--standalone` is absent.

The export was moved to `to_delete/` after its junction was removed.

[OPEN] A timestamp-free reproducible build needs a recorded `standalone_link` time stamp. That is a
stage 1 option, and its value must be labelled a chosen build setting, not a historical recovery.
