# Status

The single current status page for every historical executable and the SDL3 port, as of
**2026-10-25** on branch `sb2-matching`. Each figure is marked **fresh** (re-run for this page) or
**doc** (taken from the cited document). Progress is reported per executable; no figure here is a
combined metric. The previous SB1-only page is preserved as
[research/SB1_STATUS_2026-10-19.md](research/SB1_STATUS_2026-10-19.md).

The four kinds of milestone below are independent claims:

- STRICT coverage;
- EQUIVALENT certificates;
- whole-image identity;
- port fidelity.

A built executable is none of them ([BUILDING.md](BUILDING.md)).

## 1. Function matching: STRICT

Fresh: `tools/target_coverage.py --target <t>` over the accepted manifests. The bytes are unique game
`.text` bytes; FOLDED bodies count once, under their owner.

| Executable | Target(s) | STRICT + FOLDED records | STRICT bytes / game .text |
|---|---|---:|---:|
| Speedy Blupi 1 (1998), US | `sb1/blupi/us` (primary) | 407 + 6 | 127,456 / 141,488 (90.1%) |
| Speedy Blupi 1, German | `sb1/blupi/de` | 407 + 6 | 127,456 / 141,488 (90.1%) |
| Speedy Blupi 1, French | `sb1/blupi/fr` | 407 + 6 | 127,456 / 141,488 (90.1%) |
| Speedy Eggbert 1 (1999) | `sb1/eggbert/en`, `de`, `es`, `fr`, `it`, `pt` (each) | 403 + 6 | 123,088 / 141,328 (87.1%) |
| Speedy Blupi 1 June 1998 demo (evidence only) | `sb1/blupi/demo` | 372 + 3 | 73,248 / 140,784 (52.0%) |
| Speedy Blupi 2 (2001) | `sb2/blupi/en`, `de`, `fr` (each) | 452 + 5 | 122,240 / 173,072 (70.6%) |
| Speedy Eggbert 2 (2004) | `sb2/eggbert/en` | 450 + 12 | 136,448 / 172,992 (78.9%) |

Notes:

- **The demo's verifier figure is higher.** Fresh, the verifier emits 89,296 STRICT bytes for the
  demo. Its demo-only bodies are not promoted into accepted records, because the target is
  evidence only (docs/SB1_DEMO.md).
- **Earlier pages give the primary as "411 STRICT".** That is the verifier's emitted-record sum,
  which counts repeated bodies; there are 407 unique accepted records.

## 2. EQUIVALENT certificates

Fresh. EQUIVALENT means static local behavioural equivalence under pinned certificate assumptions. It
never counts as STRICT, and the byte status stays MISMATCH ([VERIFYING.md](VERIFYING.md)).

| Executable | Target-tier certificates | Bytes | Not covered by STRICT, FOLDED or EQUIVALENT |
|---|---:|---:|---:|
| SB1 US | 0 (the 2 standalone certificates are below) | 0 | 14,032 B |
| SB1 German, French | 2 each: ChangePhase, TreatEventBase | 14,000 | 32 B |
| Eggbert 1 (each language) | 3: ChangePhase, TreatEventBase, WindowProc | 15,360 | 2,880 B |
| SB1 demo | 0 | 0 | 67,536 B |
| Blupi 2 (each language) | 9 | 50,800 | 32 B |
| Eggbert 2 | 4 | 36,304 | 240 B |

- **SB1 US has two standalone certificates.** ChangePhase (6,928 B) and TreatEventBase (7,072 B)
  are checked by `tools/equivalence.py` on every primary verify, and both passed fresh on
  2026-10-25 ([STANDALONE.md](STANDALONE.md)). `equivalence_target.py` intentionally declares no
  target tier for the primary. Of the 14,032 uncovered bytes, 14,000 are these two functions; the
  rest are 32 bytes of DirectX import thunks and padding.
- **The 32-byte remainders** of SB1 German/French and Blupi 2 are that same linker output.
- **Eggbert 1's remaining bytes** include DoInit (relocations verified, raw comparator
  RELOC-UNVERIFIED).
- **Eggbert 2's 240 bytes** are CreateSoundBuffer (208 B, an `[OPEN]` sound.cpp line difference;
  docs/SB2.md) plus 32 bytes of thunks.

## 3. Whole image

**Fresh relinks (2026-10-25).** These are the stage 1 regression-gate relinks
(`tools/regression_gate.py`): the verified objects in the oracle-derived TU order, compared statically
with each original.

- **The primary relink runs under the historical host clock** (`--historical-host`). It is
  `557ca92e…3533`, unchanged.
- **The other relinks use the current clock.** Their PE and resource timestamp bytes therefore
  differ too.

**No executable is byte-identical to its original.**

| Executable | File size (orig / relink) | Same-offset differing bytes | Byte-exact sections | `.data` equal bytes, virtual-size delta | Accepted functions at original RVA |
|---|---|---:|---|---|---:|
| SB1 US | 535,552 / 535,552 | 121,401 | `.rsrc` | 329,727 / 329,728, 0 | 131 / 412 |
| SB1 German, French | 538,112 / 538,112 | 26,261 each | `.rsrc` | 329,715 / 329,728, 0 | 394 / 412 |
| Eggbert 1 (en, de, es, it, pt; fr) | equal for each language | 26,625; fr 26,553 | none (`.rsrc` differs in timestamps only, doc) | 329,202 / 329,216, 0 | 390 / 408 |
| SB1 demo | 533,504 / 534,016 | 185,717 | none | 307,822 / 328,704, +736 | 10 / 374 |
| Blupi 2 (each language) | equal for each language | 47,503 | `.rdata` | 347,915 / 349,184, 0 | 339 / 456 |
| Eggbert 2 | 602,624 / 602,624 | 164,336 | `.rsrc` | 347,487 / 363,520, 0 | 87 / 461 |

