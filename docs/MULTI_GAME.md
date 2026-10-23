# Multi-game layout: every Speedy Blupi and Speedy Eggbert build in one repository

Status (2026-10-22): **Speedy Blupi 1 and Speedy Eggbert 1 are done in every language**, in matching
(nine targets) and in the port (release 1.1.0). **SB2 matching has started** with Speedy Eggbert 2
(`sb2/eggbert/en`): its own oracle, tree `games/sb2/src/` and manifest `games/sb2/recon.json`;
[SB2.md](SB2.md). The SB2 port is not started.

## Model

| Game | Edition | Build | Languages on the media | Targets |
|---|---|---|---|---|
| `sb1` Speedy Blupi | `blupi` | 1998-10-14, Epsitec, version 1.0, LINK 5.10 | US (= ENGLISH), DEUTSCH, FRANCAIS | `sb1/blupi/us` (primary), `de`, `fr` |
| `sb1` | `eggbert` | 1999-06-24, eGames, version 1.1, LINK 5.00.7022 | English, Deu, Esp, Fra, Ita, Por | `sb1/eggbert/en`, `de`, `es`, `fr`, `it`, `pt` |
| `sb2` Speedy Blupi 2 | `blupi` | 2001-05-03, Epsitec, "LINK 5.00" | english, deutsch, francais | `sb2/blupi/en`, `de`, `fr` (same tree and manifest, `_EGAMES=0`) |
| `sb2` | `eggbert` | 2004-05-03, eGames, LINK 5.10 | English | `sb2/eggbert/en` (own tree and manifest) |

- A **game** is one body of C++ source.
- An **edition** is one product built from it (Blupi or Eggbert).
- A **language** is one executable of an edition.
- A **target** is `game/edition/language`: the unit with an oracle, a manifest section and a
  verifier report (`targets.json`, `tools/targets.py`).

Two goals, kept apart:
- **Matching:** every target is reproduced from the reconstructed source with its own historical
  toolchain and checked against its own executable. Every difference matters.
- **Port:** one program per game serves all its editions and languages. Edition and language
  differences come from the player's data and the language packs; code branches only where the
  data cannot carry a behaviour.

## Current state

### Matching

| Target | Strict game `.text` | Notes |
|---|---|---|
| `sb1/blupi/us` | 127,456 / 141,488 | primary; 411 STRICT, 6 FOLDED; ChangePhase and TreatEventBase EQUIVALENT; relinked image `557ca92e…` |
| `sb1/blupi/de` | 127,456 / 141,488 | `LANGUAGE_DE` (help-screen layout) |
| `sb1/blupi/fr` | 127,456 / 141,488 | strings only |
| `sb1/eggbert/en`, `fr` | 123,120 / 141,328 | `_EGAMES`; WindowProc and DoInit excluded ([EGGBERT.md](EGGBERT.md)) |
| `sb1/eggbert/de`, `es`, `it`, `pt` | 123,120 / 141,328 | also `LANGUAGE_DE` (de), `LANGUAGE_ACCENTS`, `LANGUAGE_ES` (es), `LANGUAGE_PT` (pt) |
| `sb2/eggbert/en` | 136,080 / 172,992 | accepted: 447 STRICT, 12 FOLDED, 5 EQUIVALENT (1,200 B); own tree `games/sb2/src/` ([SB2.md](SB2.md)) |
| `sb2/blupi/en` | 121,712 / 173,072 | accepted: 449 STRICT, 5 FOLDED, 9 EQUIVALENT (8,944 B); same tree, `_EGAMES=0`, VC5 RTM + LINK 5.00; comparator total 121,728 ([SB2.md](SB2.md)) |
| `sb2/blupi/de`, `fr` | 121,712 / 173,072 | as `sb2/blupi/en`: `.text` and `.data` identical to English, the language is in `.rsrc` only ([SB2.md](SB2.md)) |

SB2 accepted totals use the effective target manifest. Blupi-only proofs live in targets.json's
`accepted_functions`, already in that target's RVAs; `accepted_overrides` may explicitly clear an
inherited withdrawal with `withdrawn: null`. Both kinds are checked by the ordinary regression gate.

