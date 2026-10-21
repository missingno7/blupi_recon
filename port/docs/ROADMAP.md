# Roadmap: from faithful port to native rendering, interpolation and widescreen

Goal: a fully native, portable engine with faithful simulation, independent presentation,
optional interpolation and true widescreen. Interpolation and widescreen should follow from the
architecture instead of needing another rendering rewrite. Each stage replaces one piece of the
faithful baseline and must reproduce its results exactly before the next stage starts. Faithful
mode stays the default and the reference throughout.

Direction agreed with the user (2026-10-21):
- **GPU is optional.** GPU rasterization is not a requirement. A native software renderer with
  SDL3 presenting through the GPU is the target. The rendering abstraction (device commands,
  semantic scene) stays backend-neutral so an SDL_GPU backend can be added later without
  redesigning the game or the scene; no speculative GPU infrastructure now.
- **Rendering never touches game state.** Simulation, rendering and presentation are fully
  separated, and rendering must work at any display rate.
- **Historical order is preserved.** The order of side effects stays exactly historical
  (`m_time`, camera shake, button states, the mission-10 clamp). The scene shows the state the
  original `Build` would have drawn.
- **Two cameras.** The simulation camera and the rendering camera are independent. Widescreen
  shows more world without stretching and without changing gameplay.
- **Interpolation is visual only.** It uses stable entity identities and simulation snapshots,
  with no extra ticks and no interpolated gameplay state.
- **The software renderer stays the reference,** and all equivalence gates stay in place.
- **Missing coverage is a gap, not evidence.** Paths without test coverage get targeted coverage
  before the rendering code they use changes.
- **Order:** stage 4, then 5, then interpolation and widescreen. Android is a separate platform
  track.

Status markers: [EVIDENCE] verified in code or by a run, [OPEN] to be settled by the stage that
needs it. Source paths are historical (`src/`).

## Shortcuts we will not take

- Drawing the existing 640×480 scene more often than once per 50 ms tick. `Build` mutates
  game state (below), so the game would run faster.
- Stretching or cropping the 640×480 image for widescreen.
- Interpolating by changing movement, physics or camera code, or by adding sub-tick simulation
  steps.
- Changing simulation behaviour to make a renderer simpler: scroll speeds, the timing of the
  flying pickup icons, clamps, `m_time`.
- Editing `src/`. Port code observes `CDecor` through a pointer-to-member accessor declared in
  a derived struct. This is standard C++ and works without `#define protected public`.
  [EVIDENCE] It compiles against `decor.h` today.
- GPU read-backs at run time. The game never queries pixels: `IsIconPixel` and `BuildIconMask`
  are unused. The minimap reads the back buffer, which a render target handles.
- Declaring a stage done because it "looks right". Every stage has a mechanical equivalence gate.

## Facts the plan rests on

[EVIDENCE] from the source and the port's runs:

1. **Drawing mutates state.**
   - `CDecor::Build`:
     - increments `m_time` once per drawn frame (`decblock.cpp:1539`); `m_time` drives gameplay
       timers, `decblupi.cpp` uses `m_time % N`;
     - advances the camera-shake sequence (`DecorNextAction`: `m_decorPhase++`,
       `m_decorAction = 0`);
     - sets `m_blupiSec`.
   - `DrawInfo` contains gameplay: in mission 10 it clamps Blupi's position (`m_blupiPos.x` to 212
     or 788 depending on `m_nbTresor`, `decblock.cpp:1601-1604`).
   - `CEvent::DrawButtons` updates button enable and state flags and the text-input cursor blink.
   - `CButton::Draw` and `CJauge::Draw` clear their redraw flags.
   - `CDecor::DrawMap` (phase entry only, not per tick) reloads images, resets the play state,
     calls `Build` and rewrites Blupi and network fields.
