# UI cluster matching work

Scope: `button.cpp`, `jauge.cpp`, `menu.cpp`, and `text.cpp`, built as separate translation units with VC5 SP3. Candidate sources are in `scratch/tu/`; their include copies and the `decor.h` include-context shim are in `scratch/tu/ui_include/`. Reference sources under `external/` were read only.

## Include-context fix

**[EVIDENCE]** The imported Planet Blupi 1.8 files fail to compile together with VC5 SP3 because lower-case includes such as `sound.h`, `pixmap.h`, and `def.h` are followed through `decor.h` by mixed-case spellings such as `SOUND.H`, `PIXMAP.H`, and `DEF.H`. In this include graph VC5 processes those spellings as separate `#pragma once` identities, producing duplicate declarations (`CSound`, `CPixmap`, `Term`). The same errors occur with `/YX` enabled.

**[EVIDENCE]** The task-local header copy set compiles all four TUs after changing only the three include spellings in the shadow `decor.h` to lower case. No declaration was changed by this fix. The successful command form is recorded in `build/ui/logs/*-o2-gx.log`; the logs show VC5 SP3 11.00.7022, `/O2 /GX`, and `TMP/TEMP=build\\tmp`.

**[HYPOTHESIS]** The 1997 project likely had a consistent include graph, casing, or header/PCH arrangement that avoided reopening these headers under aliases. The Planet Blupi import has no original `.dsp` project file, so its exact historical compile workaround cannot be established. `/YX` alone was tested and does not repair this imported source graph.

Candidate edits to existing PB functions preserve their original line counts where practical; new SB2 helper functions are appended. This keeps existing `__LINE__` values stable for the edited PB routines.

## Comparison method and totals

`S` = `STRICT`; `U` = `RELOC-UNVERIFIED`; `M` = `MISMATCH`; `—` = the baseline did not contain that method. PB baseline results are from unmodified PB source with the include-only fix. SB2 is the existing SB2 `/O2 /Gy /GX` object result. Candidate O2 statuses are from `cmpfunc.py` against the current oracle. Candidate switch-table and call-target relocations marked below were resolved with explicit `--symbol-va` mappings to addresses verified in oracle disassembly; this is still strict byte comparison, not masked or normalized equality.

| TU | PB `/O2 /GX` S/U/M | SB2 `/O2 /Gy /GX` strict methods | Candidate `/O2 /GX` S/U/M |
|---|---:|---:|---:|
| button | 0/2/18 | 10 | 15/1/6 |
| jauge | 0/1/10 | 8 | 8/1/4 |
| menu | 6/2/10 | 4 | 7/1/10 |
| text | 0/2/9 | 1 | 4/0/10 |
| **total** | **6/7/47** | **23** | **34/3/30** |

SB2's per-TU objects also emit fourteen unrelated `POINT` operator helper functions from headers; those are excluded from the UI method counts. Current O2 candidate results are `build/ui/results/ui_final_mapped.json`; `/Gy` results are `ui_gy_mapped.json`. The O2 mapped result includes 34 strict functions, three unverified destructor bodies, and 30 mismatches. One byte-identical candidate getter (`CButton::GetHide`) lands at an oracle address that Ghidra labels as an anonymous body; its method identity is not established.

The final O1 candidates were also compiled and compared without relocation mappings: button 6 strict, jauge 4, menu 1, text 0. This is a lower result and is not directly comparable to mapped O2 for switch-table/call relocations. O2 with `/Gy` retained the same 15/8/7/4 strict counts as O2 without `/Gy`; the remaining losses are not explained by function packaging. The current `tools/verify.py` manifest contains only `sound.cpp`, `movie.cpp`, `misc.cpp`, and `ddutil.cpp`, so it does not accept these four UI TUs as substitutions; they were compiled with `tools/msvc.py` and compared with `tools/cmpfunc.py` directly.

