# Multi-game layout plan: every Speedy Blupi and Speedy Eggbert build in one repository

Status: **proposal for review** (2026-10-22, branch `multi-game`). No file has moved yet.

## Goal

One repository reconstructs and ports every known build of both games:

| Game | Edition | Build | Executable (asset) |
|---|---|---|---|
| `sb1` Speedy Blupi | `blupi` | 1998-10-14, Epsitec | `assets/Speedy Blupi/BLUPI.EXE` (current target) |
| `sb1` | `eggbert` | 1999-06-24, eGames (English) | `assets/eggbert/Game/egames.exe` |
| `sb2` Speedy Blupi 2 | `blupi` | 2001-05-03, Epsitec | `D:\Prog\eggbert2_recon\assets\blupi2\BLUPI.EXE` |
| `sb2` | `eggbert` | 2004-05-03, eGames (English) | `D:\Prog\eggbert2_recon\assets\eggbert2\Eggbert2.exe` |

A **game** is one body of C++ source; an **edition** is one shipped build of it; a **target** is
`game/edition`, the unit that has an oracle, a manifest and a verifier report. The SB1 matching
results stay exactly as they are at every step.

## Evidence the plan rests on

Static reads of the four executables (no execution), 2026-10-22. Measures: whole functions of
SB1's inventory searched byte-for-byte with image addresses and call targets masked
("byte-exact"), the same with operands ignored ("same instruction sequence"), and shared
12-instruction windows over all of `.text`.

- [EVIDENCE] **The editions of one game share their C source.** SB1/blupi against SB1/eggbert:
  358 of 361 functions byte-exact (94% of the bytes; 96-97% of windows shared). The compiler
  output is the same; the three differences are real source edits:
  - `ReadConfig` (`blupi.cpp`, 0x1030): Eggbert adds a `GetCurrentDirectory` step;
  - the `event.cpp` function at 0x19290: SB1 builds a path and calls a helper, Eggbert returns 1;
  - `CEvent::ChangePhase` (0x19310): one constant (0x634 vs 0x614), one extra block, and other
    register choices in one block. This is the function SB1 has not matched yet, so a second
    compiled version of it is new evidence for that residue.
- [EVIDENCE] SB2/blupi against SB2/eggbert: 94% of windows shared. Not yet measured per function.
- [EVIDENCE] **The games share their engine, not their game logic.** SB1/blupi against
  SB2/eggbert: 74% of SB1's windows recur; byte-exact 16%, same instruction sequence 28%.
  - Engine and UI TUs, by instruction sequence: `ddutil`, `movie`, `network`, `text`, `jauge`,
    CRT 100%; `misc` 86%; `decnet` 88%; `button` 79%; `pixmap` 76%; `menu` 73%; `sound` 71%.
  - Game logic: `event` 24%, `decmove` 25%, `decblock` 16%, `decor` 14%, `decblupi` 8%,
    `decdesign` 5%. The sequel rewrote it.
  - `pixmap` is 76% by sequence but 0% byte-exact: same code, shifted member offsets.
- [EVIDENCE] **Two linker families**, crossing games and editions:
  - SB1/blupi (1998) and SB2/eggbert (2004): LINK 5.10, Rich header with the same entries (270
    and 271 objects, one `id6.b1668`).
  - SB1/eggbert (1999) and SB2/blupi (2001): "LINK 5.00", no Rich header, a separate `.idata`
    section.
- [EVIDENCE] **Each build records its source folder** (the `__FILE__` of `TRY_DS` in
  `sound.cpp`, the only `__FILE__`/`__LINE__` use in the source):
  - SB1/blupi and SB1/eggbert: `F:\Profil\sound.cpp`;
  - SB2/blupi: `D:\Blupi\Profil\sound.cpp`;
  - SB2/eggbert: `D:\Blupi\Profil egames 2\sound.cpp`.
- [HYPOTHESIS] The "LINK 5.00" family is MSVC 5.0 RTM's linker (5.00.7022, which predates the
  Rich header). To be confirmed by relinking.
- [HYPOTHESIS] Epsitec kept a separate source folder per eGames edition (`Profil egames 2`) and
  edited it. The plan still models editions as switches in one source: the C differs in a few
  functions, and one copy per edition would duplicate 99% of the code.
- [EVIDENCE] The 1.0.0 port runs SB1/eggbert's data unchanged (headless attract demo, English
  "Demo" caption). Its `.rsrc` has the same resource types and counts as SB1/blupi; `.data` is
  0x144 bytes smaller.

