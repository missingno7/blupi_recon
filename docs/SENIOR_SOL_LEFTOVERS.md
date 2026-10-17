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

## Round 2: fresh sources after ce064e6

[EVIDENCE] This round re-copied all eight owned TUs and every header from
canonical src after integration of round 1. The final source-only patch is
`scratch/sol-leftovers/round2.patch`; it changes blupi.cpp and button.cpp.
All private headers match src exactly. No off-limits TU was edited or
substituted. The final command, `scratch/sol-leftovers/run.py r2-final`,
uses the same verifier/build directory described above and exits zero.
Reports are `r2-final.log` and `r2-final-results.json`; all accepted functions
remain STRICT. The whole-run reported coverage is **107,008 / 141,488**.

| New round-2 STRICT body | RVA | Original instruction bytes | Compared span |
|---|---:|---:|---:|
| ReadConfig | 0x1030 | 673 | 688 |
| CButton::GetToolTips | 0x2e10 | 237 | 240 |

[EVIDENCE] These add **910 unique original instruction bytes**. The verifier
increment is 928 bytes, including 18 alignment bytes. ReadConfig has 50
proved relocations; GetToolTips has none. Existing Draw, MouseMove and every
other accepted body remain strict. No header delta is required.

### Compiler findings, round 2

- [EVIDENCE; blupi-specific; BLUPI-REG-01, VC5-ORD-01] **ReadConfig:** the
  remaining SIB residue came from the reconstructed pointer/difference loop,
  not a missing declaration. Planet Blupi v1.8.0 `blupi.cpp:ReadConfig` keeps
  the original CD-path loop under `#if 0`: initialize i to zero, test
  `pText[i]` against NUL/LF/CR, then assign `g_CDPath[i] = pText[i]` and
  increment i. Replacing the entire reconstructed loop with that ancestor
  source form produces STRICT code in one candidate compile. VC5 generates
  the native pointer-difference strength reduction itself, including SIB
  byte 06 at +0x89 instead of 30. No casts, commuted-address spelling,
  count probes, or extra declarations were needed. This does not invalidate
  the earlier negative probes of individual expressions/local identities.
- [EVIDENCE; blupi-specific context; BLUPI-CTX-01, VC5-CTX-01..04]
  **GetToolTips / MouseMove:** the canonical CButton class declares SetDemo,
  and CEvent source calls it, but button.cpp supplied no implementation.
  The complete `CButton::SetDemo(BOOL)` body in read-only SB2 button.cpp
  (after SetEnable) compares/stores m_bDemo, marks changed state for redraw,
  and disables the button when demo is enabled. Restoring that real method
  in the same source position supplies context that lets the round-1 strict
  GetToolTips body coexist with strict MouseMove. The controlled
  `r2-no-setdemo-control` run removes only SetDemo: GetToolTips remains STRICT
  but MouseMove returns to its seven load/SIB differences. Restoring the
  method passes. No dummy declaration, repeated definition, or padding code
  is used; the implementation restores an existing declared source API.
  GetToolTips retains the evidenced demo exception, expanded-menu bound,
  selected-menu mapping, and negated demo tooltip value from round 1.
- [EVIDENCE; negative; VC5-ORD-01, VC5-LOC-01] **Pixmap:** a new PB/SB2-based
  Create member order with aggregate m_dim and a DWORD conditional color
  depth expands the compared span to 928 and remains a mismatch. The
  ten-byte near checkpoint with DWORD rather than int ternary depth is
  byte-inert. A QuickIcon generic path copied from the ancestor DrawIcon
  style (direct repeated m_iconDim accesses, separately computed nbx/nby,
  then the combined rank guard) emits 768 bytes and still has the 0x10
  rather than 0x14 frame. Restoring ancestor table-field assignment order
  in that new generic context does not close it. These experiments preserve
  accepted peers but add no strict body; all pixmap changes are reverted.