`tools/verify.py` without `--target` is the primary target, byte-identical to its report before the
registry existed. `--target <id>` verifies another one on the same `src/`; targets with their own
oracle (`oracle/sb1-eggbert-en/`, from `tools/derive_target.py`) go through
`tools/cmpfunc_target.py`, because `tools/cmpfunc.py` is pinned by the equivalence certificates.

[OPEN] For Eggbert: `WindowProc`'s three padding bytes (and `DoInit`, which depends on it),
`ChangePhase` (the primary target's residue), and a whole-image relink (`tools/relink.py` knows only
the 1.0 layout).

### Port

`port/common/` (`compat`, `platform`, `render`) names no game; `port/games/sb1/` is the SB1 program
for both editions ([../port/docs/ARCHITECTURE.md](../port/docs/ARCHITECTURE.md), section 9).
- The edition is recognised from the title image, the installed language from the installed
  executable's size and hash (never run).
- Languages are packs ([../port/docs/LANGUAGES.md](../port/docs/LANGUAGES.md)): nine built in,
  generated from the executables, plus players' packs; switchable at run time.
- The build switches get their port values in `port/games/sb1/game/port_switches.h`: `LANGUAGE_DE`
  and `VERSION_TEXT` at run time, `LANGUAGE_ES` set.
- One edition branch: Speedy Eggbert quits without the goodbye screen.
- CDs work (the data at the root, `SOUND` from a language folder).
- Gates: `ctest` (95 on Windows, 94 on Linux, which has no Eggbert data), golden traces for both
  editions and the German tour.

[OPEN] An Android button to add a language file (today: put it into the game folder before
importing). Custom accent glyphs (Czech háček and ring) wait for custom graphics.

## Evidence

Static reads of the executables (no execution), 2026-10-22. Measures:
- whole functions of SB1's inventory searched byte for byte, with image addresses and call
  targets masked ("byte-exact");
- the same with operands ignored ("same instruction sequence");
- shared 12-instruction windows over all of `.text`.

### Editions

- [EVIDENCE] **The editions of one game share their C source.** SB1/blupi against SB1/eggbert:
  358 of 361 functions byte-exact (94% of the bytes; 96-97% of windows shared). The compiler
  output is the same; the differences are real source edits, all under `_EGAMES` now
  ([EGGBERT.md](EGGBERT.md)). Speedy Eggbert 1 is version 1.1, with no gameplay change.
- [EVIDENCE] SB2/blupi against SB2/eggbert: 94% of windows shared. Not yet measured per function.
- [EVIDENCE] **The editions' data differ in a few files** (SB1/blupi against SB1/eggbert
  installations):
  - `IMAGE08` and `IMAGE16` `INIT.BLP` (the title image), `IMAGE08/TEXT.BLP` (the font) and
    `IMAGE08/BUTTON00.BLP`, and `DATA/Config.def`, differ;
  - every level, demo and sound is identical.

  So the port can tell the editions apart from the data alone, and a language pack depends on
  its edition's title image and font.
- [EVIDENCE] The port reads only `DATA`, `IMAGE08`, `IMAGE16` and `SOUND`, plus the size and hash
  of an installed executable to pick the first language. Its strings are the language packs. The
  few values the original read past the ends of its tables are constants in the port's source.

### Languages

- [EVIDENCE] **A language is an executable.** In every game the language builds have the same
  code size; they differ in:
  - **the string table (`.rsrc`)**, in every language. The resource language ID is neutral (0)
    everywhere, so a language is identified by its folder, not by the resource.
  - **one layout difference in German**: the help screen (`WM_PHASE_HELP`, in `DrawButtons`)
    stacks two texts 13 px apart, where the US build puts them side by side. The same change is
    in SB1/blupi's and SB1/eggbert's German builds.
  - **the character table `table_char`** (SB1 0x7b530: 6 bytes for each of 256 codes, a base
    glyph plus an optional accent glyph and offset) in SB1/eggbert's Deu, Esp, Ita and Por
    builds, and Spanish's width of `¡`.
  - SB2/blupi's German and French builds differ only in `.rsrc`.
