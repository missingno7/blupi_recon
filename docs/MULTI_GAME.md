# Multi-game layout plan: every Speedy Blupi and Speedy Eggbert build in one repository

Status: **proposal for review** (2026-10-22, branch `multi-game`). No file has moved yet.

## Goal

One repository reconstructs and ports every known build of both games, in every language:

| Game | Edition | Build | Languages on the media |
|---|---|---|---|
| `sb1` Speedy Blupi | `blupi` | 1998-10-14, Epsitec, LINK 5.10 | US (= ENGLISH), DEUTSCH, FRANCAIS |
| `sb1` | `eggbert` | 1999-06-24, eGames, "LINK 5.00" | English, Deu, Esp, Fra, Ita, Por |
| `sb2` Speedy Blupi 2 | `blupi` | 2001-05-03, Epsitec, "LINK 5.00" | english, deutsch, francais |
| `sb2` | `eggbert` | 2004-05-03, eGames, LINK 5.10 | English |

- A **game** is one body of C++ source.
- An **edition** is one product built from it (Blupi or Eggbert).
- A **language** is one executable of an edition.
- A **target** is `game/edition/language`, the unit that has an oracle, a manifest section and a
  verifier report. Today's only target is `sb1/blupi/us`.

The SB1 matching results stay exactly as they are at every step. The port should let a player
point at their game folder (an installation, or a CD) and play any of the four in the language
they have.

## Evidence

Static reads of the executables (no execution), 2026-10-22. Measures:
- whole functions of SB1's inventory searched byte for byte, with image addresses and call
  targets masked ("byte-exact");
- the same with operands ignored ("same instruction sequence");
- shared 12-instruction windows over all of `.text`.

### Editions

- [EVIDENCE] **The editions of one game share their C source.** SB1/blupi against SB1/eggbert:
  358 of 361 functions byte-exact (94% of the bytes; 96-97% of windows shared). The compiler
  output is the same; the three differences are real source edits:
  - `ReadConfig` (`blupi.cpp`, 0x1030): Eggbert adds a `GetCurrentDirectory` step;
  - the `event.cpp` function at 0x19290: SB1 builds a path and calls a helper, Eggbert returns 1;
  - `CEvent::ChangePhase` (0x19310): one constant (0x634 vs 0x614), one extra block, and other
    register choices in one block. This is the function SB1 has not matched yet, so a second
    compiled version of it is new evidence for that residue.
- [EVIDENCE] SB2/blupi against SB2/eggbert: 94% of windows shared. Not yet measured per function.
- [EVIDENCE] The 1.0.0 port runs SB1/eggbert's data unchanged (headless attract demo, English
  "Demo" caption).

### Languages

- [EVIDENCE] **A language is an executable.** In every game the language builds have the same
  code size; they differ in:
  - **the string table (`.rsrc`)**, in every language. The resource language ID is neutral (0)
    everywhere, so a language is identified by its folder, not by the resource.
  - **one layout difference in German**: the help screen (`WM_PHASE_HELP`, in the 7,419-byte
    `event.cpp` function at 0x16f70) stacks two texts 13 px apart, where the US build puts them
    side by side. The same change is in SB1/blupi's and SB1/eggbert's German builds.
  - **about 15 bytes of one data table** in SB1/eggbert's Deu, Esp, Ita and Por builds.
    [OPEN] which table.
  - SB2/blupi's German and French builds differ only in `.rsrc`.
- [EVIDENCE] **Data and sounds are shared by all languages.** The `SOUND` folder of every
  language folder is identical to the main one (SB1: 76 files; SB2: 103 files); DATA and IMAGE
  folders exist once. SB1/eggbert's `SOUND` is also identical to SB1/blupi's.
- [EVIDENCE] SB1/blupi's US and ENGLISH executables and the installed one are identical; so are
  SB2/blupi's main and english ones.

### Games

- [EVIDENCE] **The games share their engine, not their game logic.** SB1/blupi against
  SB2/eggbert: 74% of SB1's windows recur; byte-exact 16%, same instruction sequence 28%.
  - Engine and UI TUs, by instruction sequence: `ddutil`, `movie`, `network`, `text`, `jauge`,
    CRT 100%; `misc` 86%; `decnet` 88%; `button` 79%; `pixmap` 76%; `menu` 73%; `sound` 71%.
  - Game logic: `event` 24%, `decmove` 25%, `decblock` 16%, `decor` 14%, `decblupi` 8%,
    `decdesign` 5%. The sequel rewrote it.
  - `pixmap` is 76% by sequence but 0% byte-exact: same code, shifted member offsets.