**Causes** (doc):

- **SB1 US:** almost all of its differences are a +16 shift. LINK keeps a different copy of a folded
  empty function (an ICF survivor), plus the CRT filter survivor and the import order. Its Rich count
  is 268 against 270 ([SB1_CLOSURE.md](SB1_CLOSURE.md), [BLOCKER_CHECKS.md](BLOCKER_CHECKS.md)).
- **The original linker inputs are underdetermined,** and that limit applies to every target.
- **SB1 demo:** it has no demo resource script, and its text is 32 bytes short. It is evidence only.

**SB2 `.data` virtual size.** [EVIDENCE] The fresh relinks show the same `.data` virtual size as the
originals for all four SB2 targets. docs/SB2.md round 20 had recorded +32. [OPEN] The cause of the
change was not investigated in this consolidation; the hotspot-table correction (`96afd2e`) followed
that round.

**Experimental witness.** Branch `sb1-histlink` holds a 412/412-at-RVA linker-input candidate. It is
evidence only, not recovered source (SB1_CLOSURE.md).

## 4. Ordinary historical build

Fresh: consolidation stage 1, on commit `a4a2352`. Details are in [BUILDING.md](BUILDING.md).

- **All 14 executables build with `tools/build.py`,** using their historical compiler and linker.
  No original executable, oracle, certificate or research tool is read; `tests/test_build_boundary.py`
  enforces this.
- **The clean-export test passed:** `tools/check_clean_build.py`, a pruned `git archive` with no
  assets or oracle.
  - Every image equals an independent build apart from its PE and resource-directory timestamps.
  - Every toolchain component matched `toolchains.json`.
  - Every historical `__FILE__` path was established.
- **The SB1 demo builds with the retail resource script.** It is labelled PARTIAL.

## 5. SDL3 port

The port is two native programs, `blupi-sdl3` (SB1, Eggbert 1) and `blupi2-sdl3` (Blupi 2, Eggbert 2).
They compile the historical game TUs unchanged: 12 from `src/` and 11 from `games/sb2/src/`
([port/README.md](../port/README.md)).

**Fresh results (2026-10-25):**

- **Windows test suite: 269 / 269 passed**, serially, in 2,303 s: `ctest --test-dir build/port`
  with the MinGW-x64 Ninja build. Linux and Android were not re-run (doc: the release receipts in
  port/docs/RELEASING.md and ROADMAP.md).
- **The provenance map passes:** `port/tools/check_fidelity.py --check`, with 139 edges and 0 issues.
  The categories describe source relationships, not percentages of fidelity:

| Target | Direct shared | Verified derivative | Verified platform replacement | Intentional difference | Unverified | Edges |
|---|---:|---:|---:|---:|---:|---:|
| SB1 US, German, French (each) | 12 | 24 | 16 | 13 | 6 | 71 |
| Eggbert 1 (each language) | 12 | 24 | 16 | 14 | 6 | 72 |
| Blupi 2 (each language) | 11 | 30 | 17 | 15 | 7 | 80 |
| Eggbert 2 | 11 | 27 | 17 | 15 | 7 | 77 |
| SB1 demo | not supported by the port (F454) | | | | | |

**Runtime evidence** (doc: [port/docs/ORACLE.md](../port/docs/ORACLE.md) and
[port/oracle/SB1_SAVE.md](../port/oracle/SB1_SAVE.md)):

- **Cross-build display oracle against the rebuilt historical executables.** Attract mode is 2,200 of
  2,200 frames identical for SB1 and for each SB2 edition, at 8 and 16 bpp.
- **SB1 screen tour:** identical except for the multiplayer provider list.
- **SB1 US save interchange:** identical in one recorded scenario, at both depths.

These are bounded receipts, not whole-game certification.

**The port is not functionally complete.** These services are unavailable or differ:

- **Multiplayer:** SB1 has native multiplayer (release candidate 1.2.0-rc1,
  [port/docs/MULTIPLAYER.md](../port/docs/MULTIPLAYER.md)). It works for two to four players across
  Windows, Linux and Android, with LAN discovery, a content check and `--net-join`. It has been tested
  automatically, with little real-device play. SB2 multiplayer is still unavailable (roadmap stage 10).
- **AVI movies:** not played; the phase is skipped.
- **SB2 CD audio:** fails, logged once.
- **MIDI:** played with a modern SoundFont, not the 1998 device's timbre.
- **Coverage gaps:** hardware gamepads and Android keyboard/background are open. SB2 on Android
  (1.2.0-rc2) is checked on the emulator only: start, menus, touch controls and play.

## 6. Where work stands

- **Matching.** The SB1 and SB2 function residues are at the limit of the current evidence: the
  open functions need authentic source or context evidence (docs/SB1_CLOSURE.md, docs/SB2.md
  section 12). Matching research resumes only on new evidence.
- **Whole image.** The original linker inputs (dead contributions, TU population, order) are
  underdetermined by the binaries. Experimental witnesses, such as branch `sb1-histlink`, are
  evidence, not recovered source.
- **Consolidation.** Stage 1 (ordinary builds) and stage 2 (this documentation) are done; stages
  3 to 5 follow (docs/CONSOLIDATION_PLAN.md).
- **Port.** Native multiplayer, roadmap stage 10, is in progress:
  - SB1 ships in 1.2.0-rc1;
  - next: real-network tests, in-game address entry, then SB2.
