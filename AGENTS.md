Speedy Blupi 1 (1998) matching decompilation. Goal: C/C++ source that, compiled with the
historical toolchain, reproduces BLUPI.EXE machine code exactly. Matching comes first; no SDL,
no modernization, no renderer changes in matching source.

The native SDL3 port lives only in `port/` (branch `port/sdl3`) and has its own rules in
`port/AGENTS.md`. It compiles `src/` unchanged; never edit `src/` for the port, and never let
port work change the matching build or its acceptance status.

Hard rules:
- `assets/` is immutable original material; `external/` holds read-only reference repos
  (planetblupi, sb2-decomp, free-eggbert). Never modify, clean or commit either.
- Never execute the original BLUPI.EXE or programs from the ISO. Per the user's
  2026-10-19 runtime authorization, rebuilt game executables may be smoke-tested
  with cnc-ddraw, in an isolated build/ copy of the data. Bound each run, close its
  process, and keep runtime evidence separate from strict/equivalence proofs.
- Historical toolchains live under `C:\tools` (msvc5_rtm, msvc5_sp2, msvc5_sp3, msvc5_cc_*,
  msvc5_link_*, msvc6_rtm, dx5sdk, dx6sdk, dx61sdk, ghidra_*). Treat them as read-only. Run cl.exe with
  TMP/TEMP pointed at a directory inside this repo (e.g. `build/tmp`).
- No binary patching, no copied original bytes / `__asm _emit` in source, no artificial
  constructs whose only purpose is to force bytes. Masked/normalized equality is never a
  strict match; report it as what it is.
- Generated data, disassembly, objects and scratch work go under ignored `build/` or
  `scratch/`. Commit only small canonical inputs, focused scripts and concise docs.
- Separate proven facts from hypotheses in docs ([EVIDENCE] / [HYPOTHESIS] / [OPEN]).
- Do not run git commit; the supervisor integrates. Do not leave background processes running.
- Host safety (the PC froze twice from too many concurrent sandboxed processes): run shell commands one at a
  time and to completion - no parallel/background shell sessions, no fanning out several compile loops at once.
  All compiles go through tools/msvc.py, which enforces a machine-wide cap (BLUPI_COMPILE_SLOTS, default 6);
  never call cl.exe directly. Keep BLUPI_JOBS / permute --jobs at their defaults.
- Matching loop: candidate in scratch -> `tools/verify.py --tu src/X.cpp=<candidate>` (docs/WORKFLOW.md). When a
  function does not match, consult msvc5kb/README.md (`msvc5kb/tools/kb.py hints <func>`) before guessing; Blood II
  rules are hypotheses until confirmed on SB1. Record verified compiler findings and negatives in the catalog.
- sound.cpp compiles as its historical path `F:\Profil\sound.cpp` (temporary drive mapping, docs/BUILD_PATH.md).
  Inside the Codex sandbox the mapping is denied: "gate INCOMPLETE" for sound.cpp is expected there. Never add
  `#line` or other source changes for build-path strings, and never map drives yourself.
- Python: use `.pyenv\python.exe` (repo-local, ignored PyPy 3.11 with pefile + capstone 5.0.3).
  Prefer a few focused scripts over frameworks.