For the no-`/Gy` O2 run, the explicit `--symbol-va` targets were `$L33192=0x00402dd0`, `?MouseDown@CButton@@IAEHUtagPOINT@@@Z=0x00402f70`, `$L33075=0x00402daa`, `$L33073=0x00402d94`, `$L33077=0x00402dc0`, `$L33067=0x00402dc9`, `$L33272=0x0041f0b8`, `$L33203=0x0041f083`, `$L33201=0x0041f06d`, `$L33205=0x0041f099`, `$L33206=0x0041f0af`, `?DrawText@@YAXPAVCPixmap@@UtagPOINT@@PADH@Z=0x00423770`, `?DrawChar@@YAXPAVCPixmap@@PAUtagPOINT@@DH@Z=0x004236b0`, `?DrawCharSingle@@YAXPAVCPixmap@@UtagPOINT@@HH@Z=0x00423660`, and `?GetCharWidth@@YAHDH@Z=0x00423610`. These switch labels are compiler-output-specific; `/Gy` labels were mapped independently. The JSON stores each resolved relocation target.

## Oracle layout evidence

The offsets below are from static disassembly of the original executable. “COFF bytes” is the candidate object function size; “oracle bytes” is the Ghidra `oracle/functions.csv` size. The comparator status is reported separately because Ghidra function boundaries and COFF symbol extents are not always the same.

### `CButton`

**[EVIDENCE]** The oracle directly accesses these `this` offsets:

| Field/use | Offset |
|---|---:|
| `m_hWnd` | `+0x00` |
| `m_pPixmap` | `+0x04` |
| `m_pSound` | `+0x0c` |
| `m_type` | `+0x10` |
| `m_bEnable` | `+0x14` |
| `m_bDemo` | `+0x18` |
| `m_bHide` | `+0x1c` |
| `m_message` | `+0x20` |
| `m_pos` | `+0x24`, `+0x28` |
| `m_dim` | `+0x2c`, `+0x30` |
| `m_state` / `m_mouseState` | `+0x34` / `+0x38` |
| `m_iconMenu[20]` / `m_toolTips[20]` | `+0x3c` / `+0x8c` |
| `m_nbMenu` / `m_nbToolTips` | `+0xdc` / `+0xe0` |
| `m_selMenu` / `m_bMouseDown` | `+0xe4` / `+0xe8` |
| `m_bMinimizeRedraw` / `m_bRedraw` | `+0xec` / `+0xf0` |

The candidate adds the demo flag at `+0x18`, plus SB2-style `SetIconMenu` and `SetToolTips` methods. **[OPEN]** The header retains a `CDecor*` in the pointer slot at `+0x08`; these direct method disassemblies do not establish the purpose or use of that slot.

The oracle `Create` at `0x2a30` ends in `ret 0x20`, consistent with eight stack dwords: window, pixmap, sound, two `POINT` coordinates, type, minimize-redraw, and message. Menu and tooltip arrays are set by separate functions at `0x2ae0` and `0x2b10`. Oracle `Draw` at `0x2b40` returns early when hidden and uses demo/disabled icon ranks 5 and 4. Oracle `MouseDown` (`0x2f70`) and `MouseMove` (`0x2fd0`) contain no `PostMessage(WM_UPDATE)` call.

### `CJauge`

**[EVIDENCE]** Oracle methods use `m_hWnd +0x00`, `m_pPixmap +0x04`, `m_pSound +0x08`, `m_bHide +0x0c`, `m_pos +0x10`, `m_dim +0x18`, `m_type +0x20`, `m_level +0x24`, `m_bMinimizeRedraw +0x28`, and `m_bRedraw +0x2c`. The candidate removes the PB `CDecor*` member so these fields line up. `GetLevel`, `GetType`, and `GetHide` read `+0x24`, `+0x20`, and `+0x0c` respectively.

### `CMenu`

**[EVIDENCE]** Oracle constructor, delete, detect, and mouse code directly establish `m_pos +0x10/+0x14`, `m_dim +0x18/+0x1c`, button count `+0x20`, `m_nbCel.y +0x28`, selection rank `+0x210`, and mouse position `+0x214/+0x218`. Constructor/delete initialize button count to zero and selection rank to `-1`. Other menu member offsets are not claimed here.

