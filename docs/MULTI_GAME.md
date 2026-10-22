# Multi-game layout plan: every Speedy Blupi and Speedy Eggbert build in one repository

Status (2026-10-22, branch `multi-game`): **steps 1 and 2 done** (open items below); step 3 (SB2)
not started.

## Progress

Step 1, port:
- [EVIDENCE] Language packs (`port/docs/LANGUAGES.md`): nine built-in packs generated from the
  executables. The US pack equals `src/res/blupi.rc` (`ctest -R language`).
- [EVIDENCE] Run-time switching works on the Display options screen; the German help layout follows
  the language (screenshots of the German and US help screens).
- [EVIDENCE] Recognition: the edition from the title image, the installed language from the
  installed executable. CDs work (a CD extracted from the SB1 ISO: SOUND from `US`).
- [EVIDENCE] Gates: `ctest` 94/94, including the German tour and SB1/eggbert's attract mode as
  golden traces.

Step 1, matching:
- [EVIDENCE] `targets.json` registers every build; `verify.py --target`. Without `--target` the report
  is byte-identical to before, and so is the relinked image (SHA-256 `557ca92e…`).
- [EVIDENCE] `sb1/blupi/fr` verifies exactly like US: 127,456 / 141,488 bytes, with no source change.
- [EVIDENCE] `sb1/blupi/de`: 127,456 / 141,488 bytes once `DrawButtons` has the German help layout
  (`LANGUAGE_DE`, arithmetic form).
- [EVIDENCE] Speedy Eggbert 1 is version 1.1 of the game, and all six of its targets (en, de, es, fr,
  it, pt) verify at 123,120 / 141,328 strict bytes. It was linked with VC5 RTM's LINK 5.00.7022;
  it has no gameplay changes. Differences, matching details and what they mean for the port:
  [docs/EGGBERT.md](EGGBERT.md).
- [OPEN] For Eggbert: `WindowProc`'s three padding bytes (and `DoInit`, which depends on it),
  `ChangePhase` (the primary target's residue), and a whole-image relink.

Step 2:
- [EVIDENCE] `port/` is split into `common/` (compat, platform, render) and `games/sb1/game/`; the
  includes are unchanged and `ctest` passes 94/94.

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
- [EVIDENCE] **The editions' data differ in a few files** (SB1/blupi against SB1/eggbert
  installations):
  - `IMAGE08` and `IMAGE16` `INIT.BLP` (the title image), `IMAGE08/TEXT.BLP` (the font) and
    `IMAGE08/BUTTON00.BLP`, and `DATA/Config.def`, differ;
  - every level, demo and sound is identical.

  So the port can tell the editions apart from the data alone, and a language pack depends on
  its edition's title image and font.
- [EVIDENCE] The port reads nothing from the original executable today. It compiles `src/`, embeds
  the reconstructed `src/res/blupi.rc`, and reads only `DATA`, `IMAGE08`, `IMAGE16` and `SOUND`.
  The few values the original read past the ends of its tables are constants in the port's source.

### Languages

- [EVIDENCE] **A language is an executable.** In every game the language builds have the same
  code size; they differ in:
  - **the string table (`.rsrc`)**, in every language. The resource language ID is neutral (0)
    everywhere, so a language is identified by its folder, not by the resource.
  - **one layout difference in German**: the help screen (`WM_PHASE_HELP`, in the 7,419-byte
    `event.cpp` function at 0x16f70) stacks two texts 13 px apart, where the US build puts them
    side by side. The same change is in SB1/blupi's and SB1/eggbert's German builds.
  - **the character table `table_char`** (SB1 0x7b530: 6 bytes for each of 256 codes, a base
    glyph plus an optional accent glyph and offset) in SB1/eggbert's Deu, Esp, Ita and Por
    builds: about 15 bytes, which compose a few more accented letters than the English build.
  - SB2/blupi's German and French builds differ only in `.rsrc`.
- [EVIDENCE] **Data and sounds are shared by all languages.** The `SOUND` folder of every
  language folder is identical to the main one (SB1: 76 files; SB2: 103 files); DATA and IMAGE
  folders exist once. SB1/eggbert's `SOUND` is also identical to SB1/blupi's.
- [EVIDENCE] SB1/blupi's US and ENGLISH executables and the installed one are identical; so are
  SB2/blupi's main and english ones.
