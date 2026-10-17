# Decor translation-unit reconstruction

The owned candidate is [scratch/tu/decor.cpp](../scratch/tu/decor.cpp), with private headers in [scratch/tu/decor_include](../scratch/tu/decor_include/). The generator is [scratch/decor/build_candidate.py](../scratch/decor/build_candidate.py). It combines 21 methods in the SB1 oracle order from read-only SB2 decor.cpp, decio.cpp, and decblock.cpp sources. The canonical src tree and external reference repositories were not changed.

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

The target writes a dword zero at save offset `+0x0c` after the `lg`, `majRev`, and `minRev` header fields. The private header uses `int reseve1[50]` (200 bytes) rather than a short array, preserving the following map offset while representing the observed store width. A fresh SP3 layout probe reports `sizeof(DescGameFile) == 0xd490` and the offsets in the table above.

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
| m_blupiPerso | 0xc970 | m_blupiNoBarre | 0xc978 |
| m_blupiFifoPos | 0xc97c | m_blupiStartPos | 0xc9cc |
| m_jauges | 0xcd30 | m_bPrivate | 0xcdb0 |
| m_bJoystick | 0xcdc8 | m_mission | 0xcdd0 |
| m_doors | 0xcdd4 | m_nbVies | 0xce9c |
| m_scrollFollow | 0xceac | m_voyageIcon | 0xcec0 |
| m_decorAction | 0xcee0 |  |  |

m_jauges at +0xcd30 supports the private MAXNOTIF=4 constant: the original five notification rows placed it 100 bytes later. The private def.h records this target-supported change.

## [EVIDENCE] Comparison with the engine worker header

The independently maintained scratch/tu/engine_include/decor.h was compiled through the same VC5 layout-value probe. The comparison below records both headers’ measured offsets; the engine header was not edited.

| Structure/member | Decor candidate | Engine header | Result |
|---|---:|---:|---|
| DescFile size | 0x364 | 0x364 | agrees |
| DescFile posDecor / name | 0xcc / 0x178 | 0xcc / 0x178 | agrees |
| DescGameFile size | 0xd490 | 0xdeb0 | differs |
| DescGameFile decor / bigDecor | 0xd4 / 0x4ef4 | 0xd4 / 0x4ef4 | agrees |
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

Private header deltas used by this candidate: `decor.h` has the target 2-byte Cellule and 48-byte MoveObject records, the measured DescFile and DescGameFile layouts above, target CDecor offsets in the table, `short doors[100]`, and the two-argument CurrentWrite declaration. `def.h` uses MAXNOTIF=4, supported by m_jauges at +0xcd30. `sound.h` declares `PlayImage(int channel, POINT pos, int rank=-1, int playFlag=0)`: the target VoyageInit call pushes five stack words (channel, the two POINT words, rank -1, and a trailing zero), and the target callee at RVA 0x230c0 ends with `ret 0x14` and forwards that trailing word to its helper. The original parameter name is not established. The duplicate scratch/tu/decor.h copy has the same target layout declarations. Other inherited private declarations and constants remain compilation context, not evidence of SB1 layout unless cited above.

[OPEN] The private CDecor header omits SB2's `reserve[11]` and `dummy[100000]` tail placeholders. It retains inherited `m_lastDecorIndexes[200]`, whose SB1 tail placement is not established by these functions. No target access in the owned block reaches those tail members, so `sizeof(CDecor)` and the remaining tail layout are still unknown.

## [HYPOTHESIS] Function assignment and current results

Method identities use SB2 ancestry plus target body shapes, constants, and offsets. The current candidate has 12 STRICT functions. The remaining listed methods are mismatches.

| Oracle RVA | Candidate method | Result |
|---:|---|---|
| 0x13d50, 0x13d70, 0x13ef0, 0x14050 | VoyageGetPosVie, VoyageInit, VoyageStep, VoyageDraw | STRICT |
| 0x140c0, 0x143d0, 0x144d0 | IsRightBorder, AdaptMidBorder, AdaptBorder | MISMATCH |
| 0x14550 | DrawMap | MISMATCH |
| 0x14880 | GetMissionPath | STRICT |
| 0x148e0, 0x14af0 | Write, Read | MISMATCH |
| 0x14d60 | DeleteMission | STRICT |
| 0x14da0 | CurrentWrite | STRICT |
| 0x153d0, 0x159f0 | CurrentRead, FileExist | MISMATCH |
| 0x15c10 | SearchWorld | STRICT |
| 0x15de0 | SearchDoor | STRICT |
| 0x15e30 | AdaptDoors | MISMATCH |
| 0x16180 | OpenDoorsWin | STRICT |
| 0x161a0, 0x161d0 | OpenGoldsWin, DoorsLost | STRICT |

## [EVIDENCE] Strict comparison

The final candidate is compiled with VC5 SP3, /W3 /GX /O2 /ML /Gd /Gy, WIN32, NDEBUG, _WINDOWS, and nullptr=0. TMP/TEMP remain under build/decor/tmp. cmpfunc.py compared the candidate together with all objects in build/verify/objects and forced the key save functions and SearchWorld to their oracle RVAs.

| Candidate method | RVA | Strict object span | Oracle function body | Verified relocations |
|---|---:|---:|---:|---:|
| VoyageGetPosVie | 0x13d50 | 32 | 31 | 0 |
| VoyageInit | 0x13d70 | 384 | 377 | 8 |
| VoyageStep | 0x13ef0 | 352 | 345 | 6 |
| VoyageDraw | 0x14050 | 112 | 100 | 1 |
| GetMissionPath | 0x14880 | 96 | 86 | 6 |
| DeleteMission | 0x14d60 | 64 | 61 | 2 |
| CurrentWrite | 0x14da0 | 1,584 | 1,574 | 14 |
| SearchWorld | 0x15c10 | 464 | 451 | 0 |
| SearchDoor | 0x15de0 | 80 | 69 | 0 |
| OpenDoorsWin | 0x16180 | 32 | 17 | 0 |
| OpenGoldsWin | 0x161a0 | 48 | 34 | 0 |
| DoorsLost | 0x161d0 | 80 | 73 | 0 |