- [EVIDENCE] **Data and sounds are shared by all languages.** The `SOUND` folder of every
  language folder is identical to the main one (SB1: 76 files; SB2: 103 files); DATA and IMAGE
  folders exist once. SB1/eggbert's `SOUND` is also identical to SB1/blupi's.
- [EVIDENCE] SB1/blupi's US and ENGLISH executables and the installed one are identical; so are
  SB2/blupi's main and english ones.
- [EVIDENCE] **The font** (SB1/blupi/us `table_char`): plain glyphs for ASCII, lower-case
  `à â ä ç è é ê ë î ï ô ö ù û ü`, and `¡ ¿`; `á í ñ ó ú` composed from a base letter and an
  accent glyph; no upper-case accented letters and no `ß`.

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
- [EVIDENCE] SB1/eggbert's is VC5 RTM's LINK 5.00.7022 (probe links, [EGGBERT.md](EGGBERT.md)).
  [HYPOTHESIS] SB2/blupi's is the same.
- [EVIDENCE] **Each build records its source folder** (the `__FILE__` of `TRY_DS` in
  `sound.cpp`, the only `__FILE__`/`__LINE__` use in the source):
  - SB1/blupi and SB1/eggbert: `F:\Profil\sound.cpp`;
  - SB2/blupi: `D:\Blupi\Profil\sound.cpp`;
  - SB2/eggbert: `D:\Blupi\Profil egames 2\sound.cpp`.
- [HYPOTHESIS] Epsitec kept a separate source folder per eGames edition (`Profil egames 2`) and
  edited it. The reconstruction still models editions and languages as switches in one source:
  the C differs in a few functions, and a copy per build would duplicate 99% of the code.

## Source rules (matching)

1. **Editions and languages are build-wide switches** (`_EGAMES`, `LANGUAGE_*`, `VERSION_TEXT`),
   set by the target's entry and defaulted in `src/def.h`; never per-TU views. A switch surrounds
   only code or data that differs in that target's executable, and says so. Branches inherited
   from the SB2 or Planet Blupi sources that no SB1 executable shows are removed.
2. **Forms the port can bind.** A switch the port needs at run time is written as an expression
   (`140 - 20 * LANGUAGE_DE`, `(char*)VERSION_TEXT`), only where every target's report stays
   byte-identical; otherwise `#if`. [EVIDENCE] An `if (LANGUAGE_DE)` block changed
   `TreatEventBase`'s code in the US build; the arithmetic form did not.
3. **A switch must not move code in a TU where `__LINE__` or `__FILE__` is observable.** Only
   `sound.cpp` is one, and it is identical in every SB1 build.
4. **Per-game trees, no cross-game includes.** When SB2 exists, each game's TUs resolve includes
   in its own tree. A shared `common/src` is not planned: a file would qualify only when its text
   is identical in both trees and every target that compiles it is STRICT for it, and class
   layouts differ (`pixmap` above). Small duplication between the games is accepted.