### Text channel constants

**[EVIDENCE]** Oracle `DrawCharSingle` at `0x23660` passes channel constants 7 and 6. The candidate shadow `def.h` therefore sets `CHLITTLE=7` and `CHTEXT=6` (PB had 8 and 13). The shadow header also undefines the Windows `DrawText` and `GetCharWidth` aliases after their original definitions so these project functions keep their intended symbol names rather than becoming `DrawTextA`/`GetCharWidthA`.

**[HYPOTHESIS]** The appended glyph table in the candidate comes from SB2 `texttables.h`. Its equality with the SB1 table has not been established; `DrawChar` still mismatches, so table identity and glyph logic remain open.

## Per-function results

Sizes are written `oracle RVA / oracle bytes; candidate COFF bytes`. `—` means no one-to-one oracle address was established, not proof that a semantically related routine is absent. The PB/SB2 column reports their baseline `S/U/M` status in that order. Candidate status is the mapped O2 result.

### `button.cpp`

| Function | Oracle RVA / bytes; candidate bytes | PB/SB2 | Candidate | Source basis; remaining cause |
|---|---:|:---:|:---:|---|
| `CButton` | `0x29f0 / —; 64` | M/S | S | PB constructor plus corrected field layout. |
| `~CButton` | `— / —; 16` | U/U | U | PB; duplicate short destructor bodies leave location unverified. |
| `Create` | `0x2a30 / 161; 176` | M/M | M | SB2 signature/body; oracle has the eight-dword stack cleanup, but emitted body still differs. |
| `SetIconMenu` | `0x2ae0 / 45; 48` | —/S | S | SB2 method copied into candidate. |
| `SetToolTips` | `0x2b10 / 48; 48` | —/S | S | SB2 method copied into candidate. |
| `Draw` | `0x2b40 / 325; 336` | M/M | M | PB body adjusted for oracle hide/demo behavior; remaining code differs. |
| `Redraw` | `— / —; 16` | M/— | M | PB; no exact oracle target assigned. |
| `GetState` | `0x2c90 / 4; 16` | M/S | S | PB accessor. |
| `SetState` | `0x2ca0 / 35; 48` | M/S | S | PB accessor. |
| `GetMenu` | `0x2cd0 / 7; 16` | M/S | S | PB accessor. |
| `SetMenu` | `0x2ce0 / 33; 48` | M/S | S | PB accessor. |
| `GetEnable` | `— / —; 16` | M/— | M | PB; no exact oracle target assigned. |
| `SetEnable` | `0x2d10 / 38; 48` | M/S | S | PB behavior plus oracle-proven clear of demo flag when enabling. |
| `GetHide` | `0x21030 / 4; 16` | M/— | S* | PB; byte-identical anonymous oracle body, method identity open. |
| `SetHide` | `0x2d40 / 27; 32` | M/S | S | PB accessor. |
| `TreatEvent` | `0x2d60 / 111; 144` | M/U | S | PB switch logic; local jump-table targets mapped to oracle addresses. |
| `MouseOnButton` | `0x2df0 / 18; 32` | U/U | S | PB; direct `Detect` call target mapped. |
| `GetToolTips` | `0x2e10 / 237; 192` | M/M | M | PB; remaining body/source or layout difference is open. |
| `Detect` | `0x2f00 / 101; 112` | M/S | S | PB hit-test body. |
| `MouseDown` | `0x2f70 / 84; 96` | M/M | M | PB `PostMessage` removed; body remains 12 bytes larger than oracle. |
| `MouseMove` | `0x2fd0 / 201; 208` | M/M | S | PB with `PostMessage` removed; exact body now matches. |
| `MouseUp` | `0x30a0 / 85; 96` | M/U | S | PB; exact body match. |

### `jauge.cpp`

