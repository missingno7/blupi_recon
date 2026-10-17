# Core PB-derived translation units

## Evidence and method

- [EVIDENCE] Candidate source files are under `scratch/tu/`; headers are under `scratch/tu/core_include/`. The candidate objects use VC5 SP3 11.00.7022 with `/O2 /Gy /GX`; `/Gy` is only an experimental function-splitting flag and remains unproven for the historical build. `TMP` and `TEMP` pointed inside `build/tmp`. No executable was run.

- [EVIDENCE] The direct multi-object comparison snapshot is `build/core/results/core_candidate_raw.json`; a second pass with direct oracle call-target mappings is `build/core/results/core_candidate_mapped.json`. The fresh CPixmap isolated comparison is `build/core/results/pixmap_candidate_standalone.json`.

- [EVIDENCE] Oracle sizes below come from `oracle/functions.csv` (Ghidra function extents); COFF sizes are the compiler-emitted function sizes. They can differ because Ghidra boundaries do not always include the whole matched COFF extent. The explicit-VA pass is recorded separately; the tables use final `cmpfunc.py` statuses from the fresh objects. `~RVA` denotes an identity/address hypothesis.

## Candidate source changes

- [EVIDENCE] `sound.cpp` retains the PB line numbering for all existing `__LINE__` uses; `CSound::StopSound(int)` was inserted before `Play` while replacing six existing comment/blank lines. The source remains 648 lines and every original `TRY_DS`/`__LINE__` site retains its line number. Its body matches the oracle at RVA `0x23050` strictly.

