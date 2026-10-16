# Planet Blupi v1.8.0 → Speedy Blupi 1 ancestry map

**PB provenance for every source row and candidate below:** tag `v1.8.0`, commit `203ccc8b5908d4075e97be4f958fdfc1c42f862c` (2017-01-21 source import; original project release dated 1997). The tree has known post-1997 edits, so it is a comparison snapshot, not byte-authoritative pristine source.

## Findings

[EVIDENCE] A substantial support layer from Planet Blupi is present in BLUPI.EXE: application startup/configuration, DirectDraw helpers, cursor/path utilities, DirectDraw/DirectSound diagnostics, movie playback, sound/MIDI, event handling, and world save/load routines. The strongest direct code anchors are exact PB strings referenced by instructions, imports used in the matching function, and matching source control flow. Four static initializer byte sequences also occur exactly in the oracle.

[EVIDENCE] The executable also contains Speedy-specific changes. `AddCDPath` tests `data\\`, `image\\`, `image08\\`, `image16\\`, and `movie\\`; `AddUserPath` uses `c:\\Speedy Blupi\\`. The PB snapshot has a simpler `AddCDPath` and Planet Blupi paths. The save/load code is further from PB: the oracle writes a 0x364-byte header with revision 1.3, while the PB source sets revision 1.5; the oracle writes two 10,000-element, 2-byte map planes and 200 0x30-byte records. The PB declarations describe 16-byte `Cellule` entries and `MAXBLUPI=100`.

[HYPOTHESIS] The best ancestry model is a mixed one: the application, graphics, audio, movie, and utility layers retain identifiable PB functions, while the world schema and gameplay state were substantially adapted for Speedy Blupi. The evidence does not establish that the full Planet Blupi simulation source was carried into SB1 unchanged.

[OPEN] No positive code anchor was found for many gameplay-only PB files. A missing literal/table anchor is not proof that a source file is absent: the code could be rewritten, merged, or stripped as unreferenced.

## Evidence and method

- Extracted the pinned PB C++ source and compared every same-named PB/SB2 `src/` pair. Normalized ratios collapse whitespace only; none of the pairs is identical after that normalization. Highest ratios are `wave.cpp` 0.9996, `wave.h` 0.9975, `movie.cpp` 0.9968, and `misc.cpp` 0.9675. This comparison indicates source reuse across generations, but does not by itself identify SB1 code.
- Scanned PE `.data` and `.rdata` for PB string literals after removing C/C++ comments, then matched absolute x86 operands in Ghidra-bounded `.text` functions with Capstone 5.0.3. Current output is 240 source-literal records and 232 direct code references. These counts include repeated source occurrences and pooled literals, not 240 unique strings.
- Mapped direct IAT calls and import-thunk calls to Ghidra function spans. Headless analysis found 579 function bodies. Function names in reports are Ghidra-generated, not original symbols; method names below are inferred from PB string locations, imports, and control flow.
- Parsed 138 numeric initializer arrays from PB `.cpp` files and searched for exact byte sequences in oracle `.data`/`.rdata`. Four sequences matched. Scanned data for contiguous pointers to analyzed internal function entries; two short repeated-pointer runs were found, but neither resembles a useful multi-method application vtable.
- All analysis scripts, extracted source, function spans, and TSV evidence are under `scratch/pb/` and `build/pb/`. BLUPI.EXE was analyzed statically and was not executed.

## Strong code and data anchors