2. **The simulation camera is gameplay state.**
   - It is computed in `BlupiStep` (`m_posDecor = GetPosDecor(m_scrollPoint)`,
     `decblupi.cpp:3687`; centring and clamping by 640×480 at `decblupi.cpp:3713-3725`).
   - The flying pickup icons ("voyages") start at the object's screen position relative to that
     camera (`decblupi.cpp:3044-3049`). Their duration is the screen distance / 10
     (`decor.cpp:35`), and the treasure, egg and character counters rise only when they land
     (`decor.cpp:79-90`). So the moment a treasure counts, and therefore when a door opens,
     depends on the 640×480 camera.
   - Sound pan and volume use the camera too.
   - Nothing is culled, activated or spawned by visibility. Move objects are only *drawn* when
     within the screen ±64.
   - `LYIMAGE` doubles as the world height when `m_dimDecor.y == 0` (`decblupi.cpp:3556`).
3. **Draw passes in `Build`, in order:**
   1. parallax background (2/3 camera speed, 640×480 image tiled 2×2);
   2. big decor tiles;
   3. editor markers;
   4. network Blupis;
   5. Blupi behind the decor (the "hide" effect composes into the 64×64 `CHTEMP` surface with
      mask keys);
   6. move objects;
   7. decor tiles;
   8. lifts;
   9. animated overlay tiles;
   10. move objects of types 8–11;
   11. Blupi in front;
   12. `DrawInfo` (HUD: screen-anchored, except the editor highlight);
   13. `VoyageDraw` (screen-anchored).

   Passes 1–11 are world-anchored, offset by the shaken camera.
4. **Shared code uses only a narrow CPixmap surface:** `QuickIcon` (40 call sites), `DrawIcon`
   (17), `DrawPart` (6), `SetClipping`/`GetClipping`, `Cache`, the mouse-sprite calls and
   `DrawMap`. `DrawImage` and `Display` come only from the main loop.
5. **Determinism tools exist:** frame-indexed traces, the original demos (attract mode), headless
   runs, and MS `rand`.

## Stage 0: oracles before changes

Every later gate compares against these.

- **0.1 State hash.** A port-side `DecorAccess` hashes the simulation state of `CDecor` every
  frame, by value and excluding pointers: camera, Blupi fields, move objects, cells, counters,
  `m_time`, voyages, shake. The same goes for the `CEvent` phase, demo and key state. Add it to
  `--trace`.
- **0.2 Golden corpus.** Generate per-frame traces (phase, state hash, screen hash) in both
  colour modes for:
  - the attract mode (all seven demos);
  - menu tours (`--input`);
  - one scripted pass through every in-game screen.

  Commit them as small text files, then turn them into ctest gates. From here on, any change
  that alters a trace is a regression, unless a stage explicitly expects it (none below does
  in faithful mode).
- **0.3 Cross-build oracle.** Capture the historical rebuild's frames without touching `src/`.
  Run it with a small `ddraw.dll` proxy that hashes the back buffer at each primary `Blt` and
  logs the frame index. This is the only way to prove the port's *reference renderer* against
  the original and to settle the [HYPOTHESIS] items in `ARCHITECTURE.md` (GDI 565 truncation,
  colour-key matching).

  Demos make the frames comparable. [OPEN] Whether wall-clock pacing of the historical run lets
  frames be matched one to one. If not, compare state transitions and still images at known
  demo events.

**Status of 0.1–0.3 (2026-10-21): done.**
- **0.1:** `game/state_hash.cpp`, with a `--state-dump` per-field view for diagnosis. It
  uncovered uninitialised members, so the game objects are now constructed in zero-filled
  memory (`game/main.cpp`).
- **0.2:** `tests/golden/*` with ctest gates, plus a gate that the state does not depend on
  colour depth.
- **0.3:** `docs/ORACLE.md`. Display-index matching resolved the [OPEN] point. The results are
  bit-identical to the historical rebuild except the multiplayer provider list. Three port
  corrections came out of it: the 8 bpp palette entries 0/255, message order before the timer
  tick, and `WM_CHAR` for posted keys.