- [EVIDENCE] `misc.cpp` changes the active `AddUserPath` folder literal from Planet Blupi to `c:\Speedy Blupi\`, matching the oracle string at RVA `0x78044`.

- [EVIDENCE] `pixmap.h` adds one `BOOL m_unknown08` after `m_mouseType`; the constructor initializes it to `TRUE`. The field is required by target reads/writes at `this+8` and shifts later members by four bytes. Its semantic name remains unknown.

- [EVIDENCE] `movie.cpp` and `ddutil.cpp` remain PB-derived copies.

## Comparison summary

| TU | Functions | Raw strict / unverified / mismatch | With explicit helper map |
|---|---:|---:|---:|
| `sound.cpp` | 28 | 22 / 1 / 5 | 22 / 1 / 5 |
| `movie.cpp` | 13 | 13 / 0 / 0 | 13 / 0 / 0 |
| `misc.cpp` | 12 | 6 / 3 / 3 | 6 / 3 / 3 |
| `ddutil.cpp` | 9 | 8 / 0 / 1 | 8 / 0 / 1 |
| `pixmap.cpp` (isolated) | 45 | 20 / 4 / 21 | same |

- [EVIDENCE] Direct call operands in matched oracle bodies identify `OutputDebug` at VA `0x0041F010`, `GetCurrentDir` at `0x0041F360`, and `TraceErrorDS` at `0x0041FFA0`. In the fresh five-object comparison, verified call targets place `OutputDebug` at `0x1F010`; this resolves the sound, TraceErrorDS, and DDUtil call relocations. Repeating the comparison with explicit `--symbol-va` mappings produces the same counts. The `__FILE__` difference in `CreateSoundBuffer` remains.

- [EVIDENCE] `GetCurrentDir` itself remains relocation-unverified because its global data target has no unique reconstructed definition. Its call target from `PlayMusic` is still known from the oracle operand.

## Class layout delta

### `CSound`

[EVIDENCE] The PB layout fits the oracle; no member insertion is currently required. Oracle code uses these offsets:

| Field | Offset | Basis |
|---|---:|---|
| `m_hWnd` | `+0x00` | constructor and methods |
| `m_bEnable`, `m_bState` | `+0x04`, `+0x08` | control checks in sound methods |
| `m_lpDS`, `m_lpDSB[100]` | `+0x0C`, `+0x10` | DirectSound pointer and buffer indexing |
| `m_channelBlupi[100]` | `+0x1A0` | target body `0x230C0` indexes this array; likely `PlayImage` |
| `m_MidiDeviceID`, `m_MIDIFilename[50]` | `+0x268`, `+0x26C` | music methods |
| `m_audioVolume`, `m_midiVolume`, `m_lastMidiVolume`, `m_nbSuspendSkip` | `+0x2A0`, `+0x2A4`, `+0x2A8`, `+0x2AC` | volume and suspend methods |

### `CPixmap`

[EVIDENCE] Insert `m_unknown08` after `m_mouseType`; all following PB fields shift by `+4`. Target constructor `0x21040` stores `1` at `+8`; `Create`-like body `0x21190` writes its argument there; other target routines read it. This layout change yields 20 strict, 4 relocation-unverified, and 21 mismatching functions in the fresh isolated comparison.

[HYPOTHESIS] The slot may correspond to SB2's `m_bTrueColorBack`, but SB2's surrounding members do not match SB1 offsets. Keep the neutral field name until more evidence identifies its role.

| Target-consistent field | Offset | Evidence / model |
|---|---:|---|
| fullscreen / mouse type / unknown slot | `+0x00 / +0x04 / +0x08` | constructor and create-path stores |
| debug / palette flag / window | `+0x0C / +0x10 / +0x14` | direct target reads/stores |
| dimensions / clip rect / mouse position | `+0x18 / +0x20 / +0x30` | direct target accesses |
| mouse sprite / hotspot / back position | `+0x38 / +0x3C / +0x44` | constructor and mouse routines |
| mouse back / mouse show / back displayed | `+0x4C / +0x50 / +0x54` | constructor and mouse routines |
| DirectDraw / primary / back / mouse surfaces / palette | `+0x58 / +0x5C / +0x60 / +0x64 / +0x68` | destructor and restore paths |
| surfaces array / clipper | `+0x6C / +0x1FC` | restore and destructor |
| palette / system palette / color surfaces / filename array | `+0x200 / +0x600 / +0xA00 / +0xD20` | palette, surface, and restore routines |
| total dimension / icon dimension / `DDBLTFX` | `+0x14F0 / +0x1810 / +0x1B30` | layout model; constructor initializes `DDBLTFX.dwSize` |

## Per-function comparison tables

[EVIDENCE] Statuses are from the fresh VC5 SP3 objects and `cmpfunc.py`; the separate explicit-VA pass produces the same counts. Mismatch rows with a `~RVA` are placement hypotheses; blank RVAs mean the oracle body is not yet uniquely assigned.

### `sound.cpp`

| Function | Oracle RVA | Oracle bytes | COFF bytes | Status | Evidence / cause |
|---|---:|---:|---:|---|---|
| `CSound::CreateSoundBuffer` | 0x22940 | 206 | 208 | RELOC-UNVERIFIED | [EVIDENCE] body matches at this location; `__FILE__` data differs (`F:\Profil\sound.cpp` is the oracle path). |
| `CSound::ReadData` | 0x22A10 | 290 | 304 | STRICT | [EVIDENCE] full body and relocation match; verified call targets place `OutputDebug` at `0x1F010`. |
| `CSound::CreateBufferFromWaveFile` | 0x22B40 | 236 | 240 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::StopAllSounds` | — | — | 112 | MISMATCH | [EVIDENCE] no unique target body found; oracle bodies at `0x22F00`/`0x22FC0` have playback-call signatures, not this zero-argument method. |
| `CSound::PlaySoundDS` | — | — | 128 | MISMATCH | [EVIDENCE] no unique target body found; target playback helpers take 3 or 4 stack arguments, unlike this two-argument PB method. |
| `InitMidiVolume` | 0x22C30 | 124 | 128 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::CSound` | 0x22CB0 | 88 | 96 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::~CSound` | 0x22D10 | 67 | 80 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::Create` | 0x22D60 | 82 | 96 | MISMATCH | [EVIDENCE] located at this RVA; `DirectSoundCreate` resolves to thunk `0x423890` instead of the expected IAT slot `0x42C014`, and the full relocation match still fails after helper mapping. |
| `CSound::GetEnable` | 0x22DC0 | 4 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::SetState` | 0x22DD0 | 10 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::SetAudioVolume` | 0x22DE0 | 13 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::GetAudioVolume` | 0x22DF0 | 17 | 32 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::SetMidiVolume` | 0x22E10 | 13 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::GetMidiVolume` | 0x22E20 | 17 | 32 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::CacheAll` | 0x22E40 | 64 | 64 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::Cache` | 0x22E80 | 63 | 64 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::Flush` | 0x22EC0 | 51 | 64 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::StopSound` | 0x23050 | 99 | 112 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::Play` | ~0x23220 | 278 | 160 | MISMATCH | [HYPOTHESIS] target wrapper calls helper `0x22FC0` after volume arithmetic; candidate body differs. |
| `CSound::PlayImage` | ~0x230C0 | 351 | 352 | MISMATCH | [HYPOTHESIS] target reads `m_channelBlupi` at `+0x1A0` and calls helper `0x22F00`; candidate body differs. |
| `CSound::PlayMusic` | 0x23340 | 431 | 432 | STRICT | [EVIDENCE] full body and relocations match using verified `OutputDebug` and `GetCurrentDir` call targets. |
| `CSound::RestartMusic` | 0x234F0 | 80 | 80 | STRICT | [EVIDENCE] full body and relocation match using the verified `OutputDebug` call target. |
| `CSound::SuspendMusic` | 0x23540 | 77 | 80 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::StopMusic` | 0x23590 | 17 | 32 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::IsPlayingMusic` | 0x235B0 | 14 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::AdaptVolumeMusic` | 0x235C0 | 49 | 64 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CSound::SetSuspendSkip` | 0x23600 | 13 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |

### `movie.cpp`

| Function | Oracle RVA | Oracle bytes | COFF bytes | Status | Evidence / cause |
|---|---:|---:|---:|---|---|
| `CMovie::initAVI` | 0x20150 | 68 | 80 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::termAVI` | 0x201A0 | 33 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::positionMovie` | 0x201D0 | 35 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::fileCloseMovie` | 0x20200 | 67 | 80 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::fileOpenMovie` | 0x20250 | 357 | 368 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::playMovie` | 0x203C0 | 128 | 128 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::CMovie` | 0x20440 | 16 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::~CMovie` | 0x20450 | 5 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::Create` | 0x20460 | 35 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::GetEnable` | 0x20490 | 3 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::IsExist` | 0x204A0 | 171 | 176 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::Play` | 0x20550 | 94 | 96 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CMovie::Stop` | 0x205B0 | 18 | 32 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |

### `misc.cpp`

| Function | Oracle RVA | Oracle bytes | COFF bytes | Status | Evidence / cause |
|---|---:|---:|---:|---|---|
| `InitHInstance` | 0x1F1E0 | 10 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `OutputDebug` | 0x1F010 | 1 | 16 | STRICT | [EVIDENCE] placed by verified oracle call targets; Ghidra marks a one-byte body extent. |
| `LoadStringA` | 0x1F1F0 | 28 | 32 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `ChangeSprite` | ~0x1F210 | 285 | 320 | MISMATCH | [HYPOTHESIS] address assigned by contiguous TU order and oracle disassembly; candidate body differs. Cause remains open. |
| `ConvLongToPos` | 0x1F330 | 17 | 32 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `InitRandom` | 0x1F350 | 11 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `Random` | — | — | 32 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `GetCurrentDir` | 0x1F360 | 170 | 176 | RELOC-UNVERIFIED | [EVIDENCE] body matches at this location; referenced global data has no unique reconstructed definition. |
| `AddCDPath` | ~0x1F410 | 324 | 336 | MISMATCH | [HYPOTHESIS] address assigned by contiguous TU order and oracle disassembly; candidate body differs. Cause remains open. |
| `AddUserPath` | 0x1F560 | 300 | 304 | RELOC-UNVERIFIED | [EVIDENCE] body matches at this location; referenced global data has no unique reconstructed definition. |
| `TraceErrorDD` | 0x1F690 | 2075 | 2320 | RELOC-UNVERIFIED | [EVIDENCE] body matches at this location; referenced global data has no unique reconstructed definition.; referenced helper has no unique verified target RVA. |
| `TraceErrorDS` | 0x1FFA0 | 423 | 432 | STRICT | [EVIDENCE] full body and relocations match after `OutputDebug` is placed by verified call targets. |

### `ddutil.cpp`

| Function | Oracle RVA | Oracle bytes | COFF bytes | Status | Evidence / cause |
|---|---:|---:|---:|---|---|
| `DDSetDebug` | 0x03100 | 10 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `DDConnectBitmap` | — | — | 192 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `DDLoadBitmap` | 0x03110 | 397 | 400 | STRICT | [EVIDENCE] full body and relocations match in the five-object comparison. |
| `DDReLoadBitmap` | 0x032A0 | 154 | 160 | STRICT | [EVIDENCE] full body and relocations match in the five-object comparison. |
| `DDCopyBitmap` | 0x03340 | 499 | 512 | STRICT | [EVIDENCE] full body and relocations match in the five-object comparison. |
| `DDLoadPalette` | 0x03540 | 608 | 608 | STRICT | [EVIDENCE] full body and relocations match in the five-object comparison. |
| `DDColorMatch` | 0x037A0 | 255 | 256 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `DDSetColorKey` | 0x038A0 | 48 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `DDSetColorKey2` | 0x038D0 | 62 | 64 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |

### `pixmap.cpp`

| Function | Oracle RVA | Oracle bytes | COFF bytes | Status | Evidence / cause |
|---|---:|---:|---:|---|---|
| `CPixmap::CPixmap` | ~0x21040 | 133 | 144 | MISMATCH | [HYPOTHESIS] target constructor identified by its field initialization pattern; candidate layout/code still differs. |
| `CPixmap::~CPixmap` | 0x210D0 | 148 | 160 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::SetDebug` | 0x21170 | 19 | 32 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::Create` | ~0x21190 | 902 | 832 | MISMATCH | [HYPOTHESIS] target create body identified by its fullscreen, mouse-mode, and `+8` stores; candidate body differs. |
| `CPixmap::Flush #1` | 0x21520 | 6 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::Restore` | 0x21530 | 11 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::InitSysPalette` | 0x21540 | 87 | 96 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::IsPalette` | — | — | 16 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::Fill` | — | — | 16 | RELOC-UNVERIFIED | [EVIDENCE] body matches at this location; byte-identical/trivial body has multiple possible oracle locations. |
| `CPixmap::RestoreAll` | 0x215A0 | 172 | 176 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::BltFast #1` | 0x21650 | 228 | 240 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::BltFast #2` | 0x21740 | 88 | 96 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::SavePalette` | 0x21810 | 40 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::RestorePalette` | 0x21840 | 35 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::SearchColor` | ~0x21B20 | 770 | 400 | MISMATCH | [HYPOTHESIS] likely target search body from source order and color-search behavior; not byte-matched. |
| `CPixmap::Cache #1` | 0x21870 | 391 | 400 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::Cache #2` | — | — | 64 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::Cache #3` | — | — | 144 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::Flush #2` | 0x21A00 | 44 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::SetTransparent` | 0x21A30 | 60 | 64 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::SetTransparent2` | 0x21A70 | 67 | 80 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::SetClipping` | 0x21AC0 | 33 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::GetClipping` | 0x21AF0 | 36 | 48 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::IsIconPixel` | — | — | 240 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::DrawIcon` | — | — | 320 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::DrawIconDemi` | — | — | 400 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::DrawIconPart` | — | — | 336 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::DrawPart` | — | — | 192 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::DrawImage` | 0x221C0 | 115 | 128 | RELOC-UNVERIFIED | [EVIDENCE] body matches at this location; referenced helper has no unique verified target RVA. |
| `CPixmap::BuildIconMask` | — | — | 480 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::Display` | 0x22240 | 145 | 160 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::SetMousePosSprite` | 0x222E0 | 67 | 80 | RELOC-UNVERIFIED | [EVIDENCE] body matches at this location; referenced helper has no unique verified target RVA. |
| `CPixmap::SetMousePos` | — | — | 48 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::SetMouseSprite` | 0x22330 | 41 | 48 | RELOC-UNVERIFIED | [EVIDENCE] body matches at this location; referenced helper has no unique verified target RVA. |
| `CPixmap::MouseShow` | 0x22360 | 10 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::MouseUpdate` | ~0x22370 | 326 | 320 | MISMATCH | [HYPOTHESIS] target mouse update body by source order and size; code differs. |
| `CPixmap::MouseQuickDraw` | 0x224C0 | 186 | 192 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::MouseInvalidate` | 0x22580 | 8 | 16 | STRICT | [EVIDENCE] full candidate body and relocations match the located oracle body. |
| `CPixmap::MouseBackClear` | — | — | 16 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::MouseBackDraw` | — | — | 224 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::MouseBackSave` | — | — | 240 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::MouseBackRestore` | — | — | 192 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::MouseBackDebug` | — | — | 160 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::MouseRectSprite` | — | — | 224 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |
| `CPixmap::MouseHotSpot` | — | — | 48 | MISMATCH | [OPEN] no unique full-body location; PB-derived body does not match the oracle candidate body. |

