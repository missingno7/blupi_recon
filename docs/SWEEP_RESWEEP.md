# Engine near misses after the corrected CDecor layout

## Result

[EVIDENCE] A clean four-TU verification used the corrected canonical headers and the pinned MSVC 5.0 SP3 compiler (`/W3 /GX /O2 /ML /Gd /Gy`). The corrected-layout baseline had 14 STRICT in `decor.cpp`, 36 in `decdesign.cpp`, 19 in `decmove.cpp`, and 14 in `decblupi.cpp`. The final candidates retain the first two counts and raise the latter counts to 24 and 15. No regressions were reported in these four TUs.

Six newly STRICT, distinct target bodies add 682 unique oracle bytes. The `candidate bytes` column is the COFF function extent; the `oracle bytes` column is the Ghidra body size from `oracle/functions.csv`.

| Function | RVA | Oracle bytes | Candidate bytes | Relocation proof |
|---|---:|---:|---:|---|
| `IsOutWater` | `0x102e0` | 75 | 80 | `IsPassIcon` |
| `NetDataFlush` | `0x10a00` | 86 | 96 | `NetMessageIndexFlush` |
| `NetAdjustToLift` | `0x11160` | 72 | 80 | none |
| `NotifPush` | `0x11510` | 130 | 144 | `NotifPop`, four-argument `PlayImage` |
| `MoveChargeDetect` | `0x13800` | 159 | 160 | imported `IntersectRect` |
| `MoveObjectPriority` | `0x13c10` | 160 | 160 | `UpdateCaisse` |

All six are `STRICT` with proven relocations where present. The candidate sources are [decblupi.cpp](../scratch/resweep/decblupi.cpp) and [decmove.cpp](../scratch/resweep/decmove.cpp). No canonical source or header was changed.

## Compiler findings

- [EVIDENCE] **VC5-SEM-01; Blupi-specific.** In the corrected `decmove.cpp` context, `NetDataFlush` clearing `m_netTimeSincePacket[i]` produced the wrong member displacement; changing it to `m_netPrevTimes[i]` matched the target store at `CDecor+0xca84`. `NetAdjustToLift` checked `m_netBlupiPos[i].x`; changing the guard to `m_netBlupiIcon[i]` matched the target read at `CDecor+0xca34`. Each one-line source correction made its function STRICT under SP3. These are SB1 field/source corrections, not transferable compiler rules.
- [EVIDENCE] **VC5-SEM-01; Blupi-specific.** In `decblupi.cpp`, replacing the chained `icon != 91 && icon != 92 && IsPassIcon(icon)` return with explicit rejection of 91/92 followed by `return IsPassIcon(icon)` made `IsOutWater` STRICT.
- [EVIDENCE] **VC5-SEM-01; Blupi-specific.** In `decmove.cpp`, leaving `tinyRect` uninitialized before its output-only `IntersectRect` call removed the target-absent zero stores and made `MoveChargeDetect` STRICT.
- [EVIDENCE] **VC5-SEM-01; Blupi-specific.** `NotifPush` became STRICT with a local destination pointer and the full-array fallback through `NotifPop`, plus an explicit fourth `PlayImage` argument of `0`. The condition-only and pointer/goto variants without that explicit argument did not match; the final form also proves the selected four-argument overload relocation.
- [EVIDENCE] **VC5-SEM-01; Blupi-specific.** `MoveObjectPriority` became STRICT with a bounded `for` loop, a jump to the rank check when a non-ball is found, and loop exhaustion falling through to return. The original `while` and a `for` with a post-loop rank check remained mismatches. This records the measured SP3 source-form result, not a general recommendation to use `goto`.

The tested compiler and flags were the same for each result; candidates used the corrected canonical header set. No header delta was needed. These entries describe target-specific source findings and do not add or update catalog rules.

## Remaining small residue

[EVIDENCE] `AdaptBorder` at `0x144d0` remains `MISMATCH`: candidate COFF extent 128 bytes, target Ghidra body 114 bytes (128-byte target span), `asmscore` 29. The remaining differences are register/SIB allocation and instruction ordering; the call relocations agree. Scratch-only count probes with 7 and 14 declarations at TU top, and 7, 14, 23, and 30 immediately before `AdaptBorder`, all remained mismatches. This negative probe result is associated with the context diagnostic in `VC5-CTX-01`; it does not justify a semantic rewrite. A private `AdaptMidBorder(POINT)` to `(int,int)` header probe also failed the accepted-symbol identity check and was not integrated.

[HYPOTHESIS] The remaining `AdaptBorder` residue may still depend on genuine TU declarations or source form. No source-supported change was found in this pass. The size-based scanner's `MoveObjectPlouf` pairs are excluded: one proposed RVA (`0x11160`) is already strictly owned by `NetAdjustToLift`, so those are ambiguous heuristic placements rather than actionable matches.

[OPEN] Keep `AdaptBorder` as a mismatch until authentic declaration context or source evidence explains its remaining register/SIB and ordering differences. No header delta is pending from this sweep.