- **0.4 Linux build.** It is cheap now and keeps the code honest about portability: case-sensitive
  data lookup is done, and the forced header covers `rand`. Run the corpus on both systems.

## Stage 1: draw commands as the renderer's input

- `CPixmap` (port-owned) records every drawing call as a command instead of only rasterising:
  `Blit` (channel, source rect, destination, keyed or opaque, key variant), `SetClip`, target
  (back, `CHTEMP`, minimap), `Stretch` (minimap) and the cursor sprite.
- A `RenderBackend` interface executes a frame's command list. The existing software renderer
  becomes the first backend; `PortPixmapPresentIfDirty` goes through it.
- **Gate:** replaying the recorded list gives bit-identical frames to direct rasterisation for
  the whole corpus.

Nothing else changes. This also gives exact per-frame draw logs for debugging.

**Status (2026-10-21): done.** It was implemented slightly differently from the plan above,
for the GPU stage:
- **No clip command.** `CPixmap` keeps deciding everything DirectDraw decided from a CPU-side
  registry (surface sizes, formats, the colour key in force), so the device receives only
  valid, self-contained operations and never reports errors. Clipping stays in the historical
  `CPixmap` bodies, so no `SetClip` command is needed.
- **Implementation:**
  - `render/device.h` defines `Device` with the commands Create, Destroy, Upload, Palette,
    Blit (carrying its colour key) and Stretch, plus `SoftwareDevice`, `Recorder` and `replay`;
  - `game/pixmap.cpp` submits everything through it, including the screen, the back buffer,
    the saved mouse background, `CHTEMP` and the minimap stretch.
- **Gate** (`ctest -R commands`, `--check-commands`): every frame's commands, replayed on an
  independent device, give the same back buffer, screen and palette. Result: 7511 of 7511
  attract displays and the full tour, in 16 and 8 bpp. The golden traces are unchanged, so the
  refactor changed no pixel and no state.

## Stage 2: GPU backend at 640×480 (SDL_Renderer)

**DEFERRED (2026-10-21, user decision), not cancelled.** Reconsider GPU rendering only if
real profiling shows a performance or power-efficiency problem, especially on Android, or if a
future graphical feature genuinely needs it. The reasons for deferring: it is not needed for
interpolation, widescreen or Android, and doing it faithfully costs more than planned.
- **The software renderer is fast enough.** [EVIDENCE] Game logic plus full software rendering
  measure 0.5–0.7 ms per frame (headless, 7500 frames). A 144 Hz frame budget is 6.9 ms, and a
  wider viewport grows the cost only linearly. Window scaling is already done on the GPU by SDL.
- **Faithful colour keys need shaders.** Keys compare raw pixel values (palette indices in 8 bpp,
  RGB565 words in 16 bpp). Some keyed sources are drawn into at run time: `CHTEMP` in the hide
  effect, and the minimap. `SDL_Renderer` cannot evaluate a raw-value key on the GPU.
  - A faithful backend therefore needs `SDL_GPU` with shaders: raw-value textures, keying in the
    fragment shader, palette expansion at present. That adds a shader toolchain to the build.
  - The plan below (`SDL_Renderer`, keys turned into alpha at upload) is only exact for static
    sources.
- **Agreed order:** stage 4, then 5, then 7 and 8 on the software renderer. A GPU backend
  comes back only if something needs it (HD effects, very large viewports), and then as
  `SDL_GPU` with raw-value shaders, gated by the same command replay.