## Oracle body inventory and remaining blockers

### Sound and misc body assignments

| RVA | Oracle bytes | Likely identity | Basis |
|---:|---:|---|---|
| `0x22F00` | 189 | playback helper with 4 stack arguments | [EVIDENCE] sets buffer volume/pan and optionally plays; called from `0x230C0` and `0x1C0B0`. Exact name remains open. |
| `0x22FC0` | 131 | playback helper with 3 stack arguments | [EVIDENCE] sets buffer volume/pan; called from `0x23220`. Exact name remains open. |
| `0x23050` | 99 | `CSound::StopSound` | [EVIDENCE] fresh candidate function matches strictly; oracle calls buffer `Stop` and `SetCurrentPosition(0)`. |
| `0x230C0` | 351 | `CSound::PlayImage` | [HYPOTHESIS] uses `m_channelBlupi` at `+0x1A0` and calls the helper at `0x22F00`. |
| `0x23220` | 278 | `CSound::Play` | [HYPOTHESIS] volume arithmetic and call to the helper at `0x22FC0` identify a playback wrapper; candidate body still differs. |
| `0x1F210` | 285 | `ChangeSprite` | [HYPOTHESIS] body accesses sprite-selection globals and matches TU order. |
| `0x1F410` | 324 | `AddCDPath` | [HYPOTHESIS] body performs path work and matches TU order. |

