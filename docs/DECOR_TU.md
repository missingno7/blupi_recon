# Decor translation-unit reconstruction

The owned baseline [scratch/tu/decor.cpp](../scratch/tu/decor.cpp) is refreshed byte-for-byte from `src/decor.cpp`. The current best target-shaped candidate is [scratch/tu/decor_include/clean_final/decor.cpp](../scratch/tu/decor_include/clean_final/decor.cpp), compiled with current canonical headers plus the single private `DescFile::reserve1` width delta documented below. It combines the 21 SB1 methods in oracle order from read-only SB2 decor.cpp, decio.cpp, and decblock.cpp. The canonical `src/` tree and external reference repositories were not changed.

## [EVIDENCE] SB1 map and save layouts

The SB1 decor block spans RVAs 0x13d50–0x161d0. Target CurrentWrite at 0x14da0 allocates and writes 0xd490 bytes; CurrentRead at 0x153d0 reads and validates the same size. CurrentWrite clears 0x3524 dwords, writes lg=0xd490, majRev=1, and minRev=2.

VC5 SP3 layout probes report Cellule=2 bytes, MoveObject=48 bytes, DescFile=868 bytes, and DescGameFile=0xd490 bytes. The map save consists of the 868-byte DescFile header, two 100×100 short grids, and 200 MoveObject records: 50,468 bytes total, matching the world-map format evidence.

The saved record offsets below come from target CurrentWrite/CurrentRead field stores and reads. Array extents are established by the target copy counts.

| DescGameFile member | Target offset | Evidence |
|---|---:|---|
| decor | 0xd4 | 5,000 dwords |
| bigDecor | 0x4ef4 | 5,000 dwords |
| moveObject | 0x9d14 | 2,400 dwords |
| missionTitle | 0xc380 | 25 dwords |
| rankCaisse | 0xc4b0 | 200 dwords |
| linkCaisse | 0xc7d4 | 200 dwords |
| blupiFifoPos | 0xcc6c | 20 dwords |
| blupiStartPos | 0xcd84 | 8 dwords |
| jaugeHide | 0xce7c | state fields begin here |
| playHelicoHigh | 0xcf64 | sound flags |
| bPrivate | 0xd040 | state flag |
| mission | 0xd058 | 4-byte mission field |
| doors | 0xd05c | 100 two-byte entries |
| nbVies | 0xd124 | immediately after the 200-byte doors span |
| decorAction | 0xd3c0 | last saved action fields |

The earlier 0xd488 save-record model was eight bytes short. The target endpoints establish a 200-byte gap from the end of blupiFifoPos (0xccbc) to blupiStartPos (0xcd84); the private DescGameFile represents that interval as `short reseve5[100]`, adding the missing four shorts relative to the old model. The SB1 field order also differs from SB2: blupiOver moves before Jeep, Invert/Balloon/Ecrase precede the position fields, and Dynamite moves before FIFO. The serialized SB1 record omits blupiCle, the SB2 time fields, actionOuf, and FifoNb. With those observed stores and reads represented, the VC5 size is 0xd490; this is not an appended tail pad.

The target writes a dword zero at `DescGameFile+0x0c` after the `lg`, `majRev`, and `minRev` fields. Canonical `DescGameFile` already uses `int reseve1[50]`, a 200-byte span consistent with that store. Separately, target `Write` at 0x148e0 writes a dword zero at `DescFile+0x04`; the private decor.h changes `DescFile::reserve1` from `short[100]` to `int[50]`. This preserves the 200-byte span and every following offset while representing the observed store width. Under current canonical headers plus only this private delta, `Write` is STRICT. SP3 layout probes report `sizeof(DescFile) == 0x364` and `sizeof(DescGameFile) == 0xd490`.

CurrentRead stores nbLinkCaisse from save offset +0xc7d0 into CDecor offset +0xc5a4 after restoring rankCaisse. CurrentWrite stores that counter at save offset +0xc7d0. CurrentRead also restores nbVies, nbTresor, totalTresor, and goalPhase from save offsets +0xd124, +0xd128, +0xd12c, and +0xd130 to CDecor offsets +0xce9c through +0xcea8, after copying doors. These accesses are now represented in the candidate.

