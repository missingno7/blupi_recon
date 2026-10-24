# Releasing the port

A release is three packages built from one commit, each tested before it is packed:

| Package | Built on | Contents |
|---|---|---|
| `speedy-blupi-sdl3-<version>-windows-x64.zip` | Windows, MinGW-w64 GCC (`mingw-x64` preset) | `blupi-sdl3.exe`, `blupi2-sdl3.exe` (GCC runtimes linked in, stripped), `SDL3.dll`, `soundfont.sf2` |
| `speedy-blupi-sdl3-<version>-linux-x86_64.tar.gz` | WSL **Ubuntu 22.04** (glibc 2.35) | `blupi-sdl3`, `blupi2-sdl3` (`BLUPI_PORTABLE`: RUNPATH `$ORIGIN`, static libstdc++/libgcc), `libSDL3.so.0` built from SDL 3.4.16 with its backends loaded at run time, `soundfont.sf2` |
| `speedy-blupi-sdl3-<version>-android.apk` | Windows, Gradle `assembleRelease` | arm64-v8a and x86_64, signed with the release key |

Every desktop package also has `README.txt` (the player's guide, `port/release/README.txt`),
`COPYING.txt` (GPLv3, the port's licence: its source derives from Planet Blupi and the SB2
reconstruction), `THIRD_PARTY.txt` and the licences of SDL and GeneralUser GS. The APK has the
same notices under `assets/licenses/`. The README names the source: the public repository at
the release tag. Game data is never packaged: players use their own copy.

Both desktop programs use `common/app` for discovery, options/help, preferences,
SoundFont lookup and the main loop. CMake targets remain `blupi` and `blupi2`; the
single `blupi_package_app` helper in `cmake/package_app.cmake` packages their runtime
dependencies and selects the Windows GUI subsystem outside Debug. Each can run from
its original game's folder or use the first-run chooser. Settings and environment
names are separate (`blupi-sdl3.json`/`BLUPI_DATA`, `blupi2-sdl3.json`/`BLUPI2_DATA`).
Android still contains SB1 only. The Linux scripts copy SB2's historical source tree
and enable both SB2 editions' tests when their data is present. Desktop archives now
carry both executables; `package_release.py windows` verifies both version strings.

## Version

`port/VERSION.txt` holds the version (`1.0.0`). CMake makes it the project version and
`BLUPI_PORT_VERSION` (`--version`, the start-up log, the Display options screen, the Windows
version resource). The Android build reads it as `versionName`, with `versionCode`
major·10000 + minor·100 + patch. Git tags are `port-v<version>`, which keeps them apart from
the matching reconstruction's history.

## Icon

`port/res/icon.png` (the game's waving Blupi, sprite 47 of `BLUPI000.BLP`, on a 64×64 canvas)
and `port/res/icon.ico` (Windows, 16–256 px) are made by
`.pyenv\python.exe port\tools\make_icon.py "<game folder>"` and committed. The Linux window
loads the PNG (`common/platform/icon.cpp`), the Windows executable carries the ICO, and the Android
build draws its launcher icons from the PNG (`launcherIcon` task).

## Android signing key

The release key is `%USERPROFILE%\.android\speedy-blupi-release.jks`, described by
`speedy-blupi-release.properties` next to it (`storeFile`, `storePassword`, `keyAlias`,
`keyPassword`). Both stay outside the repository; back them up together. Android installs an
update only when it is signed with the same key, so every release must use this one.
Certificate SHA-256: `9e:fe:f0:c4:13:d0:22:9b:a3:49:5c:4e:66:05:f5:7b:b2:4b:d1:7f:40:ce:8b:69:19:f9:b0:0b:b6:d6:fd:12`.
Without the properties file `assembleRelease` leaves the APK unsigned; another location can be
given with `-Pblupi.signing=<file>`.

An APK signed with the release key does not install over a debug build (a different key):
uninstall the debug build first, which also removes its imported game data.

## Steps

1. Set the version in `port/VERSION.txt`.
2. Windows: build and test, then pack.
   ```powershell
   $env:SDL3_DIR = "C:\tools\sdl3-3.4.16-mingw\SDL3-3.4.16\x86_64-w64-mingw32"
   $env:BLUPI_DATA = "D:\Prog\blupi_recon\assets\Speedy Blupi"
   cmake --preset mingw-x64 -S port
   cmake --build build/port -j 4
   ctest --test-dir build/port --output-on-failure     # SDL3.dll on PATH
   .pyenv\python.exe port\tools\package_release.py windows
   ```
3. Linux, in the Ubuntu 22.04 distro (builds SDL and the game, runs the whole suite, packs):
   ```powershell
   wsl -d Ubuntu-22.04 -u root -e sh port/tools/linux_release.sh
   ```
   It prints the newest glibc symbol version the package needs; it must stay at 2.35.
4. Android:
   ```powershell
   $env:JAVA_HOME = "<a JDK 17>"; $env:ANDROID_HOME = "$env:LOCALAPPDATA\Android\Sdk"
   cd port\android; .\gradlew.bat --no-daemon assembleRelease; cd ..\..
   .pyenv\python.exe port\tools\package_release.py android sums
   ```
5. Smoke-test each package from a clean folder (bounded runs; on Windows with a PATH of only
   `C:\Windows\System32`, so no MinGW DLL is picked up by accident).
6. Tag `port-v<version>` on the release commit and publish a GitHub release with the three
   packages, `SHA256SUMS.txt` and the notes from `docs/RELEASE_NOTES.md`.

## The Ubuntu 22.04 build distro

Installed once with `wsl --install -d Ubuntu-22.04 --no-launch` and used as root. Its packages:

```
apt-get install -y build-essential cmake ninja-build rsync pkg-config \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxss-dev libxtst-dev \
  libxfixes-dev libxkbcommon-dev libwayland-dev wayland-protocols libdecor-0-dev \
  libegl-dev libgles-dev libgl-dev libdrm-dev libgbm-dev \
  libasound2-dev libpulse-dev libpipewire-0.3-dev libudev-dev libdbus-1-dev libibus-1.0-dev
```

The everyday Linux check (`port/tools/linux_check.sh`) keeps running in the default distro with
its system SDL; the release distro only exists so the package runs on older systems.