These are 3,328 distinct strict object bytes, including 110 matched alignment bytes; the oracle function bodies account for 3,218 bytes. All 37 relocations are verified. No folded or ambiguous duplicate bytes are counted.

## Compiler findings

- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] SearchWorld initially mismatched only on immediate values. Target pseudocode/disassembly requires the first two left-neighbor checks at x>0 and x>1, not SB2’s x>1 and x>2; it also compares icon - Level_1 + 1 against world in that operand order. With all three target-backed changes, SearchWorld became STRICT at 0x15c10 (464-byte object span, 451-byte body). The combined edit was compiled under the flags above.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] VoyageInit was reconstructed from the SB1 target trigger cases: the target uses its incoming icon parameter, has five supported icon/channel cases, and pushes the trailing PlayImage zero shown in the header delta above. Matching the call ABI and target icon/channel cases made the 384-byte object span STRICT against the 377-byte body at 0x13d70, with eight verified relocations under the SP3 decor-TU flags.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] VoyageStep now uses the SB1 completion cases and effects. Changing the final comparison from `m_voyageTotal <= m_voyagePhase` to target-order `m_voyagePhase >= m_voyageTotal` made the 352-byte object span STRICT against the 345-byte body at 0x13ef0, with six verified relocations under the same SP3 decor-TU flags.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] VoyageDraw’s target only interpolates the start/end position and draws the active icon. Removing SB2’s special icon-40 animation/effects and using the target interpolation and QuickIcon call made the 112-byte object span STRICT against the 100-byte body at 0x14050 under the same flags; the single relocation is verified.
- [EVIDENCE; confirmed Blupi-specific] The target’s 100-word FileExist scan and offsets prove doors[100] as 16-bit elements. Under the same SP3 decor-TU flags, the target OpenGoldsWin store at `m_doors[m_mission / 10]` (without SB2’s `+180` index or bounds guard) made OpenGoldsWin STRICT at 0x161a0. DoorsLost’s ten 16-bit stores starting at `(m_mission / 10) * 10` are STRICT at 0x161d0. FileExist remains a 544-byte candidate versus a 540-byte target body.
- [EVIDENCE; confirmed Blupi-specific, VC5-SEM-01] CurrentWrite initially differed in its start-direction copy and final fwrite condition. Replacing four independent `blupiStartDir` assignments with one 16-byte `memcpy`, then assigning `fwrite` to the existing signed `int nb` and testing `nb < 1`, changed the compare from `block_reorder,condition_differs` to STRICT at 0x14da0. The 1,584-byte object span includes the matched 10-byte alignment gap around the 1,574-byte body; 14 relocations are verified. Flags/TU context: VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy`, same private decor headers.
- [EVIDENCE; Blupi-specific, still mismatched] CurrentRead’s target restores nbVies, nbTresor, totalTresor, and goalPhase immediately after the 100-short door array. Adding those four stores (and the earlier nbLinkCaisse store) moved the candidate from 1,536 to 1,584 bytes against the 1,564-byte target. The member accesses now match, but the target falls through its buffer-validation failure cleanup and branches on exact size to the success body; the candidate still branches to its shared error tail. Label-based cleanup, nested success guards, and an immediate-return cleanup form under the SP3 decor-TU flags above remained non-STRICT; the immediate-return form also remained 1,584 bytes. Related rule: VC5-SEM-01.
- [EVIDENCE; negative, VC5-SEM-01] For FileExist, a nested success guard under the same flags changed the candidate from 544/540 to 528/540 bytes; the explicit target-shaped success label returned it to 544/540. These validation and cleanup layouts remain mismatches despite the correct 100-word scan and record size. No compiler-only rule is inferred from these variants.
- [EVIDENCE; negative] Rewriting every large CurrentWrite/CurrentRead memcpy as independent pointer-increment dword loops produced 1,744/1,648-byte functions versus the 1,574/1,564-byte target bodies; it did not produce a strict save routine. The current candidate keeps the inherited memcpy forms while retaining target-evidenced size, revision, and counter stores.
- [EVIDENCE; negative] Target pseudocode shows AdaptDoors scans ten door entries and a full 100×100 map. Under the SP3 decor-TU flags above, applying those bounds and scan direction to the SB2-derived source leaves a 704-byte candidate against the 840-byte target body. Remaining control-flow and call differences indicate more SB1-specific source work is needed.
- [EVIDENCE; compiler adaptation] SB2 decio.cpp redeclares i in two for loops and fails VC5 with C2086. The scratch candidate reuses the existing function-scope i; this enables SP3 compilation but is not itself a code match.
- [EVIDENCE; negative, earlier baseline] An earlier same-header sweep produced four strict functions at /O2 and only VoyageGetPosVie at /O1. /O1 was not rerun after SearchWorld and door-layout fixes. Related catalog rule: VC5-FLG-01.

## [OPEN] Next work

The save structure and door element width agree with target offsets. CurrentWrite is STRICT; CurrentRead and FileExist remain mismatches, as do AdaptDoors and the earlier border/map methods. The next high-value work is CurrentRead’s validation/cleanup layout, then FileExist’s error/result branch and AdaptDoors’s SB1-only state transitions.