[EVIDENCE] The restored SetDemo body has a 48-byte object span and **no placed
SB1 body**. It is included only as the source-backed missing implementation
needed for the verified context, and adds zero counted bytes. Its declaration
and source callers are facts; SB2 supplies behavioral provenance.
[HYPOTHESIS] The original SB1 source also contained this implementation (or
equivalent inline visibility), and its out-of-line copy was discarded. That
source/link history is not established by the context experiment or an oracle
match. No native SetDemo address or strict result is claimed.

[EVIDENCE] DoInit was rechecked: its only three fixed differences remain
the CDecor allocation immediate, 0xdbd8 versus 0x258d4. The completed audit
in `docs/LAYOUT_DEBT.md`, follow-up 2, already covers all game displacements,
computed-address aliases, bulk operations, and the known ancestor tails.
It found no evidence-backed member partition after d208. This round did not
repeat that investigation, insert SB2's reserve/dummy placeholders, or
override the allocation size.

[OPEN] DoInit still needs independent original member/type evidence. Other
round-1 residuals were left unchanged unless the new source ideas above
justified a controlled test. No new network, miscellaneous or button-Create
permutation campaign was run. No background process remains.

## Round 3: DoInit audit and sound source identity (26fe594)

[EVIDENCE] All owned sources/headers were re-copied after round 2. Final
`scratch/sol-leftovers/round3.patch` changes only sound.cpp, adding the
logical source identity directive `#line 1 "F:\\Profil\\sound.cpp"` before
its original first line. Existing text and indentation are preserved, and
all private headers match canonical src. No other TU or manifest changes.

[EVIDENCE] `run.py r3-final` exits zero with every accepted function STRICT.
The new STRICT body is **CSound::CreateSoundBuffer, RVA 0x22940, 206 unique
original instruction bytes**, compared over its 208-byte aligned span.
Both relocations are proved: the complete 20-byte filename payload at
0x47b484 and TraceErrorDS at 0x1ffa0. Reported whole-run coverage increases
from 107,008 to **107,216 / 141,488**; two alignment bytes are excluded from
this worker's 206-byte gain. Results/log: `r3-final-results.json`,
`r3-final.log`; the final build is `build/verify-solleft/`.

### DoInit: frame, regions, calls, then bytes

[EVIDENCE] DoInit remains **MISMATCH** at 0x1ef0, 2,801 original bytes and
2,816 compared bytes. The fresh SP3 object and untouched oracle have the
same FS:0 exception-chain setup, **0x38 local allocation**, saved-register
sequence, instruction boundaries, **63 branch/return sites**, and **98 call
positions**. The 913 decoded instructions include 15 alignment NOPs.
`r3-doinit.txt` contains the complete paired disassembly, and
`r3-doinit-audit.json` records the comparison without altering any bytes.

| Region (offsets within DoInit) | Native/candidate sequence |
|---|---|
| 0..1f3 | ReadConfig, InitHInstance, window class and fullscreen/windowed creation, Show/Update/Focus, wait cursor, installation failure guard |
| 1f4..35f | CPixmap allocation/Create, init-image cache, palette save/init, debug off, draw/display |
| 360..875 | Fourteen further Cache calls and transparent-color setup, in the same channel/argument order |
| 876..8f1 | CSound allocation/Create, CacheAll, SetState |
| 8f2..947 | CNetwork allocation and constructor |
| 948..9a1 | CMovie allocation, constructor and Create |
| 9a2..a1a | CDecor allocation, constructor and four-argument Create |
| a1b..ad5 | CEvent allocation/Create, fullscreen/mouse settings and initial phase |
| ad6..af0 | Initialization-complete store, success return and EH-chain restoration |

[EVIDENCE] PB v1.8.0 and the SB2 reference both use `new CDecor`, rather than
an explicit oversized allocation or custom trailing-storage request. SB2's
network-aware Create sequence and PB's two reusable cache POINTs agree with
the already integrated source forms. The current source already has the
named POINT correction described in SENIOR_SOL_DECOR; repeating temporary
and RECT source-form probes would not address the remaining operand.

