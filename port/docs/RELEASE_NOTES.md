# Speedy Blupi SDL3 port 1.0.0

The first release of the native port of **Speedy Blupi** (Epsitec, 1998) for Windows, Linux and
Android. It is built from this repository's reconstruction of the original program's source,
compiled unchanged with a modern compiler, plus a small platform layer on SDL3.

**The original game's files are not included.** You need your own copy of Speedy Blupi (the CD
or the installed game). On the first start the port asks for its folder and remembers it.

## Downloads

| File | For |
|---|---|
| `speedy-blupi-sdl3-1.0.0-windows-x64.zip` | 64-bit Windows (tested on Windows 11) |
| `speedy-blupi-sdl3-1.0.0-linux-x86_64.tar.gz` | x86-64 Linux with glibc 2.35 or newer (Ubuntu 22.04, Debian 12, Fedora 36 and later), X11 or Wayland |
| `speedy-blupi-sdl3-1.0.0-android.apk` | Android 7.0 or newer, arm64 phones and tablets (and x86_64) |
| `SHA256SUMS.txt` | checksums |

Windows and Linux: unpack anywhere and start `blupi-sdl3`. The Windows executable is not
code-signed, so SmartScreen may ask for confirmation ("More info", "Run anyway").

Android: install the APK (allow installing from this source). On the first start, choose the
folder with your copy of the game; the app copies it into its own storage.

## What you get

- **The original game, exactly.** 640×480, the original 50 ms tick, 8- and 16-bit colour. Its
  frames equal the historical rebuild's pixel for pixel (checked over the attract mode and a
  menu tour), and the attract-mode demos replay identically on Windows and Linux.
- **Music** from the game's MIDI files, played with the GeneralUser GS SoundFont (bundled).
- **Optional extras** under *Display options* on the game's settings screen; none of them
  changes how the game plays:
  - widescreen from 4:3 up to 21:9, following the window's shape, in play and in the editor;
  - smooth motion between ticks;
  - redraw at the monitor's rate or the original 20 per second;
  - window or full screen (Alt+Enter), and fit-the-window or whole-pixel scaling.
- **Gamepads** through the game's own joystick option.
- **Android:** always full screen and landscape. Menus are tapped directly. In play there is a
  touch joystick with jump, fire and pause buttons, and the editor has a scroll pad. The Back
  button acts as Escape.

## Not in this release

- Network multiplayer: the original used DirectPlay. The multiplayer screens open, but there is
  nothing to join or create.
- The intro and ending movies: the original skipped them when the system could not play AVI,
  and the installed game does not include them.

## Known limitations

- On Linux the first-run folder dialog needs the desktop's file chooser portal (or `zenity`).
  Without it, start the game with `--data "<game folder>"`, or put the port's files into the
  game's folder.
- On Android, gamepads and running in the background have had little testing.

## Licences

The port is free software under the GNU GPL version 3 (`COPYING.txt`), since its source derives
from Planet Blupi (GPLv3+) and the Speedy Eggbert 2 reconstruction (GPLv3). The source is this
repository at tag `port-v1.0.0` (folder `port/`). Third-party components: SDL 3.4.16 (zlib),
TinySoundFont and TinyMidiLoader (MIT, zlib) and the GeneralUser GS SoundFont; see
`THIRD_PARTY.txt` in each package. Speedy Blupi and its data are © Epsitec SA and are not
included; this is a fan project, not affiliated with Epsitec.
