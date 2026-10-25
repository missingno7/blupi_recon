# Speedy Blupi and Speedy Eggbert — native SDL3 port

The modern, portable build of the reconstructed games: `blupi-sdl3` (Speedy Blupi 1 and Speedy
Eggbert 1) and `blupi2-sdl3` (Speedy Blupi 2 and Speedy Eggbert 2). It lives entirely under `port/`
and builds from the historical sources in `../src` and `../games/sb2/src` **without modifying them**. The matching reconstruction
(`src/`, `tools/`, `oracle/`, `msvc5kb/`, evidence and acceptance status) is unaffected by
anything here. Rules for working in this directory: [AGENTS.md](AGENTS.md). Design and analysis:
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

Current release: **1.1.0** for Windows x64, Linux x86-64 and Android
([release notes](docs/RELEASE_NOTES.md); how a release is built and packed:
[docs/RELEASING.md](docs/RELEASING.md)). Players use their own copy of the game.

## Source ownership

| Path | Owner | Contents |
|---|---|---|
| `../src/*.cpp, *.h, res/` | historical reconstruction | Compiled unchanged. 12 TUs are shared as-is: `button decblock decblupi decdesign decmove decnet decor dectables event jauge menu text` |
| `../games/sb2/src/*.cpp, *.h` | SB2 historical reconstruction | Eleven shared TUs (`button decblock decblupi decdesign decmove decnet decor dectables jauge menu text`), compiled unchanged with SB2's own headers and compile context; never included through SB1's context. `event.cpp` has a port derivative for runtime edition flows |
| `common/compat/include/` | port | Declaration-only Win32/DirectX/winmm names the shared TUs need, with SDK values; `port_forced.h` (forced include: `fopen`→port file system, `rand`→MS LIBC generator; no game names) |
| `common/compat/src/win32.cpp` | port | Behaviour behind those names: posted-message queue, cursor, rectangles, file APIs, winmm joystick on SDL gamepads, MS `rand` |
| `common/compat/src/hist_stdio.cpp` | port | `BlupiFile` streams for the shared TUs' file I/O (`common/compat/include/port_hist_stdio.h`) |
| `games/sb1/game/` | port, partly derived from historical TUs | Speedy Blupi 1's and Speedy Eggbert 1's port code. Replacements of the 7 platform TUs: `main.cpp`←`blupi.cpp`, `pixmap.cpp`, `sound.cpp`, `misc.cpp`, `movie.cpp`, `network.cpp`; `ddutil.cpp` is not needed. Each file header names its historical origin; copied method bodies are kept verbatim where possible. Port-owned: scene builder and end of tick (`scene.cpp`, `end_of_tick.cpp`), wide editor, languages and edition recognition (`language.cpp`), the build switches' port values (`port_switches.h`, second forced include), state hash and test probes. Display options/settings moved to `common/ui/`; `settings.h` forwards the common header |
| `games/sb2/` | port, partly derived from historical TUs | `blupi2`: six platform adapters in `game/` (`main`←`blupi`, pixmap, sound, misc, movie, network), derived from SB1's adapters and SB2's reconstructed bodies; `ddutil` is replaced by the render device. `game/event.cpp`←`games/sb2/src/event.cpp` selects the original edition's phase data and quit/intro/retry behavior. SB2 state hash, edition/language service (version and optional credit data), forced switches, const scene builder/end of tick (SB2 `decblock.cpp`/`decor.cpp` using SB1's split), wide editor derivative (SB2 `event.cpp`/`decdesign.cpp`, all eleven tools), test probes and original resource strings in `languages/`. `res/`: its icon, the same waving pose from SB2's green Blupi sheet (`tools/make_icon.py --game sb2`). Optional wide/interpolated/display-rate presentation follows SB1; faithful 640×480 tick rendering remains the reference. Status: [SB2_PORT.md](docs/SB2_PORT.md) |
| `common/scene/commands.h` | port, shared by both games | Scene command storage, layer/coverage types and faithful replay (including gauge snapshots); adapters supply historical types, text calls and channel/size constants. Entity identities, visibility, scene builders, views and end-of-tick changes remain game-owned |
| `common/ui/` | port, shared by both games | Display options framework (posted commands, game buttons/fonts, presentation overlay) compiled in each game's own header context; entry placement supplied by the game. Plain JSON settings load/save; each game supplies its file name and user root |
| `common/presentation/timing.h` | port, shared by both games | Fixed tick scheduling with dropped missed ticks, presentation alpha and timing diagnostics; no game or compat headers |
| `common/render/` | port | Rendering device interface and commands (`device.h`), software reference renderer (8 bpp palettized and RGB565 surfaces, DirectDraw blit rules), BMP reader. No SDL, no Win32 names |
| `common/platform/` | port | SDL3 host: window/presentation, input, touch controls, timing, audio mixer, MIDI synthesizer, file system roots, the language-pack format, icon, `SDL_main` entry (calls the game's `BlupiMain`). No game names: the game passes its own (e.g. its user folder's) |
| `third_party/tinysoundfont/` | vendored | TinySoundFont v0.9 (MIT) and TinyMidiLoader v0.7 (zlib) headers, unchanged; provenance in its README |
| `third_party/generaluser-gs/` | git submodule | GeneralUser GS v2.0.3 SoundFont, the author's repository at a pinned commit (shallow); provenance, licence and SHA-256 in [third_party/README.md](third_party/README.md) |
| `tests/` | port | Unit tests, music tests, golden traces (`tests/golden/`) |
| `languages/` | port, generated from the original executables | The built-in language packs (`tools/language_pack.py`; [docs/LANGUAGES.md](docs/LANGUAGES.md)) |
| `res/` | port, derived from game data | The port's icon: the game's waving Blupi sprite (`icon.png`) and the Windows `icon.ico`, made by `tools/make_icon.py` |
| `common/app/` | port | SB1/SB2 launcher: options and combined help, discovery/chooser, settings precedence, user folder, SoundFont search, scripted input, host-message dispatch and fixed-tick main loop; adapters supply names and game hooks |
| `release/` | port | What the packages carry besides the program: the player's `README.txt`, `COPYING.txt` (GPLv3), `THIRD_PARTY.txt` |
| `VERSION.txt` | port | The release version, read by CMake and the Android build |
| `oracle/`, `tools/` | port test tooling | Frame-capture `ddraw.dll` proxy for the historical rebuild, capture and diff scripts ([docs/ORACLE.md](docs/ORACLE.md)); not part of the game build |