The target FileExist loop starts at save offset +0xd05c, compares 100 words, and advances by two bytes each time. This proves DescGameFile::doors is 100 shorts (200 bytes), rather than the SB2 char[200]. The target CDecor doors field is also 200 bytes at +0xcdd4, so the private CDecor uses short m_doors[100].

The target CDecor offsets measured by save/I/O stores and the compile-time layout probe are:

| CDecor member | Target offset | CDecor member | Target offset |
|---|---:|---|---:|
| m_decor | 0x10 | m_bigDecor | 0x4e30 |
| m_moveObject | 0x9c50 | m_posDecor | 0xc1d8 |
| m_phase | 0xc1fc | m_missionTitle | 0xc218 |
| m_rankCaisse | 0xc284 | m_linkCaisse | 0xc5a8 |
| m_blupiPos | 0xc8c8 | m_blupiCle | 0xc96c |
| m_blupiShield | 0xc934 | m_blupiPower | 0xc938 |
| m_blupiPerso | 0xc970 | m_blupiDynamite | 0xc974 |
| m_blupiNoBarre | 0xc978 | m_blupiFifoPos | 0xc97c |
| m_blupiStartPos | 0xc9cc | m_blupiStartDir | 0xc9ec |
| m_jauges | 0xcd30 | m_blupiLevel | 0xcd90 |
| m_bFoundCle | 0xcda8 | m_bPrivate | 0xcdac |
| m_bCheatDoors | 0xcdb0 | m_bSuperBlupi | 0xcdb4 |
| m_bDrawSecret | 0xcdb8 | m_bJoystick | 0xcdc8 |
| m_mission | 0xcdd0 | m_doors | 0xcdd4 |
| m_nbVies | 0xce9c | m_scrollFollow | 0xceac |
| m_voyageIcon | 0xcec0 | m_decorAction | 0xcee0 |

m_jauges at +0xcd30 is consistent with MAXNOTIF=4. That value is already the same in the canonical and recovered def.h copies.

CurrentWrite maps saved bFoundCle, bPrivate, bCheatDoors, bSuperBlupi, and bDrawSecret to CDecor offsets +0xcda8, +0xcdac, +0xcdb0, +0xcdb4, and +0xcdb8. AdaptDoors stores its bPrivate argument at +0xcdac and tests the door override at +0xcdb0. This corrects the earlier table entry that placed m_bPrivate at +0xcdb0: that address is m_bCheatDoors.

## [EVIDENCE] Comparison with the engine worker header

The independently maintained scratch/tu/engine_include/decor.h was compiled through the same VC5 layout-value probe. The comparison below records both headers’ measured offsets; the engine header was not edited.

| Structure/member | Decor candidate | Engine header | Result |
|---|---:|---:|---|
| DescFile size | 0x364 | 0x364 | agrees |
| DescFile posDecor / name | 0xcc / 0x178 | 0xcc / 0x178 | agrees |
| DescFile reserve1 element type | int[50] | short[100] | same 200-byte span; target dword store at +0x04 supports int[50] |
| DescGameFile size | 0xd490 | 0xdeb0 | differs |
| DescGameFile decor / bigDecor | 0xd4 / 0x4ef4 | 0xd4 / 0x4ef4 | agrees |
| DescGameFile reseve1 element type | int[50] | short[100] | same 200-byte span; target dword store at +0x0c supports int[50] |
| DescGameFile moveObject | 0x9d14 | 0xa73c | differs |
| DescGameFile missionTitle | 0xc380 | 0xcda8 | differs |
| DescGameFile rankCaisse / linkCaisse | 0xc4b0 / 0xc7d4 | 0xced8 / 0xd1fc | differs |
| DescGameFile FIFO / start positions | 0xcc6c / 0xcd84 | 0xd698 / 0xd7a4 | differs |
| DescGameFile jaugeHide | 0xce7c | 0xd89c | differs |
| DescGameFile bPrivate / mission / doors | 0xd040 / 0xd058 / 0xd05c | 0xda60 / 0xda78 / 0xda7c | differs |
| DescGameFile nbVies / decorAction | 0xd124 / 0xd3c0 | 0xdb44 / 0xdde0 | differs |

The current engine private header was freshly compiled with the same SP3 layout probe. Its DescGameFile retains balleTraj and moveTraj between bigDecor and moveObject, accounting for the 0xa28 moveObject displacement, and otherwise keeps SB2 state/reserve fields. Its doors member is char[200]; the target member is short[100]. Both occupy 200 bytes, but the target element width is proven by FileExist’s word loads. In CDecor, the engine header also has trajectory arrays after m_lastDecorIndexes; this agrees that they are absent before m_moveObject, while their SB1 tail placement remains unproven.

