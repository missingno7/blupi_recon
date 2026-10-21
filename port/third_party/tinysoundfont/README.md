# TinySoundFont / TinyMidiLoader

Single-header SoundFont2 synthesizer and MIDI loader by Bernhard Schelling,
https://github.com/schellingb/TinySoundFont

| File | Version | Licence | SHA-256 |
|---|---|---|---|
| `tsf.h` | TinySoundFont v0.9 | MIT (licence text in the file header) | `70d55963c98f60ebb81518eaa1f25d46888d5180eb5f5289fd6b74ffc177d197` |
| `tml.h` | TinyMidiLoader v0.7 | zlib (licence text in the file header) | `93257db259e0efb2ea2037d7157841bec8cb4a2d7986286e43c8090705326546` |

Copied unchanged from the `external/` folder of Free API
(https://github.com/openeggbert/free-api, commit `8337b2499ca28423623f624d4351992ca3f2a22f`),
which vendors the upstream headers. Not re-diffed against upstream.

Used only by `port/platform/music.cpp`; the implementations are compiled there
(`TSF_IMPLEMENTATION`, `TML_IMPLEMENTATION`).
