# Speedy Blupi SDL3 port 1.2.0-rc1 (release candidate)

The native port of **Speedy Blupi** (Epsitec, 1998) and **Speedy Blupi 2** (2001), and of their
eGames editions **Speedy Eggbert** and **Speedy Eggbert 2**, for Windows, Linux and Android. It is
built from this repository's reconstruction of the original programs' source, compiled unchanged with
a modern compiler, plus a small platform layer on SDL3.

**This is a release candidate.** It brings the first native multiplayer, which has been tested
automatically but has had little real-world play. Reports are welcome.

**The original games' files are not included.** You need your own copy of the game: the CD or the
installed game. On the first start the port asks for its folder and remembers it.

## New in 1.2

- **Speedy Blupi 2 and Speedy Eggbert 2** (Windows and Linux), as `blupi2-sdl3`. The program
  recognises the edition from your data. It plays like the original, with the same optional
  widescreen, smooth motion and Display options as Speedy Blupi.
- **Multiplayer for Speedy Blupi and Speedy Eggbert**, again: the original's network game for two to
  four players, rebuilt on a native network layer in place of DirectPlay. The original game logic is
  unchanged.
  - **Choosing a session:** use the game's own multiplayer screens. Choose the provider "Native network
    (UDP)", then create a session, or join one found on your local network.
  - **Platforms:** Windows, Linux and Android players of the same edition can play together. Players
    need the same edition (Speedy Blupi or Speedy Eggbert) and the same multiplayer maps; any language
    works. Other sessions are not listed.
  - **Internet play:** start the joining player's game with `--net-join HOST[:PORT]` to add a host by
    address. Hosts use UDP ports 28640 (game) and 28641 (discovery); allow them in the firewall. Use
    `--net-port N` for other ports.
  - **If the host leaves,** the session ends for everyone, as the original effectively did (host
    migration is not supported).
- **Speedy Eggbert's title screen** says "Version 1.1" like the original, and the Spanish `¡` has the
  original width.

## Downloads

| File | For |
|---|---|
| `speedy-blupi-sdl3-1.2.0-rc1-windows-x64.zip` | 64-bit Windows (tested on Windows 11): `blupi-sdl3.exe` and `blupi2-sdl3.exe` |
| `speedy-blupi-sdl3-1.2.0-rc1-linux-x86_64.tar.gz` | x86-64 Linux with glibc 2.35 or newer (Ubuntu 22.04, Debian 12, Fedora 36 and later), X11 or Wayland |
| `speedy-blupi-sdl3-1.2.0-rc1-android.apk` | Android 7.0 or newer, arm64 phones and tablets (and x86_64); Speedy Blupi and Speedy Eggbert |
| `SHA256SUMS.txt` | checksums |

- **Windows and Linux:** unpack anywhere and start `blupi-sdl3` (Speedy Blupi, Speedy Eggbert) or
  `blupi2-sdl3` (Speedy Blupi 2, Speedy Eggbert 2). The Windows executables are not code-signed, so
  SmartScreen may ask for confirmation ("More info", "Run anyway").
- **Android:** install the APK; it updates 1.1.0 in place and keeps your imported game. On a first
  install, choose the folder with your copy of the game; the app copies it into its own storage.

## What you get

- **The original games, exactly:** 640×480, the original 50 ms tick, 8- and 16-bit colour. Their
  frames equal the historical rebuilds' pixel for pixel (checked over the attract modes and menu tours),
  and the demos replay identically on Windows and Linux.
- **Music** from the games' MIDI files, played with the GeneralUser GS SoundFont (bundled).
- **Optional extras** under *Display options* on the settings screen; none of them changes how the game
  plays:
  - widescreen from 4:3 up to 21:9, following the window's shape, in play and in the editor;
  - smooth motion between ticks;
  - redraw at the monitor's rate or the original 20 per second;
  - window or full screen (Alt+Enter), and fit-the-window or whole-pixel scaling;
  - the language.
- **Languages:**
  - Speedy Blupi: English, German, French.
  - Speedy Eggbert: English, German, Spanish, French, Italian, Portuguese.
  - Speedy Blupi 2: English, German, French.
  - Your own language pack: `--export-language`.
- **Gamepads** through the game's own joystick option.
- **Android:** always full screen and landscape. Menus are tapped directly. In play there is a touch
  joystick with jump, fire and pause buttons, and the editor has a scroll pad. The Back button acts as
  Escape.

## Not in this release

- **Multiplayer for Speedy Blupi 2 and Speedy Eggbert 2.** Their multiplayer screens open, but there is
  nothing to join or create.
- **Speedy Blupi 2 and Speedy Eggbert 2 on Android.**
- **The intro and ending movies.** The original skipped them when the system could not play AVI, and
  the installed games do not include them.
- **Speedy Blupi 2's CD audio tracks.**

## Known limitations

- **Multiplayer has been tested automatically** (two and four players on one machine, Windows and
  Linux), but has had little play between real devices and networks. Also:
  - there is no in-game field for a host's address: use `--net-join`, or the local-network list;
  - an Android player who switches away from the game is likely to drop out of a session.
- **Android:** a player's own language file goes into a `languages` folder inside the game folder before
  importing it. Gamepads and running in the background have had little testing.
- **Linux:** the first-run folder dialog needs the desktop's file chooser portal (or `zenity`). Without
  it, start the game with `--data "<game folder>"`, or put the port's files into the game's folder.