### Toolchain and build paths

- [EVIDENCE] **Two linker families**, crossing games and editions:
  - SB1/blupi (1998) and SB2/eggbert (2004): LINK 5.10, Rich header with the same entries (270
    and 271 objects, one `id6.b1668`).
  - SB1/eggbert (1999) and SB2/blupi (2001): "LINK 5.00", no Rich header, a separate `.idata`
    section.
- [HYPOTHESIS] The "LINK 5.00" family is MSVC 5.0 RTM's linker (5.00.7022, which predates the
  Rich header). To be confirmed by relinking.
- [EVIDENCE] **Each build records its source folder** (the `__FILE__` of `TRY_DS` in
  `sound.cpp`, the only `__FILE__`/`__LINE__` use in the source):
  - SB1/blupi and SB1/eggbert: `F:\Profil\sound.cpp`;
  - SB2/blupi: `D:\Blupi\Profil\sound.cpp`;
  - SB2/eggbert: `D:\Blupi\Profil egames 2\sound.cpp`.
- [HYPOTHESIS] Epsitec kept a separate source folder per eGames edition (`Profil egames 2`) and
  edited it. The plan still models editions and languages as switches in one source: the C
  differs in a few functions, and a copy per build would duplicate 99% of the code.

## Recommended order

**SB1 first, in every edition and language; SB2 prepared, not started.**

1. **Port: multilingual SB1 and Eggbert 1** (release 1.1). The strings come from the player's
   own executable, so the language they installed just works, and a CD offers all of its
   languages. Nothing new is embedded and `src/` does not change. This is quick and useful to
   players.
2. **Matching: SB1's other targets.**
   - Eggbert 1 English: 358 of 361 functions already identical; three edition switches.
   - The German and French builds: a per-language `.rc`, and the help-screen switch.
   - The Eggbert languages: `.rc`, plus the data table.
   - These are cheap targets on today's source, and they add evidence for `ChangePhase`.
3. **SB2**, once 1 and 2 are done:
   - move SB1 into `games/sb1/`;
   - seed `games/sb2/` from the SB1 engine TUs and `external/sb2-decomp`;
   - Eggbert 2 first, since it has SB1/blupi's toolchain;
   - the port for SB2 last.

SB2 is a second large reconstruction: its game logic is mostly new, and the port can only run
it once its source exists. Preparing for it now means the target registry, the tools' `--target`
option and the port's split into common and per-game code. The steps above need all three anyway.

## Target layout (end state)

```
games/
  sb1/
    src/              the SB1 source (today's src/), edition/language switches where the C differs
    src/res/          resources: blupi.rc per language (us, de, fr; eggbert: en, de, es, fr, it, pt)
    recon.json        game manifest: TUs, toolchain, one section per edition and language
    oracle/<edition>-<language>/  functions.csv, tu_map.csv, ... (today's oracle/ is blupi-us)
    evidence/  steering/  docs/
  sb2/
    src/ ... same shape
common/
  src/                TUs and headers proven identical in both games (empty at first)
tools/                shared, taking --target sb1/blupi/us (the default) etc.
msvc5kb/              shared; catalog entries name the targets they were proven on
docs/                 method, toolchain, workflow, status across targets, this plan
tests/
port/
  common/             compat/, render/, platform/, settings, options screen, touch, release/,
                      game-folder and language detection, string tables from the executable
  games/sb1/          game/ replacements (main, pixmap, sound, misc, movie, network),
                      scene builder, end of tick, editor, probe, state hash, golden traces
  games/sb2/          the same for SB2, when it starts
  android/            one Gradle project
targets.json          target registry: game, edition, language, asset path, SHA-256, linker,
                      build path
```

In step 1 only `port/` is split. `src/` stays where it is until step 3.

## Source model (matching)

1. **Per-game trees first.** Every TU starts in its game's `src/`. `#include` resolves within
   the game's tree, then `common/src`. There are no cross-game include paths.
2. **Editions and languages are build-wide switches.** A target compiles its game's source with
   the defines its manifest section names (`EDITION_EGGBERT`, `LANGUAGE_DE`, ...), the same for
   every TU. A switch may only surround code or data that differs between the targets' binaries,
   and it cites the oracle addresses it explains.
   - This is not a per-TU header view (`msvc5kb` BLUPI-CTX rule): every TU of a target sees the
     same headers and defines.
   - A switch must not move code that follows it in a TU where `__LINE__` or `__FILE__` is
     observable. Only `sound.cpp` is such a TU, and it is identical in every SB1 build.
   - Strings live in one `.rc` per language; tables use switched entries only where the bytes
     differ.
