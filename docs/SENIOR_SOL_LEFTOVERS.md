# sol-leftovers matching pass (2026-10-17)

## Delivery and verification

[EVIDENCE] `scratch/sol-leftovers/strict.patch` contains only four candidate
TUs (`button.cpp`, `decnet.cpp`, `network.cpp`, `text.cpp`) and the evidenced
`text.h` declaration change. Canonical source, manifest, assets, references,
toolchains, and the compiler catalog were not edited. No commit was made.
All shell work was sequential; no original executable or ISO program ran.

[EVIDENCE] The patch applies to both task-start HEAD `24ce276` and the
integration HEAD `6dd29a5`: these five source/header inputs are unchanged
between those commits. Other workers' integrations changed overall coverage
during this pass; their gains are not counted here.

| Newly strict function | RVA | Original instruction bytes | Compared span |
|---|---:|---:|---:|
| CButton::Draw | 0x2b40 | 325 | 336 |
| CDecor::ObjectStart | 0x106b0 | 489 | 496 |
| CNetwork::JoinSession | 0x20cb0 | 237 | 240 |
| CNetwork::CreateSession | 0x20dd0 | 234 | 240 |
| DrawChar | 0x236b0 | 144 | 144 |

[EVIDENCE] These five distinct bodies add **1,429 unique original instruction
bytes**. Their 1,456 compared bytes also contain 27 alignment bytes. Every
relocation is STRICT; no folded, ambiguous, masked, or relocation-unverified
body is included. The final verification exits zero and preserves every
accepted function, including CButton::MouseMove and DrawTextA.

The reproducible final command is:

```
.pyenv/python.exe scratch/sol-leftovers/run.py final
```

[EVIDENCE] That focused scratch driver invokes `tools/verify.py`, substitutes
the eight owned candidate TUs, and always uses `--build-dir solleft`. Final
`blupi.cpp`, `pixmap.cpp`, `misc.cpp`, and `sound.cpp` are unchanged copies.
`final.log` and `final-results.json` preserve the result; the last build is
also in `build/verify-solleft/`. The observed whole-run coverage is 106,080
reported bytes, not this worker's gain.

## Header delta

[EVIDENCE] Only `text.h` differs from the current shared headers:
`DrawChar(..., char c, ...)` becomes `DrawChar(..., unsigned char c, ...)`,
with the matching definition change in text.cpp. Native DrawChar begins with
a full dword parameter load and an `and eax,0xff`; signed char instead emits
a byte load and different register scheduling. Unsigned char gives the full
144-byte STRICT body, all table payload/addend proofs, and both helper calls.
Accepted DrawTextA remains STRICT and its call binds to the new unsigned-char
decorated symbol at 0x236b0. No class layout or member offset changes.

[OPEN] Infra should integrate this one declaration into the canonical shared
header and run its normal whole-tree gate. This pass used private headers
beside owned candidates; it did not edit or substitute other workers' TUs.
The temporary network Send BOOL declaration and cursor-constant experiments
are reverted and excluded from the patch.

## Compiler findings

All experiments used pinned VC5 SP3, manifest `/W3 /GX /O2 /ML /Gd /Gy`, no
PCH, and the current canonical headers except the explicitly described
private experiments. Rules/hints were consulted with msvc5kb/tools/kb.py.

- [EVIDENCE; blupi-specific; VC5-CTL-01, VC5-CTX-01..04] **Draw:** replacing
  the disabled rank ternary `m_bDemo?5:4` with ordinary if/else DrawIcon calls
  removes the candidate's `setne`/add-rank arithmetic and produces the native
  branch/push sequence and entire STRICT 325-byte body. No comma expression
  or pointer-walk rewrite is retained. Removing the unused inherited RECT
  declaration is required context: the controlled `draw-rect-control` run
  adds only that declaration back, leaves Draw STRICT, and regresses accepted
  MouseMove at seven load/SIB bytes. Restoring the final source passes. No
  claim is made that all unused declarations have this effect.
- [EVIDENCE; blupi-specific; VC5-LOC-01, BLUPI-REG-01] **ObjectStart:** the
  original initialization order was POINT, speed copy, distance zero. Moving
  distance zero before speed/POINT restores the native speed/distance
  registers and redundant branch-local zeroing, leaving six store-order
  bytes. The order distance zero, POINT copy, speed copy closes those six
  bytes and makes all 489 original bytes STRICT. No semantics, packet fields,
  direction values, member offsets, or helper signatures changed.