### CPixmap oracle bodies not assigned to a strict or relocation-verified candidate

[OPEN] The following oracle entries have no candidate function placed at their entry in the isolated CPixmap comparison. Identities marked as hypotheses are based on neighboring source order and body shape; the target bodies indicate broader SB1 changes than the single `+8` layout slot explains.

| RVA | Oracle bytes | Candidate identity |
|---:|---:|---|
| `0x21000` | 35 | [OPEN] unassigned |
| `0x21030` | 4 | [OPEN] unassigned |
| `0x21040` | 133 | [HYPOTHESIS] `CPixmap` constructor |
| `0x21190` | 902 | [HYPOTHESIS] `CPixmap::Create` |
| `0x217A0` | 98 | [OPEN] could be `Fill` or `IsPalette`; not proven |
| `0x21B20` | 770 | [HYPOTHESIS] `SearchColor` |
| `0x21E30` | 719 | [OPEN] drawing or icon routine; not proven |
| `0x22100` | 185 | [OPEN] drawing or icon routine; not proven |
| `0x22370` | 326 | [HYPOTHESIS] `MouseUpdate` |
| `0x22590` | 18 | [HYPOTHESIS] `MouseBackClear` |
| `0x225B0` | 268 | [HYPOTHESIS] `MouseBackDraw` |
| `0x226C0` | 241 | [HYPOTHESIS] `MouseBackSave` |
| `0x227C0` | 193 | [HYPOTHESIS] `MouseBackRestore` |
| `0x22890` | 123 | [HYPOTHESIS] `MouseBackDebug` |
| `0x22910` | 43 | [OPEN] likely `MouseRectSprite` or `MouseHotSpot`; not proven |

- [OPEN] `StopAllSounds` and `PlaySoundDS` have no unique target-body assignments; oracle playback helpers at `0x22F00` and `0x22FC0` leave a layout gap before the strict `StopSound` body. The PB definition order also places `Play` before `PlayImage`, while the target bodies are assigned at `0x23220` and `0x230C0`; restore the SB1 order with those helpers. `Create`, `Play`, and `PlayImage` still need SB1 source reconstruction. `CreateSoundBuffer` retains the expected `F:\Profil\sound.cpp` versus candidate-path literal difference.