3. **`common/` holds only proven sharing.** A file moves to `common/src` when its text is
   identical in both games' trees and every target that compiles it is STRICT for it. A tool
   (`tools/commoncheck.py`) lists candidates and refuses a move that would change any target's
   report. Headers stay per game until the same holds for them, since class layouts differ
   (`pixmap` above).
4. **Build paths per target.** SB1 keeps `F:\Profil\` (the existing drive mapping). SB2 uses
   `D:\Blupi\Profil\` and `D:\Blupi\Profil egames 2\`. D: is a real drive here, so
   `tools/buildpath.py` gains a second mode, which you approved on 2026-10-22:
   - it creates the folder on the existing drive for one compile;
   - it stages the source there, compiles, and removes only what it created;
   - it refuses when the folder already exists with anything it did not create.
5. **Provenance** stays per game (`games/sb2/src/PROVENANCE.md` for code from
   `external/sb2-decomp`, GPLv3).

## Manifest and tools

- `targets.json` (committed) names each target's executable by path and SHA-256, its linker
  family and its build path. Asset paths may point outside the repository (SB2's assets are in
  `D:\Prog\eggbert2_recon\assets`). `assets/` stays read-only for every tool; the registry
  follows whatever layout it has. The language executables are reference copies kept apart from
  the installed games (`assets/blupi_variants`, `assets/eggbert_variants`), since an
  installation holds only one.
- `recon.json` keeps today's format for `sb1/blupi/us`, plus a `targets` object for what differs
  per target: defines, oracle folder, resources, accepted functions, symbol mappings, forced
  locations, exclusions, equivalent functions, link order. A function accepted for one target is
  not assumed for another; each is proven against its own oracle.
- Tools take `--target`, defaulting to `sb1/blupi/us`, so every documented command keeps working.
  The hard-coded paths go through one module, `tools/targets.py`:
  - the asset `assets/Speedy Blupi/BLUPI.EXE` in `cmpfunc.py`, `oracle.py`, `relink.py`,
    `equivalence.py` and `fnprobe.py`;
  - `recon.json` and `src/` in `verify.py`, `permute.py` and `relink.py`.
- The verifier reports per target. `--target all` prints one line each; the README's acceptance
  numbers become a table.
- The LINK 5.00 family needs its exact linker found: relink SB1/eggbert's objects with the RTM
  linker and compare the layout.

## Port

### Game folder and language (step 1)

How the originals are installed (from the CD images, 2026-10-22):
- **SB1 and SB2 CDs:** `DATA`, `IMAGE08` and `IMAGE16` at the root, and one folder per language
  (`US`, `ENGLISH`, `DEUTSCH`, `FRANCAIS`; SB2 in lower case) holding that language's executable
  and a `SOUND` folder. The installer asks for a language and copies only that executable.
- **Eggbert 1 CD:** an installer, plus `Deu`, `Esp`, `Fra`, `Ita` and `Por` folders each holding a
  language `EGAMES.EXE`. The game data is only inside the installer.
- **Eggbert 2 CD:** an installer only.

So an installed game has **one executable in one language**. The port therefore:
- **Follows the installed language.** It reads the strings from the executable in the game folder
  (a small PE resource reader in `port/common/platform`), in place of the embedded US table.
  Whatever language the player installed works, with nothing new embedded in the port.
- **Recognises the build** (game, edition, language) from that executable, using a table of known
  builds by size and `.text` hash. An unknown executable still works with its strings, as
  "unknown build".
- **Offers a choice only when there is one.** Pointed at a CD (or a folder with several language
  executables in subfolders), it lists them. The chosen language folder becomes an overlay over
  the data root, which is where the CD keeps `SOUND`. The choice is a Language entry on the
  Display options screen, kept in `blupi-sdl3.json`, and is shown only when there is more than
  one executable. Changing it restarts the game's screens, since the original loads strings when
  a screen opens.
- **Android.** The import copies the executable along with the data. Only its string table is
  read, never its code.
- **Fallback.** Without any executable (data copied without it), the embedded SB1 US table is
  used, as today.
- **Code that differs by language or edition** (German help layout, Eggbert's three functions
  and its data table) cannot be chosen at run time while the port compiles `src/` as it is. Two
  options, for review:
  - **(a)** the port builds the `blupi`/US variant of that code for every language: the German
    help screen keeps the US layout (to be checked on screen), and Eggbert's CD/path differences
    are already replaced by the port's own `main.cpp`;
  - **(b)** the matching source writes those switches as `if (LANGUAGE_DE)` on a defined
    constant, but only if the verifier proves the bytes identical to `#if`. The port then
    defines the constant as its run-time language. This is a source form chosen with the port in
    mind, which the rules ("never edit `src/` for the port") would have to allow explicitly.
  - Recommended: (a) now, (b) only if a layout difference turns out to matter.

