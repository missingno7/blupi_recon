# Speedy Blupi 1 for Android

The SDL3 port (`port/`) as an Android app: the same CMake project builds `libmain.so`
(the game) and `libSDL3.so`; SDL's Java activity runs it. Roadmap stage 6
(`../docs/ROADMAP.md`).

## What the app does

- **Your own copy of the game.** The app contains no game data. On first run it asks for the
  folder of an installed Speedy Blupi, the one with `DATA`, `IMAGE08`, `IMAGE16` and `SOUND`,
  or a folder above it. It uses the system folder picker and copies the files once into its
  private storage (`SetupActivity`, `files/game`). Later runs start the game directly.
  Uninstalling the app deletes that copy, the saves and the settings.
- **Full screen, landscape**, with the 4:3 game frame centred. Widescreen (Display options)
  uses the whole screen.
- **Touch:**
  - In menus, tap what you want; there is no cursor.
  - While playing, a joystick on the left moves Blupi: sideways to walk, a clear push up or
    down for doors, ladders and ducking, diagonals allowed.
  - Buttons on the right: jump (double arrow), fire (dot) and pause (Escape, as the Back
    button).
  - The controls sit in the bars beside the frame, or lie lightly over the frame's corners
    when there are no bars.
  - In the editor, a scroll pad at the bottom right (the game's own button and arrow
    graphics) scrolls while held; tap tools and cells directly.
- **Gamepads** work as on the desktop: choose the gamepad under "Choose input device" in the
  game's settings, and Start pauses.
- **The keyboard** appears only while the game asks for a name.
- **Music:** the GeneralUser GS SoundFont is packaged as an asset (`soundfont.sf2`).

## Build

Requirements:
- the Android SDK with NDK 28.2.13676358, platform 35 and CMake 3.22.1;
- a JDK 17 for Gradle 8.12 / AGP 8.7.3 (for example IntelliJ's or Android Studio's bundled JBR);
- the SDL 3.4.16 source tree, by default at `build/third_party/SDL3-3.4.16` of the repository.
  Get it with `git clone --depth 1 --branch release-3.4.16 https://github.com/libsdl-org/SDL`,
  or point `-Pblupi.sdl3.dir=...` elsewhere. CMake builds it, and its Java glue
  (`android-project/app/src/main/java/org/libsdl/app`) is compiled into the app; neither is
  copied into this repository.
- the SoundFont submodule: `git submodule update --init port/third_party/generaluser-gs`.

```powershell
$env:JAVA_HOME = "<a JDK 17>"
$env:ANDROID_HOME = "$env:LOCALAPPDATA\Android\Sdk"
cd port\android
.\gradlew.bat assembleDebug    # build\port-android\app\outputs\apk\debug\app-debug.apk
```

Output goes to the repository's ignored `build/port-android/`. Native compiles are capped at 4
(CMake job pools) and Gradle runs one task at a time (`gradle.properties`), per the repository's
host-safety rule. The ABIs are arm64-v8a (phones) and x86_64 (emulators).

## Development: game data without the picker

With a debug build, copy the data into the app's private storage as the app's own user:

```powershell
adb shell mkdir -p /data/local/tmp/blupi-game
adb push "<game folder>\DATA" "<game folder>\IMAGE08" "<game folder>\IMAGE16" "<game folder>\SOUND" /data/local/tmp/blupi-game/
adb shell run-as io.github.missingno7.speedyblupi sh -c "'rm -rf files/game && cp -r /data/local/tmp/blupi-game files/game'"
adb shell rm -rf /data/local/tmp/blupi-game
```

A folder pushed by adb into the app's external files directory belongs to the shell user, and
the app cannot read it on current Android versions.

The game's command-line options can be passed with an `args` extra. For example, `--verbose`
logs a presentation timing line every 5 s (`adb logcat | grep perf:`):

```powershell
adb shell am start -n io.github.missingno7.speedyblupi/.SetupActivity --es args "--verbose"
```

## Performance

The game's native code is optimised in debug builds too (`RelWithDebInfo`). Every presented
frame is redrawn in software at the game's resolution (640×480, or the widescreen width × 480);
SDL_Renderer then scales it to the screen on the GPU.

[EVIDENCE] Galaxy S24 Ultra (SM-S928B, 3120×1440, 120 Hz), widescreen (1040×480) and smooth
motion on, attract demo:

| Native build | Presents/s | Render | Present (incl. vsync wait) |
|---|---|---|---|
| `-O0` (first debug APK) | 20–37, falling | 11–22 ms | 15–27 ms |
| `RelWithDebInfo` | 120 (the display rate) | 0.2–0.9 ms | ~8 ms (one 120 Hz period) |

With the unoptimised build smooth motion could not work: a 50 ms tick got one or two presents
instead of six.

## Status (2026-10-22)

[EVIDENCE] Checked on the Android 16 (API 36) x86_64 emulator, 2340×1080:
- **First run:** the import screen, the system folder picker (Download/SpeedyBlupi), the
  access confirmation, the copy and the game start.
- **Rendering:** the title screen, menus and the world-1 hub, landscape and full screen.
- **Input:** menus by tapping; walking both ways with the joystick; jumping; no native crash.

[EVIDENCE] On a physical phone (Galaxy S24 Ultra, Android 16, arm64): installed and played by
the user. That first build showed slow presentation, which the optimised native build fixes
(Performance, above).

Not yet checked: gamepads on Android, the name keyboard, and the app in the background.