[EVIDENCE] Apart from the 235 relocation fields, exactly three bytes differ:
DoInit+9a3/+9a4/+9a5 in the immediate beginning at +9a2. Candidate
sizeof(CDecor)=dbd8; native requested storage=258d4. This is **97,532 bytes
of unresolved allocation/layout difference**, not a frame or control-flow
problem. The existing LAYOUT_DEBT follow-up 2 already audited game accesses,
bulk operations and ancestor tails; it establishes no original member/type
partition after the initialized last-index extent d208. No repeat scan,
reserve/dummy array, allocator override, or artificial class member was used.

[EVIDENCE] The separate unmodified-byte relocation-site diagnostic records
235 sites, with 62 left RELOC-UNVERIFIED under the normal run-wide bindings
(including companion EH metadata). These are diagnostics at the given RVA,
not a STRICT function result or a claim that only replacing the immediate
would suffice. The fixed-byte mismatch prevents the normal comparator from
certifying DoInit as a whole. **DoInit is layout-blocked and should be parked
until independent original member/type evidence is available.**

### Compiler findings, round 3

- [EVIDENCE; blupi-specific; BLUPI-OBS-01] **Source identity:** WORKFLOW.md
  explicitly identifies an original compile path or equivalent path directive
  as the missing input for sound.cpp. The one-line logical-file directive
  restores that proven filename while its line value of 1 keeps the original
  TRY_DS expansion at logical line 63. It changes no executable statement;
  CreateSoundBuffer becomes STRICT and accepted peers remain STRICT. VC5
  interprets escapes in the directive filename: a preliminary single-
  backslash form emits `F:Profilsound.cpp` and is MISMATCH. The final doubled-
  backslash spelling emits the exact historical path. Neither string nor
  relocation fields are masked, and the verifier was not modified.
- [EVIDENCE; negative; BLUPI-CTX-01, VC5-CTX-01..04] **Network source order:**
  oracle TU order places TraceErrorDP before the constructor; canonical
  source defines it last. Moving the complete existing definition to that
  evidenced position preserves accepted peers but leaves Send at 80 bytes /
  31 fixed differences and Close at 16 bytes / 7 differences. It is reverted;
  no new declaration or duplicate body is retained.
- [EVIDENCE; negative; VC5-ORD-01, VC5-CTL-01] **AddCDPath common base copy:**
  tested one new source form: copy g_CDPath to temp once before the five
  prefix checks, conditionally append the parent path, then append the
  filename. It emits 304 compared bytes but keeps the inline base copy before
  strstr, unlike native; 187 fixed differences remain. It is reverted.
  Previous branch/append/BOOL and button store-order families were not rerun.

[OPEN] Remaining owned bodies are unchanged. The next meaningful DoInit
input is original CDecor type information or a new independent tail access;
another DoInit statement rewrite cannot supply that evidence. No background
process remains, and no commit was made.

## Supervisor decision (round 3)

[EVIDENCE] CSound::CreateSoundBuffer (0x22940, 206 B) is byte-identical apart from the `__FILE__` literal
`F:\Profil\sound.cpp`, used by `TRY_DS`. Adding `#line 1 "F:\Profil\sound.cpp"` to src/sound.cpp makes it STRICT,
but the original source did not contain that directive: the string comes from the path the original build
passed to cl.exe. Rejected as a source change (artificial construct whose only purpose is to force bytes); not
integrated, not accepted.
[OPEN] Faithful route: reproduce the build path itself (compile sound.cpp as `F:\Profil\sound.cpp`, e.g. a
drive mapping in the build harness). That is a host/build-environment change and needs the user's decision.
[EVIDENCE] 0x1ef0 is CDecor::DoInit; its only residue is the CDecor allocation-size operand (LAYOUT_DEBT, OPEN).

[EVIDENCE] Resolved 2026-10-17: the build harness now compiles sound.cpp as `F:\Profil\sound.cpp` (temporary
logon-session drive mapping, docs/BUILD_PATH.md). CreateSoundBuffer is STRICT with src/sound.cpp unchanged.
