# Cross-build oracle: historical rebuild versus SDL3 port

Roadmap stage 0.3. This is runtime evidence: per the repository rules it is kept separate from
the strict/equivalence proofs of the matching build.

## What is compared

- **Historical side:** `build/standalone-validation/BLUPI.EXE`, the validated source rebuild
  (never the original executable). It runs under cnc-ddraw in an isolated copy of the data.
  `port/oracle/ddraw_proxy.cpp` is a 32-bit `ddraw.dll` placed in front of cnc-ddraw (renamed
  `cnc-ddraw.dll`). It forwards every call and wraps only the primary surface. `CPixmap::Display`
  shows a frame with one `Blt` of the whole 640×480 back buffer, and at that moment the proxy
  hashes the back buffer's raw rows (8 bpp indices or RGB565 words).
- **Port side:** `blupi-sdl3 --headless --display-log` computes the same hash at every
  `CPixmap::Display` of the reference renderer.
- **Sequence:** both start with one `Display` at start-up and ten in `Benchmark()`, then one per
  timer frame. Frames are compared by display index, so wall-clock timing does not matter.
- **Scripted input (`-InputScript`):** the proxy posts the port's `--input` messages during the
  display before the frame they belong to. They are then queued ahead of that frame's
  `WM_UPDATE`, as in the port.
- **Configuration:** both sides use the same `config.def`, with two oracle-specific settings:
  - `MouseType=2`, because the graphic cursor would be drawn at the OS cursor position, which
    depends on the desktop and not on the game;
  - cnc-ddraw `adjmouse=false`; the OS cursor is parked in the screen corner.

```powershell
powershell -ExecutionPolicy Bypass -File port\tools\oracle_capture.ps1 -Frames 2200 -ColorDepth 16
powershell -ExecutionPolicy Bypass -File port\tools\oracle_capture.ps1 -Frames 811 -InputScript port\tests\golden\tour.input
.\.pyenv\python.exe port\tools\oracle_diff.py build\oracle\run-<timestamp>   # pixel diff of dumped frames
```

- A game window is open for the duration of a run. Do not touch the mouse or keyboard.
- The script rebuilds the proxy (MinGW i686, `C:\msys64\mingw32\bin` or `$env:MINGW32_BIN`) when
  its source changed. Each run has its own `build/oracle/run-*/` with `compare.json`.

## Results (2026-10-21, final binaries)

| Run | Frames | Identical | Note |
|---|---:|---:|---|
| Attract mode, 16 bpp | 2200 | 2200 | title idle + all of DEMO200 + start of DEMO201 |
| Attract mode, 8 bpp | 2200 | 2200 | after the palette fix below |
| Screen tour, 16 bpp | 811 | 781 | displays 201–230 = multiplayer screen |
| Screen tour, 8 bpp | 811 | 781 | same 30 frames |
| Editor session, 16 bpp | 371 | 370 | `tests/golden/editor.input`: editor, minimap stretch, test-play, pause screen; display 51 is a timing race (below) |

**Timing race on click frames.** In the historical build the multimedia timer thread posts
`WM_UPDATE` asynchronously. When it fires between the handling of a scripted `WM_LBUTTONUP` and
the button command that handler posts, the command takes effect one frame later. This was seen
once (editor run, display 51) and not in the tour run with the same click at the same frame. The
port's deterministic order corresponds to the timer firing later. It is not a port difference.

[EVIDENCE] The port's reference renderer and the shared game code reproduce the historical
build's back buffer bit for bit. That covers:
- GDI image conversion in both colour depths, colour keys, clipping and draw order;
- the HUD, text and the font;
- the menus and settings screen;
- gameplay simulation during the recorded demo and the scripted hub walk.

The only difference is expected: the historical build lists the machine's real DirectPlay
service providers on the multiplayer screen, and the port, without multiplayer, lists none.

## What the oracle corrected

Each fix has a golden-trace update with the reason recorded here:

1. **8 bpp palette** (`port/game/pixmap.cpp`, `load_palette`). `DDLoadPalette` calls
   `CreatePalette(DDPCAPS_8BIT)` without `DDPCAPS_ALLOW256`, and DirectDraw then keeps entries 0
   and 255 black and white. The historical primary palette has exactly that. With the fix,
   GDI's nearest-colour mapping is matched as well. The previous [HYPOTHESIS] (nearest palette
   entry, exact match first) is confirmed for this data.