- [OPEN] `misc.cpp`: `Random` has no unique body assignment; `ChangeSprite` and `AddCDPath` are identified by block position and behavior but not strict. `AddUserPath`, `GetCurrentDir`, and `TraceErrorDD` retain unresolved global-data targets.

- [OPEN] `ddutil.cpp`: only `DDConnectBitmap` remains unmatched; the other eight candidate functions compare strict in the five-object run.

- [OPEN] `pixmap.cpp`: 21 of 45 candidate functions still mismatch; 4 are relocation-unverified. Several oracle bodies in the inventory are likely additional/mutated SB1 mouse and drawing methods, but names are not proven.

## Verification commands

```powershell
$env:TEMP = "D:\Prog\blupi_recon\build\tmp"; $env:TMP = $env:TEMP
New-Item -ItemType Directory -Force -Path $env:TEMP | Out-Null
foreach ($tu in @('sound','movie','misc','ddutil','pixmap')) {
  $out = "build\core\obj\${tu}_candidate_o2_gy_gx.obj"
  & .\.pyenv\python.exe tools\msvc.py "scratch\tu\$tu.cpp" --tree sp3 --out $out --include scratch\tu\core_include --include C:\tools\dx6sdk\INCLUDE --define nullptr=0 --flag /O2 --flag /Gy --flag /GX
}
& .\.pyenv\python.exe tools\cmpfunc.py build\core\obj\sound_candidate_o2_gy_gx.obj build\core\obj\movie_candidate_o2_gy_gx.obj build\core\obj\misc_candidate_o2_gy_gx.obj build\core\obj\ddutil_candidate_o2_gy_gx.obj build\core\obj\pixmap_candidate_o2_gy_gx.obj --json-out build\core\results\core_candidate_raw.json --summary
& .\.pyenv\python.exe tools\cmpfunc.py build\core\obj\sound_candidate_o2_gy_gx.obj build\core\obj\movie_candidate_o2_gy_gx.obj build\core\obj\misc_candidate_o2_gy_gx.obj build\core\obj\ddutil_candidate_o2_gy_gx.obj build\core\obj\pixmap_candidate_o2_gy_gx.obj --symbol-va '?TraceErrorDS@@YAXJPADH@Z=0x0041FFA0' --symbol-va '?GetCurrentDir@@YAXPADH@Z=0x0041F360' --symbol-va '?OutputDebug@@YAXPAD@Z=0x0041F010' --json-out build\core\results\core_candidate_mapped.json --summary
```

[EVIDENCE] The compile banner is VC5 compiler 11.00.7022; all five fresh object builds succeeded. The fresh raw comparator result is 22/1/5 sound, 13/0/0 movie, 6/3/3 misc, 8/0/1 ddutil, and 20/4/21 pixmap when all five objects are compared together. The isolated CPixmap compare is also 20/4/21. An explicit target-call map pass produces the same counts.
## Residuals round (2026-10-17)

[EVIDENCE] This section updates older residual statuses where the same target body was rechecked with the current source and comparator. All candidate and private-header changes below are under `scratch/`; no canonical source or header was changed by this worker. Compiler: VC5 SP3 11.00.7022, `/W3 /GX /O2 /ML /Gd /Gy`, with the per-TU defines from `recon.json`; TMP/TEMP were under `build/tmp`.

### Sound helpers and cross-TU declaration context

[EVIDENCE] The copied-tree experiment at `scratch/coreres/src_minimal_sound_order/` starts from the current `src/`, substitutes the scratch `sound.cpp`, and tests a private `sound.h`. Its minimum successful declaration set keeps the current public `Play` and `PlayImage` API, declares `StopSound(int)` after `Play`, retains the inherited `PlaySoundDS(DWORD,DWORD)` declaration, and adds `PlaySoundDS(int,int,int,int)` and `PlaySoundDS(int,int,int)` after it. `StopAllSounds()` is omitted in this successful variant. The all-object VC5 build and `cmpfunc.py` output are in `build/coreres/minimal_sound_order/`; the private header has since been copied to `scratch/tu/sound.h`.

| Strict candidate body | RVA | Unique oracle bytes |
|---|---:|---:|
| `CSound::PlaySoundDS(int,int,int,int)` | 0x22F00 | 189 |
| `CSound::PlaySoundDS(int,int,int)` | 0x22FC0 | 131 |
| `CSound::StopSound(int)` | 0x23050 | 99 |
| `CSound::PlayImage(int,POINT,int,int)` | 0x230C0 | 351 |
| `CSound::Play(int,int,int)` | 0x23220 | 278 |

[EVIDENCE] All five methods are `STRICT` together in the all-TU copied-tree comparison. The helper bodies at `0x22F00` and `0x22FC0` clean four and three stack arguments respectively; the strict caller bodies at `0x230C0` and `0x23220` confirm the argument use. The overload names are the source-level reconstruction; the oracle itself has no C++ symbols for these bodies.