- **API choice:** `SDL_Renderer` (SDL3's 2D API) rather than `SDL_GPU`. It has textures, render
  targets and nearest sampling, and runs on Direct3D, Vulkan, Metal and OpenGL ES (Android). It
  needs no shader toolchain. Nothing planned needs custom shaders; revisit only if one appears.
- **Channels:** one texture per channel image. The images already are atlases (the icon tables
  in `pixtables.h`), so no repacking is needed.
- **Colour keys become alpha at upload.** Key changes are rare and known (the mask trick keys
  channel 1 red). Each (channel, key) pair gets a lazily built texture variant.
- **8 bpp mode:** upload palette-expanded textures and rebuild them when the palette changes
  (`Cache` with `bUsePalette`). That covers every palette change the game makes; movie
  palettes come later with movies.
- **Render-to-surface:**
  - `CHTEMP` becomes a 64×64 render target;
  - the back buffer becomes a 640×480 render target that is presented;
  - the minimap (`DrawMap`) is a nearest stretch from the back-buffer target into the channel-8
    target.
- **Gate:** in tests only, read the GPU frame back and compare it with the software backend.
  It must be pixel-equal for the whole corpus, in both colour modes. [OPEN] Some drivers may
  differ in nearest-sampling edge rules for the minimap stretch. Integer-aligned blits are exact
  by construction; if the stretch differs, emulate it with per-row blits rather than accept a
  difference.
- **Result:** native GPU rendering with identical output; the software backend remains the
  reference and the headless backend.

## Stage 3: scene builder (semantic scene instead of blits)

- A port-owned `SceneBuilder`, derived from `CDecor::Build`, `DrawInfo` and `VoyageDraw` and
  documented as such, reads `CDecor` state through `DecorAccess`. It emits a scene with no side
  effects:
  - **World layer:** draw items in the historical pass order, each with a stable identity
    (Blupi, network Blupi n, move object index, tile cell, overlay cell, lift), a world position,
    channel/icon and key variant.
  - **Screen layer:** HUD, jauges, texts, the voyage icon.
  - **Camera:** the simulation camera plus the shake offset, kept separate.
  - **Viewport:** a parameter (640×480 in faithful mode).
- Flattening a scene at 640×480 produces draw commands (stage 1).
- **Gate:** for every corpus frame, the flattened scene's command list equals the list captured
  from the historical `Build` call by call (same order, rectangles, keys, clip). This proves the
  scene builder is a faithful re-expression of `Build`.

**Status (2026-10-21): done.**
- **Implementation:** `game/scene.h` and `game/scene.cpp`, derived from `Build`,
  `DecorNextAction`, `DrawInfo` and `VoyageDraw`.
  - **Purity:** state is read through pointer-to-member accessors bound to const references,
    and drawing goes to a recorder whose methods mirror `CPixmap`. Writing game state or
    reaching the real `CPixmap` does not compile.
  - **Annotation:** each item carries a layer (background, world, screen, target), an entity
    and index, and a world position. The scene also records the simulation camera and the
    shaken view.
- **Gate** (`ctest -R scene`, `--check-scene`): the scene is built before each `Build` call
  and must flatten to Build's exact commands. Result: 0 mismatches in 16 and 8 bpp over 6509
  attract calls, 310 tour calls and 200 editor calls.
- **New golden scenario:** `tests/golden/editor.input` (design screen, editor, test-play,
  pause) brings the editor paths into the gate, the golden traces and the oracle (370 of 371
  identical, see `ORACLE.md`).
- **Not reached by any gate yet:**
  - shield, cloud and hide effects (so the `CHTEMP` composition);
  - notification texts;
  - the mission-10 tutorial hint;
  - multiplayer.

  These parts are marked in `scene.cpp`.

## Stage 4: separate simulation from presentation

- A port-owned `EndOfTick()` performs, in historical order, the state changes that drawing made:
  - `m_time++`;
  - the shake step;
  - `m_blupiSec`;
  - the mission-10 clamp from `DrawInfo`;
  - the button and jauge flag updates of `DrawButtons`;
  - the text-cursor blink.

  It is written against `DecorAccess`, so `decblock.cpp` stays shared and unmodified. The
  ordering matters: the clamp happens after `MoveStep` and before the next tick's input, exactly
  as when it ran inside drawing.
- In native mode the main loop runs `UpdateFrame`, then `EndOfTick`, then takes a scene snapshot.
  The historical `Build` is no longer called. Reference mode keeps calling it.
- `CDecor::DrawMap` stays historical: it runs on phase entry, inside phase logic, and its state
  effects are part of that logic.
- **Gate:** native and reference mode produce identical state hashes and screen hashes for the
  whole corpus.
- **Result:** presentation is a pure function of an immutable scene snapshot.

**Status (2026-10-21): done.** Native mode is the default; `--reference` keeps the historical
`SetDecor` body.
- **Coverage first** (`game/probe.cpp`, `tests/golden/effects.*`). Test-only state probes,
  applied between ticks or just before drawing, reach the paths no demo or menu reaches:
  - shield (steady and blinking), power, cloud, and hide (`CHTEMP` composition);
  - Blupi in front of the decor;
  - notification texts;
  - the mission-10 clamp and tutorial hint (walking right);
  - four network Blupis with effects.

  The scene gate (440 `Build` calls) and the command gate pass on them before anything changed.
- **`EndOfTick`** (`game/end_of_tick.cpp`) makes, at the point where `Build` ran and in `Build`'s
  order, every state change the historical `Build`, `DecorNextAction`, `DrawInfo` and
  `VoyageDraw` make: the shake step, `m_blupiSec`, the jauge redraw flags, the mission-10 clamp,
  and `m_time`. The list comes from an audit of every member write in those functions.
- **The UI.** `CEvent::DrawButtons` (700 lines of per-phase UI logic) still runs once per tick
  at its historical point, so every button, enable and blink change happens in the historical
  order. Its drawing is captured as commands.
- **The frame.** Each tick produces a frame as data: the scene, or the captured background
  image, plus the captured UI commands. `PortRenderFrame` draws it without reading game state;
  jauges draw from a snapshot, not through `CJauge::Draw`.
- **Gates** (all in `ctest`):
  - every golden scenario passes in native mode, in reference mode (`*_reference`), and with
    every frame rendered three extra times (`*_rerender`);
  - the state hash now includes all jauge and button members, redraw flags included;
  - a mutation test (disable each `EndOfTick` step in turn) failed a gate every time;
  - the native editor session matches the historical rebuild on 371 of 371 frames.

## Stage 5: present at display rate

- The simulation keeps its 50 ms fixed step (the current accumulator and drop rule). The renderer
  presents the latest snapshot at every vsync.
- **Mouse cursor:**
  - becomes an overlay drawn at present rate from the current mouse position, with the sprite
    and hotspot the game chose;
  - the back-buffer save/restore and `MouseQuickDraw` path is retired in native mode;
  - [OPEN] whether to use an SDL hardware cursor built from the sprite. That gives lower latency,
    but must match the graphic cursor's hotspot table.
- **Gate:** frames sampled at tick boundaries equal the faithful frames, and the traces are
  unchanged.

**Status (2026-10-21): done.** `--present display` is the default; `--present tick` keeps the
vanilla behaviour of presenting once per tick.
- The host loop handles input every iteration. `PortPresentNow` (game/main.cpp) redraws the
  latest tick's frame, draws the cursor at the position the input handling has just set, and
  presents. It reads no game state and changes none. With vsync, `SDL_RenderPresent` paces the
  loop; without it, the loop waits for input for at most 4 ms (about 250 presents per second at
  most) and never past the next tick.
- The cursor uses the existing `MouseBackDraw` path on the redrawn frame. It therefore moves at
  display rate with the sprite and hotspot the game chose. The SDL hardware cursor question
  stays [OPEN].
- **Gates** (`*_present` in `ctest`): every golden scenario with the frame presented three extra
  times after each tick gives the same traces.
- [EVIDENCE] A bounded windowed run presented 1223 frames in 5.1 s (239 per second) over 100
  game frames, at about 0.5–0.7 ms per frame for logic and software rendering.

## Stage 6: Android (independent track, after 0.4; benefits from 2 and 5)

Rendering does not block Android, since the software path also presents through SDL_Renderer.
It fits best once the GPU path and present-rate cursor exist, because touch input belongs to the
presentation layer.

- **Data:** `vfs` reads through `SDL_IOStream` for APK assets; the user root is the SDL pref path.
- **Lifecycle:** map to `WM_ACTIVATEAPP`; the game already pauses and suspends music.
- **Touch:** taps synthesize the existing mouse messages. An on-screen d-pad, jump and fire
  synthesize the existing key messages. No new input path in game code.
- **ARM64:** layout asserts already cover saved files.
- **Gate:** the corpus traces are identical on device (headless mode where possible).

## Stage 7: interpolation (opt-in)

Built on stage 3's identities and stage 4's snapshots.

- Present `lerp(snapshot[n-1], snapshot[n], alpha)` for world positions and the camera, with
  `alpha` from the tick accumulator. Animation frames, icons and HUD stay discrete; they switch
  at tick boundaries as in the original.
- **Snap instead of blending** when:
  - an entity identity appears or disappears;
  - its position jumps beyond a threshold (teleporters, respawn, lift wrap);
  - the phase changes or a level loads;
  - a voyage starts.

  The camera shake is applied on top of the interpolated camera, still advancing per tick.
- **Gate:**
  - with interpolation on, `alpha = 1` frames equal faithful frames;
  - state traces are identical with interpolation on or off (it cannot touch simulation);
  - targeted visual checks on the corpus at 144 Hz pacing.

**Status (2026-10-21): done, opt-in (`--interpolate`).**
- **Scene coverage.** `BuildScene` takes a `Coverage`. With it, the tile loops and the
  move-object bounds tests also record the items `Build` would draw at a view shifted within
  the margin, flagged `extended`. `FlattenScene` skips them, so the faithful frame is unchanged
  (`scene_*_interp` runs the scene gate with the coverage on).
- **Build's visibility rules travel with the items.** Each world item carries its rule: the
  big-decor range, the decor/overlay range, or the bounds test of `posCurrent`, `posStart` or
  `posEnd`. `DrawView` evaluates the rule at the view it draws, so a view between two ticks shows
  exactly what `Build` would have shown there, pop-in at the 64-pixel bounds included. The
  parallax background is regenerated for the drawn view with `Build`'s own algorithm.
- **Identities and snapping** (`PairScenes`, once per tick):
  - Moving items (Blupi and its effects, network Blupis, move objects, lifts, the voyage icon)
    pair with the previous scene by (entity, index, occurrence).
  - An item that has no counterpart, or moved more than 64 pixels, snaps.
  - The whole scene snaps when the phase changed, the previous frame was not a world frame, or
    the camera moved more than 64 pixels.
  - Tiles, markers, HUD and texts are not interpolated; animation frames and icons are the
    current tick's.
- **Camera.** The simulation camera is interpolated. The current tick's shake offset is added
  on top and clamped like `DecorNextAction`, and items placed with the unshaken camera (editor
  highlight) keep their offset.
- **Timing.** `alpha` is the time since the latest tick over the tick interval, clamped to 1.
  The view therefore runs up to one tick (50 ms) behind the simulation; the cursor does not lag.
  The tick's own `Display` still shows the faithful frame, so traces and the oracle are
  unaffected.
- **Gates** (`ctest`):
  - `golden_*_interp`: every scenario with presents at alpha 1/3, 2/3 and 1 after each tick
    gives the golden traces;
  - `--check-view` (in the same tests): the view renderer at alpha 1 issues exactly the faithful
    frame's commands, with and without the motion;
  - a mutation test that disables the visibility rules in `DrawView` fails `--check-view` (2
    attract frames, large explosion sprites the original culls).
- [EVIDENCE] Visual check on the attract demos: between two ticks with an 8-pixel scroll, the
  alpha-0.5 view is shifted by 4 pixels (2 for a 4-pixel scroll). The alpha-0 view equals the
  previous tick's frame except for the animation frames, and alpha 0.99 equals the current one.

## Stage 8: widescreen (opt-in)

- **The simulation camera stays exactly as it is.** It decides when collected pickups count (see
  facts) and also drives sound panning.
- **The view camera is derived from it:** same centre, wider viewport, clamped to the world.
  Worlds with `m_dimDecor.x == 0` are one screen wide; they are shown centred, with side fill.
- **World layer:** drawn for the wider viewport by the scene builder (tile loops and move-object
  cull bounds use the view rectangle); the parallax background tiles across the extra width.
- **Screen layer:** HUD and voyage stay in a centred 640×480 frame.
  - Voyages are computed in simulation-camera screen space, so they are drawn in that frame;
    their start point still matches the object's position in the wide view.
  - [OPEN] Whether to anchor some HUD elements to the screen edges later.
- **Menus and the editor:** stay 4:3, centred, until a separate UI decision.
- **Gate:** state traces are identical with widescreen on or off. The centre 640×480 region of
  a wide frame equals the faithful frame whenever the view is not clamped differently.

**Status (2026-10-21): done, opt-in.** Widescreen is set on the Display options screen
(stage 9), or for one run with `--wide auto|off|W|A:B`.
- **Automatic aspect.** The view takes the window's shape between 4:3 (640, widescreen off in
  effect) and 21:9 (1120) at the frame height of 480, rounded to an even width so the frame
  sits on whole pixels. The host recomputes it when the window's pixel size changes and
  letterboxes other shapes. A new window opens at 16:9; fullscreen takes the display's shape.
  A fixed width (`--wide 854`, `--wide 16:9`) is for tests and comparisons.
