# MSVC 5.0 matching knowledge base

This SB1-focused catalog supports matching work with MSVC 5.0 SP3 (`cl 11.00.7022`, `/O2 /GX`, `/ML`) and the 5.10 linker family. Transferred VC5 rules remain hypotheses until SB1 evidence validates them. Blood II is a provenance reference only; this repository has no build-time or run-time dependency on it. Intel compiler findings are excluded.

See [`docs/TOOLCHAIN.md`](../docs/TOOLCHAIN.md) for identification evidence and [`docs/MATCHING.md`](../docs/MATCHING.md) for strict matching requirements.

## Matching loop

1. Compile the owning translation unit with the pinned compiler, flags, headers, defines, PCH mode, and source path.
2. Compare with `tools/cmpfunc.py`; require `STRICT` plus verified relocations. Keep `FOLDED`, `RELOC-UNVERIFIED`, and normalized similarity distinct.
3. Characterize the instruction mismatch with `kb.py tags` or inspect the complete compare report.
4. Consult `kb.py hints`, then read the rule with `kb.py rule ID`.
5. Choose one source-supported hypothesis and predict the instruction changes and peer effects.
6. Test one minimal candidate in `scratch/`; verify the owning TU with `tools/verify.py --tu` when it is integrated.
7. Record the result, including failed or inert experiments, in `catalog.json` and `docs/MSVC5KB_VALIDATION.md`.

## Read the mismatch in this order

- Register-, x87-, or SIB-order only: check TU/compiler context before changing semantics.
- Load count, store order, branch layout, or call list: investigate the source form and control structure.
- Constant, condition, displacement, or layout mismatch: check the real source value, type, or structure first (`VC5-SEM-01`).

## Run the tools

```powershell
.pyenv\python.exe msvc5kb\tools\kb.py tags --json build\kb\sync\compare.json --function '?DecoratedSymbol@@...'
.pyenv\python.exe msvc5kb\tools\kb.py hints '?DecoratedSymbol@@...' --json build\kb\sync\compare.json
.pyenv\python.exe msvc5kb\tools\kb.py rule VC5-SEM-01
.pyenv\python.exe msvc5kb\tools\kb.py query 'field offset'
.pyenv\python.exe msvc5kb\tools\vc5probe.py probe --tu scratch\kb\sync\test.cpp --function '?DecoratedSymbol@@...' --rva 0x1234 --position top --count 0 --output-dir build\kb\probe
.pyenv\python.exe msvc5kb\tools\vc5probe.py sweep --tu scratch\kb\sync\test.cpp --function '?DecoratedSymbol@@...' --rva 0x1234 --trees sp3,sp2 --output-dir build\kb\sweep
```

`kb.py` reads `cmpfunc.py` JSON or an object/symbol/oracle-RVA pair. Forced or heuristic addresses remain advisory; tags never count as a match. `vc5probe.py` only handles scratch count-probe placement (`top`, `end`, `before:FUNCTION`) and compiler/flag sweeps. It delegates compilation and strict comparison to the repository tools. Set `TMP` and `TEMP` under `build/tmp` for direct compiler runs.

## Recording evidence

Classify each finding as **confirmed-transferable**, **Blupi-specific**, **unverified hypothesis**, or **negative**. Update the existing rule’s `blupi_status` and `blupi_evidence`, or add a `BLUPI-xxx` rule. Keep IDs and field names compatible for future bidirectional exchange; there is no sync mechanism. Optional `matching_effect` states what the rule predicts for a candidate (which source change, which bytes); `scope` says whether a rule is general VC5, flag/TU-specific, Blupi-specific or a negative.

Record: triggering mismatch; original and candidate constructs; compiler and flags; TU/header/PCH context; one-variable change and code effect; compile count; strict result and peer gains/losses; provenance reference; limits and untested conditions. Use `validated` only for measured SB1 evidence, `transferred-unverified` for VC5 findings not checked on SB1, and `inert-here` or `contradicted` for SB1 negatives.

## SB1 notes

- SP3 is pinned for matching. The controlled SP2/RTM sweep is target-specific; RTM changed two sound helpers even though the selected sound function stayed strict.
- The linker is from the PE 5.10 family. Fifty-six `OutputDebug` call sites resolve to the bare `ret` owned by `CMenu::~CMenu` at `0x1f010`; `cmpfunc.py` models these as `FOLDED`.
- Keep compiler code generation separate from linker folding and from SB1 source conventions such as headers, macros, and `__FILE__`/`__LINE__` inputs.
- Private evidence appears only as repository-path/commit references in `catalog.json`; review all transferred text before public release.