| PB source | Concrete oracle evidence | Assessment |
|---|---|---|
| `blupi.cpp` | `data\\config.def` is at data RVA `0x2E094` and is pushed at code RVA `0x0103C` inside function `0x01030` (673-byte body). The config keys `CD-Rom=`, `-nocd`, `SpeedRate=`, `Timer=`, `FullScreen=`, and `MouseType=` are referenced by that same function. Startup diagnostics and object creation strings are referenced in `0x01EF0`; DirectX error strings and MessageBox/DestroyWindow calls are in `0x01C90`. | Strong application-shell ancestry; initialization is adapted. |
| `ddutil.cpp` | `DDLoadBitmap` messages reference function `0x03110` (397 bytes), which calls `GetObjectA`, `LoadImageA`, and `DeleteObject`. `DDCopyBitmap` errors reference `0x03340` (499 bytes), with `CreateCompatibleDC`, `SelectObject`, `StretchBlt`, and `DeleteDC`. Palette messages reference `0x03540` (608 bytes), which uses `FindResourceA`, `LoadResource`, `LockResource`, `_lopen`, and `_lclose`. | Strong utility-layer ancestry. These are GDI/resource helper calls; the main DirectDraw interface calls are in CPixmap. |
| `misc.cpp` | The DirectDraw error-name strings and `DirectDraw Error %s in file %s at line %d` are referenced in `0x1F690` (2,075 bytes); DirectSound error-name strings and the corresponding format string are referenced in `0x1FFA0` (423 bytes). Cursor names reference `0x1F210` (285 bytes); `\\debug\\` references `0x1F360` (170 bytes). | Strong ancestry, but path logic is demonstrably adapted (see below). |
| `decio.cpp` / `CDecor` | The exact path `data\\world%.3d.blp` is in data RVA `0x34440`; code at `0x148BB` pushes it from function `0x14880` (86 bytes), called by the neighboring writer/reader routines `0x148E0` and `0x14AF0`. The writer allocates/writes a 0x364-byte header, stores revision 1.3, writes two arrays as `(count=0x2710, element size=2)`, then `(count=0xC8, element size=0x30)`. | Strong ancestry for file/path operations; strong evidence that the serialized schema changed from PB. |
| `event.cpp` / `CEvent` | `Version 1.0` references `0x16F70` (7,419 bytes). Music and play/win movie paths reference `0x19310` (6,919 bytes); the `.blp` extension references `0x1C0B0` (349 bytes); demo paths reference `0x1C6F0` (230 bytes) and `0x1C7E0` (447 bytes); `movie\\essai.avi` is in `0x1D230` (6,616 bytes). | Strong ancestry for event/media flows; SB2’s same-named `event.cpp` has diverged substantially (ratio 0.2072). |
| `pixmap.cpp` / `CPixmap` | `DirectDrawCreate` is called from `0x21190` (902 bytes); the same body references DirectDraw setup/surface/clipper diagnostics. `CPixmap::RestoreAll` is referenced from `0x215A0` (172 bytes); palette/cache diagnostics reference `0x21870` (391 bytes). The bodies make indirect COM vtable calls on DirectDraw interfaces. | Strong class ancestry, adapted field layout. |
| `sound.cpp` / `CSound` | `DirectSoundCreate` is called at `0x22D6C` inside `0x22D60` (82 bytes); `sound\\sound%.3d.blp` references `0x22E40` (64 bytes). `PlayMusic` strings and MCI error calls are in `0x23340` (431 bytes); `RestartMusic` is in `0x234F0` (80 bytes); MCI stop logic is in `0x23540` (77 bytes). MIDI output calls occur at `0x22C4F` and `0x22C8F`. | Strong ancestry; direct imports and field offsets agree with PB `CSound`. SB2 later splits sound into other files. |
| `movie.cpp` / `CMovie` | `avivideo` is referenced by functions `0x20150` (68 bytes) and `0x201A0` (33 bytes). They call `mciSendCommandA` and `mciGetDeviceIDA`; adjacent functions use `MoveWindow`, `InvalidateRect`, `UpdateWindow`, and MCI open/status/play/pause/close operations. | Strong ancestry; the PB/SB2 source pair is 99.68% similar after whitespace collapse, with the main source-level change being an explicit `(LPSTR)` cast. |

### Exact numeric data

Only 4 of the 138 scanned initializer arrays matched as exact contiguous bytes:

| PB source array | Exact oracle bytes |
|---|---|
| `decor.cpp:table_object` (26 shorts) | `.data` RVA `0x51D7C` |
| `decor.cpp:table_floor` (8 shorts) | `.data` RVA `0x5FE24` |
| `menu.cpp:table_button_icon` (35 shorts) | `.data` RVA `0x77F38` |
| `text.cpp:table_width_little` (128 bytes) | `.data` RVA `0x7BBC0` |

