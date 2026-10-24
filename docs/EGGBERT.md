# Speedy Eggbert 1: version 1.1 of Speedy Blupi

Speedy Eggbert (eGames, 1999-06-24, `assets/eggbert/Game/egames.exe`) is version 1.1 of Speedy
Blupi (Epsitec, 1998-10-14, version 1.0). This file records how the two differ, how the
reconstruction builds both, and what it means for the port. Static reads only (2026-10-22).

## Toolchain

- [EVIDENCE] Linked with **VC5 RTM's LINK 5.00.7022**. A probe program linked with each installed
  linker gives the same fingerprint as `egames.exe` only with RTM's:

  | Linker | Version field | Rich header | Separate `.idata` |
  |---|---|---|---|
  | RTM 5.00.7022 | 5.00 | no | yes |
  | SP2 5.02.7132 | 5.02 | no | yes |
  | SP3 5.10.7303 | 5.10 | yes | no |
  | `egames.exe` | 5.00 | no | yes |
  | `BLUPI.EXE` (1.0) | 5.10 | yes | no |

- [EVIDENCE] The code is SP3's, not RTM's. With the RTM compiler, 17 functions differ; with SP3,
  only the version 1.1 changes do.
- [EVIDENCE] The target-scoped EQUIVALENT tier (`tools/equivalence_target.py`, certificates in
  `evidence/equivalence/sb1-*`; `scratch/w-sb1equiv/REPORT.md`) now covers SB1's secondary targets:
  `ChangePhase` and `TreatEventBase` on all eight (14,000 bytes), plus Speedy Eggbert 1's `WindowProc`
  (its three differing bytes are unreachable padding before the jump table; 15,360 bytes per Eggbert
  language). `DoInit`'s 235 relocations are all verified (its raw status stays RELOC-UNVERIFIED). SB1's
  pinned primary checker and certificates are unchanged; EQUIVALENT never counts as STRICT.
- [OPEN] `WindowProc` differs from SP3's output in three alignment-padding bytes before its jump
  table: `90 8b ff` in 1.1, `8d 49 00` from every installed compiler (RTM, SP2, SP3, NT5 1592 and
  1691, VC6). `WindowProc`, and `DoInit` (whose reference to it is proven only once it matches),
  are excluded for the Eggbert targets.
- [EVIDENCE] This linker places uninitialised data straddling the end of `.data`'s raw bytes
  (`g_CDPath`). `tools/cmpfunc.py` now accepts that (all zero either way); the certificates were
  re-certified, and only the comparator hash changed.
- [EVIDENCE] It also keeps a different body for identical-code folding: `CJauge::~CJauge` in
  place of `CMenu::~CMenu`. These are per-target expectations in `targets.json`.

## What version 1.1 changes

- [EVIDENCE] **No gameplay code changed.** Every function of `decmove`, `decblupi`, `decblock`,
  `decdesign` and `decor` is identical, so 1.1 contains no gameplay bug fixes. The levels, demos and
  sounds in the data are identical too.
- [EVIDENCE] **No CD requirement** (`_EGAMES`):
  - `ReadConfig` takes the current directory when `config.def` has no `CD-Rom=` line, and does not
    check for a CD drive; 1.1's `config.def` has no such line.
  - `CheckWorld1` always passes.
- [EVIDENCE] **Quitting**: `ChangePhase` closes the program on `WM_PHASE_BYE`, the goodbye screen,
  where 1.0 does on `WM_PHASE_DOQUIT`; 1.1 shows no goodbye screen.
- [EVIDENCE] **A background image that fails to load** leads straight to the insert-CD screen; 1.0
  first logs `[Cache error]` and waits for the mouse.
- [EVIDENCE] **"Version 1.1"** on the title screen.
- [EVIDENCE] **Presentation and localisation**:
  - the title image (`INIT.BLP`);
  - one button colour (`BUTTON00.BLP`: the sixth button red instead of green);
  - four accent glyphs in the font (`TEXT.BLP`: acute, grave, `^`, `~` of the third font);
  - the German, Spanish, Italian and Portuguese builds compose `Á Ç ã õ` (and `ì`, except
    Portuguese), and Spanish draws `¡` narrower;
  - the names in the string table.
