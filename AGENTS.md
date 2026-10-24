Speedy Blupi 1 (1998) matching decompilation. Goal: C/C++ source that, compiled with the
historical toolchain, reproduces BLUPI.EXE machine code exactly. Matching comes first; no SDL,
no modernization, no renderer changes in matching source.

The native SDL3 port lives only in `port/` and has its own rules in `port/AGENTS.md`. It compiles `src/` unchanged; never edit `src/` for the port, and never let
port work change the matching build or its acceptance status.

Targets (`targets.json`, `tools/targets.py`, docs/MULTI_GAME.md): every build of the game is a matching
target `game/edition/language`. `sb1/blupi/us` (BLUPI.EXE) is the primary one; every acceptance number
refers to it, and tools default to it. Other targets are verified with `tools/verify.py --target` on the
same `src/`:
- A target's differences are build-wide defines, set by its entry and defaulted in `def.h`; never per-TU
  defines. The eGames edition (Speedy Eggbert 1, version 1.1) is the original's own `_EGAMES`; languages are
  `LANGUAGE_DE` (code) and `LANGUAGE_ACCENTS`, `LANGUAGE_ES`, `LANGUAGE_PT` (data tables). A switch surrounds
  only code or data that differs in that target's executable and says so in a comment. Branches inherited
  from the SB2 or Planet Blupi sources that no SB1 executable shows are removed, not kept behind a switch
  (docs/EGGBERT.md).
- Only `LANGUAGE_DE` and `VERSION_TEXT` (the title screen's version label, a `def.h` define derived from
  `_EGAMES`) may appear outside `#if`: the port defines them as run-time calls
  (`port/games/sb1/game/port_switches.h`). The other switches are used in `#if` only; the port leaves them
  at their defaults, except `LANGUAGE_ES` (set: it only narrows `¡`, which only Spanish text uses).
- Write a switch as arithmetic on the constant (`140 - 20 * LANGUAGE_DE`), so the port can bind it at run
  time. [EVIDENCE] An `if (LANGUAGE_DE)` block changed TreatEventBase's code in the US build; the arithmetic
  form did not. Keep a form only when the primary report stays byte-identical; otherwise use `#if`.
- Every change to `src/` for a secondary target keeps the primary report byte-identical, and the relinked
  image hash unchanged.
- `tools/cmpfunc.py` and `tools/equivalence.py` are pinned by the certificates: targets with their own
  inventory (`oracle/<target>/`, from `tools/derive_target.py`) go through `tools/cmpfunc_target.py`.
- SB2 (`sb2/eggbert/en`) is a separate game: its own tree `games/sb2/src/`, manifest `games/sb2/recon.json`
  and inventory `oracle/sb2-eggbert-en/` (docs/SB2.md). Speedy Blupi 2 (`sb2/blupi/en|de|fr`) uses
  `_EGAMES=0` and the whole-file `games/sb2/blupi/` overlay declared by `source_overlays` in targets.json.
  Logical TU names stay in `games/sb2/src/`; tools/source_tree.py stages the effective tree so quoted
  includes use edition headers in every TU. Add only files with evidenced edition requirements; the
  port continues to compile `games/sb2/src/` unchanged. Never edit `src/` for SB2; SB2 source changes only from SB2 binary evidence. Run
  `tools/crossmap.py --target sb2/eggbert/en --audit` before `--accept`.
- Historical targets are matched independently (user policy, 2026-10-24). The matching reconstruction exists to
  reproduce each original executable, not to compile every edition from one source: Speedy Blupi 1, Speedy
  Eggbert 1, Speedy Blupi 2 and Speedy Eggbert 2 (and their language and demo builds) are optimized separately.
  Share a matching source file only while the evidence shows the originals were the same; where editions need
  different implementations, declarations, headers or translation-unit context, give them edition-specific
  source rather than many conditional macros or compiler steering. No edition may be made to lose STRICT
  coverage so another can gain; an EQUIVALENT fallback for one edition is a temporary research state.
  Report progress per executable, never as a combined metric. Edition-specific source still needs historical
  evidence: no dummy declarations, synthetic functions or meaningless changes that steer symbol numbering.
  The SDL3 port (`port/`) keeps its own shared architecture and is not bound to the matching source layout.

Hard rules:
- `assets/` is immutable original material; `external/` holds read-only reference repos
  (planetblupi, sb2-decomp, free-eggbert). Never modify, clean or commit either. Every original game binary or
  download belongs in `assets/` (downloaded editions in `assets/downloads/`, e.g. the 1998 demo), never in
  `C:\tools`, which holds only toolchains and tools.
- Never execute any original executable: BLUPI.EXE, EGAMES.EXE, Eggbert2.exe and every language build,
  installers and launchers (UNWISE.EXE, gbrowser.exe, setup), or programs from the ISOs. Per the user's
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
  `#line` or other source changes for build-path strings, and never map drives yourself. The SB2 builds record
  `D:\Blupi\Profil\` and `D:\Blupi\Profil egames 2\`; the user allowed creating those folders on D: for one
  compile at a time (2026-10-22), through tools/buildpath.py only, removing what it created.
- Python: use `.pyenv\python.exe` (repo-local, ignored PyPy 3.11 with pefile + capstone 5.0.3).
  Prefer a few focused scripts over frameworks.