| Function | Oracle RVA / bytes; candidate bytes | PB/SB2 | Candidate | Source basis; remaining cause |
|---|---:|:---:|:---:|---|
| `CJauge` | `0x1edd0 / —; 32` | M/S | S | PB constructor with corrected layout. |
| `~CJauge` | `— / —; 16` | U/U | U | PB; short destructor location ambiguous. |
| `Create` | `0x1edf0 / 83; 96` | M/S | S | PB signature/body with corrected layout. |
| `Draw` | `0x1ee50 / 255; 256` | M/M | M | PB; fields now line up, but one-byte length difference and body mismatch remain. |
| `Redraw` | `— / —; 16` | M/M | M | PB; no exact oracle target assigned. |
| `GetLevel` | `0x1ef50 / 4; 16` | —/S | S | SB2 accessor; oracle reads `+0x24`. |
| `SetLevel` | `0x1ef60 / 38; 48` | M/S | S | PB accessor. |
| `GetType` | `0x1ef90 / 4; 16` | —/S | S | SB2 accessor; oracle reads `+0x20`. |
| `SetType` | `0x1efa0 / 24; 32` | M/S | S | PB accessor. |
| `GetHide` | `0x1efc0 / 4; 16` | M/S | S | PB accessor; oracle reads `+0x0c`. |
| `SetHide` | `0x1efd0 / 24; 32` | M/S | S | PB accessor. |
| `GetPos` | `— / —; 32` | M/— | M | PB; no exact oracle target assigned. |
| `SetRedraw` | `— / —; 16` | M/— | M | PB; no exact oracle target assigned. |

### `menu.cpp`

| Function | Oracle RVA / bytes; candidate bytes | PB/SB2 | Candidate | Source basis; remaining cause |
|---|---:|:---:|:---:|---|
| `GetText` | `— / —; 32` | M/— | M | PB helper; target association open. |
| `GetErr` | `— / —; 32` | M/— | M | PB helper; target association open. |
| `CMenu` | `0x1eff0 / 20; 32` | S/S | S | PB constructor. |
| `~CMenu` | `— / —; 16` | U/U | U | PB; short destructor location ambiguous. |
| `Create` | `— / —; 272` | M/— | M | PB; no exact oracle target assigned. |
| `Update` | `— / —; 112` | M/— | M | PB; no exact oracle target assigned. |
| `Delete` | `0x1f020 / 18; 32` | S/S | S | PB method; oracle initializes count and selection fields. |
| `Draw` | `— / —; 752` | M/— | M | PB; no exact oracle target assigned. |
| `GetSel` | `— / —; 32` | M/— | M | PB; target association open. |
| `GetRank` | `— / —; 16` | M/— | M | PB; target association open. |
| `IsError` | `— / —; 48` | M/— | M | PB; target association open. |
| `IsExist` | `— / —; 16` | M/— | M | PB; target association open. |
| `TreatEvent` | `0x1f040 / 117; 144` | U/U | S | PB switch logic; jump-table entries mapped to oracle addresses. |
| `Detect` | `0x1f0d0 / 107; 112` | S/S | S | PB hit-test body. |
| `MouseDown` | `0x1f140 / 5; 16` | S/S | S | PB method. |
| `MouseMove` | `0x1f150 / 95; 96` | S/U | S | PB method. |
| `MouseUp` | `0x1f1b0 / 44; 48` | S/U | S | PB method. |
| `Message` | `— / —; 32` | M/— | M | PB; no exact oracle target assigned. |

### `text.cpp`