- [EVIDENCE] Branches inherited from the Speedy Eggbert 2 and Planet Blupi sources are not in 1.1,
  and were removed: the "Eggbert" window title (1.1's is "Blupi"), an epsitec line, disabled
  movies, and a Planet Blupi user folder.

## Matching

| Target | Strict bytes | Notes |
|---|---|---|
| `sb1/eggbert/en` | 123,120 / 141,328 | `_EGAMES`; WindowProc and DoInit excluded |
| `sb1/eggbert/de` | 123,120 / 141,328 | also `LANGUAGE_DE` (help layout), `LANGUAGE_ACCENTS` |
| `sb1/eggbert/es` | 123,120 / 141,328 | `LANGUAGE_ACCENTS`, `LANGUAGE_ES` (width of `¡`) |
| `sb1/eggbert/fr` | 123,120 / 141,328 | strings only |
| `sb1/eggbert/it` | 123,120 / 141,328 | `LANGUAGE_ACCENTS` |
| `sb1/eggbert/pt` | 123,120 / 141,328 | `LANGUAGE_ACCENTS`, `LANGUAGE_PT` (no `ì`) |

- [EVIDENCE] A negative control fails as it should: Spanish without its defines mismatches in
  `GetCharWidth` and `DrawChar`.
- [EVIDENCE] `ChangePhase`'s 1.1 changes (`scratch/w-eggcp/REPORT.md`): after a background-load or a
  mission-read failure, version 1.1 logs `[Cache error]`, waits for the mouse and returns to the title
  (`WM_PHASE_INIT`) instead of retrying via the disc screen, as Speedy Eggbert 2 does. With them, the
  function's differences fall from 4,374 to 279 bytes in all six languages; it still carries the primary
  target's guard-order residue plus 247 bytes [OPEN]. The relinked `.text` now has the original's size, and
  365/408 accepted functions land at their original RVA (`--order eggbert-jauge-before-menu`). The
  return-TRUE body at 0x19270 is now owned by `CEvent::CheckWorld1` (event.cpp's region) with
  `CPixmap::Flush` FOLDED into it; coverage is unchanged.
- [EVIDENCE] `tools/relink.py --target` relinks every 1.1 target with its own linker, order and generated
  resource script (`src/res/sb1-*.rc`, `tools/make_rc.py --target`; `scratch/w-sb1relink/REPORT.md`): no
  unresolved or duplicate symbols; Speedy Eggbert 1: 365/408 accepted functions at their original RVA (with `ChangePhase`'s 1.1 code and
  `--order eggbert-jauge-before-menu`),
  `.rsrc` equal except 71 timestamp fields; German/French Speedy Blupi: 394/412, `.rsrc` raw exact.
  [OPEN] Full `.text`/`.data` image equality.

## What it means for the port

- **Gameplay:** nothing to change. The port runs 1.1's data with the same game code.
- **Quitting:** with Eggbert data the port closes where 1.1 does. A `WM_PHASE_BYE` message
  becomes `WM_PHASE_DOQUIT` (`games/sb1/game/main.cpp`).
- **CD and paths:** the port's own `main.cpp` and file system replace them for both editions.
- **Presentation and languages:** from the data (title, font, buttons) and the language packs
  (strings and `[chars]`).
- [EVIDENCE] **The version label** is `VERSION_TEXT` (`src/def.h`): "Version 1.1" under `_EGAMES`,
  otherwise "Version 1.0". Every target's preprocessed source is unchanged, and so are all nine
  reports and the relinked image. The port defines it as the recognised edition's label
  (`port/games/sb1/game/port_switches.h`, `language.cpp`), so Eggbert's title screen says
  "Version 1.1" (screenshot, 2026-10-22; the Eggbert golden trace changed in those frames only,
  `port/docs/ORACLE.md`).
- [EVIDENCE] **The Spanish `¡`:** the port compiles `text.cpp` with `LANGUAGE_ES` set, so glyph 5
  has 1.1's Spanish width (5 instead of 10) in every language. Glyph 5 is drawn only for `¡`, and
  only the Spanish pack has `¡`, so no other language's text changes (`ctest -R text`).
- **A background image that fails to load** behaves as in 1.0 in both editions (the port compiles
  `_EGAMES` as 0): it logs, waits for a click and shows the insert-CD screen. Only a damaged game
  folder reaches it.
- The port's own release numbers (1.0.0, 1.1.0, ...) are unrelated to the game's 1.0 and 1.1.

## Shared getter owner (2026-10-24)

[EVIDENCE] In all six Speedy Eggbert 1 originals the 16-byte body at `0x20be0` (identical bytes to
`CJauge::GetHide` and `CNetwork::GetNbSessions`) lies in CNetwork's source-order slot between `EnumSessions`
(`0x20b30`) and `GetSessionName` (`0x20bf0`); jauge.cpp has no free slot between `SetType` and `SetHide`. The
shared inventory (`oracle/sb1-eggbert-en/tu_map.csv`) now credits it to network.cpp: `GetNbSessions` STRICT,
`GetHide` FOLDED into it, in all six targets (`targets.json` overrides). Every other function record, the
primary report and the primary relink (`557ca92e…`) are unchanged; the 18 Eggbert 1 certificates were re-issued
after their proofs re-passed and differ only in the pinned inventory hash (`scratch/w-egginv/REPORT.md`). This
also removes the GetHide/GetNbSessions owner cycle the verifier showed with external network callbacks.
