# MSVC 5.0 code-generation field guide

This directory is a reusable, evidence-backed reference for matching 32-bit x86 code emitted by the MSVC 5.0 compiler family. Start here, then search [`index.tsv`](index.tsv) for the assembly symptom and read the linked rule. The rules distinguish compiler mechanisms from source conventions, build flags, and linker behavior.

## Scope and evidence

- Primary engine evidence: MSVC 5.0 SP3 (`c1xx`/`c2` 11.x), Blood II Server `/O1` and Client `/O2`. This corpus does not cover every VC5 project or every optimizer path.
- Speedy Blupi evidence: cl 11.00.7022, `/O2 /GX`, single-thread `LIBC.LIB`; the candidate linker family is 5.10. See [`docs/TOOLCHAIN.md`](../docs/TOOLCHAIN.md) and [`docs/MATCHING.md`](../docs/MATCHING.md).
- Controlled SB1 checks here covered VC5 RTM, SP2, SP3, NT5 1592/1691 and VC6 RTM on two TUs. Those results are limited to the functions and flags reported in [`docs/MSVC5KB_VALIDATION.md`](../docs/MSVC5KB_VALIDATION.md).
- Intel C/C++ 2.4/3.0 renderer observations are excluded. Do not attribute ICL output to MSVC. Blood II macros, project headers, and wrapper call patterns remain source conventions, not compiler features.
- Evidence labels mean: **proven** = repeated controlled measurement under stated conditions; **supported** = one or a few controlled outcomes with a bounded interpretation; **hypothesis** = proposed mechanism without isolated confirmation; **negative** = a tested lever failed within the stated scope.

## The six-step loop

1. **Classify the mismatch.** Check instruction size, operand/register order, SIB addressing, scheduling, stack/local use, calls/inlining, EH cleanup, and relocation status.
2. **Look up the symptom.** Search `index.tsv`, then read the rule section. Separate compiler output from linker placement and project-specific source forms.
3. **Choose one hypothesis.** State what instruction bytes should change and which functions may be affected.
4. **Build a minimal experiment.** Compile a scratch TU with fixed compiler, flags, includes, defines, PCH mode, and target window. Change one source form at a time.
5. **Evaluate strictly.** Require full function bytes and verified relocations at a unique target. Record reloc-unverified and normalized instruction similarities as non-strict results.
6. **Update the evidence.** Record exact inputs, compile count, strict gains and losses, and confidence. Retain a negative result when a bounded search repeatedly fails.

## Triage order

1. Verify compiler tree, compiler components, optimization/runtime/EH flags, include paths, PCH settings, and translation-unit boundaries.
2. Compare source form: expression types, statement sequence, value lifetime, lexical scope, and actual inline/wrapper call sites.
3. Only then test counter context, declaration/include position, or other TU-wide numbering hypotheses. A reachable count is a context clue, not evidence that artificial declarations existed in the original source.

## Hard rules

- Never add padding, dummy declarations, unused locals, or artificial constructs to canonical matching source for the purpose of forcing bytes. Counter probes operate only on scratch copies.
- Keep exact byte and relocation checks strict. Do not replace them with masked equality, normalized disassembly, or same-size similarity.
- Preserve source semantics. A codegen change is not a candidate fix until its behavior and all peer-function effects are understood.
- Compare compiler objects before drawing linker conclusions. Report compiler-function equality separately from COMDAT folding, PE metadata, imports, thunks, and final image layout.
- Cite private Blood II evidence by repository path and measured counts only. Do not copy private source, disassembly, or file contents into public material.

## Rule map

- [`rules/tu-context.md`](rules/tu-context.md): counters, TU placement, header/PCH context, and compiler pass isolation.
- [`rules/statement-order.md`](rules/statement-order.md): statement cadence and `/O1` versus `/O2` scheduling.
- [`rules/register-operands.md`](rules/register-operands.md): x87 operands, SIB order, pointer access, and loop-head lifetimes.
- [`rules/member-lifetimes.md`](rules/member-lifetimes.md): member reloads, locals, declaration order, and floating-point lifetimes.
- [`rules/inlining.md`](rules/inlining.md): per-caller budgets, nested expansion order, wrappers, and flag effects.
- [`rules/eh.md`](rules/eh.md): `/GX`, cleanup paths, and scope shape.
- [`rules/templates-comdat.md`](rules/templates-comdat.md): delayed template instances and per-TU COMDAT generation.
- [`rules/flags-builds.md`](rules/flags-builds.md): compiler/flag sensitivity and bounded negative results.
- [`rules/linker-vs-compiler.md`](rules/linker-vs-compiler.md): COMDAT folding, linker fingerprints, and target-specific rankings.
- [`rules/project-conventions.md`](rules/project-conventions.md): wrapper calls, vector macros, and interface signatures.
- [`rules/negative-results.md`](rules/negative-results.md): failed local-order, cleanup, and padding searches.

## Probe driver

`tools/vc5probe.py` has four subcommands: `listing`, `probe`, `sweep`, and `diff`. It needs the repo-local `.pyenv\python.exe`, `pefile`, and Capstone. It imports `tools/msvc.py` to resolve compiler trees/environment and `tools/cmpfunc.py` for COFF parsing plus relocation-aware strict PE comparison. Those two repository modules are an explicit extraction dependency; a standalone package must vendor or replace their generic parts.

All generated outputs must stay under `build/kb/` or `scratch/kb/merge/`. `probe` rewrites one stable-path scratch copy between serial compile variants so path-sensitive constructs such as `__FILE__` do not vary with the count. Supply `--target-length` whenever the full oracle function size is known. For internal peer calls whose target is independently known, pass `--symbol-va SYMBOL=VA`; otherwise the tool preserves `RELOC-UNVERIFIED` instead of guessing.

Example against a function whose target mapping is already established:

```powershell
.pyenv\python.exe msvc5kb\tools\vc5probe.py listing `
  --tu scratch\kb\merge\baseline_src\sound.cpp --tree sp3 `
  --include C:\tools\dx6sdk\INCLUDE --define WIN32 --define NDEBUG `
  --flag /O2 --flag /GX --flag /ML --flag /Gd `
  --function '?CreateBufferFromWaveFile@CSound@@IAEHHPAD@Z'

.pyenv\python.exe msvc5kb\tools\vc5probe.py probe `
  --tu scratch\kb\merge\baseline_src\sound.cpp --tree sp3 `
  --include C:\tools\dx6sdk\INCLUDE --define WIN32 --define NDEBUG `
  --flag /O2 --flag /GX --flag /ML --flag /Gd `
  --function '?CreateBufferFromWaveFile@CSound@@IAEHHPAD@Z' `
  --target-pe 'assets\Speedy Blupi\BLUPI.EXE' --rva 0x22b40 --target-length 240 `
  --symbol-va '?CreateSoundBuffer@CSound@@IAEHHKKKKH@Z=0x422940' `
  --symbol-va '?ReadData@CSound@@IAEHPAUIDirectSoundBuffer@@PAU_iobuf@@KK@Z=0x422a10' `
  --position top --max-count 32 --step 1
```

`diff` classifies instruction streams heuristically; it cannot resolve COFF relocation fields. Only raw complete-byte equality or `cmpfunc.py`'s `STRICT` result is a match. See the validation record for measured commands, baselines, outcomes, and limits.