The two CDecor headers agree through m_blupiCle at +0xc96c and m_blupiPerso at +0xc970, and both put the FIFO array at +0xc97c. They differ afterward: target m_blupiNoBarre is +0xc978 and start positions are +0xc9cc; the engine header places NoBarre at +0xc9cc and start positions at +0xc9e8 because it retains the FIFO count and six timers after the FIFO array. Both headers put m_decor, m_bigDecor, m_moveObject, m_posDecor, m_phase, m_missionTitle, m_rankCaisse, and m_linkCaisse at the target offsets listed above.

| Later CDecor member | Decor candidate | Engine header |
|---|---:|---:|
| m_jauges | 0xcd30 | 0xcdb0 |
| m_bJoystick / m_mission | 0xcdc8 / 0xcdd0 | 0xce4c / 0xce54 |
| m_doors / m_nbVies | 0xcdd4 / 0xce9c | 0xce58 / 0xcf20 |
| m_scrollFollow | 0xceac | 0xcf34 |
| m_voyageIcon / m_decorAction | 0xcec0 / 0xcee0 | 0xcf48 / 0xcf68 |

The engine header retains MAXNOTIF=5, m_bNetDebug, m_detectIcon, and byte doors. The target offsets support MAXNOTIF=4, omit the debug and detect-icon members from these positions, and use short doors[100]. CDecor’s m_doors occupies 200 bytes in both headers. The current engine layout agrees with the decor candidate through m_decor, m_bigDecor, m_moveObject, m_posDecor, m_phase, m_missionTitle, m_rankCaisse, and m_linkCaisse; it differs from the decor candidate after m_blupiCle/m_blupiPerso. Specifically, engine m_blupiNoBarre is +0xc9cc versus target +0xc978, engine m_blupiStartPos is +0xc9e8 versus target +0xc9cc, and engine m_jauges is +0xcdb0 versus target +0xcd30. Both headers put m_blupiFifoPos at +0xc97c, but the engine calls the preceding dword m_blupiFifoNb at +0xc978 while the target accesses that position as m_blupiNoBarre. The engine keeps its two trajectory arrays after m_lastDecorIndexes; SB1 evidence proves only their absence before m_moveObject, not their tail placement.

## [EVIDENCE] Header recovery now integrated in canonical src headers

Before commit 554cc5d, the private set in `scratch/decor/recovered_headers/` restored four functions lost under the then-current canonical headers, taking that candidate from 8 to 12 STRICT functions. The supervisor integrated those header changes; they are now part of the canonical header set. The rows below preserve the measured rationale for those accepted changes. Against the current canonical headers, the latest candidate needs only the separate, still-private `DescFile::reserve1` width change documented above to make `Write` STRICT. No class method declaration-order change was needed for either recovery.

| Header change formerly needed against pre-integration canonical | Target evidence and strict effect |
|---|---|
| `decor.h`: move `m_blupiShield` before `m_blupiPower`. This places shield at `+0xc934` and power at `+0xc938`; canonical order reverses those names. | Target CurrentWrite stores those two state fields from these offsets. A controlled header variant that reverted only this order lost CurrentWrite (12→11 STRICT); restoring it restored CurrentWrite. |
| `decor.h`: recovered copy spells the `+0xc978` slot as `int m_blupiNoBarre`; canonical uses an anonymous union with `m_blupiFifoNb`. | Both are one dword at the same offset. Replacing the recovered int with the canonical union did not change the strict count, so this spelling is not required for the matches. The target proves the slot and offset, not which alias name is canonical. |
| `def.h`: `CHBUTTON` 10→4. | Target VoyageInit uses channel 4 for the button sound case. Together with the sound declaration below, the recovered header set restores VoyageInit. |
| `def.h`: `ACTION_STOP` 0→1. | Target VoyageStep stores 1 to the action field at `CDecor+0xc8d8`; the target-backed value restores VoyageStep. |
| `def.h`: define `LEGACY_CODE` (the value is immaterial to `#ifndef LEGACY_CODE`). | The candidate's guarded bounds check otherwise emits code absent from target OpenDoorsWin, whose body is a 17-byte store. Defining the macro restores the strict body. |
| `sound.h`: replace the split three- and four-argument `PlayImage` declarations with `PlayImage(int channel, POINT pos, int rank=-1, int playFlag=0)`. | Target VoyageInit passes five stack words, including a trailing zero; callee RVA 0x230c0 uses `ret 0x14` and forwards that word. The unified defaulted signature selects this call shape and contributes to strict VoyageInit. The original parameter name is not established. |