- **Nothing in the simulation changes.** The game keeps its 640×480 back buffer, its screen and
  its camera; sound panning and pickup voyages still use the simulation camera. The wide view is
  a presentation of the same scene (`DrawView` with a `ViewSpec`) into a separate target
  (`kWideView`), which the host shows instead of the screen.
- **View placement** (`PlaceView`). In play, the view is centred on the shaken frame and clamped
  to the world.
  - A world narrower than the view (`m_dimDecor.x == 0`) is centred, with black sides.
  - The scene coverage reaches the widest view's extra width on each side, so views clamped
    at world edges, and any window width, are covered.
  - The parallax background is placed by the view camera: where the view is centred on the
    frame it is `Build`'s own position. Where the view is clamped at a world edge it stays
    still with the world. The first version placed it relative to the frame, so the background
    slid while the clamped view stood still (reported by the user, fixed).
- **Screen layer.** HUD, texts, jauges and the captured UI stay in the centred 640×480 frame. A
  voyage icon starts at the object's place in the view (the frame offset) and ends at the HUD
  counter (centred offset), blended by its own phase.
- **The editor** (`WM_PHASE_BUILD`) also shows the wide world, with its frame anchored to the
  view's left edge (`ViewSpec::frameLeft`).
  - The toolbar, buttons, tooltips and caption sit at the screen's left edge as in the
    original, and the extra world is shown to the right.
  - The view starts at the simulation camera and is never clamped (black beyond the world).
    Every position inside the frame, on a button or on the map, therefore reaches the game
    unchanged with the original mapping. The host reports mouse positions relative to wherever
    the frame lies now (`host::set_frame_x`, updated at every present).
  - Shifting only the toolbar was rejected. The game uses one coordinate for its buttons and
    its map, so a moved toolbar would need every click split between two mappings, with hidden
    buttons left at their old place.
  - Positions right of the frame, which the original rejects as outside its window, go to
    `game/editor_wide.cpp`. That is a derivative of `CEvent::BuildDown`/`BuildMove` and
    `ScreenPosToCelPos`, without the frame bounds test. It places, deletes and highlights
    cells with the selected tool exactly as a click inside the frame would. Cells outside the
    world are ignored, like positions outside the original window.