2. **Message order** (`common/app/launcher.cpp`, host loop). Queued messages, and those they post
   (a button's command after `WM_LBUTTONUP`), are now handled before the timer's `WM_UPDATE`, as
   the original message loop did. Before, a click took effect one frame late.
3. **TranslateMessage** for posted key presses. The original loop generated `WM_CHAR` for them;
   the port now does the same for scripted keys.

Also confirmed: 16 bpp loading converts by truncation to RGB565 (former [HYPOTHESIS] in
`ARCHITECTURE.md`).

The Linux build (roadmap 0.4, AddressSanitizer) found reads the original makes just outside
its static tables. Each used whatever BLUPI.EXE has next to the table, and the port now
reproduces that explicitly instead of reading its own, compiler-dependent neighbours
(`port/game/pixmap.cpp`). [EVIDENCE] The values were read from BLUPI.EXE's `.data` with pefile;
nothing was executed.
- `table_icon_explo` declares 54 icons but holds 53 and five values. Explosion icon 53, a
  frame of the cloud effect, takes its height from the next table: 95. It is now drawn
  124×95. The port had drawn whatever height its own memory gave, which was not 95, since the
  screen hash changed. This is the one golden-trace update: the screen hash of the effects scenarios at the cloud
  frames (200–230 and 480–490). Their state hashes are unchanged.
- An element icon of rank −1 reads the object table's tail and padding. It is 0 pixels wide
  in the original, so it is never drawn. There was no visible change.
- `CPixmap::MouseHotSpot` indexes its 4-entry table up to `SPRITE_WAIT` (11), reading text in
  the original. The hotspots are then about 1.7e9 and the wait cursor is never drawn. This
  also crashed the Linux build.
- `CEvent::MouseOnButton` reads the phase table at index −1 once, at start-up, before the
  first phase is set. That is in shared code (`src/event.cpp`), so it stays. Its result is
  overwritten by `WaitMouse` immediately afterwards, and it reads nearby static data on every
  platform tried.

One more golden-trace update came from the state hash itself, not from a fix. For roadmap
stage 4, the hash (`port/game/state_hash.cpp`) was widened to every value member of the jauges
and buttons, redraw flags included, so that the moved `Build` state changes are covered. Only
the state-hash column of the attract, tour and editor traces changed. [EVIDENCE] Frame, phase
and screen hash are identical to the previous traces at every checkpoint.

The Speedy Eggbert trace (`eggbert_attract16.trace`) was updated once (2026-10-22), when its title
screen started to say "Version 1.1" like the original 1.1 (`VERSION_TEXT`,
[../../docs/EGGBERT.md](../../docs/EGGBERT.md)). [EVIDENCE] Only the screen hash of the 99
title-screen checkpoints (frames 10–990, `WM_PHASE_INIT`) changed; frame, phase and state hash are
identical at all 750 checkpoints. The Speedy Blupi traces did not change.

## Not covered yet

- **Graphic cursor (MouseType 1):** its blits are the same code paths as icons, but not compared
  under this setup.
- **Not visited by the tour:** music and sound state (not in frames); the mission editor; save
  and load; later demos (DEMO202–206: the runs stop at frame 2200, while the golden traces
  cover them on the port side only).
- **Multiplayer.**

## SB2 bring-up: local regression evidence (2026-10-23)

[EVIDENCE] The four `sb2_{eggbert,blupi}_attract{8,16}.trace` files are new port
regression baselines, not historical cross-build comparisons. Each records 800 checkpoints
(every 10 frames of 8,000). The game's own attract system visits `DEMO300..309` and wraps
back to 300; `sb2_golden.cmake` checks that sequence independently of the hashes. Each run
also replays every rendering command on a second software device and requires equal
pixels (8,011 displays including startup). SB1's golden files were not regenerated.

[OPEN] No SB2 historical rebuild was run or captured in this task. SB1's pixel-oracle
results above do not establish SB2 historical pixel equality. SB2 screenshots are local
runtime evidence under `build/`, listed in [SB2_PORT.md](SB2_PORT.md).

## SB2 M3 edition flows (2026-10-23)

[EVIDENCE] `sb2_{eggbert,blupi}_{tour,editor}{8,16}.trace` add eight local regression
baselines. The tour enters official world 1, pauses/resumes, returns through player/title
menus and quits. Eggbert closes at input frame 190; Blupi draws BYE through frame 230,
then closes on dismissal. The editor tour opens a private mission, scrolls, saves it into
the user root, enters playtest and returns to the editor. Every display is independently
replayed by the command checker. These are port regressions, not historical pixel proofs.

[EVIDENCE] Only two existing SB2 baselines changed: `sb2_blupi_attract{8,16}.trace`.
All 800 checkpoints in each retain exactly the same frame, phase and pixel hash. Their
state hashes change because Blupi now uses its original 38-phase table: `CEvent::m_index`
is two lower than in the previous Eggbert superset. Dumps against the retained M2 port
executable at frames 10, 1000, 4000 and 7500 differ only in that field. Audit evidence:
`scratch/w-portm3/audit_baselines.py`, `build/w-portm3/baseline-audit/`.
Eggbert's existing attract traces and all SB1 golden files are unchanged.

[EVIDENCE] Focused tests also check restoration from a real 57,008-byte user save after a
short walk, read/cache failure recovery, and the 41-tick folder retry. A test-only phase
probe exercises Eggbert's optional 61-tick intro transitions with copies of the title image
as placeholder art in the test user root (the shipped `_INTRO=0` folder has no publisher
images); Blupi rejects those phases even with those files present. This does not establish
the publisher screens' historical pixels. No original executable was run.

## SB2 cross-build capture

`tools/oracle_capture_sb2.ps1` uses `blupi2-sdl3` and the edition's validated source
relink. It does not change SB1's `oracle_capture.ps1` defaults. The default data
folders are `assets/eggbert2` and `assets/blupi2`, selected from `targets.json`.
The default rebuilds are `build/sb2gate/relink-rsrc-eggbert-en/Eggbert2.exe` and
`build/sb2gate/relink-rsrc-blupi-en/BLUPI.EXE`; override with `-BuiltExe` for a fresh
relink. The script requires sibling `gate.json` with `status=PASS` and a successful
`build.json` for the selected target, including source-built resources. It refuses
original executable hashes, including renamed originals from the target catalog.

To create a reference from already verified objects (no game execution):

```powershell
.\.pyenv\python.exe tools\relink.py --target sb2/eggbert/en --objects-dir build\verify-sb2-eggbert-en\objects --out-dir build\oracle-relink-eggbert
.\.pyenv\python.exe tools\relink.py --target sb2/blupi/en --objects-dir build\verify-sb2-blupi-en\objects --out-dir build\oracle-relink-blupi --historical-cvtres-only
```

Use the output only if its gate is PASS; a sound build-path gate may require the
supervisor's normal historical-path environment. Supply the resulting executable
with `-BuiltExe`. No original executable is launched or copied into the run folder.

[EVIDENCE] Startup indexing, from `games/sb2/src/blupi.cpp` and `pixmap.cpp`:
`DoInit` calls `CacheAll(TRUE)`, which draws and displays `init.blp` once (index 0).
`CPixmap::Benchmark` then performs blits only, with no Display. After DoInit,
the application `Benchmark` performs ten UpdateFrame/SetDecor/Display iterations
(indices 1..10). The first timer update is display index 11. Thus `-Frames N`
means **N displays**, and the port runs `--frames (N - 11)`. This also matches the
proxy's existing scripted-input offset: input timer frame F is posted at display
F+10 and affects display F+11. CacheAll during a later phase can add displays on
both sides; this is why the logs use display indices, not timer ticks.

The port captures the reference device's raw back buffer at each game Display,
before presentation. Both games use `common/render/display_capture.cpp`: the
existing SB1 hash seed/multiplier, `index bpp 16-digit-hash` log, and BLPF dump
format (including the RGBX palette in 8-bit mode). The capture script explicitly
selects `--reference`, so it compares the historical SetDecor/Build path.

The isolated config is byte-identical for both sides. `Benchmark=6666` enables
SB2's color options; `TrueColorBack` and `TrueColorDecor` both select the requested
depth. `MouseType=2` removes the desktop-dependent graphic cursor. `CDAudio=0`
avoids CD audio. `-nocd` bypasses Blupi 2's drive-type check. Eggbert 2's
`AddCDPath` prefixes the configured folder directly, so it gets `CD-Rom=.` (the staged
installation folder). Blupi 2's keeps SB1's CD layout: `data\`, `image*\` and `movie\`
lie one level above the configured folder (the shipped config.def says `CD-Rom=H:\US`),
so it gets `CD-Rom=US` and an empty `US` folder in the copy. [EVIDENCE] With `CD-Rom=.`
the Blupi 2 rebuild's `CheckWorld1` found no `..\data\world001.blp` and showed the
insert-CD screen at the first locked world (display 81 of the tour and editor runs);
the port has no CD check. Timer=50 and SpeedRate=1 use game defaults.
The complete config fits in ReadConfig's 199-byte read. cnc-ddraw comes from the
edition's folder and is renamed `cnc-ddraw.dll` behind the same 32-bit proxy.

Preparation only (copies data, builds the proxy if needed, validates and writes
`compare.json` with `status=PREPARED`; **launches neither game**):

```powershell
powershell -ExecutionPolicy Bypass -File port\tools\oracle_capture_sb2.ps1 -Edition eggbert -ColorDepth 16 -PortExe build\port-oracle-make\blupi2-sdl3.exe -PrepareOnly
powershell -ExecutionPolicy Bypass -File port\tools\oracle_capture_sb2.ps1 -Edition blupi -ColorDepth 8 -PortExe build\port-oracle-make\blupi2-sdl3.exe -PrepareOnly
```

Supervisor's windowed captures (one at a time; do not touch mouse/keyboard):

```powershell
powershell -ExecutionPolicy Bypass -File port\tools\oracle_capture_sb2.ps1 -Edition eggbert -ColorDepth 16 -Frames 2200 -PortExe build\port-oracle-make\blupi2-sdl3.exe
powershell -ExecutionPolicy Bypass -File port\tools\oracle_capture_sb2.ps1 -Edition eggbert -ColorDepth 8 -Frames 2200 -PortExe build\port-oracle-make\blupi2-sdl3.exe
powershell -ExecutionPolicy Bypass -File port\tools\oracle_capture_sb2.ps1 -Edition blupi -ColorDepth 16 -Frames 2200 -PortExe build\port-oracle-make\blupi2-sdl3.exe
powershell -ExecutionPolicy Bypass -File port\tools\oracle_capture_sb2.ps1 -Edition blupi -ColorDepth 8 -Frames 2200 -PortExe build\port-oracle-make\blupi2-sdl3.exe
```

`-InputScript port\tests\golden\sb2_tour.input` (`-Frames 201` for Eggbert 2, `241`
for Blupi 2: the tour quits the game) or `sb2_editor.input` (`-Frames 260`) adds menu navigation (click and
key/keydown/keyup only; unsupported input is rejected). Each run lives under
`build/oracle/sb2-<edition>-<timestamp>/`. `compare.json` records executable,
gate/build, port, proxy, cnc-ddraw and config hashes, arguments, exit status,
frame counts and first difference. Status is MATCH only for a complete successful
capture with every requested display equal; incomplete runs remain INCOMPLETE.
`-DumpEvery 0` disables raw dumps. Use `oracle_diff.py RUN_DIR` as for SB1.

### SB2 results (2026-10-24, relinks from `build/sb2gate/relink-rsrc-<edition>-en`)

| Run | Eggbert 2, 16 bpp | Eggbert 2, 8 bpp | Blupi 2, 16 bpp | Blupi 2, 8 bpp |
|---|---:|---:|---:|---:|
| Attract mode (2200 displays) | 2200/2200 | 2200/2200 | 2200/2200 | 2200/2200 |
| `sb2_tour.input` (201 / 241 displays) | 201/201 | 201/201 | 241/241 | 241/241 |
| `sb2_editor.input` (260 displays) | 260/260 | 260/260 | 260/260 | 260/260 |

[EVIDENCE] Every run is MATCH: the port's reference renderer and the shared SB2 code
reproduce both rebuilt editions' back buffers bit for bit in both colour depths, through
attract-mode demo playback, the menus, world selection, the editor and the quit screens.
This is runtime evidence about the rebuilt executables (whose code is STRICT or certified
EQUIVALENT, docs/SB2.md), not a strict/equivalence proof. No timing race was seen.