[EVIDENCE] The same copied-tree comparison has all three affected UI peers `STRICT`: `CButton::TreatEvent` at `0x2D60` (111 oracle bytes), `CButton::MouseMove` at `0x2FD0` (201), and `CMenu::TreatEvent` at `0x1F040` (117). The current `cmpfunc.py` proves their `$L...` switch-table relocations as same-function section-relative labels. This resolves the older `RELOC-UNVERIFIED` label from the stored pre-update report; it is a comparator proof, not a source change. Final one-TU verification is `build/verify-coreres_sound_minimal_verify/results.json`.

[HYPOTHESIS] The successful context is consistent with the inherited `PlaySoundDS(DWORD,DWORD)` declaration contributing to VC5's TU-wide decisions (`VC5-CTX-01..04`). A separate tested variant that kept `StopAllSounds()` but omitted the inherited two-DWORD `PlaySoundDS` left `MouseMove` mismatched; the variants also differ in other API declarations, so this does not isolate a single cause. `StopAllSounds()` has no unique body in the target sound block and its SB1 signature remains open. The successful variant does not need its declaration. Do not promote a canonical sound-header change from this scratch result alone.

**Compiler findings**

- `[BLUPI-CTX-01]` **Blupi-specific / source-context hypothesis:** StopSound-only declaration context broke the three UI peers; keeping the inherited PlaySoundDS declaration and adding the reconstructed helper overloads restored strict bodies in the full copied-TU build. Variation: private header only, with real source declarations; result: all five sound bodies and all three UI peers strict. Related: `VC5-CTX-01..04`. Exact original SB1 declaration order remains unproven.

### Pixmap layout and mouse source forms

[EVIDENCE] Static target offsets establish `m_bFullScreen` at `+0`, `m_bTrueColorBack` at `+4`, and `m_mouseType` at `+8`; later member offsets remain unchanged. The private `pixmap.h` now uses this order and declares the target five-argument `Create(HWND,POINT,BOOL,BOOL,int)`. Target `Create` stores the true-colour argument at `+4`, uses `+8` for mouse type, creates a 64x64 mouse surface, and selects 16-bit versus 8-bit display mode from the true-colour field. The candidate is still `MISMATCH` at `0x21190`: 928 candidate bytes versus 902 oracle bytes.

[EVIDENCE] The private `def.h` sets `CHBLUPI=10`; the target mouse routines access the surface pointer at `this+0x94`, which is index 10 from the array at `+0x6C`. The target cursor IDs in the scratch header are Arrow 1, Pointer 2, directional arrows 3 through 10, Wait 11, Empty 12, Disable 13. Target mouse routines skip 12/13, and `MouseBackClear` also skips 13.

| Strict candidate body | RVA | Unique oracle bytes |
|---|---:|---:|
| `CPixmap::CPixmap` | 0x21040 | 133 |
| `CPixmap::MouseUpdate` | 0x22370 | 326 |
| `CPixmap::MouseBackClear` | 0x22590 | 18 |
| `CPixmap::MouseBackSave` | 0x226C0 | 241 |
| `CPixmap::MouseBackRestore` | 0x227C0 | 193 |
| `CPixmap::MouseHotSpot` | 0x22910 | 43 |

[EVIDENCE] `MouseHotSpot` uses a four-pair private table that matches the target's initialized data at `0x47B260`; the target code and both data relocations verify `STRICT`. The target accepts sprite values 1 through 11 while only the first four table pairs are established here. Reads for values 4 through 11 reach adjacent target data, so their source-level behavior remains open.

[EVIDENCE] `MouseBackDraw` now calls a `void MouseRectSprite(RECT*,POINT*)` helper and uses the target rectangle-based clipping expressions. Target call setup and `ret 8` identify this helper ABI at `0x22890`; it selects ranks 37, 38, and 39 for the default, pointer, and wait cursors and reads six shorts per rank from static metadata at `0x47A712`. The candidate helper has the correct ABI and replaces the unsupported `MouseBackDebug` body, but remains `MISMATCH` at 112/123 bytes. `MouseBackDraw` remains `MISMATCH` at 272/268 bytes. The atlas metadata source is not reconstructed, and no binary-derived table was copied into candidate source.

[EVIDENCE] The final isolated-TU verification is `build/verify-coreres_pixmap_final/results.json`: 29 strict, 1 relocation-unverified, and 28 mismatching candidate functions. The strict target spans listed above total 954 unique oracle bytes.

**Header deltas and compiler findings**

- `scratch/tu/pixmap.h`: field order places true-colour at `+4` and mouse type at `+8`; `Create` takes five logical arguments; `MouseRectSprite` takes `RECT*` and `POINT*`. Evidence is the constructor/Create stores, target helper call setup, and `ret 8`. These are private scratch declarations only.
- `scratch/tu/def.h`: `CHBLUPI=10` and the target cursor ID sequence above. Evidence is target indexed surface access and the ChangeSprite/mouse-body immediates. No canonical header delta is proposed.
- `[BLUPI-LAY-01]` **Blupi-specific, confirmed:** swapping the two four-byte fields and correcting `CHBLUPI` restores strict constructor, MouseUpdate, MouseBackSave and MouseBackRestore bodies; the evidence is the exact target offsets and full-TU comparison. `Create` remains a source/layout residual.
- `[VC5-SEM-01]` **Blupi-specific, confirmed for these methods:** using the target cursor IDs and disable/empty guards yields strict MouseUpdate, MouseBackClear, MouseBackSave and MouseBackRestore bodies. The four-pair MouseHotSpot data correction also yields strict code and verified initialized-data relocations. Do not generalize this to other TUs.
- `[VC5-SEM-01]` **Unverified source reconstruction:** the Create path and atlas metadata still differ; KB hints rank constants, displacements, loads and structure before source-form guessing. No candidate table was added for the unresolved atlas data.

