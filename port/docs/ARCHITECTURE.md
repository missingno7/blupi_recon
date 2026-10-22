# SDL3 port — architecture notes

Findings are marked [EVIDENCE] (checked in code, data or by a run), [HYPOTHESIS] (plausible,
unverified) or [OPEN]. Paths without a prefix are historical sources in `src/`.

## 1. Integration strategy: native adapter, not DirectX emulation

Evaluated for the first build (2026-10-21): Free Direct (DirectX 3 2D subset over SDL3, MIT,
~4.5k lines), Free API (WinAPI subset over SDL3, MIT, ~7.7k lines) and how Free Eggbert (GPLv3,
SB2) uses them.

[EVIDENCE] Reasons not to adopt them as the bridge for SB1:

- Free Eggbert still edits game sources to run on them (ddutil 32-bit colour-mask fix, about 200
  lines of `sound.cpp`, a throttle in `blupi.cpp`). We must not edit `src/`.
- Free Direct surfaces are effectively 32 bpp only: the requested display depth is ignored and
  blits between depths copy nothing. SB1's two modes are 8 bpp palettized and 16 bpp.
- Colour-key ranges are compared on packed 32-bit values, not on the display format.
- They ship `windows.h`, `ddraw.h`, `io.h`, `direct.h` shadowing system headers globally (the
  Free Eggbert CMake warns about breaking libstdc++), need C++20, and run `timeSetEvent`
  callbacks on SDL's timer thread.
- Several calls succeed without doing anything (cursors, resource `LoadImage`, AVI), which would
  hide missing work.

What SB1 actually needs is small. [EVIDENCE] Outside the 7 platform TUs, the shared game code
uses: about 20 window messages and VK codes, `PostMessage`, cursor position and visibility,
rectangle helpers, `LoadString`, `fopen`/`_findfirst`/`_mkdir`/`DeleteFile`, `rand`, the winmm
joystick calls, and the DirectX *type names* in the class declarations (`CPixmap`, `CSound`,
`CNetwork`). It never calls DirectDraw, DirectSound or DirectPlay itself.

So the port keeps the historical class interfaces and replaces their implementations:

- `common/compat/include` declares names only (types, SDK constant values, function prototypes). The
  DirectX interface types are opaque; the port's `CPixmap`/`CSound` define their bodies.
- `common/compat/src/win32.cpp` implements the ~30 functions, each with the semantics the game relies
  on. This is the only Win32-shaped code. It shrinks as callers move to port services.
- `port_forced.h` (forced include) maps `fopen` to the port file system and `rand`/`srand` to the
  MS LIBC generator. It includes `<cstdio>`/`<cstdlib>` first, because libstdc++'s `<cstdio>`
  `#undef`s `fopen` and would silently drop the redirect in any TU that includes a C++ header
  later. [EVIDENCE] This bug happened in the first build.

Ideas taken from Free Direct and Free API, without copying code: colour-key ranges
compared in the display format, and nearest-neighbour stretch for `DrawMap`. Music uses the same
synthesizer as Free API (TinySoundFont headers vendored from its `external/`), but with the port's
own player (section 8).

## 2. x86 versus x64

[EVIDENCE] x64 is the first target and works:

- On-disk structures contain only `short`, `int`, `BOOL`, `char` and `POINT` (two `LONG`s), no
  pointers. The compat headers define `LONG` as `int32_t`, so sizes are identical on x86, x64
  and LP64. `tests/test_layout.cpp` asserts the historical sizes.
- One pointer/integer pun: `CEvent::LoadState` writes a `BOOL` through `(BOOL*)&pEvent` and
  reads `(int)pEvent`. On little-endian x64 the low 32 bits round-trip; it compiles with
  `-fpermissive` and `-fno-strict-aliasing`.
- `LPARAM`/`WPARAM` are pointer-sized; mouse coordinates use `LOWORD`/`HIWORD` like Win32.

The game also builds with `-fwrapv`, keeping the wrap-around signed arithmetic the 1998
compiler produced in practice.

## 3. Renderer: `CPixmap` today and tomorrow

### What the game does with `CPixmap` [EVIDENCE]

- Fifteen image "channels" (`m_lpDDSurface[ch]`):
  - 0 background screen;
  - 1 objects, 2 and 11–13 Blupi variants, 9 explosions and 10 elements (these four use the
    per-icon rectangles in `pixtables.h`);
  - 3 decor background (tiled 2×2 with parallax at 2/3 of the camera speed);
  - 4 buttons, 5 gauges, 6 text font, 7 little font;
  - 8 minimap;
  - 14 a 64×64 scratch surface (`CHTEMP`).

  Each channel is an 8 bpp `.blp` BMP converted to the screen format at load. The background
  and decor may be 24 bpp files from `IMAGE16`.