| Function | Oracle RVA / bytes; candidate bytes | PB/SB2 | Candidate | Source basis; remaining cause |
|---|---:|:---:|:---:|---|
| `GetOffset` | `0x23600 / 13?; 48` | M/M | M | PB; `0x23600` is a likely ordering-based association only, size/body unverified. |
| `GetCharWidth` | `0x23610 / 66; 64` | M/M | M | PB name/macro corrected; algorithm or glyph data still differs. |
| `DrawText` | `0x23770 / 48; 64` | M/M | M | SB2-style loop through `DrawChar`; body/call sequence still differs. |
| `DrawTextPente` | `— / —; 128` | M/— | M | PB; no exact oracle target assigned. |
| `DrawTextRect` | `— / —; 256` | M/— | M | PB; no exact oracle target assigned. |
| `DrawTextCenter` | `0x237a0 / 177; 192` | U/U | S | PB wrapper; `DrawText` and `GetTextWidth` targets mapped. |
| `GetTextHeight` | `— / —; 176` | M/M | M | PB; no exact oracle target assigned. |
| `GetTextWidth` | `0x23860 / 44; 48` | U/U | S | PB method; `GetCharWidth` target mapped. |
| `GetBignumInfo` | `— / —; 48` | M/— | M | PB; no exact oracle target assigned. |
| `DrawBignum` | `— / —; 160` | M/— | M | PB; no exact oracle target assigned. |
| `GetBignumWidth` | `— / —; 96` | M/— | M | PB; no exact oracle target assigned. |
| `DrawCharSingle` | `0x23660 / 76; 80` | —/S | S | SB2 body plus oracle channel constants 7 and 6. |
| `DrawTextLeft` | `0x23740 / 34; 48` | —/U | S | SB2 wrapper; direct `DrawText` target mapped. |
| `DrawChar` | `0x236b0 / 144; 144` | —/M | M | SB2 helper and unverified SB2 glyph table; table/source cause open. |

## Reproduction artifacts

- PB baseline JSON: `build/ui/results/pb_o2_gx.json`.
- Candidate O2 strict comparison with explicit relocation mappings: `build/ui/results/ui_final_mapped.json`.
- Candidate O2 `/Gy`: `build/ui/results/ui_gy_mapped.json`.
- Candidate O1 comparison without mappings: `build/ui/results/ui_o1_unmapped.json`.
- Compile commands and VC5 logs: `build/ui/logs/`.
- Candidate sources and include-context headers: `scratch/tu/` and `scratch/tu/ui_include/`.
- No oracle executable was run. No source uses embedded oracle bytes or assembly byte emission.


## Residuals round (2026-10-17)

This round supersedes the earlier candidate-status rows above for these four TUs. Candidates started from current `src/` and were verified together with VC5 SP3 `/O2 /GX /ML` using `tools/verify.py --tu ... --build-dir uires-final`; all accepted checks passed. Canonical `src/` files were not changed. The final run has 14/14/9/8/5 STRICT bodies in button/ddutil/jauge/menu/text; relative to the source-to-self baseline, this round adds 255 unique `CJauge::Draw` function bytes and 48 `DrawText` bytes (303 total, excluding the gauge boundary NOP).

### Strict additions

- **[EVIDENCE] `CJauge::Draw` at `0x1ee50`: STRICT.** The private `def.h` value `CHJAUGE=5` matches the oracle's `DrawPart` channel. All three call relocations (candidate offsets 88, 179, 246) resolve to oracle function entries. The oracle function record is 255 bytes; the verifier compares a 256-byte candidate extent, whose last byte is the matching `0x90` at `0x1ef4f`, outside the recorded function. Count 255 unique function bytes. This resolves the three relocation gaps noted in `BLUPI-SEM-01` validation.
- **[EVIDENCE] `DrawText` at `0x23770`: STRICT, 48 bytes.** The source loop `while (*pText != 0) DrawChar(..., *pText++, font)` matches the oracle body, including its call relocation to `DrawChar` at `0x236b0`.

### Private header deltas

