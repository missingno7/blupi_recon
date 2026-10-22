# Languages

The port shows the game in any language it has a **language pack** for. A pack is one UTF-8
text file. The port has the original languages built in, and players can add their own.

## For players

- **Changing the language:** on the game's settings screen, open **Display options**. The
  **Language** row's arrows go through every language available for your edition. The change
  applies at once and is kept in `blupi-sdl3.json`. `--language <code>` sets it for one run.
- **First start:** the port uses the language of your installed game. It recognises the
  installed executable by its size and checksum only (it never runs it). Unknown or missing
  executable: English.
- **Languages per edition:**

  | Edition | Built-in packs |
  |---|---|
  | Speedy Blupi (1998) | `us` English (US), `de` Deutsch, `fr` Français |
  | Speedy Eggbert (1999) | `en` English, `de` Deutsch, `es` Español, `fr` Français, `it` Italiano, `pt` Português |

  The port tells the editions apart by their title image (`IMAGE08\INIT.BLP`).
- **A CD:** point the port at the CD's root (Speedy Blupi 1 and 2). It uses the data there and
  the `SOUND` folder of a language folder (the same in all of them).

## For translators: adding a language

1. Export a built-in pack as a starting point:

   ```
   blupi-sdl3 --export-language us mylanguage.txt
   ```

   (`de`, `fr`, ... for another language; `eggbert-es` for an Eggbert pack.)
2. Edit it in any text editor, saving as UTF-8: set `name` and `code` at the top, then translate
   the text after each `=`.
3. Put it in a `languages` folder in the user folder, which is also where the saves are:
   - Windows: `%APPDATA%\Epsitec\Speedy Blupi SDL3\languages\`
   - Linux: `~/.local/share/Epsitec/Speedy Blupi SDL3/languages/`

   A `languages` folder inside the game folder works too.
4. It appears in the Language row the next time the game starts. A pack with a built-in code
   (for example `de`) replaces that built-in pack.

A missing line falls back to the edition's English text, so an unfinished translation is
playable. Problems are written to the log (run with `--verbose`); they never stop the game.

## The format

```
# Speedy Blupi language pack           <- comments start with #
name    = Deutsch                      <- shown in the Language row
code    = de                           <- identifies the pack
game    = sb1                          <- Speedy Blupi 1 / Speedy Eggbert 1
edition = blupi                        <- optional: only for this edition (blupi, eggbert)
variant = de                           <- the original's code variant (see below)

[chars]                                <- how letters are drawn (optional)
Ç = ç                                  <- like another letter
Á = A + acute @ 2,-2                   <- a letter plus an accent, at an offset (pixels)

[strings]
100 = training %d, time %d             <- string id = text
169 = "  (%d open doors)"              <- quotes keep spaces at the start or end
112 = Open game in folder \\User       <- \\ is a backslash, \n a new line
```

- **Strings.** The ids are those of the original's string table; keep `%d`, `%s` and the like
  as they are. Texts on the original screens have fixed places, so a much longer text can run
  past its space.
- **Letters.** The game draws single-byte characters (Windows-1252) with its own bitmap font,
  which has:
  - ASCII;
  - lower-case `à â ä ç è é ê ë î ï ô ö ù û ü` and `¡ ¿`;
  - `á í ñ ó ú`, composed from a letter and an accent.

  A letter the font lacks is drawn as its base letter (`č` as `c`, `ý` as `y`) and reported in
  the log. A `[chars]` line can do better by composing it:
  - from a letter and one of the font's accents: `acute`, `grave`, `tilde`, or a glyph number
    `#xx`;
  - optionally with an offset for the base letter: `... base @ x,y`.

  Accents the font does not have (the Czech háček and ring) need custom font graphics, which
  will come later.
- **Variant.** Most of the original's languages differ only in text. The German build also lays
  out the help screen differently, for its longer texts. `variant = de` asks for that layout;
  any other value uses the original US one. (The layout switch arrives with the matching work
  on the German build; see `docs/MULTI_GAME.md`.)
- **Port texts.** The Display options screen's own texts can be translated too, with these
  ids:

  | Id | English |
  |---|---|
  | 9000 | Display options |
  | 9001 | Widescreen |
  | 9002 | Window shape |
  | 9003 | Off (4:3) |
  | 9004 | Smooth motion |
  | 9005 | On |
  | 9006 | Off |
  | 9007 | Frame rate |
  | 9008 | Monitor |
  | 9009 | Original |
  | 9010 | Display |
  | 9011 | Window |
  | 9012 | Full screen |
  | 9013 | Scaling |
  | 9014 | Fit the window |
  | 9015 | Whole pixels |
  | 9016 | Language |

## How the built-in packs are made

`port/languages/*.txt` are generated from the original executables by
`port/tools/language_pack.py`:
- the string table, converted to UTF-8;
- the `[chars]` lines where the executable's character table (`table_char`) differs from its
  edition's English build. Only Eggbert's German, Spanish, Italian and Portuguese builds have
  any: `Á Ç ã ì õ`.

They are embedded in the program at build time. The US pack must equal the reconstructed
`src/res/blupi.rc` (`ctest -R language`).

The built-in packs keep the original's bytes exactly, including the few letters the font cannot
draw (Eggbert's Italian `È`, shown as the font's blank glyph, as in 1999). Only players' packs
fall back to base letters.

## Implementation

- `common/platform/language.cpp`: the format, UTF-8 and Windows-1252. No game types, so every game
  can use it.
- `games/sb1/game/language.cpp`: recognising the edition and the installed build, the packs, applying a
  pack. Its strings answer `LoadString`, and its `[chars]` entries go into `table_char`, the
  font table, which is restored before each pack. A letter outside Windows-1252 gets a code
  the font leaves blank.
- `games/sb1/game/options_screen.cpp`: the Language row. A change enters the current phase again, since
  the original draws a screen's texts when the phase starts.