- **Menus** show the 640×480 frame centred in the wide view.
- **Mouse.** Positions are reported in frame coordinates (the host subtracts the frame offset).
  The game's cursor sprite is drawn into the wide view. Over the side areas it follows the mouse
  (presentation only), and the game is not told the mouse left its window.
- **Gates** (`ctest`):
  - `golden_*_wide`: every scenario with a 16:9 view, interpolation and three presents per tick
    gives the golden traces;
  - `wide_attract16/8` (`--check-wide`): wherever the view is centred on the frame, its centre
    640×480 region equals the faithful frame pixel for pixel. The comparison uses `Build`'s rules
    at the frame, since a wider view legitimately draws objects the original culls. That is
    5980 frames per depth, with 529 clamped frames skipped.
  - `wide_editor16/8`: the same at 21:9 on the editor session, comparing the frame region at
    the view's left edge on every editor frame (131 compared; the 69 skipped frames are its
    test play).
  - A mutation test (parallax placed without the frame offset) fails all 5980.
- [EVIDENCE] Windowed runs: `--wide 16:9 --interpolate`, then automatic widescreen from a
  settings file. Each made about 1200 presents in 5.0 s (240 per second) over 100 game frames,
  with a clean exit; the automatic run chose 854×480 for its 16:9 window. Screenshots of the
  attract demos, the title screen (pillarboxed), the world-1 hub (clamped at the left edge, HUD
  centred) and the editor at 21:9 look as intended. Resizing the window by hand is not yet
  verified.