- [EVIDENCE] **The font** (SB1/blupi/us `table_char`):
  - plain glyphs for ASCII, lower-case `à â ä ç è é ê ë î ï ô ö ù û ü`, and `¡ ¿`;
  - `á í ñ ó ú` composed from a base letter and an accent glyph;
  - no upper-case accented letters and no `ß`.

  A new language can use every plain glyph and compose more letters from the accents the font
  has. Letters needing other accents (Czech háček and ring) need those accent glyphs added to the
  font image first.

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

1. **Language packs and SB1's other targets, together.** Both start from the same
   per-language `.rc` files and the same switches.
   - Matching:
     - Eggbert 1 English: 358 of 361 functions already identical; three edition switches.
     - The German and French builds: a per-language `.rc`, and the help-screen switch.
     - The Eggbert languages: `.rc`, plus the character-table entries.
     - These are cheap targets on today's source, and they add evidence for `ChangePhase`.
   - Port: the language-pack format, built-in packs from the `.rc` files, user packs, the
     Language option, build recognition and CD layouts.
2. **Port structure for several games:** split `port/` into `common` and `games/sb1`. Steps 1
   and 2 are then merged into main together and released as 1.1.
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
                      language packs, build recognition, game-folder and CD detection
  games/sb1/          game/ replacements (main, pixmap, sound, misc, movie, network),
                      scene builder, end of tick, editor, probe, state hash, golden traces
  games/sb2/          the same for SB2, when it starts
  android/            one Gradle project
targets.json          target registry: game, edition, language, asset path, SHA-256, linker,
                      build path
```

`port/` is split in step 2; `src/` stays where it is until step 3.

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
  family and its build path. `assets/` stays read-only for every tool; the registry follows its
  layout: one installed folder per game and edition (`Speedy Blupi`, `eggbert`, `blupi2`,
  `eggbert2`) and the reference language executables beside them (`blupi_variants`,
  `eggbert_variants`, `blupi2_variants`), since an installation holds only one.
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

So an installed game has one executable in one language. The port goes further: **every
language is switchable at run time**, and new ones can be added.

**Language packs.** A language is a UTF-8 text file the port reads, holding everything a
language changes:

```
# Speedy Blupi language pack
name     = Deutsch
code     = de
game     = sb1                    # whose string ids these are
edition  = blupi                  # optional: names, help texts of one edition
layout   = help-stacked           # language layout switches the port knows (here: one)

[chars]                           # Unicode letter -> game character code and its glyphs
ä = 0xe4                          # a plain glyph of the font
á = 0xe1 a + accent 0x02 @ 2,2    # composed: base letter, accent glyph, offset
č = 0x8d c + accent caron @ 0,0   # needs a caron glyph added to the font (custom graphics)