- Draw calls:
  - `QuickIcon`, `DrawIcon`, `DrawIconPart` and `DrawPart` are source-colour-keyed or opaque
    `BltFast` blits, clipped by `CPixmap` to `m_clipRect`.
  - `DrawImage` is an opaque full-rectangle blit.
  - `DrawMap` stretches the back buffer into the minimap surface (channel 8).
- Render-to-surface: the "hide" power-up composes into `CHTEMP`:
  1. draw the mask icon with the key temporarily switched to red;
  2. draw Blupi opaque;
  3. draw the second mask;
  4. `QuickIcon` the result (`decblock.cpp:1335-1338`).

  `BuildIconMask` exists but is not called by the shared game code.
- CPU pixel reads: `IsIconPixel` (GDI `GetPixel`) is **not called** by the game code, so no
  read-back path is needed today. `DrawMap` reads the back buffer, which is a read-back on a GPU.
- Software mouse (MouseType 1):
  - the sprite is drawn into the back buffer;
  - the pixels under it are saved to and restored from a 64×64 surface;
  - between frames, `MouseUpdate` copies small rectangles straight to the screen (`MouseQuickDraw`).
- Colour keys: white at load, then blue `RGB(0,0,255)` per channel. Mask drawing switches the key
  to `0x0000ff` (red) and restores it with `SetTransparent2`.

### The device seam (implemented, roadmap stage 1)

`CPixmap` (`games/sb1/game/pixmap.cpp`) does not touch pixels. A surface handle (`IDirectDrawSurface`) is a
registry entry holding:
- the surface id: image channels 0–99, back buffer 100, screen 101, saved mouse background 102;
- the size and format;
- the colour key in force;
- the image as loaded, for CPU queries.

The historical method bodies clip as before. `blt_fast`/`blt_stretch` then apply DirectDraw's
validity rules (`render::blit_valid`/`stretch_valid`): an invalid rectangle fails and draws
nothing. Valid operations are submitted to a `render::Device` as self-contained commands:
- Create, Destroy;
- Upload of a converted image;
- Palette;
- Blit with its colour key;
- Stretch.

The software reference device executes them. Any later backend (GPU) only has to execute the
same commands; it never makes a DirectDraw decision.

### The reference renderer (implemented)

- `render::Surface` holds 8 bpp indices or RGB565 words, the two exclusive-mode formats the game
  selects (`TrueColor` / `Benchmark>=3100` → 16 bpp).
- `blit` follows DirectDraw: a source or destination rectangle outside its surface fails without
  drawing (`DDERR_INVALIDRECT`); the key is an inclusive raw-pixel range.
- `stretch_blit` is nearest-neighbour.
- Loading converts the way GDI did:
  - 8 bpp: nearest palette entry, exact matches first, with palette entries 0 and 255 forced to
    black and white (`DDPCAPS_8BIT` without `DDPCAPS_ALLOW256`);
  - 16 bpp: truncation to 565.

  [EVIDENCE] Both rules are confirmed bit for bit against the historical rebuild by the
  cross-build oracle ([ORACLE.md](ORACLE.md)).
- Colour keys go through the same conversion (`DDColorMatch`). The screen palette is the one
  loaded with `bUsePalette` and is applied only at presentation, like hardware.
- `CPixmap` method bodies are the historical ones with `BltFast`/`Blt` replaced.
- The primary surface is the "screen". `Display` and `MouseQuickDraw` copy into it, and the host
  presents it once per loop iteration.

[HYPOTHESIS] The original windowed mode used the desktop format (32 bpp on modern systems). The
port always uses the exclusive-mode formats, which is what full-screen players saw.

### Toward a GPU renderer (later)

- Keep `CPixmap`'s interface as the seam. Channels become textures (or atlas regions, since the
  icon rectangles are already tables). The colour key becomes alpha at load time, because keys
  only change for the mask trick.
- `CHTEMP` composition and `DrawMap` need render targets. The minimap can instead be drawn from
  the world at small scale, avoiding the back-buffer read.
- The software renderer stays as the reference for pixel comparisons, and is the
  `--headless` backend.
- The game never queries pixels, so no read-back path is needed.

## 4. Timing and simulation coupling

[EVIDENCE] Historical loop (kept exactly in `games/sb1/game/main.cpp`):