Historical bodies copied into the two games' port adapters name their source in the file headers.
Both historical source trees remain unchanged.

## Build (Windows x64, MinGW-w64 GCC)

Requirements: CMake ≥ 3.20 and Ninja (Visual Studio 2022 ships both), MinGW-w64 GCC (tested:
MSYS2 GCC 12.2.0), SDL3 development package (tested: 3.4.16 `SDL3-devel-3.4.16-mingw.zip`,
`x86_64-w64-mingw32` folder).

```powershell
git submodule update --init port/third_party/generaluser-gs   # SoundFont for the music (~62 MB, shallow)
$env:SDL3_DIR = "C:\tools\sdl3-3.4.16-mingw\SDL3-3.4.16\x86_64-w64-mingw32"
$env:BLUPI_DATA = "D:\Prog\blupi_recon\assets\Speedy Blupi"   # optional: enables replay tests
cmake --preset mingw-x64 -S port
cmake --build build/port -j 4
ctest --test-dir build/port --output-on-failure   # the build copies SDL3.dll next to the tests
```

The executable links the GCC runtimes statically, so it needs only `SDL3.dll` next to it. The
`msvc-x64` preset (Visual Studio 2022 + SDL3 VC package) is provided but **not yet tested**.

## Build (Linux)

Requirements: CMake ≥ 3.20, Ninja, GCC or Clang with C++17, the SDL3 development files
(Ubuntu 26.04: `sudo apt install libsdl3-dev ninja-build`). Tested: Ubuntu 26.04 in WSL2, GCC
15.2, SDL 3.4.2.

```bash
git submodule update --init port/third_party/generaluser-gs
BLUPI_DATA="$HOME/speedy-blupi" cmake --preset linux -S port
cmake --build build/port-linux -j 4
ctest --test-dir build/port-linux --output-on-failure
```

