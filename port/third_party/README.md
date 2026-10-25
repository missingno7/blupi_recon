# Third-party components of the port

Unmodified upstream material. Do not edit files in here; update by replacing them (vendored
headers) or moving the submodule pin, and record the new version and checksum below.

## tinysoundfont/ (vendored headers)

TinySoundFont v0.9 (MIT) and TinyMidiLoader v0.7 (zlib) by Bernhard Schelling; details and
SHA-256 in [tinysoundfont/README.md](tinysoundfont/README.md).

## enet/ (vendored sources)

ENet 1.3.18 (MIT) by Lee Salzman, the UDP transport under the native multiplayer
(`port/common/net/`); release tag, archive and per-file SHA-256 in [enet/README.md](enet/README.md).

## generaluser-gs/ (git submodule)

GeneralUser GS v2.0.3, the General MIDI / GS SoundFont by S. Christian Collins that the port uses
to play the game's MIDI music (the original used the Windows MIDI synthesizer through MCI).

| | |
|---|---|
| Repository | https://github.com/mrbumpy409/GeneralUser-GS (the author's), project page https://www.schristiancollins.com/generaluser |
| Pinned commit | `684543d5e5efaef08d02be50dcda8d552478fa60` ("update documentation to r6", 2026-02-23) |
| File used | `GeneralUser-GS.sf2`, 32,319,396 bytes, SHA-256 `9575028c7a1f589f5770fccc8cff2734566af40cd26ed836944e9a5152688cfe` (identical to the copy Free Eggbert ships) |
| Licence | GeneralUser GS License v2.0, `generaluser-gs/documentation/LICENSE.txt`: free use including in software projects; the author asks for local copies rather than links to his download files |

The submodule is `shallow = true` in `.gitmodules`: initialising it fetches only the pinned
commit (about 62 MB, of which 31 MB are upstream's `demo MIDIs` folder that the port does not
use), instead of the full 94 MB history. Our repository stores only the commit pin.

```powershell
git submodule update --init port/third_party/generaluser-gs
```

The build copies `GeneralUser-GS.sf2` next to the executable as `soundfont.sf2`
(`BLUPI_BUNDLE_SOUNDFONT`, on by default). Without the submodule, CMake warns and the game runs
without music.
