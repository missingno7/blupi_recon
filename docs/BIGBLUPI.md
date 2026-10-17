# BlupiStep matching

## Scope and current result

[EVIDENCE] Oracle RVA `0x9280` is one 25,269-byte function. The canonical-header baseline compiles `CDecor::BlupiStep` to 31,264 bytes and remains `MISMATCH` when forced at `0x9280`.

[HYPOTHESIS] The closest SB2 counterpart is `CDecor::BlupiStep` in `external/sb2-decomp/src/decblupi.cpp` (starts at line 389). It is the same movement/action-state-machine role and references the march, swim, and surf speed tables. SB2 is behavioral ancestry only; the oracle decides SB1 differences.

## Prefix and size log

All checks used VC5 SP3, `/O2 /GX /ML /Gd /Gy`, the complete decblupi TU, all run objects together, and a forced comparison at `?BlupiStep@CDecor@@QAEXXZ=0x9280`. “Raw prefix” counts byte-for-byte equality from the function entry; the prologue allocation immediate is part of that count.

| Candidate | Code bytes (target 25,269) | Raw prefix | Prologue frame | Result |
|---|---:|---:|---:|---|
| Canonical-header baseline | 31,264 | 2 | `0xc8` (target `0xa4`) | MISMATCH |
| Entry snapshot cleanup | 31,232 | 2 | `0xc8` | MISMATCH |
| Direct first collision rectangle, no point reuse | 31,200 | 2 | `0xc8` | MISMATCH |
| Current candidate with private layout and safe point reuse | 31,264 | 2 | `0xc4` | MISMATCH |

[EVIDENCE] From RVA `0x92c8` through `0x9331` (a 106-byte span), all 102 non-relocation bytes match; the four relocation bytes are masked. This covers endpoint setup and the first fixed-rectangle path. It is a localized byte comparison, not a strict function match. The first raw mismatch remains the frame-size immediate at byte offset 2, followed by register and stack-slot differences.

[EVIDENCE] The final full-TU verification retains the existing seven STRICT decblupi functions and two relocation-unverified bodies. `BlupiStep` adds no STRICT bytes. This worker changed no canonical source, manifest, or header.

## Header delta

[EVIDENCE] In the target pseudocode, the value at `this+0xc924` is cleared before vent-tile detection and set when the vent-icon range matches. The same early path tests `this+0xc928` as a separate state flag. Candidate source uses `m_blupiVent` in the clear/set path and `m_blupiSurf` in that state check.

[HYPOTHESIS; high confidence] The target member order is `m_blupiVent` at `0xc924`, then `m_blupiSurf` at `0xc928`; the canonical header has them in the reverse order. A private copy at `scratch/bigblupi/decor.h` swaps those declarations. The header delta preserves class size and improves the field offsets used by this function. Integrate only after cross-TU review.

## Compiler findings

- [BLUPI-specific; source correction] The baseline calls `BlupiAdjust()` before its snapshots and initializes `tinyPoint2` from `m_blupiPos`. The target entry instead snapshots the air flag and previous action, then builds the endpoint; the target body contains no `BlupiAdjust` call. Removing that call/initializer and placing the snapshots before endpoint calculation changed the body from 31,264 to 31,232 bytes, but remained MISMATCH.
- [BLUPI-specific; source correction] The first target collision rectangle is fixed at position offsets `(+12,+11)` through `(+48,+58)`. Replacing `BlupiRect(m_blupiPos)` followed by top/bottom overwrites with those four source assignments reduced the no-reuse candidate from 31,232 to 31,200 bytes and produced the equal non-relocation run above. Full result remained MISMATCH.
- [VC5-LOC-03; negative here] Reusing non-overlapping `POINT` temporaries reduced the observed frame from `0xc8` to `0xc4`, but did not reach the target `0xa4`; tested variants were not STRICT and code size ranged from 31,264 to 31,328 bytes. Reusing `tinyPoint5` for `tinyPoint2` did not reduce the frame and increased code size by 32 bytes. This does not confirm general stack-slot reuse for SB1.
- [VC5-CTX-01/03; negative probe] A diagnostic-only candidate with seven top-of-file `extern` declarations did not change `BlupiStep` size, frame, or prefix. This probe was removed and is not claimed source.

## Open work

[OPEN] The current code is 5,995 bytes larger than the oracle, with a 32-byte remaining frame difference. Continue from the first differing local/register sequence after the prologue, then walk through the vent-tile and movement branches. The current private layout swap has only been checked in the decblupi TU; the seven pre-existing STRICT bodies stayed strict in that run.