From WSL with the repository on a Windows drive, run `sh port/tools/linux_check.sh` instead.
It copies the sources and the data to a case-sensitive folder (`~/blupi-linux`) and builds and
tests there.

`port/tools/export_graphics.py <game folder> <output folder>` exports the game's images as PNG
(16-bit set when there is one, otherwise 8-bit indexed) with an `index.md`, for reference or
for drawing replacements.

## Build (Android)

`port/android` is a Gradle project around the same CMake build: `.\gradlew.bat assembleDebug`
with a JDK 17, the Android SDK/NDK 28.2 and the SDL 3.4.16 source. Players supply their own copy
of the game; the app imports it on first run. Controls are a touch joystick and buttons, or a
gamepad. See [android/README.md](android/README.md).

## Run

The desktop build produces `blupi2-sdl3` beside `blupi-sdl3` (CMake targets `blupi2`
and `blupi`). Put `blupi2-sdl3.exe` and `SDL3.dll` into a Speedy Eggbert 2 or Speedy
Blupi 2 folder and run it; release builds open no console. Alternatively:

```powershell
build\port\blupi2-sdl3.exe --headless --data assets\eggbert2 --user build\sb2-user --frames 8000
```

It also accepts Blupi 2 data (`assets\blupi2`), `--color 8|16`, `--trace`, `--screenshot`, and
`--demo-log` (attract-demo transitions). Configure `BLUPI2_TEST_DATA` (or `BLUPI2_DATA` in the
environment) for Eggbert 2's golden tests and `BLUPI2_TEST_DATA_BLUPI` for Blupi 2's. Only
`BLUPI2_UPDATE_GOLDEN` updates SB2 traces. See [the bring-up report](docs/SB2_PORT.md) for current limits.

Both editions support mission play, pause, save/load and the editor. Blupi 2 uses its goodbye
screen; Eggbert 2 quits directly. Blupi 2's English, German and French packs can be selected
with `--language en|de|fr`; Eggbert 2 has its own English pack. The supplied data determines
the edition. As in SB1, CD checks are omitted; Blupi's error screen retries the supplied folder.