- **[EVIDENCE] `scratch/tu/uires/jauge/def.h`:** `CHJAUGE` 12 to 5, supported by the channel argument in oracle `CJauge::Draw` and the strict result above.
- **[EVIDENCE] `scratch/tu/uires/text/def.h`:** adds `#undef DrawText` and `#undef GetCharWidth` at the end of private `def.h`, after the Windows headers introduce those aliases. Without these undefs, the object emits `DrawTextA` and `GetCharWidthA`, so the SB1 symbols cannot be compared. `CHLITTLE=7` and `CHTEXT=6` already match current `src/def.h`; no change was needed here.
- **[HYPOTHESIS] Tested and reverted:** `CHBUTTON` 10 to 4, following the `m_type+4` argument in oracle `CButton::Draw`. It removed the immediate-difference tag but did not make `Draw` strict, and the candidate made `TreatEvent` RELOC-UNVERIFIED. No member or declaration deltas were made.

### Compiler findings

- **[EVIDENCE] `DrawText`, text TU, `/O2 /GX /ML`:** replaced the temporary `char` plus separate pointer reads with the post-increment loop above; `DrawText` changed from mismatch to 48-byte STRICT with its relocation proven. Related rule `VC5-CTL-01`; classification: blupi-specific source-form match.
- **[EVIDENCE] `CButton::Create`, button TU, `/O2 /GX /ML`:** reordered independent member stores to the oracle pseudocode order. It stayed MISMATCH at 176 candidate bytes versus the 161-byte oracle body; KB tags remain `reorder_only,block_reorder,store_reorder`. `VC5-ORD-01`; classification: negative on SB1 for this store-order rewrite.
- **[EVIDENCE] `CButton::Draw`, button TU:** replacing `CHBUTTON=10` with 4 (and separately spelling `m_type+4`) removed the immediate-difference symptom, but the body remained MISMATCH at 336/325 bytes; the changed TU context also made `TreatEvent` relocation-unverified. `VC5-SEM-01`, `VC5-CTX-04`; classification: unverified source-value correction with a peer regression.
- **[EVIDENCE] `CButton::GetToolTips`, button TU:** a source reconstruction from oracle control flow added the demo exception and the different tooltip bounds for expanded menus. It remained MISMATCH at 240/237 bytes and the experiment made accepted `TreatEvent` relocation-unverified and `MouseMove` mismatch. `VC5-SEM-01`, `VC5-CTX-04`; classification: semantic evidence is blupi-specific, exact source form remains open.
- **[EVIDENCE] `DrawChar`, text TU:** the current `index=c` emits 144 bytes and differs at a displacement. Testing `(unsigned char)c`, as suggested by the oracle's `param & 0xff`, emitted 160 bytes versus 144 and still mismatched. `VC5-SEM-01`; classification: negative for this cast under VC5 SP3.
- **[EVIDENCE] `GetCharWidth` table experiment:** in a combined trial with the `DrawChar` cast, replacing `GetOffset(c)` with `table_char[(unsigned char)c * 6]` emitted an 80-byte candidate and produced a byte hit at `0x23610`, but status is `RELOC-UNVERIFIED`: `oracle/tu_map.csv` assigns that 66-byte body to `sound.cpp`, not `text.cpp`, so this is not counted as a text-TU match. `VC5-SEM-01`; classification: unverified ownership and source reconstruction.

### Remaining mapped residuals

- **[OPEN]** `CButton::Create` (`0x2a30`, 161 bytes), `Draw` (`0x2b40`, 325), `GetToolTips` (`0x2e10`, 237), and `MouseDown` (`0x2f70`, 84) remain mismatches; current candidate extents are respectively 176, 336, 192, and 96 bytes. Oracle `MouseDown` pseudocode agrees with the current hit-test, state, redraw, and click-sound actions, but the emitted body is still larger.
- **[OPEN]** `DrawChar` remains a 144/144-byte mismatch, with the SB2 glyph table not proven equal to SB1. Current `GetCharWidth` at 64 bytes does not match the 66-byte body mapped to `sound.cpp`.
- **[EVIDENCE]** All mapped `CMenu` bodies remain strict in this run. `CMenu::Create`, `Draw`, and `Update` have no body assignment in `oracle/tu_map.csv`, so they were not treated as targets.

Reproduction outputs: `build/verify-uires-final/results.json` and `build/verify-uires-final/logs/`.
