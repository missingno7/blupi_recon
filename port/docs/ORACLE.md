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
2. **Message order** (`port/game/main.cpp`, host loop). Queued messages, and those they post
   (a button's command after `WM_LBUTTONUP`), are now handled before the timer's `WM_UPDATE`, as
   the original message loop did. Before, a click took effect one frame late.
3. **TranslateMessage** for posted key presses. The original loop generated `WM_CHAR` for them;
   the port now does the same for scripted keys.

Also confirmed: 16 bpp loading converts by truncation to RGB565 (former [HYPOTHESIS] in
`ARCHITECTURE.md`).

## Not covered yet

- **Graphic cursor (MouseType 1):** its blits are the same code paths as icons, but not compared
  under this setup.
- **Not visited by the tour:** music and sound state (not in frames); the mission editor; save
  and load; later demos (DEMO202–206: the runs stop at frame 2200, while the golden traces
  cover them on the port side only).
- **Multiplayer.**