- [EVIDENCE; blupi-specific; VC5-ORD-01, VC5-SEM-01] **JoinSession and
  CreateSession:** target setup zeroes DPNAME after successful Open, then
  writes dwSize and the two name pointers without an extra dwFlags write.
  ZeroMemory plus the old explicit flags store still mismatches; removing
  that redundant explicit store makes JoinSession STRICT. CreateSession
  additionally needs descriptor assignments in size, flags, application
  GUID, maximum players, session-name order; this closes its remaining
  19 fixed bytes and makes it STRICT. Whole-TU accepted peers stay STRICT.
- [EVIDENCE; blupi-specific; VC5-SEM-01] **DrawChar:** changing only its
  character parameter and prototype to unsigned char, retaining canonical
  x-before-y assignments, produces STRICT code and preserves DrawTextA.
  Swapping those assignments to y-before-x instead leaves 13 fixed-byte
  differences plus wrong table relocation addends. The earlier int-parameter
  experiment's caller regression is not repeated or accepted.
- [EVIDENCE; negative; VC5-ORD-01] **Button Create:** scalar width/position
  reorder, aggregate POINT copies, flags-after-dimensions, flags-only reorder,
  and earlier window assignment were tested. Compared spans remain 176;
  fixed differences range from 6 to 65. None is retained. These do not
  overturn the earlier negative store-order investigations.
- [EVIDENCE; negative/incomplete; VC5-ORD-01, VC5-SEM-01] **Pixmap Create:**
  using the bTrueColor argument for SetDisplayMode and ordering clip-bottom
  initialization with the other clip fields reduces 597 baseline fixed-byte
  differences to 10. Remaining differences are two instruction-order swaps
  at +0x67 and +0x12d. Aggregate m_dim assignment, component orders, testing
  the mouseType argument, and reused/separate bit-depth locals did not close
  them. Span stays 912. `pixmap-near.cpp` preserves the ten-byte checkpoint;
  it is excluded from the integration patch.
- [EVIDENCE; negative/incomplete; VC5-LOC-01, VC5-SEM-01] **QuickIcon:** the
  target lacks the inherited generic-channel bounds guard and computes both
  grid dimensions before testing rank. Removing that guard, separating
  HRESULT/clipping temporaries, changing comparison operand order, and
  caching icon dimensions shrink 800 to 768 compared bytes but do not match
  the 770-byte original. The target's 0x14 frame versus the candidate's 0x10
  frame and grid-division spill remain open. Declaration-order variants are
  inert here; no unneeded local or forced spill is retained.
- [EVIDENCE; negative; VC5-CTL-01, VC5-ORD-01] **AddCDPath:** starting from
  Luna's evidenced five-prefix correction, common versus duplicated filename
  append, inverted prefix conditions/arms, and an intermediate relative-path
  BOOL do not reproduce native arm order/tail sharing. The closest candidate
  stays 336 bytes with 80 fixed differences. `misc-near.cpp` preserves it.
- [EVIDENCE; negative; VC5-SEM-01] **Send:** BOOL versus DWORD third parameter
  and the guaranteed-send ternary leave the same 31 fixed-byte differences
  and 80-byte span. The header and body are restored. Prior Close and
  ReadConfig negative families were read and not rerun.

## Held results and remaining blockers

[EVIDENCE] **GetToolTips (0x2e10, 237 original bytes)** is individually STRICT
in `button-held.cpp`: demo-enabled tooltips, expanded-menu bounds, and demo
negation reproduce the complete body and 240-byte compared span. The
one-body-only `held-tips-control` run confirms it regresses accepted
MouseMove at the same seven load/SIB bytes. Positive/nested guard forms,
separate tooltip-value local, and negation assignment forms keep this peer
failure. It is excluded; the final candidate retains canonical GetToolTips.
Its independently strict bytes are **not** included in the delivered count.

[EVIDENCE] **ChangeSprite (0x1f210, 285 bytes)** uses direction values 3..10
for ARROWU, ARROWD, ARROWL, ARROWR, ARROWUL, ARROWUR, ARROWDL, ARROWDR. With
those private constants and a first-arrow if/else default-handle assignment,
the remaining native default path reloads the argument from the stack while
the candidate copies EDI (two bytes shorter). Constants/default-path changes
are not retained because the function is not STRICT. SPRITE_MAP's meaning
is not established by that experiment.

[EVIDENCE] ReadConfig remains the documented one-byte SIB residue at +0x89.
DoInit remains three differing bytes in the CDecor allocation immediate
(candidate 0xdbd8, native 0x258d4). No additional count campaign, unknown
class tail, or allocation override was added. CreateSoundBuffer remains
RELOC-UNVERIFIED because __FILE__ differs from `F:\Profil\sound.cpp`;
its byte hit is not counted.

[OPEN] Next useful work is authentic button/pixmap TU context for the held
GetToolTips and scheduling residues, and identifying CDecor's real missing
tail for DoInit. Further equivalent branch/store rewrites have diminishing
returns without new source or layout evidence. No background process remains.