### Several games (step 3)

- `port/common/` is today's `compat/`, `render/`, `platform/`, settings, the Display options
  screen, touch controls, `member_access.h`, `release/`, and the new folder and language
  detection. `port/games/sb1/` is the rest of today's `port/game/`, with its golden traces.
- Each game is its own program (`blupi-sdl3`, `blupi2-sdl3`). Their classes share names with
  different layouts, so they cannot be linked into one binary. A small launcher (desktop) or a
  native-library choice (Android) starts the right one for the chosen folder, so the player
  still only points at a game folder.
- The engine replacements derived from the engine TUs (`pixmap`, `sound`, `movie`, `network`,
  `misc`) are expected to serve SB2 with small changes. They move to `port/common` only when SB2
  builds with them.
- Golden traces per game and edition; SB1/eggbert's attract mode becomes a new gate in step 1.

## Docs during the move

- Live docs (README, AGENTS, STATUS, WORKFLOW, TOOLCHAIN, BUILD_PATH, port docs) are updated to
  the new paths in the step that moves the files.
- Research records (the per-TU, sweep and study docs: about 50 files and 233 lines that name
  `src/`) are not rewritten. They get one note at the top of `docs/` and in each game's `docs/`:
  paths before the move refer to the old layout (`src/` = `games/sb1/src/`).
- `AGENTS.md` gains the multi-target rules:
  - targets;
  - the switch rule;
  - the common-code rule;
  - per-target gates;
  - "never execute any original executable", extended to every edition and language, including
    installers and launchers such as `UNWISE.EXE` and `gbrowser.exe`.

## Migration gates

| Step | Work | Gate |
|---|---|---|
| 1 | Port: split `port/` into `common` and `games/sb1`; strings from the game's executable; build recognition; CD layout with a language choice; Android import of the executable | Port `ctest` on Windows and Linux; SB1/eggbert golden attract trace; strings of every available language read; matching untouched |
| 2 | `targets.json`, `tools/targets.py`, `--target`; oracles for SB1's other targets; edition and language switches, per-language `.rc`; LINK 5.00 found | `sb1/blupi/us` verifier report identical (411 STRICT, 6 FOLDED, 127,456 / 141,488 B; same per-TU counts). New targets' reports recorded |
| 3a | `git mv` SB1 into `games/sb1/`; manifest paths rewritten by a script; port paths | Every SB1 target's verifier, relink and equivalence results identical; port gates |
| 3b | SB2 bring-up: registry, Ghidra inventory, `games/sb2/src`, `buildpath.py` D: mode; Eggbert 2 first | SB2 report; SB1 gates unchanged |
| 3c | `common/` extraction by `commoncheck.py` | Every target's report identical before and after |
| 3d | Port for SB2, launcher | SB2 golden traces; SB1 port gates unchanged |

## Decisions

Taken (2026-10-22):
- SB1 is moved into `games/sb1/` (in step 3a, behind the identical-report gate).
- Temporary build-path folders on D: are allowed for SB2 (`D:\Blupi\Profil\`,
  `D:\Blupi\Profil egames 2\`, removed after each compile).
- SB1 and Eggbert 1, all languages, come first; SB2 is prepared, not started.

For review:
1. The port's handling of language- and edition-specific code: option (a) or (b) above.
2. Where the SB2 assets live: copied into `blupi_recon/assets/`, or registered in
   `D:\Prog\eggbert2_recon\assets`.
3. Release 1.1 for multilingual SB1 and Eggbert 1 after step 1, before the matching work of
   step 2?

## Risks

- Moving SB1 can change what the gates prove if any tool still resolves an old path, so the
  step 3a gate compares the full verifier output, not only the totals.
- A switch can hide a real difference behind a guess. Each must cite the oracle bytes it
  explains, and `sb1/blupi/us` must stay identical.
- Strings read from a player's executable are data from an unknown file: the PE reader must
  bound every offset and size, and treat a malformed file as "no strings".
- SB2 is a second large reconstruction. Its integration rounds stay separate from SB1's, so
  SB1's acceptance never depends on SB2 work in progress.
