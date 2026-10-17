# sol-small continuation (HEAD 999fdeb)

## Prior hypotheses already disproven

[EVIDENCE] Read the target notes in LUNA_LEFTOVERS, all three
SENIOR_SOL_LEFTOVERS rounds, CORE_CLUSTER, UI_CLUSTER, NETWAVE_TU, and
SENIOR_SOL_EVENT (including its three rounds) before candidate compilation.
The following bounded families are negative; they are not general optimizer
laws and will not be repeated without new evidence.

- Button Create: pseudocode/full/focused field-store reordering, width and
  position scalar order, POINT aggregate copies, flag placement/order,
  early window assignment, and valid-range versus early-return guard.
- Pixmap Create: aggregate/component dimension order, mouse argument versus
  field guard, reused/separate depth locals, signed/unsigned depth ternary,
  and the tested PB/SB2 member/depth order. The prior checkpoint has two
  instruction scheduling swaps, ten differing fixed bytes.
- QuickIcon: generic-channel guard removal, both grid divisions before rank
  checks, separate HRESULT/clipping locals, comparison operand reversal,
  cached versus repeated icon dimensions, declaration ordering, table-read
  ordering, and the SB2 inline-channel helper. The extra native four-byte
  frame slot remains unexplained.
- AddCDPath: common versus duplicated filename append, inverted prefix
  conditions/arms, intermediate relative-path BOOL, and base copy before
  all prefix checks. Five prefixes and the parent-path literal are already
  established; these experiments did not match native tail sharing.
- ChangeSprite: target cursor constants plus first-arrow/default-handle
  if/else. The default path still copies EDI instead of reloading the stack.
- Network Send: separated HRESULT assignment, normalized flag local,
  BOOL/DWORD third parameter and guaranteed-send ternary. Close: direct,
  guard, ternary, local Boolean, explicit-branch and bool-return forms.
  Moving existing TraceErrorDP to its native source-order position changes
  neither remaining body. Join/CreateSession are already integrated strict.
- Event parked bodies: prior scalar, Boolean, branch, expression, context
  and array campaigns are exhausted; ChangePhase, DrawButtons and
  TreatEventBase will stay byte-identical to canonical source.

## Current leads

[EVIDENCE] The event target at 0x1b090 is AddUserExportPath (181 B). Native
calls mkdir with "\\User", copies "\\User\\", appends the supplied filename,
then checks/appends ".xch". Canonical source's second strcpy overwrites the
prefix. This is a new semantic correction, separate from parked handlers.

## Compiler findings

All controls used fresh canonical headers (except the explicitly rejected
cursor-constant probe), VC5 SP3 11.00.7022 and manifest
`/W3 /GX /O2 /ML /Gd /Gy` flags through `verify.py --build-dir solsmall`.
No compiler catalog or canonical source was edited. Each named run below
has source, object, verifier JSON/log and oracle/candidate disassembly in
`scratch/sol-small/runs/`. Fixed-offset diagnostic differences are not matches.