## Target layout

```
games/
  sb1/
    src/              the SB1 source (today's src/), edition switches where the C differs
    src/res/          resources; per-edition .rc where the strings differ
    recon.json        game manifest: TUs, toolchain, and one section per edition
    oracle/blupi/     functions.csv, tu_map.csv, inventory.csv, vtables.csv (today's oracle/)
    oracle/eggbert/
    evidence/         equivalence certificates (today's evidence/)
    steering/         (today's steering/)
    docs/             SB1-specific findings (today's per-TU docs)
  sb2/
    src/ ... same shape
common/
  src/                TUs and headers proven identical in both games (empty at first)
tools/                shared, taking --target sb1/blupi (the default) etc.
msvc5kb/              shared; catalog entries name the targets they were proven on
docs/                 method, toolchain, workflow, status across targets, this plan
tests/                shared tool tests
port/
  common/             compat/, render/, platform/, settings, options screen, touch, release/
  games/sb1/          game/ replacements (main, pixmap, sound, misc, movie, network),
                      scene builder, end of tick, editor, probe, state hash, golden traces
  games/sb2/          the same for SB2, when it starts
  android/            one Gradle project; a flavour per game
targets.json          target registry: game, edition, asset path, SHA-256, linker, build path
```

## Source model

1. **Per-game trees first.** Every TU starts in its game's `src/`. `#include` resolves within
   the game's tree, then `common/src`. No cross-game include paths.
2. **Editions are build-wide switches.** An edition compiles the game's source with one define
   (`EDITION_EGGBERT`), set for the whole build in the edition's manifest section. It may only
   surround code that differs between the editions' binaries. Each switch cites the oracle
   addresses it explains.
   - This is not a per-TU header view (`msvc5kb` BLUPI-CTX rule): every TU of an edition sees
     the same headers and defines.
   - A switch must not move code that follows it in a TU where `__LINE__` or `__FILE__` is
     observable. Only `sound.cpp` is such a TU today, and it is identical in both SB1 editions.
   - Data and resources follow the same rule: per-edition `.rc` or switched table entries only
     where the bytes differ.
3. **`common/` holds only proven sharing.** A file moves to `common/src` when its text is
   identical in both games' trees and every target that compiles it is STRICT for it. A tool
   (`tools/commoncheck.py`) lists candidates and refuses a move that would change any target's
   report. Headers stay per game until the same holds for them, since class layouts differ
   (`pixmap` above).