[EVIDENCE] These matches tie small pieces of `CDecor`, `CMenu`, and the free text-rendering helpers to the oracle independently of names or strings. The large PB `tableFloor` and `tableObject` arrays in `tablefloor.cpp` and `tableobj.cpp` (10,000 integers each) did not appear as exact byte sequences. Large `decgoal.cpp`/`decmove.cpp` tables and `action.cpp:rotate_table` also did not match exactly.

### Imports, member offsets, and vtables

- `CPixmap::Create` calls `DDRAW!DirectDrawCreate` at code RVA `0x21253`; subsequent calls through COM vtables (for example `[edx+0x50]`, `[edx+0x54]`, and surface methods `[ecx+0x18]`, `[ecx+0x20]`, `[ecx+0x70]`) match the source’s DirectDraw setup sequence. `ddutil.cpp` itself accounts for the GDI/resource imports listed above.
- `CSound::Create` stores the DirectSound interface at `this+0x0C`, matching PB’s `HWND`, `m_bEnable`, `m_bState`, then `m_lpDS` field order. In `CSound::PlayMusic`, oracle accesses around `this+0x268`, `+0x26C`, `+0x2A4`, `+0x2A8`, and `+0x2AC` align with PB `m_MidiDeviceID`, `m_MIDIFilename`, MIDI/last-MIDI volume, and suspend-skip fields. This is strong evidence that the `CSound` layout prefix and tail fields survived.
- `CMovie` has an especially clear field match: PB’s five 32-bit fields place the MCI device ID at `+4`, movie window at `+8`, playing flag at `+0x0C`, and open flag at `+0x10`. Oracle `fileCloseMovie` reads the device ID at `+4` and clears `+0x0C`/`+0x10`; `positionMovie` reads the window at `+8`. The implied PB object size is `0x14` bytes, with the same observed field offsets in the oracle.
- `CPixmap` shows a four-byte layout shift after its first four flags. PB’s header predicts `m_lpDD` at `+0x54`, surface-pointer array at `+0x68`, filename array at `+0xD1C`, and dimension arrays at `+0x14EC`/`+0x180C`. Oracle code stores the DirectDraw interface at `+0x58`; `RestoreAll` loops 100 surface pointers from `+0x6C`, walks 20-byte filename slots from `+0xD20`, and uses dimension arrays at `+0x14F0`/`+0x1810`. [HYPOTHESIS] SB1 inserted a four-byte member before the DirectDraw handles; the later arrays all move by exactly four bytes.
- None of the PB class declarations has a `virtual` method or base class, so those classes do not imply C++ vtables. The two oracle pointer runs at `.data` RVAs `0x7BD30` (`0x278D0` twice) and `0x7E580` (`0x2B2D0` six times) are repeated targets, not persuasive class vtables. The indirect calls described above are COM interface vtables.
- PB `text.h` declares free drawing/measurement functions, not a `CText` class. `fifo.h` declares `CPileTriee`, not a `CFifo` class.

## PB-to-SB2 source comparison

| Same-named pair | Normalized source ratio | SB1 implication |
|---|---:|---|
| `movie.cpp` / `src/movie.cpp` | 0.9968 | MCI implementation remained almost verbatim across source generations; oracle MCI function cluster confirms this implementation family. |
| `wave.cpp` / `src/wave.cpp` | 0.9996 | Almost verbatim between source snapshots, but no non-comment PB string from `wave.cpp` was found in BLUPI.EXE. Its presence in SB1 remains open; it may have been unreferenced and stripped. |
| `misc.cpp` / `src/misc.cpp` | 0.9675 | Similar file, but both generations changed behavior from PB. Oracle code at `0x1F410` checks the five path prefixes `data\\`, `image\\`, `image08\\`, `image16\\`, and `movie\\`; `0x1F560` references `c:\\Speedy Blupi\\` at data RVA `0x78044`. PB v1.8 checks fewer prefixes and uses Planet Blupi install paths. |
| `ddutil.cpp` / `src/ddutil.cpp` | 0.8148 | DirectDraw/GDI helper family survives, but file-level similarity is insufficient to claim all helper bodies are unchanged. |
| `event.cpp`, `decor.cpp`, `decmove.cpp`, `decblupi.cpp`, `obstacle.cpp` | 0.2072, 0.0687, 0.0447, 0.0225, 0.0141 | Same-name files are substantially different by SB2. This does not prove SB1 lacked PB ancestry; it marks the gameplay files that need function-level matching next. |