- `timeSetEvent(Timer)` (50 ms) fires `TimerStep`. `TimerStep` posts a single `WM_UPDATE`
  while none is pending (`g_timer` gate), so missed ticks are dropped rather than queued.
- `WM_UPDATE` runs `UpdateFrame`, then `SetDecor`, then `Display`.
- `UpdateFrame`:
  - `ReadInput`;
  - `speed` × (`MoveStep` + `DemoStep`), where `speed = GetSpeed()*SpeedRate`, 1 or 2
    (`SetSpeed` clamps to 2);
  - phase transitions.
- `SetDecor` calls `CDecor::Build` (or draws the static background), then `DrawButtons`.
- Window deactivation stops the updates (`g_bActive`).

Rendering mutates game state:

| Where | Mutation | Consequence for high refresh rates |
|---|---|---|
| `CDecor::Build` (`decblock.cpp:1539`) | `m_time++` once per drawn frame (not per `MoveStep`) | `m_time` drives gameplay: `decblupi.cpp` uses `m_time % N` for timers and behaviour, net packets carry it, saves store it. Drawing more often would speed up the game |
| `CDecor::DecorNextAction` (called by `Build`) | advances `m_decorPhase` and clears `m_decorAction` (camera shake sequences) | shake would run faster and is saved state |
| `CDecor::Build` | sets `m_blupiSec` (shield, power, cloud, hide) | sent in network packets; derived from simulation flags |
| `CPixmap` mouse save/restore | back-buffer bookkeeping | presentation only |

The camera (`m_posDecor`) is simulation state updated in `BlupiStep` (scroll margins
`SCROLL_MARGX/Y`). The shake offset is added at draw time.

So the port renders exactly once per logical frame, like the original. The full staged plan,
including two more draw-time mutations found later (the mission-10 clamp in `DrawInfo`, the
button/jauge flags) and the camera dependence of the pickup "voyages", is in
[ROADMAP.md](ROADMAP.md). Its first outline was:

1. Move the three `Build` mutations into a per-frame "end of tick" step called from
   `UpdateFrame` order-equivalently. This needs a port-side copy of `CDecor::Build`/
   `DecorNextAction`, a `decblock.cpp` fork documented as derived.
2. Make `Build` a pure function of a snapshot (camera, actor positions, icons, `m_time`).
3. Keep the 50 ms fixed step. Present at display rate, interpolating camera and actors between
   the last two snapshots. Snap on teleports, phase changes and level loads (detect from
   `ChangePhase` and position jumps larger than a threshold).

`m_time` must stay "frames drawn at logical rate" to keep demos and saves compatible.

**Implemented (2026-10-21; [ROADMAP.md](ROADMAP.md) stages 4, 5 and 7).** The outline holds,
with one change: `decblock.cpp` stays shared and unmodified.
- `games/sb1/game/scene.cpp` builds the scene from const state.
- `games/sb1/game/end_of_tick.cpp` makes `Build`'s state changes at `Build`'s point and in its
  order.
- `DrawButtons` still runs once per tick, its drawing captured.

A tick therefore produces a frame as data. The host loop presents it at the display rate,
optionally interpolated between the last two scenes (`--interpolate`, visual only). The
simulation still runs one `UpdateFrame` + `SetDecor` per 50 ms tick, and every mode keeps the
golden state traces.

## 5. Widescreen assumptions

[EVIDENCE] Uses by TU (counts): `LXIMAGE`/`LYIMAGE`:

- event 40/9: button layout and text placement;
- decblock 12/12: tile culling `posDecor/DIMOBJ … + LXIMAGE/DIMOBJX + 3`, the decor parallax
  tiling over `DIMDECORX/Y` = 640×480, and the HUD;
- decblupi 2/3: scroll limits;
- sound 6/4: stereo pan and volume falloff are computed against the 640-wide screen;
- pixmap 8/8: mouse clipping;
- camera clamps (`MAXCELX*DIMOBJX - LXIMAGE`): the camera centring in `decblupi.cpp:3713-3715`,
  `DecorNextAction`, and the designer scrolling in `MoveStep`;
- decdesign 1: `ScreenPosToCelPos` screen bounds;
- decmove 1: the notification sound plays at the screen centre.

Plan:

- Introduce a port-side logical viewport size (`viewW` × 480) for gameplay drawing: tile loop,
  camera clamp, parallax tiling (draw three columns when `viewW > 640`), sound pan.
