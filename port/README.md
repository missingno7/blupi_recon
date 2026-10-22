# Speedy Blupi 1 — native SDL3 port

The modern, portable build of the reconstructed game. It lives entirely under `port/` and builds
from the historical source in `../src` **without modifying it**. The matching reconstruction
(`src/`, `tools/`, `oracle/`, `msvc5kb/`, evidence and acceptance status) is unaffected by
anything here. Rules for working in this directory: [AGENTS.md](AGENTS.md). Design and analysis:
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

Current milestone: **faithful 640×480 SDL3 desktop baseline** (Windows x64 first).

## Source ownership

| Path | Owner | Contents |
|---|---|---|
| `../src/*.cpp, *.h, res/` | historical reconstruction | Compiled unchanged. 12 TUs are shared as-is: `button decblock decblupi decdesign decmove decnet decor dectables event jauge menu text` |
| `compat/include/` | port | Declaration-only Win32/DirectX/winmm names the shared TUs need, with SDK values; `port_forced.h` (forced include: `fopen`→port file system, `rand`→MS LIBC generator) |
| `compat/src/win32.cpp` | port | Behaviour behind those names: posted-message queue, cursor, rectangles, file APIs, winmm joystick on SDL gamepads, MS `rand` |
| `compat/src/hist_stdio.cpp` | port | `BlupiFile` streams for the shared TUs' file I/O (`compat/include/port_hist_stdio.h`) |
| `game/` | port, derived from historical TUs | Replacements of the 7 platform TUs: `main.cpp`←`blupi.cpp`, `pixmap.cpp`, `sound.cpp`, `misc.cpp`, `movie.cpp`, `network.cpp`; `ddutil.cpp` is not needed. Each file header names its historical origin; copied method bodies are kept verbatim where possible |
| `render/` | port | Rendering device interface and commands (`device.h`), software reference renderer (8 bpp palettized and RGB565 surfaces, DirectDraw blit rules), BMP reader. No SDL, no Win32 names |
| `platform/` | port | SDL3 host: window/presentation, input, timing, audio mixer, MIDI synthesizer, file system roots, embedded string table, `SDL_main` entry |
| `third_party/tinysoundfont/` | vendored | TinySoundFont v0.9 (MIT) and TinyMidiLoader v0.7 (zlib) headers, unchanged; provenance in its README |
| `third_party/generaluser-gs/` | git submodule | GeneralUser GS v2.0.3 SoundFont, the author's repository at a pinned commit (shallow); provenance, licence and SHA-256 in [third_party/README.md](third_party/README.md) |
| `tests/` | port | Unit tests, music tests, golden traces (`tests/golden/`) |
| `oracle/`, `tools/` | port test tooling | Frame-capture `ddraw.dll` proxy for the historical rebuild, capture and diff scripts ([docs/ORACLE.md](docs/ORACLE.md)); not part of the game build |

Nothing in `src/` is copied into `port/` except the bodies inside the `game/` replacements of the
platform TUs, which are marked as such.

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
ctest --test-dir build/port --output-on-failure   # SDL3.dll must be on PATH or next to the tests
```

The `msvc-x64` preset (Visual Studio 2022 + SDL3 VC package) is provided but **not yet tested**.

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

`port/tools/export_graphics.py <game folder> <output folder>` exports the game's images as PNG
(16-bit set when there is one, otherwise 8-bit indexed) with an `index.md`, for reference or
for drawing replacements.
It copies the sources and the data to a case-sensitive folder (`~/blupi-linux`) and builds and
tests there.

## Build (Android)

`port/android` is a Gradle project around the same CMake build: `.\gradlew.bat assembleDebug`
with a JDK 17, the Android SDK/NDK 28.2 and the SDL 3.4.16 source. Players supply their own copy
of the game; the app imports it on first run. Controls are a touch joystick and buttons, or a
gamepad. See [android/README.md](android/README.md).

## Run

```powershell
build\port\blupi-sdl3.exe --data "D:\Prog\blupi_recon\assets\Speedy Blupi"
```

`--data` is the installed game folder (`DATA`, `IMAGE08`, `IMAGE16`, `SOUND`); it is only read.
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

The same options exist for one run on the command line: `--wide auto|off|16:9|854`,
`--interpolate`, `--present display|tick`, `--fullscreen`, `--integer-scale`, `--scale N`.

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

## Status (2026-10-21)

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
- **Multiplayer**: `CNetwork` enumerates no DirectPlay providers; the multiplayer screens open
  with nothing to join.
- **Gamepads** stand in for the 1998 joystick. The game's own joystick code runs unchanged on
  SDL gamepads: choose the gamepad under "Choose input device" on the settings screen (each
  session, as in the original). Stick or d-pad move; A, B, X, Y are joystick buttons 1–4;
  Start is Escape (a port addition). Tested with a virtual gamepad (`ctest -R gamepad`), not
  yet with real hardware.
- **MouseType 2/3** (Windows cursors): mapped to SDL system cursors, not the original `.cur`
  shapes. The default MouseType 1 (game-drawn cursor) is exact.
- The cross-build oracle covers the attract mode and the screen tour, not yet every screen, the
  graphic cursor, the editor or save/load ([docs/ORACLE.md](docs/ORACLE.md)).
- **Linux**: builds and passes the whole test suite with traces identical to Windows (see
  the build section).
- **Android**: a first playable build, checked on the Android 16 emulator. It covers the
  import of the player's own game folder, menus by tap, and play with the touch joystick
  ([android/README.md](android/README.md)). It has not yet been tried on a physical device.

## Next steps

The staged plan toward native rendering, interpolation, widescreen and Android, with an
equivalence gate per stage, is in [docs/ROADMAP.md](docs/ROADMAP.md). Immediate steps:

1. Run the windowed build by hand through several levels, saving and loading, the designer and
   option screens (including the music volume buttons); fix what differs. Extend the scripted
   tour and the oracle to whatever can be reached without gameplay.
2. Try interpolation and widescreen by hand at high refresh rates. Settle the [OPEN] points in
   the roadmap: the snap threshold, the cursor over the wide side areas, and HUD anchoring.
3. Android (stage 6) as a separate platform track: player-supplied data imported into app
   storage, touch controls in the presentation layer, text input only while the game asks. Stage
   2 (GPU backend) is deferred until profiling shows a need (see `docs/ROADMAP.md`).

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
  (GPLv3); see `src/PROVENANCE.md`. The port inherits those terms.