SB2 now renders a const scene once per tick and applies the historical drawing side effects in
`EndOfTick`, following SB1. `--reference` runs the unchanged historical `Build`; `--check-scene`
compares its commands with the scene and uses that reference state, and `--check-view` checks
`DrawView` at alpha 1 and 640 wide against `FlattenScene`. SB2 now offers the same optional
widescreen, interpolation, display-rate presentation and Display options flow as SB1.
Its settings entry is below the original colour-depth controls. Preferences use
`blupi2-sdl3.json` in `%APPDATA%\Epsitec\Speedy Blupi 2 SDL3\` (or `--user`), with
the shared launcher supplying data discovery and a first-run folder chooser, as below.
Headless runs load/save no preferences; CLI display overrides apply to one run. The default
view remains faithful 640x480, without interpolation. See the M4b gates in
[SB2_PORT.md](docs/SB2_PORT.md).

```powershell
build\port\blupi-sdl3.exe --data "D:\Prog\blupi_recon\assets\Speedy Blupi"
```

`--data` is the installed game folder (`DATA`, `IMAGE08`, `IMAGE16`, `SOUND`); it is only read.
Both games build `common/app/launcher.cpp`: common options/help, user folder, SoundFont
lookup, scripted input, host-message dispatch and the fixed-tick main loop. Their historical
adapters and presentation remain in each game's `main.cpp`.
Without `--data` the launcher tries `BLUPI_DATA` (SB1) or `BLUPI2_DATA` (SB2), the folder kept
in that game's settings file, the executable's folder, `host::data_dirs()` and the current
folder. Recognition also checks the game's marker (`data/demo200.blp` for SB1,
`data/bench.blp` for SB2), so the other game's folder is rejected. An invalid explicit
`--data` fails without falling back. When none holds the game,
a desktop build asks for the folder (a
message, then the system's folder dialog; a folder that contains the game, like the CD, works
too) and keeps the choice as `dataFolder` in `blupi-sdl3.json` or `blupi2-sdl3.json`.
Saves, player info, user missions and recorded demos go to `--user DIR` (default: the SDL pref
path `%APPDATA%\Epsitec\Speedy Blupi SDL3\`); a file there shadows the game file of the same
name, so a custom `data\config.def` can be placed in the user folder. `--color 8|16` overrides
the configured colour depth, `--scale N`, `--fullscreen` (Alt+Enter toggles), `--help` lists the
test options (`--headless`, `--frames`, `--trace`, `--screenshot`, `--input`).

The game always simulates at its original 50 ms tick. Display settings that the original
does not have are on the game's settings screen, under **Display options**. They are saved in
`blupi-sdl3.json` in the user folder, and none of them changes the simulation:
- **Widescreen** (off by default): the view follows the window's shape, from 4:3 up to 21:9,
  and shows more of the world around the 640×480 frame, in play and in the editor. In play the
  HUD stays in the centred 640×480 frame; menus are centred too. The editor shows the world
  the same way, with its toolbar at the screen's left edge.
- **Smooth motion** (off by default): movement and scrolling are interpolated between ticks.
  The world is shown up to one tick (50 ms) late; the cursor is not delayed.
- **Frame rate**: *Monitor* (the default) redraws at the display's rate; *Original* presents
  once per tick like the 1998 game.
- **Display** (window or full screen; Alt+Enter also toggles) and **Scaling** (fit the window,
  or whole pixels only).
- **Language**: every language pack of the edition (Speedy Blupi: US, German, French; Speedy
  Eggbert: English, German, Spanish, French, Italian, Portuguese) and the player's own packs.
  It starts as the installed game's language. See [docs/LANGUAGES.md](docs/LANGUAGES.md).

The same options exist for one run on the command line: `--wide auto|off|16:9|854`,
`--interpolate`, `--present display|tick`, `--fullscreen`, `--integer-scale`, `--scale N`,
`--language CODE`.

**Speedy Eggbert 1** (the eGames edition, version 1.1 of the game) is the same game: point the
port at its folder. Version 1.1 changes no gameplay. Like 1.1, the port then shows "Version 1.1"
on the title screen and quits without the goodbye screen ([../docs/EGGBERT.md](../docs/EGGBERT.md)).
The game's versions (1.0, 1.1) are unrelated to the port's own release numbers (`VERSION.txt`).
A **CD** (Speedy Blupi 1) works too: the port uses the data at its root and the `SOUND` folder of
a language folder.

**Music** plays the game's MIDI files through a SoundFont synthesizer. The GeneralUser GS
SoundFont (the one Free Eggbert ships) is the git submodule `third_party/generaluser-gs/`: the
author's repository at a pinned commit. The build copies it next to the executable as
`soundfont.sf2`, so music works without options. Another SoundFont can replace it; the port
tries, in order:
1. `--soundfont FILE`;
2. `%BLUPI_SOUNDFONT%`;
3. `soundfont.sf2` in the user folder;
4. `soundfont.sf2` next to the executable.

Without any SoundFont the game runs without music and logs it once. Headless runs load a
SoundFont only when `--soundfont` is given. The music volume is the game's own option, as in
1998: the 0–20 setting maps through the original `midiOutSetVolume` table.

## Status (2026-10-22)

[SNAPSHOT] The current port status is in [docs/STATUS.md](../docs/STATUS.md) under "SDL3 port". That
covers the latest test totals, the SB2 port, fidelity-map counts and the unavailable services, of which
**native multiplayer is a pilot**: SB1 two-instance sessions work on one machine; LAN, internet,
SB2 and Android follow ([docs/MULTIPLAYER.md](docs/MULTIPLAYER.md)).
The receipts below are those of 2026-10-22.

Verified ([EVIDENCE], reproduced by the commands above):

- **Bit-identical to the historical rebuild.** The cross-build oracle compares every displayed
  frame of `build/standalone-validation/BLUPI.EXE` (under cnc-ddraw, with a frame-hashing proxy)
  with the port's reference renderer. Results, in 16 and 8 bpp:
  - attract mode: 2200 of 2200 frames identical;
  - scripted screen tour: 781 of 811 identical, the rest being the multiplayer screen, where the
    original lists DirectPlay providers.

  See [docs/ORACLE.md](docs/ORACLE.md).
- **Golden traces.** Per-frame simulation state hash plus screen hash, checked in for four
  scenarios in both colour depths (`ctest -R golden`):
  - the attract mode (7500 frames, all seven demos);
  - the screen tour;
  - an editor session;
  - the effects scenario (state probes).

  The simulation state is identical in 8 and 16 bpp.
- **Renderer seam and scene builder** (roadmap stages 1 and 3):
  - every frame's rendering commands replay identically on an independent device
    (`ctest -R commands`);
  - the side-effect-free scene builder reproduces `CDecor::Build` command for command
    (`ctest -R scene`).
- **Simulation separated from presentation** (stages 4, 5, 7, 8, 9). `Build`'s state changes are
  made by `EndOfTick`, and a tick's frame is data that can be drawn any number of times. Every
  golden scenario gives the same traces in each mode (`ctest -R golden`):
  - reference mode;
  - re-rendered frames;
  - display-rate presents;
  - interpolated presents;
  - a 16:9 view.

  The interpolating renderer at alpha 1 issues exactly the faithful commands (`--check-view`).
  The centre of a centred wide view equals the faithful frame, in play at 16:9 and in the
  editor at 21:9 (`ctest -R wide`). Using the Display options screen changes no game state
  (`ctest -R options`). Test-only state probes (`tests/golden/effects.*`) cover shield, power, cloud, hide, notifications, the
  mission-10 clamp and network Blupis.

- The 12 shared historical TUs compile unchanged with GCC 12 (C++17, `-fpermissive -fwrapv
  -fno-strict-aliasing`, cp1252 source charset) against the compat headers; x64 target.
- On-disk structures keep their historical sizes: `DescFile` 868, `MoveObject` 48,
  `DescGameFile` 54416 (saved games), `DemoHeader` 204, `DemoEvent` 4 — static asserts in
  `tests/test_layout.cpp`, cross-checked against WORLD/DEMO file sizes and against the size
  immediates in `BLUPI.EXE`. The written `info001.blp` is 528 bytes like the shipped one.
- `rand()` is the MS C runtime LCG (sequence check).
- Title screen, "Choose player" menu, and the first level (world 1 door hub) render in 16 bpp
  (IMAGE16 backgrounds) and 8 bpp (IMAGE08, palette) modes; scripted clicks reach the level;
  keyboard input moves and jumps Blupi.
- The original attract mode (`CEvent::DemoStep` → `DemoPlayStart`, `DEMO200…`) starts after
  1000 idle frames and plays the recorded levels. Two headless runs of 3000 frames give identical
  per-frame screen hashes in both colour modes (`ctest -R replay`); 8 bpp and 16 bpp runs reach
  the same gameplay states at the same frames.
- Windowed run: SDL3 window, letterboxed scaling, 20 game frames per second (100 frames in
  5 s with the default `Timer=50ms`), clean exit.
- Sound effects load (`SOUND*.BLP`) and play through the SDL3 mixer with the original
  volume/pan arithmetic.
- MIDI music (`MUSIC*.BLP`) plays through TinySoundFont in the SDL3 mix, with the original MCI
  flow:
  - one sequencer "device" per song;
  - `MM_MCINOTIFY(MCI_NOTIFY_SUCCESSFUL)` at the end of a song, handled by the historical
    window-procedure code (suspend, then restart);
  - `MCI_NOTIFY_ABORTED` when a song that is still playing is closed.

  `ctest -R music` checks three things:
  - the synthesizer offline: the 361.3 s song length matches an independent MIDI parse;
  - the end-of-song restart and the abort path in-game;
  - the options volume sequence through the historical `CSound` calls: level ratios equal the
    original table, and the volume-0 quirks behave as the original code does.

  The user heard music in a windowed run (2026-10-21).

Not implemented / known gaps (each fails the way the original did when the service was missing):

- **Music without a SoundFont**: `CSound::PlayMusic` fails like an MCI open failure (logged once).
- **Music level**: the music volume option behaves exactly like the original, as checked by
  `ctest -R music_volume`. What the original cannot tell us is the output level of the 1998
  Windows MIDI synthesizer relative to the sound effects. The port fixes its synthesizer at
  -6 dB so that GeneralUser GS does not clip at full option volume.
- **AVI movies**: `CMovie` reports itself disabled, so movies are skipped as in the original
  without the MCI AVI driver. The movie files are not part of the installed data anyway.
- **Multiplayer**: SB1 has a native UDP pilot (ENet under the original `CNetwork`): two instances
  on one machine create/join a session, use the lobby and play (`--net-port`, `--net-join`). No LAN
  discovery, internet or host migration yet; SB2 keeps the unavailable stub
  ([docs/MULTIPLAYER.md](docs/MULTIPLAYER.md)).
- **Gamepads** stand in for the 1998 joystick. The game's own joystick code runs unchanged on
  SDL gamepads: choose the gamepad under "Choose input device" on the settings screen (each
  session, as in the original). Stick or d-pad move; A, B, X, Y are joystick buttons 1–4;
  Start is Escape (a port addition). Tested with a virtual gamepad (`ctest -R gamepad`), not
  yet with real hardware.
- **MouseType 2/3** (Windows cursors): mapped to SDL system cursors, not the original `.cur`
  shapes. The default MouseType 1 (game-drawn cursor) is exact.
- The cross-build oracle covers the attract mode, the screen tour and an editor session, plus a
  bounded SB1 US save interchange in one scenario ([docs/ORACLE.md](docs/ORACLE.md),
  [oracle/SB1_SAVE.md](oracle/SB1_SAVE.md)). It does not cover every screen, state, language or
  the graphic cursor.
- **Linux**: builds and passes the whole test suite with traces identical to Windows (see
  the build section).
- **Android**: playable on the Android 16 emulator and on a physical phone (Galaxy S24 Ultra):
  the import of the player's own game folder, menus by tap, play with the touch joystick, the
  editor with its scroll pad ([android/README.md](android/README.md)).
- **Release packages** (1.1.0): each is built and tested before packing; the Linux one on
  Ubuntu 22.04 so it needs only glibc 2.35 ([docs/RELEASING.md](docs/RELEASING.md)).

## Next steps

The staged plan, with an equivalence gate per stage, is in [docs/ROADMAP.md](docs/ROADMAP.md).
After 1.1.0:

1. Feedback from players of the release packages; play through more levels, saving and loading
   and the designer by hand, and extend the oracle to what can be reached without gameplay.
2. Android: gamepads, the name keyboard and the app in the background; a headless test runner
   in the APK for the corpus gate on device.
3. Custom port graphics (touch controls, icon) from a player-supplied folder; stage 2 (GPU
   backend) only if profiling shows a need.
4. **Native multiplayer for SB1 and SB2** replaces DirectPlay under the existing `CNetwork`
   interface. It is the next major milestone, after consolidation stage 3
   ([docs/ROADMAP.md](docs/ROADMAP.md), stage 10).
5. Speedy Blupi 2 and Speedy Eggbert 2 on Android (the desktop `blupi2-sdl3` exists).

## Dependencies and licences

- SDL3 (zlib licence), dynamically linked; `SDL3.dll` is copied next to the executable.
- TinySoundFont (MIT) and TinyMidiLoader (zlib): vendored headers in `third_party/tinysoundfont`,
  taken unchanged from Free API's `external/` copy.
- GeneralUser GS v2.0.3 SoundFont by S. Christian Collins (GeneralUser GS License v2.0: free use
  including in software; local copies requested). It is a shallow git submodule at a pinned commit
  of the author's repository; the repository itself stores only the pin. Details are in
  [third_party/README.md](third_party/README.md).
- No game or library code was copied from Free Eggbert (GPLv3), Free Direct or Free API (MIT);
  see the architecture notes for what was evaluated and why.
- The reconstructed game source derives from Planet Blupi (GPLv3+) and the SB2 reconstruction
  (GPLv3); see `src/PROVENANCE.md`. The port inherits those terms: its packages are GPLv3
  (`release/COPYING.txt`) and name this repository as their source.

## Historical synchronization

See [the fidelity policy](docs/FIDELITY.md) and [provenance map](provenance.json)
for source/overlay ownership, scoped evidence and change review. Run
`python port/tools/check_fidelity.py --check` or `ctest -R fidelity_map`.
