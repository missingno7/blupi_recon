Speedy Blupi 1 (1998) matching decompilation. Goal: C/C++ source that, compiled with the
historical toolchain, reproduces BLUPI.EXE machine code exactly. Matching comes first; no SDL,
no modernization, no renderer changes in matching source.

Hard rules:
- `assets/` is immutable original material; `external/` holds read-only reference repos
  (planetblupi, sb2-decomp, free-eggbert). Never modify, clean or commit either.
- Never execute BLUPI.EXE or anything from the ISO. Static analysis only.
- Historical toolchains live under `C:\tools` (msvc5_rtm, msvc5_sp2, msvc5_sp3, msvc5_cc_*,
  msvc5_link_*, msvc6_rtm, dx6sdk, dx61sdk, ghidra_*). Treat them as read-only. Run cl.exe with
  TMP/TEMP pointed at a directory inside this repo (e.g. `build/tmp`).
- No binary patching, no copied original bytes / `__asm _emit` in source, no artificial
  constructs whose only purpose is to force bytes. Masked/normalized equality is never a
  strict match; report it as what it is.
- Generated data, disassembly, objects and scratch work go under ignored `build/` or
  `scratch/`. Commit only small canonical inputs, focused scripts and concise docs.
- Separate proven facts from hypotheses in docs ([EVIDENCE] / [HYPOTHESIS] / [OPEN]).
- Do not run git commit; the supervisor integrates. Do not leave background processes running.
- Python: use `.pyenv\python.exe` (repo-local, ignored PyPy 3.11 with pefile + capstone 5.0.3).
  Prefer a few focused scripts over frameworks.