### Misc residuals

| Candidate body | Result | Evidence |
|---|---|---|
| `AddUserPath` | `STRICT`, 0x1F560, 300 oracle bytes | All relocations verified in `build/verify-coreres_misc_final/results.json`. |
| `TraceErrorDD` | `STRICT`, 0x1F690, 2,075 Ghidra bytes | Candidate COFF span is 2,320 bytes up to the next target function at `0x1FFA0`; all relocations verify. Unique-byte count follows the 2,075-byte oracle extent. |
| `GetCurrentDir` | `RELOC-UNVERIFIED`, 0x1F360, 176/170 bytes | `__pgmptr` has no unique reconstructed definition. |
| `ChangeSprite` | `MISMATCH`, forced 0x1F210, 288/285 bytes | Scratch cursor IDs now match target observations; branch/register residue remains. |
| `Random` | `MISMATCH`, no unique target location | Oracle order has `InitRandom` at `0x1F350` followed by `GetCurrentDir` at `0x1F360`; no unique `Random` body was found. |

[EVIDENCE] The original case-specific `sprintf(dderr, literal)` switch arms in the current source produce strict `TraceErrorDD`. A tested pointer-per-case plus one common `sprintf` variant is 1,968 bytes and remains `MISMATCH` at the forced `0x1F690` entry. KB tags that variation with branch, load, stack and structural differences (`VC5-CTL-01`, `VC5-CTL-03`, `VC5-INL-03`); it is a negative source-form result, not a reason to keep the pointer form. `ChangeSprite` remains a 288/285-byte mismatch after the cursor-ID correction (`VC5-SEM-01`, `BLUPI-DIA-01`).

[EVIDENCE] `AddUserPath` and `TraceErrorDD` are strict in `build/verify-coreres_misc_final/results.json`; `GetCurrentDir` remains relocation-unverified and `ChangeSprite`/`Random` remain unmatched. `scratch/tu/misc.cpp` keeps the cursor experiment and the strict case-specific `TraceErrorDD` form. No canonical edits were made.

**Compiler findings**

- `[BLUPI-MISC-01]` **Blupi-specific / negative:** correcting cursor constants in `ChangeSprite` changed the candidate to 288 bytes but did not match the 285-byte target. Related: `VC5-SEM-01`, `BLUPI-DIA-01`. Keep the target ID evidence separate from unresolved register/control-flow source form.
- `[BLUPI-MISC-02]` **Negative:** `TraceErrorDD` pointer assignment plus shared formatting changed 2,320 to 1,968 bytes and remained mismatched; the case-specific `sprintf` source form is strict. Related: `VC5-CTL-01`, `VC5-CTL-03`, `VC5-INL-03`.

[EVIDENCE] Unique oracle spans for the strict candidate methods in this round total 4,377 bytes: 1,048 sound, 954 pixmap, and 2,375 misc. UI peer bodies are omitted from that tally because they were already accepted functions. This is a scratch-candidate inventory, not a net project-byte increase.


## Refreshed residuals round (2026-10-17)

[EVIDENCE] The `scratch/tu` copies for `sound.cpp`, `pixmap.cpp`, `misc.cpp` and the private headers were refreshed from the post-`c3369b5` canonical tree before this round. Only scratch candidates and this document changed; no file under `src/` was edited. Final isolated-TU builds use `tools/verify.py --tu ... --build-dir ...`; explicit data-address proofs use all object files from the same build with `tools/cmpfunc.py`.

### Pixmap target bodies in oracle order

| Oracle RVA / bytes | Candidate identity and current result |
|---|---|
| `0x21190` / 902 | `CPixmap::Create`: `MISMATCH`, 912 candidate bytes. The current target field offsets are retained; the Create body still differs. |
| `0x217a0` / 98 | `CPixmap::DrawMap`: `STRICT`, 112-byte object span. The target body is a per-channel `Blt` wrapper with `DDBLT_WAIT`, `RestoreAll`, and a `DDERR_WASSTILLDRAWING` retry. |
| `0x21b20` / 770 | `[HYPOTHESIS]` `CPixmap::QuickIcon(int,int,POINT)`: target pseudocode has the expected channel/rank/point ABI, atlas selection, clipping and back-surface `BltFast`. The best candidate is 800 bytes and remains `MISMATCH`; target table read order was tested. An SB2-style inline channel helper grew it to 816 bytes. |
| `0x21e30` / 719 | `CPixmap::DrawIcon`: `STRICT` after proving the four atlas relocation bases (`table_icon_object=0x479b80`, `table_icon_element=0x47a710`, `table_icon_blupi=0x478dd8`, `table_icon_explo=0x47afd8`). Candidate span is 720 bytes. |
| `0x22100` / 185 | `CPixmap::DrawPart`: `STRICT`, 192-byte object span after changing the mask color to `0x000000ff`. |
| `0x225b0` / 268 | `CPixmap::MouseBackDraw`: `STRICT`, 272-byte object span. |
| `0x22890` / 123 | `CPixmap::MouseRectSprite`: `STRICT` after mapping `table_icon_element=0x47a710`; candidate span is 128 bytes. |

