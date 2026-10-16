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