5. **Build paths per target:** SB1 `F:\Profil\` (temporary drive mapping); SB2 `D:\Blupi\Profil\`
   and `D:\Blupi\Profil egames 2\` (folders created for one compile by `tools/buildpath.py`,
   approved 2026-10-22).
6. **Provenance per game:** code taken from `external/sb2-decomp` is recorded like SB1's
   (`src/PROVENANCE.md`, GPLv3). That project and Free Eggbert are not matching
   reconstructions: their code is a candidate to verify, never accepted because it compiles or runs.

## Layout

Today:

```
src/                  SB1 source (all nine targets), resources in src/res/
targets.json          target registry: executable, SHA-256, linker, build path, defines, oracle
recon.json            the primary target's manifest
oracle/               primary inventory; oracle/sb1-eggbert-en/ for the Eggbert targets
tools/ tests/ evidence/ steering/ msvc5kb/ docs/
port/common/          compat/, platform/, render/: no game names
port/games/sb1/       the SB1 program: game/, with its golden traces in port/tests/
port/languages/       built-in language packs
```

**`src/` stays where it is** (decided at the SB2 bring-up, [SB2.md](SB2.md) section 9: SB2 fits
beside it with `games/sb2/src/` and a `source_root` in its manifest). Moving it to `games/sb1/src/` would touch 8
tools, 468 entries of `recon.json`, 18 of `targets.json`, the port's build and 49 research docs,
and `tools/cmpfunc.py` (which names `src/`) is pinned by the equivalence certificates, so it
would also need re-certification. Nothing gains from the move while there is one game. It is the
first, separate commit of the SB2 bring-up (below), or it is skipped if SB2 fits as well beside
`src/`.

With SB2:

```
src/                  SB1 source (unchanged)
games/sb2/src/        SB2 source, verified function by function against its own oracle
games/sb2/recon.json  SB2's manifest ("source_root": "games/sb2/src")
oracle/sb2-eggbert-en/ SB2's own inventory (tools/target_oracle.py, tools/crossmap.py)
port/games/sb2/       later: the SB2 program, its own executable linking port/common/
```

## SB2: the first step (done 2026-10-22, [SB2.md](SB2.md))

The plan below was carried out, with the tree kept beside `src/` (step 2) and several engine TUs
seeded (step 3).

1. **Registry and inventory only.** Activate `sb2/eggbert/en` in `targets.json` (asset, SHA-256,
   LINK 5.10, build path `D:\Blupi\Profil egames 2\`) and derive its inventory with Ghidra into
   `oracle/sb2-eggbert-en/`: functions, TU map, imports. Eggbert 2 first: it has SB1/blupi's
   linker family and Rich header, so `relink.py`'s knowledge applies.
2. **Decide the tree** with that inventory in hand: move `src/` to `games/sb1/src/` (gate: every
   SB1 report, the relinked image hash and the certificates identical, the comparator re-certified
   once for its new path) or keep `src/` and add `games/sb2/src/`.
3. **Seed one engine TU** whose instruction sequence is closest to SB1's (`ddutil`, `movie`,
   `text`, `jauge`: 100%) from SB1's source, compile it at SB2's build path, and verify it against
   the SB2 oracle. `external/sb2-decomp` is consulted per function as a hint.

No SB1 file changes in step 1; the SB1 gates must stay identical throughout.

## Gates

| Change | Gate |
|---|---|
| Any `src/` edit | every SB1 target's verifier report identical (or improved by a justified match); relinked image `557ca92e…`; certificates pass |
| Port change | `ctest` on Windows and Linux; golden traces unchanged unless the change is intended and recorded in `port/docs/ORACLE.md`; Android build |
| `src/` move | full verifier output of all nine SB1 targets identical, not only the totals |
| SB2 work | its own reports; SB1 gates unchanged |

## Decisions

Taken (2026-10-22):
- `sb1/blupi/us` stays the primary matching target.
- SB1 and Speedy Eggbert 1, all languages, came first; SB2 is prepared, not started.
- Temporary build-path folders on D: are allowed for SB2, created and removed by
  `tools/buildpath.py` for one compile.
- The SB2 assets are in `assets/` (`blupi2`, `eggbert2`, `blupi2_variants`).
- Languages are switchable in the port and extensible by packs. Each edition offers its own
  languages plus custom packs; the first start uses the installed language.
- Custom accent glyphs come later, with custom graphics.
- One port program per game, sharing `port/common/`; no launcher until two games exist and the
  need is shown.
- `src/` stays in place; SB2 has its own tree `games/sb2/src/` and manifest (SB2 bring-up, 2026-10-22).
- SB2 source starts as a copy of SB1's and changes only from SB2 binary evidence; no shared source
  files between the games until the SB2 game logic shows which files stay identical.

## Risks

- Moving SB1 can change what the gates prove if any tool still resolves an old path, so the
  move's gate compares the full verifier output.
- A switch can hide a real difference behind a guess. Each must explain bytes of its target's
  executable, and `sb1/blupi/us` must stay identical.
- SB2 is a second large reconstruction. Its rounds stay separate from SB1's, so SB1's acceptance
  never depends on SB2 work in progress.