[EVIDENCE] The strict pixmap bodies above cover 1,393 unique oracle bytes: `DrawMap` 98, `DrawIcon` 719, `DrawPart` 185, `MouseBackDraw` 268, and `MouseRectSprite` 123. The default pixmap verify reports 32 `STRICT`, 3 `RELOC-UNVERIFIED`, 25 `MISMATCH`; after the atlas mappings, `cmpfunc.py` reports 34 `STRICT`, 1 `RELOC-UNVERIFIED`, 25 `MISMATCH`. The remaining relocation-unverified body is the ambiguous 16-byte `Fill` candidate. Artifacts: `build/verify-coreres_final_pixmap/results.json` and `build/coreres/pixmap_final_mapped.json`.

[OPEN] The target atlas bases are established by target code and data references, but table contents are not reconstructed. Target entry counts are object 246, element 187, blupi 291, explo 54; current `src/pixtables.h` has 441, 289, 336, 100 respectively. The scratch extern declarations prove code relocations only; do not treat the PB-derived table data as SB1 data.

[OPEN] Of the remaining 25 pixmap mismatches, 14 are emitted `POINT` operator COMDATs from headers, with no matching oracle function boundary identified. The 11 non-inline candidate mismatches are `Create`, `IsPalette`, `SearchColor`, the two smaller `Cache` overloads, `QuickIcon`, `IsIconPixel`, `DrawIconDemi`, `DrawIconPart`, `BuildIconMask`, and `SetMousePos`. `IsPalette`, `SearchColor`, and the cache/drawing helpers have no unique target placement in the current inventory. The target at `0x21b20` is the clipped icon blit described above, not a color search.

### Misc and sound residuals

[EVIDENCE] `GetCurrentDir` at `0x1f360` is `STRICT` (170 oracle bytes) when the four relocations are resolved, including `__pgmptr=0x47e814`, proven by target pseudocode. Default verification remains `RELOC-UNVERIFIED` because that global has no unique reconstructed definition. The candidate object span is 176 bytes. Artifact: `build/coreres/misc_final_mapped.json`.

[EVIDENCE] `ChangeSprite` uses the target cursor IDs in the private `def.h`: Arrow 1, Pointer 2, arrows 3 through 10, Wait 11, Empty 12, Disable 13. Target `0x1f210` confirms these IDs and has no Map or Fill cursor branch. The best candidate remains `MISMATCH`, 288 bytes versus 285. `Random` remains a 32-byte mismatch without a unique target body; oracle order places `InitRandom` at `0x1f350` and `GetCurrentDir` at `0x1f360`.

[OPEN] No SB1 `StopAllSounds` body or call target was identified. The target body at `0x23610` is a table lookup helper. SB2 ancestry supplies a protected `BOOL StopAllSounds()` prototype between `CreateBufferFromWaveFile` and `PlaySoundDS`; a copied-tree test of that declaration changed none of 248 compared accepted statuses. This is only evidence that this prototype is context-neutral in that test, not proof of the original SB1 declaration or signature. No canonical sound header change is proposed.

### Private header deltas

- `scratch/tu/pixmap.h`: exposed the existing `DrawMap(int,RECT,RECT)` and `QuickIcon(int,int,POINT)` declarations outside `SB1_GAME_API`, because the manifest flags do not define that macro. `DrawMap` is strict; `QuickIcon` remains a hypothesis and mismatch. Keep this delta private pending shared-header integration.
- `scratch/tu/def.h`: corrected cursor IDs and removed unused `SPRITE_MAP` / `SPRITE_FILL` definitions in this candidate. Evidence is the target `ChangeSprite` body at `0x1f210`.
- `scratch/tu/pixmap.cpp`: declares the four target atlas arrays as externs; the address mappings above verify code references only. No canonical header or data-table delta is proposed.
- StopAllSounds was tested only in `scratch/coreres/stopall_signature/src/sound.h`; the canonical `src/sound.h` is unchanged.

**Compiler findings**

- `[VC5-CTX-01..04]` **Confirmed SB1 context effect:** adding the real `DrawMap` body at its target TU position changed the unchanged `MouseBackDraw` body from `MISMATCH` to `STRICT` (the byte span stayed 272). This is consistent with the already confirmed TU-context behavior; no probe declaration was retained.
- `[VC5-SEM-01]` **Blupi-specific, confirmed:** target `DrawPart` uses the mask color `0xff`; the prior white `0xffffff` source did not match. The `0xff` candidate is strict at `0x22100`.
- `[VC5-SEM-01]` **Blupi-specific, semantic correction only:** the target `ChangeSprite` cursor IDs differ from the inherited PB constants. Correcting them reduced the candidate to 288 bytes, but it still mismatches the 285-byte target; branch/register form remains open.
- `[VC5-CTX-01..04]` **Negative source-form result:** target `QuickIcon` tags include frame, stack, branch, condition, immediate and displacement differences. Reordering table reads did not close the 800/770-byte gap, and the SB2 inline channel-helper form increased it to 816 bytes. No transferable source-form rule was established.