- Keep the menus and HUD on a centred 640×480 layer.
- All of these live in `CDecor` methods, so widescreen needs port-side variants of `Build`,
  `BlupiStep`'s camera code and `CSound::PlayImage`. Simulation coordinates stay untouched.

[OPEN] Whether a wider view changes gameplay. Enemies activate by distance or visibility in some
SB titles; check `decmove`/`decblupi` for `LXIMAGE`-based activation before allowing a wider
view in "faithful" mode.

**Implemented differently (2026-10-21; [ROADMAP.md](ROADMAP.md) stage 8).** The wider view is
presentation only, so the question above no longer affects it. No `CDecor` method has a
variant: the simulation camera, camera clamps, sound pan and activation code are untouched,
and the game still draws its 640×480 frame. The view renderer (`DrawView`) draws the same scene
for a wider view, evaluating `Build`'s own tile ranges and bounds tests at that view. HUD and
UI stay in the centred 640×480 frame, and menus and the editor are shown centred. Things in the
extra width behave exactly as they would off-screen in the original: what the original
simulates there, the player now sees.

## 6. Demo system as the regression harness

[EVIDENCE] `CEvent::DemoRecStart/Stop`:

- Recording starts when a player starts worlds 200–219 (`ChangePhase`).
- The file `data\demoNNN.blp` is a `DemoHeader` (204 bytes, majRev 1) followed by `DemoEvent
  {short time; short input}`.
- `input` is the `m_keyPress` bitmask (KEY_* bits), stored whenever it changes; `time` is the
  `DemoStep` counter (one per `MoveStep`).
- Playback (`DemoPlayStart`) does three things:
  1. reads world `200+n`;
  2. calls `InitRandom` (`srand(1)`), `SetTime(0)` and `speed=1`;
  3. feeds `KeyChange` from the events.

  The attract mode starts after `DEF_TIME_DEMO` = 1000 idle frames on the title screen and
  chains `DEMO200…206`.

So playback is deterministic given identical code, `rand` and frame pacing. The mouse is not
recorded.

[EVIDENCE] The port reproduces itself exactly (two runs, 3000 frames, identical per-frame screen
hashes, `ctest -R replay`).

[OPEN] Comparison with the historical build. The historical rebuild can run the same attract
mode under cnc-ddraw, but its frames are tied to wall-clock timing, so a frame-indexed capture
needs a hook outside `src/` (for example in the cnc-ddraw `Blt` path, or a debugger breakpoint on
`CPixmap::Display`). Prefer that to any change to the historical source.

## 7. Portability notes

- Paths: historical code uses relative `data\…`, `image08\…`, `\User\…` with backslashes and
  lower case; the data is upper case. `common/platform/vfs.cpp`:
  - splits on both separators;
  - resolves case-insensitively;
  - drops drive letters;
  - sends writes to the user root and reads the user root first.

  The CD-path logic is bypassed (`g_CDPath` stays empty, as for a hard-disk install), so
  `AddCDPath` is a no-op and `AddUserPath` keeps names relative.
- Saves: `CDecor::Write/Read`, `CurrentWrite/Read` and `CEvent::WriteInfo/ReadInfo` use raw
  `fwrite(sizeof struct)`. Layout is preserved (section 2). Endianness is little only. A
  big-endian target would need explicit serialization; none is planned.
- Text: strings and fonts are cp1252 byte codes; the string table is the historical `.rc`
  embedded verbatim. Text input converts UTF-8 to cp1252.
- Input: SDL keycodes map to VK codes in `common/platform/host.cpp`. Messages are synthesized in Win32
  order (`WM_KEYDOWN`, then `WM_CHAR` for Backspace/Enter/Escape/Tab; `WM_NCMOUSEMOVE` over the
  letterbox).
- Stdio in the shared TUs: `fopen/fread/fwrite/fclose` on `BlupiFile`
  (`common/compat/include/port_hist_stdio.h`, forced only for those TUs). It carries the VC5
  `_flag`/`_IOERR` bits that `CEvent::CopyMission` reads, so the historical code compiles
  against any C library. The port's own TUs keep the real `FILE`.
- Reads outside static tables made by the original are reproduced with BLUPI.EXE's neighbouring
  values (`games/sb1/game/pixmap.cpp`, `docs/ORACLE.md`), so they do not depend on a compiler's data
  layout.
- Android (later):
  - storage: SDL pref path for the user root; the game data, supplied by the player, imported
    once into app storage so the file-based `vfs` works unchanged;
  - lifecycle: map to `WM_ACTIVATEAPP`;
  - touch: synthesize mouse and key messages;
  - ARM64: the struct layout notes above apply unchanged.