[strings]                         # string table id = text
<id> = <text in UTF-8>
...
```

- **Built-in packs** are generated at build time from the matching source's per-language `.rc`
  files (`tools/make_rc.py --target`) plus each target's known switches. SB1 gets US, German and
  French; Eggbert 1 gets English, German, Spanish, French, Italian and Portuguese. The port
  already embeds the US table, so this follows the same practice.
- **User packs** in the user folder (`languages/*.txt`) add or override languages. A fan
  translation is one text file. `port/tools/language_pack.py` turns any owned executable's
  string table into a pack to start from.
- **Characters.** The game draws single-byte codes. A pack maps each letter it uses to a code and
  says how to draw it: an existing glyph, or a base letter plus an accent glyph at an offset (the
  game's own `table_char` mechanism). The port applies a pack's character entries to
  `table_char` when the language is loaded; this is font data, not simulation state. Letters
  needing an accent the font lacks wait for custom graphics.
- **Which packs an edition offers.** Each edition offers its own languages (SB1/blupi: US,
  German, French; SB1/eggbert: English, German, Spanish, French, Italian, Portuguese) plus every
  custom pack, so texts always match the title image and font. The port recognises the edition
  from its data (`INIT.BLP`).
- **First start.** The port picks the installed language by recognising the installed executable
  from its size and hash, against a table of known builds. This is the only thing it reads from
  the executable, and the executable stays optional: without a known one, the edition's English
  pack is used.

### For players: changing and adding a language

- **Changing.** On the game's settings screen, Display options gets a **Language** entry with the
  same arrow buttons as the other options. The change applies at once (the game's screen reopens
  in the new language) and is kept in `blupi-sdl3.json`. `--language <code>` sets it for one run.
  Android uses the same screen.
- **Adding.**
  1. `blupi-sdl3 --export-language us cs.txt` writes a built-in pack as a template, so a
     translator needs no other tools.
  2. Translate it in any text editor (UTF-8): name and code at the top, then the text of each line.
  3. Put it in the `languages` folder next to the saves (`%APPDATA%\Epsitec\Speedy Blupi SDL3\
     languages\`, `~/.local/share/Epsitec/Speedy Blupi SDL3/languages/`). On Android, the import
     screen gets an "Add language file" button using the system file picker.
  4. It appears in the Language list the next time Display options opens. A file with a built-in
     language's code replaces that language.
- **Forgiving.** A missing line falls back to the edition's English text, so a half-finished
  translation is playable. Problems (unknown ids, malformed lines, letters the font cannot draw,
  texts wider than their place on screen) go to a log, not into the game.
- **Letters.** A letter the font cannot draw falls back to its base letter (`č` shows as `c`), or
  the pack composes it from accents the font has. Custom accent glyphs (háček, ring) come later,
  with the custom-graphics work.
- **CDs.** Pointed at an SB1 or SB2 CD, the port uses the root's data and the `SOUND` folder of a
  language folder (identical in all of them).
- **Layout switches** (German help screen). These are code in `event.cpp`, which the port
  compiles from `src/` as it is. To switch them at run time, the matching source writes each
  language or edition switch as arithmetic on a target constant (`140 - 20 * LANGUAGE_DE`)
  rather than `#if`. The matching build defines the constant per target; the port defines it as
  its run-time language.
  - [EVIDENCE] An `if (LANGUAGE_DE)` form gave the right bytes in both builds' `DrawButtons`, but
    changed `TreatEventBase`'s code in the US build: the dead branch still shifts the 1997
    compiler's choices later in the file. The arithmetic form left the US report byte-identical.
  - A form is allowed only where the verifier proves the primary report byte-identical;
    otherwise the switch stays `#if`, and the port shows the US variant there. `AGENTS.md`
    allows this explicitly, only for switches the matching needs anyway.
- **Edition switches** (Eggbert 1's three functions) follow the same rule. The CD and path checks
  among them are already replaced by the port's own `main.cpp`.
- **Matching** keeps `sb1/blupi/us` as the primary target, the one every acceptance number
  refers to. The other languages and editions are secondary targets on the same source.

### Several games (steps 2 and 3)

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
| 1 | `targets.json`, `tools/targets.py`, `--target`; oracles and per-language `.rc` for SB1's other targets; edition and language switches (run-time-bindable form where byte-neutral); LINK 5.00 found. Port: language packs (format, built-in, user), `table_char` entries, Language option, build recognition, CD layouts | `sb1/blupi/us` verifier report identical (411 STRICT, 6 FOLDED, 127,456 / 141,488 B; same per-TU counts); new targets' reports recorded; port `ctest` on Windows and Linux with golden traces unchanged in US; SB1/eggbert golden attract trace |
| 2 | Port split into `common` and `games/sb1` | Port gates unchanged; matching untouched |
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

- The SB2 assets are in `assets/` (`blupi2`, `eggbert2`, `blupi2_variants`).
- Languages are switchable in the port and extensible by language packs. Language and edition
  switches in the source use a form the port can bind at run time (arithmetic on the constant)
  where it is byte-neutral.
- `sb1/blupi/us` stays the primary matching target.
- First start: the installed language, recognised from the installed executable's size and hash.
- Each edition offers its own languages plus custom packs.
- Custom accent glyphs come later, with custom graphics; 1.1 uses the font as it is.
- The work stays on `multi-game` until steps 1 and 2 are both done; then it is merged into main
  and released as 1.1.

For review: the language-pack format above (one text file per language: header, `[chars]`,
`[strings]`).

## Risks

- Moving SB1 can change what the gates prove if any tool still resolves an old path, so the
  step 3a gate compares the full verifier output, not only the totals.
- A switch can hide a real difference behind a guess. Each must cite the oracle bytes it
  explains, and `sb1/blupi/us` must stay identical.
- Strings read from a player's executable are data from an unknown file: the PE reader must
  bound every offset and size, and treat a malformed file as "no strings".
- SB2 is a second large reconstruction. Its integration rounds stay separate from SB1's, so
  SB1's acceptance never depends on SB2 work in progress.