## Per-file and per-class map

Confidence meanings: **strong ancestry** = exact referenced strings/imports or exact table bytes; **structural similarity** = class/member/schema shape or same-name source comparison without a decisive function anchor; **speculative** = no independent binary anchor yet. `[OPEN]` records insufficient evidence rather than a claim of absence.

| PB file | Likely SB1 status | Confidence | Evidence / limitation |
|---|---|---|---|
| `action.cpp` | Adapted or replaced | speculative | `rotate_table` (121 shorts) is not an exact `.data` sequence; no direct string/import anchor. |
| `action.h` | Open | speculative | No independent oracle anchor; likely shares declarations with the gameplay code. |
| `actions.h` | Open | speculative | No independent oracle anchor; enum/constants need function matching. |
| `arrange.cpp` | Adapted or replaced | speculative | `tableMurDir` is not an exact initializer match; no direct code anchor. |
| `blupi.cpp` | Adapted | strong ancestry | Configuration strings, `Blupi` window title, DirectX errors, startup cache/error messages, and API calls map to `0x01030`, `0x01C90`, and `0x01EF0`. |
| `button.cpp` | Likely adapted | structural similarity | UI source pair ratio 0.6951; `CEvent` embeds `CButton` objects, but no button-specific literal/table anchor. |
| `button.h` | Likely retained with edits | structural similarity | Same-name header ratio 0.8127 and `CEvent` composition only. |
| `chemin.cpp` | Possibly absent or merged | speculative | No same-named SB2 file and no binary string/table/import anchor. |
| `ddutil.cpp` | Helper routines retained, with edits possible | strong ancestry | Four helper bodies are tied by unique diagnostics, code references, and GDI/resource import patterns. |
| `ddutil.h` | Retained interface likely | strong ancestry | Same-name ratio 0.9244 and corresponding helper bodies in `.text`. |
| `decblupi.cpp` | Adapted or replaced | speculative | No direct string anchor; source-to-SB2 ratio 0.0225. The world writer’s 0x30-byte records do not match PB’s large `Blupi` state shape. |
| `decgoal.cpp` | Adapted or replaced | speculative | Numerous goal tables scanned; none matched exactly. No direct string anchor. |
| `decgoal.h` | Open | speculative | No direct anchor; depends on unverified goal code. |
| `decio.cpp` | Adapted | strong ancestry | Shared world filename/path helper and matching save/load flow, but header revision, size, array encoding, and record count differ from PB. |
| `decmap.cpp` | Open | speculative | No independent oracle anchor or same-named SB2 source. |
| `decmove.cpp` | Adapted or replaced | speculative | Same-name ratio 0.0447; movement tables were not found as exact data. |
| `decmove.h` | Open | speculative | No direct anchor; only an 80-byte declaration header and no same-named SB2 counterpart. |
| `decor.cpp` | Adapted | strong ancestry | `table_object` and `table_floor` are exact data matches; the world save routine ties CDecor behavior to the oracle, while serialized layout differs. |
| `decor.h` | Adapted | strong ancestry | CDecor survives structurally, but oracle world storage has two 10,000×2-byte planes and 200×0x30 records; PB declares 16-byte `Cellule` entries and `MAXBLUPI=100`. |
| `decstat.cpp` | Open | speculative | No independent string/table/import anchor. |
| `def.h` | Adapted | structural similarity | Same-name ratio 0.2315; shared dimensions/file code are plausible, but many constants and the cell schema changed. |
| `event.cpp` | Adapted | strong ancestry | Exact version, movie, music, demo, and extension strings map into several event-handler-sized functions. |
| `event.h` | CEvent likely retained with edits | strong ancestry | Event methods are anchored in `.text`; PB declares `CEvent` with embedded `CButton`, `CJauge`, and `CMenu`. Exact full object layout remains open. |
| `fifo.cpp` | Possibly absent | speculative | No direct binary anchor and no same-named SB2 source. |
| `fifo.h` | Possibly absent | speculative | PB declares `CPileTriee`; no oracle-specific use identified. |
| `fog.cpp` | Possibly absent or merged | speculative | No direct string/table/import anchor. |
| `jauge.cpp` | Likely adapted | structural similarity | Same-name ratio 0.7806; startup caches `jauge.blp` and `CEvent` embeds two `CJauge` objects, but no function-specific anchor. |
| `jauge.h` | Likely retained with edits | structural similarity | Same-name ratio 0.8498 and event composition only. |
| `menu.cpp` | Lookup data unchanged; methods uncertain | strong ancestry | `table_button_icon` matches 35 shorts exactly at `.data:0x77F38`; source file ratio 0.4421. |
| `menu.h` | Likely retained with edits | structural similarity | Same-name ratio 0.8958; `CEvent` embeds `CMenu`. |
| `misc.cpp` | Adapted | strong ancestry | Cursor/path/debug functions and DirectDraw/DirectSound error maps are directly referenced; Speedy path and extra image-path branches prove edits. |
| `misc.h` | Retained interface likely | structural similarity | Same-name ratio 0.9183; associated functions are present, though signatures are not recoverable from stripped code. |
| `movie.cpp` | MCI core likely unchanged; wrapper adapted | strong ancestry | MCI imports, control flow, and the `CMovie` field offsets at `+4`, `+8`, `+0x0C`, and `+0x10` match PB; PB/SB2 source pair ratio 0.9968. |
| `movie.h` | Retained with edits possible | strong ancestry | `CMovie` method cluster is present; same-name header ratio 0.8450. |
| `obstacle.cpp` | Adapted or replaced | speculative | Same-name ratio 0.0141 and no direct string/table anchor. |
| `pixmap.cpp` | Adapted | strong ancestry | DirectDraw creation, diagnostic strings, COM call structure, `RestoreAll`, and cache flow map to oracle functions; object fields are shifted by one DWORD. |
| `pixmap.h` | Adapted layout | strong ancestry | Methods are directly anchored; PB member offsets after flags differ by +4 in the oracle. |
| `resource.h` | Resource mappings retained with edits | structural similarity | Same-name ratio 0.8254; `IDR_MAINFRAME`/cursor resource names are referenced during initialization and resource use. |
| `resrc1.h` | Open; possibly resource-only or absent | speculative | No direct C++ anchor; resource script linkage needs separate `.rsrc` mapping. |
| `sound.cpp` | Adapted or retained in substantial part | strong ancestry | DirectSound, MIDI, MCI, filename, and debug anchors identify CSound routines; SB2 later splits implementations into `sound_dx.cpp` and `sound_bass.cpp`. |
| `sound.h` | CSound layout likely retained | strong ancestry | Same-name ratio 0.8483; oracle field offsets at `+0x0C` and `+0x268..+0x2AC` agree with PB declarations. |
| `tablefloor.cpp` | Static PB floor map absent or replaced | structural similarity | Its 10,000-element initializer did not occur byte-for-byte in oracle data; no function anchor. |
| `tableobj.cpp` | Static PB object map absent or replaced | structural similarity | Its 10,000-element initializer did not occur byte-for-byte in oracle data; no function anchor. |
| `text.cpp` | Lookup data unchanged; routines likely retained with edits | strong ancestry | 128-byte `table_width_little` matches at `.data:0x7BBC0`; source pair ratio 0.6695. |
| `text.h` | Free text API likely retained with edits | structural similarity | PB exposes free functions rather than a `CText` class; same-name header ratio 0.5430. |
| `wave.cpp` | Possibly absent or dead-stripped | speculative | Near-identical to SB2 source (0.9996), but no non-comment PB string or direct import/code anchor was found in BLUPI.EXE. |
| `wave.h` | Open | speculative | Near-identical to SB2 header (0.9975), but no independent oracle footprint. |