Apart from the shield/power order and the non-codegen NoBarre alias spelling above, the recovered and current canonical `decor.h` copies agree on the DescFile/DescGameFile formats and measured CDecor save/map offsets; no method declaration order change was needed to regain these anchors. `MAXNOTIF=4` is already canonical, not part of this recovery delta. The independent engine-header comparison remains useful for wider layout integration: its DescGameFile size and post-link CDecor offsets differ from SB1 evidence.

[OPEN] The private CDecor header omits SB2's `reserve[11]` and `dummy[100000]` tail placeholders. It retains inherited `m_lastDecorIndexes[200]`, whose SB1 tail placement is not established by these functions. The target `DoInit` allocation is 0x258d4 bytes, but the owned decor methods contain no indexed or array access reaching the tail, and the available `DoInit` evidence has no member reference beyond +0xdbec. Allocation size alone does not establish `sizeof(CDecor)`; do not add opaque tail padding. The remaining tail layout is unknown.

## [HYPOTHESIS] Function assignment and current results

Method identities use SB2 ancestry plus target body shapes, constants, and offsets. The best candidate was verified with current canonical headers and only the private DescFile element-width delta above: 13 STRICT functions, one RELOC-UNVERIFIED function, and 21 mismatches in the decor object. It accounts for 3,856 unique strict bytes. The current `src/decor.cpp` baseline itself remains unchanged from the refresh.

| Oracle RVA | Candidate method | Result |
|---:|---|---|
| 0x13d50, 0x13d70, 0x13ef0, 0x14050 | VoyageGetPosVie, VoyageInit, VoyageStep, VoyageDraw | STRICT |
| 0x140c0 | IsRightBorder | MISMATCH |
| 0x143d0 | AdaptMidBorder | RELOC-UNVERIFIED |
| 0x144d0 | AdaptBorder | MISMATCH |
| 0x14550 | DrawMap | MISMATCH |
| 0x14880 | GetMissionPath | STRICT |
| 0x148e0 | Write | STRICT |
| 0x14af0 | Read | MISMATCH |
| 0x14d60 | DeleteMission | STRICT |
| 0x14da0 | CurrentWrite | STRICT |
| 0x153d0, 0x159f0 | CurrentRead, FileExist | MISMATCH |
| 0x15c10 | SearchWorld | STRICT |
| 0x15de0 | SearchDoor | STRICT |
| 0x15e30 | AdaptDoors | MISMATCH |
| 0x16180 | OpenDoorsWin | STRICT |
| 0x161a0, 0x161d0 | OpenGoldsWin, DoorsLost | STRICT |

## [EVIDENCE] Strict comparison

The final candidate is compiled with VC5 SP3, /W3 /GX /O2 /ML /Gd /Gy, WIN32, NDEBUG, _WINDOWS, and nullptr=0. TMP/TEMP remain under build/decor/tmp. The fresh whole-manifest verification used isolated build directory `build/verify-decor-clean-final`, with current canonical headers plus the private `DescFile::reserve1` width delta. The comparison includes all TU objects and the function placements/relocations shown below.

| Candidate method | RVA | Strict object span | Oracle function body | Verified relocations |
|---|---:|---:|---:|---:|
| VoyageGetPosVie | 0x13d50 | 32 | 31 | 0 |
| VoyageInit | 0x13d70 | 384 | 377 | 8 |
| VoyageStep | 0x13ef0 | 352 | 345 | 6 |
| VoyageDraw | 0x14050 | 112 | 100 | 1 |
| GetMissionPath | 0x14880 | 96 | 86 | 6 |
| Write | 0x148e0 | 528 | 527 | 12 |
| DeleteMission | 0x14d60 | 64 | 61 | 2 |
| CurrentWrite | 0x14da0 | 1,584 | 1,574 | 14 |
| SearchWorld | 0x15c10 | 464 | 451 | 0 |
| SearchDoor | 0x15de0 | 80 | 69 | 0 |
| OpenDoorsWin | 0x16180 | 32 | 17 | 0 |
| OpenGoldsWin | 0x161a0 | 48 | 34 | 0 |
| DoorsLost | 0x161d0 | 80 | 73 | 0 |

