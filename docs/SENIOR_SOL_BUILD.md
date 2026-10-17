# sol-build: Build and DecorNextAction (2026-10-17)

## Outcome

[EVIDENCE] Started at HEAD `999fdebf58d54fc466596f6c72fa1ac25e38507b`
with a fresh copy of all `src/` under `scratch/sol-build/baseline-src/`.
Only `scratch/sol-build/`, `build/verify-solbuild/`, and this document were
written. Canonical source, headers, manifest and compiler catalog are unchanged.
No commit or background process was created.

[EVIDENCE] **Blocked: zero newly accepted functions and zero new code bytes.**
Final `scratch/sol-build/decblock.cpp` is byte-identical to canonical and the
fresh snapshot, with CRLF and tab indentation preserved. Header deltas: none.
The empty source-only `scratch/sol-build/integration.patch` passes
`git apply --check --allow-empty`; there is no passing source change to apply.

## Prior hypotheses excluded before compiling

[EVIDENCE] Read both rounds of `SENIOR_SOL_DECBLOCK.md`, all of
`DECBLOCK_TU.md`, and `SENIOR_SOL_DECOR.md`. Consulted the existing Build
disassemblies, SB2 ancestry, `LAYOUT_DEBT.md`, compiler hints and
`BLUPI-CTX-01/02`. The following are recorded negatives or incomplete repairs;
they were not rerun:

- [EVIDENCE] Build background assignment permutations, for/while/countdown
  forms, i/j exchange, rectangle extent increments, a working RECT, and
  moving extent assignments outside one or both loops do not close the gap.
- [EVIDENCE] Splitting division/modulo, aggregate or copied background
  POINTs, and global icon/channel sharing do not recover the target body.
- [EVIDENCE] Shared/direct normal player rendering, per-mode direct calls,
  bDraw and icon/channel sentinels, and goto completion change code but do
  not recover the complete CFG. The late-channel sentinel adds an unsupported
  compare/branch despite a better heuristic score.
- [EVIDENCE] Cloud call-local/aggregate points, invariant x, y-first stores,
  inline icon expressions, exchanged point identities, and moving shield y
  adjustment do not close the spill/cold-block differences.
- [EVIDENCE] Shared hide-image origin, root-point reuse and chained zero
  stores remain non-STRICT and can regress GetBlupiInfo. Shared remote-player
  locals and foreground bounds spelling do not close Build's size gap.
- [EVIDENCE] Top/before/end count probes leave Build byte-identical. They
  affect GetBlupiInfo but establish no legitimate missing declaration.
- [EVIDENCE] DecorNextAction point initialization before the guard changes
  269 to 267 bytes and fails. Moving the table, walking records with a for
  loop, component copies, and separate x/y clamp limits preserve the matching
  action body but do not preserve GetBlupiInfo.
- [EVIDENCE] Explicit OR temporaries, commuted OR operands and temporary
  reuse do not repair the blocked GetBlupiInfo loads; compound output writes
  add a store. Prior genuine local-sharing/scope controls also failed.
- [EVIDENCE] Defaulted PlaySound in place of the unsupported overload,
  removal of unsupported MoveStep/Build overloads, and save-path/CurrentWrite
  prototype cleanup do not close the peer gate. Broad wire-union cleanup was
  already rejected in LAYOUT_DEBT.md; it is not a new header hypothesis.

[EVIDENCE] The previous baseline has all 44 target call instructions. The
normal-code size difference is five bytes and the cold-tail difference is
15 bytes. These facts locate the unresolved source region without identifying
an additional operation. No new supported Build hypothesis survived this
review, so Build was not rewritten or subjected to another tuning campaign.

## New trial and compiler findings

[HYPOTHESIS] In DecorNextAction, the table index is dead after both coordinate
offsets are read: the matching-record arm exits the loop. Reusing that actual
local for both clamp limits could therefore recover authentic source context
without adding a declaration or changing behavior. This differs from the
previous separate-axis-limit and loop-form trials.

[EVIDENCE; VC5-CTX-01..04 / BLUPI-CTX-01 / VC5-LOC-03; negative]
Trial `t01-index-limit-reuse` changes `int num, num2` to `int num` and uses
`num` for the two bounds after the offset reads. Full decblock TU, canonical
headers, pinned SP3 `/W3 /GX /O2 /ML /Gd /Gy`, no PCH. Evaluation used only:

```powershell
.pyenv\python.exe tools\verify.py --tu src/decblock.cpp=scratch\sol-build\decblock.cpp --build-dir solbuild
```

[EVIDENCE] One candidate compile/run; exit 1 solely for accepted GetBlupiInfo.
DecorNextAction remains **STRICT**, RVA `0x5670`, **269 instruction bytes**,
six proven relocations to the complete 532-byte initialized action table.
GetBlupiInfo remains 67 instruction bytes with the same four fixed operand-load
differences. Build remains 3,846 instruction bytes versus 3,866, score 1,489.
All three complete COFF function payloads are byte-identical to the prior
`r22-current-head-action` diagnostic candidate. Removing this genuinely dead
local identity is inert at this checkpoint. No integration gain is claimed.

[EVIDENCE] Exact candidate, object, complete run results and corrected
body-byte metrics are in `scratch/sol-build/t01-index-limit-reuse/`; transcript
is `t01-index-limit-reuse.log`. The diagnostic source is retained only there.
The already recovered numeric initializer was reused as ordinary source data.

## Final gate and remaining blocker

[EVIDENCE] Restored the fresh canonical candidate and ran the same isolated
verification command. Exit **0**, with InitDecor, DrawInfo and GetBlupiInfo
STRICT and every active accepted entry passing the available regression gate.
The expected sound.cpp historical-path mapping denial leaves that TU's gate
INCOMPLETE; its accepted code-byte checks pass. Evidence:
`scratch/sol-build/final-baseline.log`, `final-baseline/results.json`,
`final-baseline/decblock.obj`, and `audit.json`.

[OPEN] DecorNextAction still cannot be integrated while preserving
GetBlupiInfo. The one new source-backed local-reuse hypothesis was disproven;
the reviewed Build and header/context families are exhausted. Stop here under
the task's diminishing-returns rule. The next useful evidence is an independent
source/CFG explanation for Build's unresolved regions or an independently
supported missing header/type declaration. No padding or count-filler proposal
is left for integration.
