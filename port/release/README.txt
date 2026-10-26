Speedy Blupi and Speedy Eggbert - SDL3 port, version @VERSION@
==============================================================

Native versions of Speedy Blupi 1 (Epsitec, 1998) and Speedy Blupi 2 (2001),
and of their eGames editions Speedy Eggbert 1 and Speedy Eggbert 2, for
today's Windows and Linux. They are built from a reconstruction of the original
programs' source. The games play like the originals, at their original 640x480
and timing, and add optional widescreen, smooth motion and display settings.
Multiplayer is not available yet.

The original games' files are NOT included: you need your own copy of the
game (the CD, or the game installed from it).
  - blupi-sdl3: Speedy Blupi 1 (version 1.0) or Speedy Eggbert 1 (1.1).
  - blupi2-sdl3: Speedy Blupi 2 or Speedy Eggbert 2.

The version number at the top is the port's own. The game's own version is
shown on its title screen.


Starting the game
-----------------

1. Unpack this folder anywhere.
2. Start blupi-sdl3.exe (Windows) or ./blupi-sdl3 (Linux).

   For Speedy Blupi 2 or Speedy Eggbert 2, start blupi2-sdl3.exe (Windows)
   or ./blupi2-sdl3 (Linux) instead. Both desktop programs are included.
   Either program can be placed with SDL3.dll (Windows) or libSDL3.so.0
   (Linux) in its original game's folder and will find that folder's data.
   Otherwise the first-run folder chooser asks where your game is installed.
   The same display and command-line options apply; use --help for the list.
   SB2 uses BLUPI2_DATA and keeps blupi2-sdl3.json and saves under
   Epsitec/Speedy Blupi 2 SDL3 (or --user). The Android app has both games.
3. On the first start the game asks for the folder where Speedy Blupi is
   installed: the one that contains DATA, IMAGE08, IMAGE16 and SOUND. A
   folder that contains it (for example the CD) works too. The choice is
   remembered.

You can also copy this port's files into the game's folder, or start it with
--data "<game folder>". The game's files are only read, never changed.

Windows: 64-bit Windows (tested on Windows 11). The program is not
code-signed, so SmartScreen may warn about an unknown publisher: choose
"More info", then "Run anyway".

Linux: x86-64 with glibc 2.35 or newer (for example Ubuntu 22.04, Debian 12
or Fedora 36 and later), under X11 or Wayland. The SDL3 library comes with
the game (libSDL3.so.0, next to the program).


Playing
-------

Everything works as in the original: the keyboard (arrow keys, Ctrl to jump,
Space to fire, Escape to pause), the mouse in the menus and the editor, and
gamepads (choose the gamepad under "Choose input device" in the game's
settings; Start pauses). Alt+Enter switches between window and full screen.

The port's own options are on the game's settings screen, under
"Display options":

- Widescreen: the view follows the window's shape, from 4:3 up to 21:9, and
  shows more of the world. Off by default.
- Smooth motion: movement and scrolling are interpolated between the game's
  20 ticks per second. Off by default.
- Frame rate: "Monitor" redraws at your display's rate, "Original" 20 times
  per second like the 1998 game.
- Display (window or full screen) and Scaling (fit the window, or whole
  pixels only).
- Language: the game's languages (Speedy Blupi: English, German, French;
  Speedy Eggbert: English, German, Spanish, French, Italian, Portuguese),
  switched at once. It starts in the language of your installed game.

None of them changes how the game plays.


Multiplayer
-----------

Two to four players, as in the original, on the game's own multiplayer
screens: choose the provider "Native network (UDP)", then create a session
or join one. Sessions on your local network are listed. Windows, Linux and
Android players can play together if they have the same game and edition
(Speedy Blupi, Speedy Eggbert, Speedy Blupi 2 or Speedy Eggbert 2) and the
same multiplayer levels; the language may differ.

To join a host over the internet, start with --net-join HOST[:PORT]. A host
uses UDP ports 28640 (game) and 28641 (discovery): allow them in your
firewall (and forward them on your router for internet play). --net-port N
chooses other ports. If the host leaves, the session ends for everyone.

Your own language
-----------------

blupi-sdl3 --export-language us mylanguage.txt writes the English texts to a
file. Translate it (UTF-8), set "name" and "code" at the top, and put it in a
"languages" folder in your files folder (below). It appears under Language.
Letters the game's font lacks are drawn as their base letter (c for č).


Your files
----------

Saved games, players, your own levels and the settings file
(blupi-sdl3.json) are kept in:

  Windows: %APPDATA%\Epsitec\Speedy Blupi SDL3\
  Linux:   ~/.local/share/Epsitec/Speedy Blupi SDL3/

To change the game folder later, edit or delete "dataFolder" in
blupi-sdl3.json.


Music
-----

The original played its MIDI music through Windows' synthesizer; the port
plays it with the GeneralUser GS SoundFont (soundfont.sf2). Another SoundFont
placed in your files folder as soundfont.sf2 replaces it.


Command line
------------

blupi-sdl3 --help lists the options, for example --data DIR, --fullscreen,
--wide auto|off|16:9, --interpolate and --version.


Licence and source
------------------

This program is free software under the GNU General Public License,
version 3 (COPYING.txt): its source derives from Planet Blupi (GPLv3 or later)
and the Speedy Eggbert 2 reconstruction (GPLv3). The complete source, with
the build scripts, is at

  https://github.com/missingno7/blupi_recon  (tag port-v@VERSION@, folder port/)

Third-party components and their licences: THIRD_PARTY.txt.

Speedy Blupi and its data (graphics, sounds, music, levels) are (c) Epsitec
SA and are not part of this package or of its licence. This port is a fan
project and is not affiliated with Epsitec.