## 8. MIDI music and the MCI notification flow

[EVIDENCE] Historical flow (`src/sound.cpp`, `src/blupi.cpp`):

- `PlayMusic` sets the midiOut volume from a 21-entry table, then opens an MCI "sequencer"
  device for the file and plays it with `MCI_NOTIFY` to the window.
- `SuspendMusic` closes the device, unless `m_nbSuspendSkip` swallows the call. It skips the
  close when the MIDI volume is 0.
- `StopMusic` is `SuspendMusic` plus forgetting the file name.
- `RestartMusic` reopens the remembered file.
- The window procedure handles `MM_MCINOTIFY` (not for movies) by calling `SuspendMusic()`.
  Then, if `wParam == MCI_NOTIFY_SUCCESSFUL`, it calls `RestartMusic()`. This is the loop.
- Callers:
  - `ChangePhase`: `StopMusic` on most phase changes, `PlayMusic` when entering play with a
    world music number, `AdaptVolumeMusic` when music is remembered;
  - pause: suspend/restart;
  - window deactivation and activation: suspend/restart;
  - options volume buttons: suspend, then `AdaptVolumeMusic` on return;
  - the first demo start (`DemoPlayStart`).

Port (`games/sb1/game/sound.cpp` + `common/platform/music.cpp`):

- The historical method bodies are kept. MCI open+play becomes `music::play`, which returns a
  non-zero "device id" per song. MCI close becomes `music::stop`.
- Music volume is the game's own option, unchanged:
  - `SetMidiVolume` (0–20) is called by the options buttons, together with `SuspendMusic`.
  - `InitMidiVolume` maps it through the historical `midiOutSetVolume` table. The port applies
    the table's left-channel word linearly as the synthesizer level ([HYPOTHESIS] for how the
    1998 device scaled it).
  - `AdaptVolumeMusic` on the return to play applies the new level and restarts the song.

  `tests/test_music_volume.cpp` replays that sequence through `CSound`. It checks level ratios
  equal to the table (10 → 0xAAAA, 15 → 0xF222) and the historical volume-0 quirks:
  `SuspendMusic` skips the close at volume 0 but forgets the device id, so that song is never
  closed. Raising the volume again then restarts the remembered file.
- The synthesizer runs at a fixed -6 dB. This stands in for the unknown output level of the
  1998 Windows MIDI device relative to DirectSound; it is not a user setting. It keeps
  GeneralUser GS at full option volume below clipping.
- The SoundFont is GeneralUser GS v2.0.3, the shallow git submodule `third_party/generaluser-gs/`
  (the author's repository at a pinned commit; same file Free Eggbert ships), copied next to the
  executable by the build.
- TinySoundFont renders inside the SDL3 audio callback, after the sound effects.
- The end of the sequence (last MIDI event reached) is noticed by `PortSoundPoll` on the main
  loop. It posts `MM_MCINOTIFY(MCI_NOTIFY_SUCCESSFUL, id)` to the window queue, just as MCI
  posted it.
- Closing a song that has not finished, while its notification is pending, posts
  `MM_MCINOTIFY(MCI_NOTIFY_ABORTED)`.

  [EVIDENCE for this choice] The historical code calls `SetSuspendSkip(1)` right after
  `StopMovie()` (an `MCI_CLOSE` of a playing AVI). That only makes sense if the close posts a
  notification that would otherwise reach `SuspendMusic` and close the music the phase change
  just started. The MCI documentation describes `MCI_NOTIFY_ABORTED` for exactly this case.

  [OPEN] Consequence, faithful to MCI: if one phase change both closes a playing song and opens
  a new one (play → play with music in both worlds), the late `ABORTED` closes the new song.
  [EVIDENCE] In the shipped data every hub world (001, 020, 030, …, 070) has music 0. Music is
  only started when entering missions, so this case does not occur in normal play. Confirming it
  on the historical build would need its MCI behaviour observed.
- Headless runs advance the synthesizer by one timer interval per game frame (output
  discarded), so music timing and notifications are deterministic in tests. Replay tests run
  without a SoundFont.
- One song plays at a time. MCI may have kept a second song running when the game opens a new
  device without closing the old one. The volume-0 quirk above is such a path: after volume
  0 → >0 the forgotten song continues and a new one is opened. [OPEN] Whether the 1998
  sequencer played both or refused the second open (MIDI port in use). The port replaces the
  old song with the restarted one.
