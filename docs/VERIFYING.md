# Verifying the reconstruction against the originals

Historical verification checks reconstructed source against each original executable. It is separate
from the ordinary build ([BUILDING.md](BUILDING.md)) and from runtime and port fidelity testing
([RUNTIME.md](RUNTIME.md), [port/docs/FIDELITY.md](../port/docs/FIDELITY.md)). The binding rules are in
[AGENTS.md](../AGENTS.md); this page explains the commands, the proof tiers and the gates. Current
numbers are in [STATUS.md](STATUS.md).

## Inputs beyond the ordinary build

- **The registered original executable** for each target: its path and SHA-256 are in `targets.json`,
  the file lives under `assets/`, and it is never executed.
- **Python with pefile and Capstone 5.0.3** (`.pyenv`).
- **Inventories:**
  - `oracle/` (functions, TU map) for SB1 Blupi;
  - `oracle/<target>/` (own inventory and address map) for Eggbert 1, the demo and SB2.
- **The acceptance record:** the manifest's `accepted_functions`, mappings and forced locations, plus
  per-target overrides in `targets.json`.
- **Equivalence certificates** under `evidence/equivalence/`.
- **CRT evidence objects** (STDARGV.C, CRT0DAT.C) compiled from the selected VC5 tree. They are never
  linked into the game.
- **The historical `__FILE__` path** for `sound.cpp`, exactly as for the build
  ([BUILD_PATH.md](BUILD_PATH.md)). Without it the gate is **INCOMPLETE**, and INCOMPLETE is never a pass.

## Proof tiers

Definitions and comparator details are in [WORKFLOW.md](WORKFLOW.md#comparator-statuses).

| Tier | Meaning | Counts toward |
|---|---|---|
| STRICT | Every non-relocation byte of the function is equal at one proven location; every relocation is bound to an independently proven target | STRICT coverage |
| FOLDED | Identical body that the original linker merged into another function, proven by verified references | counted once, under its owner |
| EQUIVALENT | Static local behavioural equivalence under pinned certificate assumptions; the byte status stays MISMATCH | its own tier, never STRICT |
| RELOC-UNVERIFIED | Code bytes equal, but a relocation target or a path-dependent string is not proven | nothing |
| MISMATCH | Code or proven data differs | nothing |

Masked, normalized or "only relocations differ" results are diagnostics, never matches. The
primary's two standalone certificates (ChangePhase, TreatEventBase; [STANDALONE.md](STANDALONE.md))
are checked by `tools/equivalence.py`. `tools/equivalence_target.py` deliberately does not declare
them as a target tier, so `target_coverage.py` shows no EQUIVALENT bytes for `sb1/blupi/us`.

## Commands

```bash
.pyenv/python.exe tools/verify.py
.pyenv/python.exe tools/verify.py --target sb2/blupi/en
.pyenv/python.exe tools/verify.py --tu src/x.cpp=scratch/me/x.cpp --build-dir me
.pyenv/python.exe tools/verify.py --accept
.pyenv/python.exe tools/target_coverage.py --target sb1/eggbert/en
.pyenv/python.exe tools/relink.py --objects-dir build/verify/objects --out-dir build/relink-us --historical-host
.pyenv/python.exe tools/relink.py --target sb2/eggbert/en --out-dir build/relink-sb2-eggbert-en
.pyenv/python.exe tools/crossmap.py --target sb2/eggbert/en --audit
```

- **`verify.py`** runs a clean build and strict comparison for the primary `sb1/blupi/us`, with its
  regression gate. Its output goes to `build/verify/`, or `build/verify-<slug>/` for other targets.
- **`verify.py --tu`** tests one candidate source for one run, without promoting it.
- **`verify.py --accept`** promotes newly STRICT/FOLDED functions. A regression or an INCOMPLETE path
  gate refuses it.
- **`target_coverage.py`** gives accepted coverage from the manifest, without building. It is the
  progress measure.
- **`relink.py`** relinks the verified objects and compares the image layout with the original; that
  is a diagnostic, not acceptance. The primary's `--historical-host` reproduces `557ca92e…`.
- **`crossmap.py --audit`** is the SB2 identity audit; it runs before any SB2 `--accept`.

The matching loop (difference-first, `residues.py`, `fnprobe.py`, `msvc5kb/tools/kb.py hints`) and
the compiler-tracing tools are described in [WORKFLOW.md](WORKFLOW.md) and
[msvc5kb/README.md](../msvc5kb/README.md). They are research aids; their results are confirmed only by
`verify.py`.

## Regression gate

Every change to source, manifests, the registry or the matching tools must pass all of the following:

1. Every target's `verify.py` result is unchanged, or improved by a justified match, with zero accepted
   regressions. Targets are matched independently (AGENTS.md): no executable may lose STRICT coverage so
   that another gains.
2. Every EQUIVALENT certificate still passes.
3. The primary relink stays `557ca92e6ec8c71cfaf5e9f2d55969d3ebcd614c771c9010e074c6ad82ba3533`.
4. The historical sound path was established (the result is not INCOMPLETE).
5. The test suites pass:
   - `.pyenv/python.exe -m unittest discover -s tests`
   - `.pyenv/python.exe tools/test_targets.py`, which `discover` does not find
   - `port/tools/check_fidelity.py --check`

The whole set for all 14 targets runs one target at a time:

```bash
.pyenv/python.exe tools/regression_gate.py run before
.pyenv/python.exe tools/regression_gate.py run after
.pyenv/python.exe tools/regression_gate.py compare before after
```

Run it once before the change and once after, then compare. The comparison requires all of these:

- byte-identical verifier results and certificate checks;
- identical transcripts, apart from staged-tree names;
- the primary relink image byte-identical;
- every other relinked image identical apart from its PE and resource-directory timestamps.

[EVIDENCE] 2026-10-25 (consolidation stage 1): all 14 targets passed this comparison across the
build/relink refactor.

## Where the details are

| Topic | Document |
|---|---|
| Target model, editions, languages, source rules | [MULTI_GAME.md](MULTI_GAME.md), [EGGBERT.md](EGGBERT.md), [SB2.md](SB2.md), [SB1_DEMO.md](SB1_DEMO.md) |
| Comparator internals, folded bodies, data proofs | [WORKFLOW.md](WORKFLOW.md), [MATCHING.md](MATCHING.md) |
| Toolchain identification | [TOOLCHAIN.md](TOOLCHAIN.md) |
| Whole-image and linker research | [SB1_CLOSURE.md](SB1_CLOSURE.md), [HISTLINK.md](HISTLINK.md), [LINK_ICF.md](LINK_ICF.md) |
| Equivalence certificates | [STANDALONE.md](STANDALONE.md), `tools/equivalence*.py`, `tests/test_equivalence.py` |
| Everything else (studies, TU reports, sweeps) | [research/README.md](research/README.md) |