These are 3,856 distinct strict object bytes, including 111 matched alignment bytes; the oracle function bodies account for 3,745 bytes. All 49 relocations are verified. No folded or ambiguous duplicate bytes are counted.

## Compiler findings

- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] SearchWorld initially mismatched only on immediate values. Target pseudocode/disassembly requires the first two left-neighbor checks at x>0 and x>1, not SB2’s x>1 and x>2; it also compares icon - Level_1 + 1 against world in that operand order. With all three target-backed changes, SearchWorld became STRICT at 0x15c10 (464-byte object span, 451-byte body). The combined edit was compiled under the flags above.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] VoyageInit was reconstructed from the SB1 target trigger cases: the target uses its incoming icon parameter, has five supported icon/channel cases, and pushes the trailing PlayImage zero shown in the header delta above. Matching the call ABI and target icon/channel cases made the 384-byte object span STRICT against the 377-byte body at 0x13d70, with eight verified relocations under the SP3 decor-TU flags.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] VoyageStep now uses the SB1 completion cases and effects. Changing the final comparison from `m_voyageTotal <= m_voyagePhase` to target-order `m_voyagePhase >= m_voyageTotal` made the 352-byte object span STRICT against the 345-byte body at 0x13ef0, with six verified relocations under the same SP3 decor-TU flags.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] VoyageDraw’s target only interpolates the start/end position and draws the active icon. Removing SB2’s special icon-40 animation/effects and using the target interpolation and QuickIcon call made the 112-byte object span STRICT against the 100-byte body at 0x14050 under the same flags; the single relocation is verified.
- [EVIDENCE; confirmed Blupi-specific] The target’s 100-word FileExist scan and offsets prove doors[100] as 16-bit elements. Under the same SP3 decor-TU flags, the target OpenGoldsWin store at `m_doors[m_mission / 10]` (without SB2’s `+180` index or bounds guard) made OpenGoldsWin STRICT at 0x161a0. DoorsLost’s ten 16-bit stores starting at `(m_mission / 10) * 10` are STRICT at 0x161d0. FileExist remains a 544-byte candidate versus a 540-byte target body.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] CurrentWrite initially differed in its start-direction copy and final fwrite condition. Replacing four independent `blupiStartDir` assignments with one 16-byte `memcpy`, then assigning `fwrite` to the existing signed `int nb` and testing `nb < 1`, changed the compare from `block_reorder,condition_differs` to STRICT at 0x14da0. The 1,584-byte object span includes the matched 10-byte alignment gap around the 1,574-byte body; 14 relocations are verified. Flags/TU context: VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy`, same private decor headers.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] The supervisor-integrated canonical headers restored the 12 previously accepted decor functions. With those current headers, changing only `DescFile::reserve1` to `int[50]` (from `short[100]`, same 200-byte span) makes target `Write` STRICT: 528 object bytes versus a 527-byte body, with 12 verified relocations.
- [EVIDENCE; negative, VC5-SEM-01] CurrentRead’s target restores nbVies, nbTresor, totalTresor, and goalPhase immediately after the 100-short door array. Adding those four stores (and the earlier nbLinkCaisse store) moved the candidate from 1,536 to 1,584 bytes against the 1,564-byte target. The member accesses now match, but the target compares fread's result with `cmp eax,1; jl`, then falls through a shared free/close/false cleanup on a bad size; only the equal-size branch skips that cleanup to the restore body. The current candidate branches to its shared error tail instead. Label-based cleanup, nested success guards, and explicit early-return cleanup under the SP3 decor-TU flags above remained non-STRICT; the immediate-return form also remained 1,584 bytes. No local-only compiler rule is confirmed.
- [EVIDENCE; negative, VC5-SEM-01] FileExist remains 544 candidate bytes versus a 540-byte target body. A nested `nb > 0` success guard changed it to 528 bytes and emitted `test`/`jle` where target uses `cmp eax,1`/`jl`; preserving the target comparison in a branch-shaped variant returned it to 544. The target's 100-word door scan and saved-record offsets are correct, but validation and cleanup layout remain mismatches. No compiler-only rule is inferred from these variants.
- [EVIDENCE; negative] Rewriting every large CurrentWrite/CurrentRead memcpy as independent pointer-increment dword loops produced 1,744/1,648-byte functions versus the 1,574/1,564-byte target bodies; it did not produce a strict save routine. The current candidate keeps the inherited memcpy forms while retaining target-evidenced size, revision, and counter stores.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] Target `IsRightBorder(x,y,dx,dy)` reads the cell at `(x,y)`; `dx/dy` only choose the tested edge. Correcting the candidate's erroneous `[x+dx][y+dy]` access to `[x][y]` moved it from 736 to 784 bytes versus the 775-byte target body, but it remains non-STRICT. A short-local/range-expression variant was 752 bytes and also non-STRICT. Target `AdaptMidBorder` looks up an 80-int table at VA 0x434298. The separate scratch candidate [decor_table/decor.cpp](../scratch/tu/decor_include/decor_table/decor.cpp) defines the 80 target values (they equal the first 80 entries of SB2's table) and reaches RELOC-UNVERIFIED: the base relocation and data bytes verify, but the one-past address at +320 cannot be assigned a unique initialized-data extent because the next target bytes begin the `image08\\decor%.3d.blp` string. No table length beyond 80 is inferred or added. `AdaptMidBorder` is 256 bytes versus a 253-byte body; `AdaptBorder` is 128 versus 114.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] Target `AdaptDoors` stores bPrivate at `+0xcdac`, mission at `+0xcdd0`, and tests the override at `+0xcdb0` (`m_bCheatDoors`). The first pass scans `m_doors[i]`; the second indexes `m_doors[mission+i]`, creates an inline MoveObject with icon `0xb6`, then updates four start positions/directions. The target-shaped direct-index candidate is 848 bytes versus 840; a pointer-local form fell to 720, also non-STRICT. The offsets and behavior are evidenced; the source form remains open.
- [EVIDENCE; negative, VC5-SEM-01] For `Read`, a target-pseudocode version guard (skip bigDecor only for version 1.0; clear and read 100 move records only for major 1/minor below 2) compiled to 624 bytes versus the 610-byte target. The SB2-derived guard candidate is 608 bytes. Neither is STRICT, so this family still needs source-form work.
- [EVIDENCE; negative] The best `DrawMap` candidate is 800 bytes versus 805. Target pseudocode calls `Build(0,0,640,480)` and then a one-argument method at pixmap RVA 0x217a0 with channel 8. Current `CPixmap` headers expose only the three-argument `DrawMap(channel,src,dst)`, and the one-argument target helper has no mapped source definition; no speculative declaration was added.
- [EVIDENCE; negative] CurrentRead cleanup reshaping with a combined validity guard did not reduce its 1,584-byte candidate against the 1,564-byte target. FileExist cleanup reshaping likewise remains 544 versus 540; the target's 100-word door scan and formatted name arguments are retained. No new compiler-only rule is inferred from these tests.
- [EVIDENCE; compiler adaptation] SB2 decio.cpp redeclares i in two for loops and fails VC5 with C2086. The scratch candidate reuses the existing function-scope i; this enables SP3 compilation but is not itself a code match.
- [EVIDENCE; negative, earlier baseline] An earlier same-header sweep produced four strict functions at /O2 and only VoyageGetPosVie at /O1. /O1 was not rerun after SearchWorld and door-layout fixes. Related catalog rule: VC5-FLG-01.

## [OPEN] Next work

The save structure, door element width, and the integrated canonical header recovery agree with target offsets. CurrentRead (1,584 candidate bytes versus a 1,564-byte target body), FileExist (544 versus 540), AdaptDoors (848 versus 840), IsRightBorder (784 versus 775), AdaptBorder (128 versus 114), DrawMap (800 versus 805), Read (608 versus 610), and the one-past table relocation in AdaptMidBorder remain unresolved. The latest clean candidate yields 13 STRICT functions, 1 RELOC-UNVERIFIED, and 21 other mismatches across the decor object; it covers 3,856 unique strict bytes. The next useful work is to identify the original source shape for the CurrentRead cleanup and FileExist validation branches, resolve the target's one-argument pixmap helper/data boundary from source ownership, then resume the border and AdaptDoors comparisons. No tested local-only rewrite has confirmed a new compiler rule.