## Stage 9: port settings and the Display options screen

- **Settings file.** `blupi-sdl3.json` in the user directory (`game/settings.cpp`): widescreen,
  interpolation, frame rate (display or original tick rate), fullscreen, whole-pixel scaling and
  the initial window scale. It is a flat JSON object read leniently. It is written when a
  setting changes on the screen or with Alt+Enter, never by headless runs. Command-line options
  override it for one run.
- **Display options screen** (`game/options_screen.cpp`). The game's settings screen (in the
  menus and in the pause menu) gets one more button in its empty lower-left panel. It opens a
  screen on the same background, built from the game's own `CButton` class, sprites, fonts and
  click sound: arrow pairs like the volume settings, and choice buttons like the input devices.
  Changes apply at once and are saved. Escape, Enter or the check button return to the game's
  settings screen.
  - It is a presentation layer, so `src/` is unchanged. It is drawn over the tick's frame when
    presenting, never into the game's own frame or screen. Its buttons post port messages that
    `DispatchPosted` handles before the window procedure. While it is open, the game's screen
    underneath gets no mouse or key events, and the cursor is moved by the port with the game's
    pointer/arrow rule.
  - The original frame rate now presents once per tick through the same path, so the screen
    stays reachable in every mode.
- **Gate:** `golden_options16_screen` opens the screen during the menu tour, changes three
  settings and closes it. Its state hashes equal `golden_options16`, which is the tour plus the
  one mouse move that reaches the game, without the screen. Screen hashes are not compared,
  since the cursor follows the clicks on the port's screen.

## Order and dependencies

```
0.1-0.3 oracles -> 1 commands -> 2 GPU backend -> 3 scene builder -> 4 sim/present split -> 5 display rate
0.4 Linux ------------------------------------------------------------> 6 Android (best after 2 and 5)
                                                     5 -> 7 interpolation
                                                     5 -> 8 widescreen -> 9 settings screen
```

Each stage lands separately, documented in `ARCHITECTURE.md`, with its gate as a ctest.
Interpolation and widescreen stay off by default; faithful mode remains the reference at every
step.