| Trigger / original construct | Controlled change and effect | Result / rule / classification |
|---|---|---|
| AddUserExportPath overwrites the copied directory with a second strcpy | Replace only that second call with strcat. Native mkdir argument, complete directory/suffix payloads, inline copy/append bodies and suffix guard all agree (`event_append`). | STRICT at 0x1b090, 181 original B, 192-byte compared span, six proven relocations. VC5-SEM-01; Blupi-specific semantic correction, not a new optimizer rule. |
| Close directly returns the comparison of the COM call and zero, emitting NEG/SBB/INC | Keep the COM result in an HRESULT local, then compare it with DP_OK (`network_close_local`). Emits native XOR EDX / TEST EAX / SETE DL / MOV EAX,EDX. First control also assigned the Boolean back to hr (`network_parameter`); removing that assignment remains strict. | STRICT at 0x20fe0, 18 original B, 32-byte compared span, no relocations. VC5-LOC-02 / VC5-CTL-03; confirmed-transferable staging evidence for this call-result form. No claim that every comparison benefits from a local. |
| Send stages its normalized flag differently | Normalize the actual dwFlags parameter before the call, rather than a separate local (`network_parameter`). | Byte-inert: same 80-byte span and 31 fixed diagnostic differences. VC5-LOC-02 / VC5-ORD-01; negative. Send restored. |
| Pixmap Create's two independent scheduling swaps | Starting from the ten-byte prior checkpoint, declare the used depth local at function root (`create_root_depth`), then use ancestor-style declaration order with caps before HRESULT (`create_ancestor_declarations`). | Both leave Create byte-identical at 912 B / ten fixed differences. VC5-CTX-01..04 / VC5-LOC-02; negative bounded context evidence. |
| Create's POINT local lifetime might affect the remaining schedule | Move the actually used mouse POINT into its branch with initialization (`create_scoped_point`). | 912 B, frame shrinks 0x78 -> 0x70, diagnostic differences grow to 55; no strict gain. VC5-LOC-02 / BLUPI-REG-01; negative. All Create controls reverted. |
| AddCDPath has different native inline-copy tail sharing | Consume strcpy's return in the parent-path strcat (`misc_chain`), then consume nested return values in both complete filename append arms (`misc_chain_both`). These differ from prior standalone/common append controls. | Both shrink to 320 B, below the native 324-byte body; different branch/copy shapes, still MISMATCH. VC5-INL-03 / VC5-ORD-01; negative. Both reverted. |
| ChangeSprite default arm reloads the stack while the prior inside-guard local copies EDI | Initialize the actual HCURSOR from sprite at root, before both guards; test with the evidenced 3..10 arrow IDs in private def.h (`misc_cursor_root`). | 288 B, changes prefix/register graph without matching. VC5-LOC-02 / VC5-SEM-01; negative for root initialization. Source and private header fully reverted; no header delta delivered. |

## Delivery and stopping point

[EVIDENCE] `scratch/sol-small/strict.patch` is source-only against
`999fdebf58d54fc466596f6c72fa1ac25e38507b`. It changes only the
AddUserExportPath body in event.cpp and Close body in network.cpp. It adds
two distinct STRICT bodies, **199 unique original instruction bytes**;
the 224 compared bytes include 25 alignment bytes, excluded from this gain.
`git apply --check` passes; applying to a fresh scratch copy reproduces the
compiled candidate files byte-for-byte. CP1252/CRLF and tabs are preserved.

[EVIDENCE] Header deltas: **none**. Every private header is byte-identical
to canonical. Button, misc and pixmap candidates are restored byte-for-byte
to current src. All event source outside the one allowed body is unchanged,
including ChangePhase, DrawButtons and TreatEventBase. Final per-TU
verification preserves all active accepted entries, with the expected
sound.cpp historical-path gate INCOMPLETE in this sandbox. No sound source
change or drive mapping was attempted.

[EVIDENCE] The final runs are `event_final` and `network_final`, both exit
zero. Reproduce serially with `.pyenv/python.exe scratch/sol-small/run.py
event event_final` and then the corresponding `network network_final`
command. Each invokes exactly one TU substitution with `--build-dir
solsmall`. All 15 successful compile/compare runs completed; the two new
functions are independently unique STRICT placements, not wildcard or
relocation-unverified results. The source-only patch is 581 bytes;
`delivery.json` records its hash and scratch-application checks.

[EVIDENCE] Native QuickIcon's extra frame dword at local +0x10 saves the
grid width across the two rank divisions (store at function +0x1af, reload
for IDIV at +0x1d6); its RECT begins at local +0x14. This identifies the
value in that slot, not the original source declaration. The prior POINT,
dimension caching, declaration-order and separate-grid-variable controls
already cover the available leads, so no new QuickIcon source campaign was
run. Button Create is still the known six scheduling bytes; no previously
disproven store/guard campaign was repeated.

[OPEN] Remaining targets need new source/context evidence. Further equivalent
branch, declaration and field-store controls have diminishing returns.
Integrate the two-body patch and run the authoritative build outside the
sandbox to verify sound's original path. No original/ISO program was run,
no git commit was made, and all this worker's compile commands completed.