## Ranked byte-match candidates for the compile/match pass

These are **candidates**, not strict matches. RVA and size are the Ghidra contiguous function envelope in the oracle; names are inferred from PB source functions and their direct evidence. The rank favors small, self-contained functions with strong source/control-flow anchors. `movie.cpp` has the strongest source-to-SB2 continuity; sound and misc candidates have stronger direct SB1 evidence than cross-generation source comparison. Every candidate’s upstream source is the PB commit recorded at the top.

| Rank | PB function | Oracle RVA | Size | Evidence supporting an exact-byte attempt |
|---:|---|---:|---:|---|
| 1 | `CMovie::positionMovie` | `0x201D0` | `0x23` (35) | Small wrapper; oracle calls `MoveWindow` with the same movie rectangle fields as PB. |
| 2 | `CMovie::termAVI` | `0x201A0` | `0x21` (33) | `mciGetDeviceID` followed by `mciSendCommand(MCI_CLOSE)`, matching the short PB routine. |
| 3 | `CMovie::initAVI` | `0x20150` | `0x44` (68) | `avivideo` reference plus `mciSendCommand(MCI_OPEN)` and the same success test. |
| 4 | `CMovie::fileCloseMovie` | `0x20200` | `0x43` (67) | MCI close, flag clears, invalidate/update sequence matches PB. |
| 5 | `CMovie::playMovie` | `0x203C0` | `0x80` (128) | Two-way MCI play/pause branch, direct imports, and PB/SB2 source continuity. |
| 6 | `CMovie::fileOpenMovie` | `0x20250` | `0x165` (357) | MCI open/status flow, filename handling, and `positionMovie` call correspond to PB method structure. |
| 7 | `CSound::CacheAll` | `0x22E40` | `0x40` (64) | Exact `sound\\sound%.3d.blp` string, 100-channel loop, and call to the per-channel cache routine. |
| 8 | `CSound::Create` | `0x22D60` | `0x52` (82) | DirectSoundCreate plus writes to `m_lpDS` at PB offset `+0x0C`. |
| 9 | `CSound::StopMusic` | `0x23540` | `0x4D` (77) | MCI stop/close behavior and MIDI-state field offsets match the class interface. |
| 10 | `CSound::RestartMusic` | `0x234F0` | `0x50` (80) | Exact debug literal and calls into the music stop/play path. |
| 11 | `CSound::ReadData` | `0x22A10` | `0x122` (290) | Sound-buffer lock/read branches and the `Data1` diagnostic correspond to PB `ReadData`. |
| 12 | `LoadString` (misc wrapper) | `0x1F1F0` | `0x1C` (28) | Minimal wrapper around `LoadStringA`; one imported call and a short return path. |
| 13 | `ChangeSprite` | `0x1F210` | `0x11D` (285) | Cursor-name references, sprite comparisons, `LoadCursorA` and `SetCursor`; PB/SB2 difference is the ANSI macro spelling. |
| 14 | `GetCurrentDir` | `0x1F360` | `0xAA` (170) | Path truncation/lowercasing loop and exact `\\debug\\` anchor match PB control flow. |
| 15 | `TraceErrorDS` | `0x1FFA0` | `0x1A7` (423) | DirectSound error-code string table and format string identify the PB switch helper. |
| 16 | `DDLoadBitmap` | `0x03110` | `0x18D` (397) | Unique helper diagnostics plus `GetObjectA`, `LoadImageA`, and DirectDraw surface creation path. |

[OPEN] The short list should be checked against the match worker’s compiled PB fingerprints and exact-byte search. No masked or normalized equality is asserted here.

## Next useful work

Prioritize function matching for `CDecor`/entity state around the save/load routines, then `CEvent` and movement/goal code. Those areas will decide whether the unanchored gameplay files are adapted PB descendants or independent replacements. Keep the serialization differences above in mind when interpreting any shared names or data structures.