4. **Build paths per target.** The manifest's `build_path` becomes per edition. SB1 keeps
   `F:\Profil\`. SB2 needs `D:\Blupi\Profil\` and `D:\Blupi\Profil egames 2\`. D: is a real drive
   here, and `tools/buildpath.py` refuses to map an existing drive, so SB2 needs a decision
   (below).
5. **Provenance** stays per game (`games/sb2/src/PROVENANCE.md` for code taken from
   `external/sb2-decomp`, GPLv3).

## Manifest and tools

- `targets.json` (committed) names each target's asset by path and SHA-256, its linker family
  and its build path. Asset paths may point outside the repository, since SB2's assets live in
  `D:\Prog\eggbert2_recon\assets` today. `assets/` itself stays immutable: nothing is moved or
  renamed inside it.
- `games/<game>/recon.json` keeps today's format, plus an `editions` object for what differs:
  defines, oracle folder, accepted functions, symbol mappings, forced locations, exclusions,
  equivalent functions, link order. Functions accepted in one edition are not assumed in another;
  each is proven against its own oracle.
- Tools take `--target game/edition`, default `sb1/blupi`, so every documented command keeps
  working unchanged. Hard-coded paths (`assets/Speedy Blupi/BLUPI.EXE` in `cmpfunc.py`,
  `oracle.py`, `relink.py`, `equivalence.py`, `fnprobe.py`; `recon.json` and `src/` in `verify.py`,
  `permute.py`, `relink.py`) go through one module (`tools/targets.py`).
- The verifier reports per target. `--target all` prints one line per target; the acceptance
  numbers in the README become a small table.
- The linker per target: `relink.py` already knows several LINK builds. The 5.00 family needs
  its exact linker found (relink SB1/eggbert's objects with the RTM linker and compare layout).

## Port

- `port/common/` is today's `compat/`, `render/`, `platform/`, settings, Display options screen,
  touch controls, `member_access.h` and `release/`. They are game-independent now.
- `port/games/sb1/` is today's `port/game/` minus those files, with `tests/golden/`. It compiles
  `games/sb1/src` exactly as the port compiles `src/` today.
- One executable per game (`blupi-sdl3`, later `blupi2-sdl3`). Editions are data: the SB1 build
  already plays Eggbert 1's data. Whether the port also needs SB1/eggbert's three edition
  functions (one is in `ChangePhase`, which the port compiles from the shared source) is
  decided after those differences are understood; until then the port builds the `blupi`
  edition.
- Golden traces per game and edition (SB1/eggbert's attract mode is a new gate).
- Android: one Gradle project with a product flavour per game (separate application IDs), the
  same player-supplied data import.
- The port's platform replacements derived from the engine TUs (`pixmap`, `sound`, `movie`,
  `network`, `misc`) are expected to serve SB2 with small changes. They move to `port/common`
  only when SB2 builds with them.

## Migration, each phase with its gate

| Phase | Work | Gate |
|---|---|---|
| 0 | This plan, reviewed | - |
| 1 | `targets.json`, `tools/targets.py`, `--target`; SB1/eggbert oracle inventory; edition switches for its three functions; its linker found | SB1/blupi verifier report identical (411 STRICT, 6 FOLDED, 127,456 / 141,488 B; same per-TU counts). SB1/eggbert report recorded |
| 2 | `git mv` SB1 into `games/sb1/` and port into `port/common` + `port/games/sb1`; manifest paths rewritten by a script; port CMake and Gradle paths | SB1 verifier, relink and equivalence gates identical; port `ctest` 86/86 on Windows and Linux; Android release build |
| 3 | SB2 bring-up: assets registered, Ghidra inventory, `games/sb2/src` seeded from `sb2-decomp` and the SB1 engine TUs; SB2/eggbert first (same toolchain as SB1/blupi) | SB2/eggbert report; SB1 gates unchanged |
| 4 | `common/` extraction by `commoncheck.py` | Every target's report identical before and after |
| 5 | Port for SB2 (`port/games/sb2`, golden traces, Android flavour) | SB2 golden traces; SB1 port gates unchanged |

Phase 1 needs no file to move and pays off at once: SB1/eggbert should verify almost for free,
and its `ChangePhase` adds evidence for SB1's open residue.

## Docs during the move

- Live docs (README, AGENTS, STATUS, WORKFLOW, TOOLCHAIN, BUILD_PATH, port docs) are updated
  to the new paths in the phase that moves the files.
- Research records (the per-TU, sweep and study docs, about 50 files and 233 lines that name
  `src/`) are not rewritten. They get one note at the top of `docs/` and in each game's `docs/`:
  paths before 2026-10 refer to the pre-move layout (`src/` = `games/sb1/src/`).
- `AGENTS.md` gains the multi-target rules: the target notion, the edition-switch rule, the
  common-code rule, per-target gates, and "never execute any original executable" extended to
  every edition (including installers and launchers such as `UNWISE.EXE` and `gbrowser.exe`).

## Decisions for review

1. **Move SB1 into `games/sb1/`** (symmetric layout; phase 2), or keep it at the root and add
   only `games/sb2/` (no churn, asymmetric)? Recommended: move, after phase 1, with the
   identical-report gate.
2. **SB2 build paths on drive D:.** Options: create `D:\Blupi\Profil\` (and
   `D:\Blupi\Profil egames 2\`) only for the duration of one compile, the way `buildpath.py`
   stages F:; or accept `sound.cpp`'s `TRY_DS` function as relocation-unverified for SB2. The
   first writes outside the repository on your D: drive, so it needs your consent.
3. **SB2 assets.** Copy them into `blupi_recon/assets/` (immutable from then on), or register
   them where they are in `D:\Prog\eggbert2_recon\assets`?
4. **Edition switches in source** (recommended) or one source copy per edition, which matches
   Epsitec's probable practice but duplicates almost all code.
5. **Port editions.** One SB1 executable for both editions' data (today), or an Eggbert build
   with its three edition functions?

## Risks

- Moving SB1 can change what the gates prove if any tool still resolves an old path, so the
  phase 2 gate compares full verifier output, not only the totals.
- An edition switch can hide a real difference behind a guess. Each switch must cite the oracle
  bytes it explains, and SB1/blupi must stay identical.
- SB2 adds a second large reconstruction. Keep its integration rounds separate from SB1's, so
  SB1's acceptance never depends on SB2 work in progress.
