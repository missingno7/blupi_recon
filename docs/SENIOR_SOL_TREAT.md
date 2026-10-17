# sol-treat: TreatEventBase and DrawButtons (2026-10-17)

[EVIDENCE] Base is `b148905a18284afa9c92c2652562c9a9e65c0414`. No new
STRICT function or original code byte was recovered. The final candidate and
all 20 private headers equal fresh canonical inputs byte-for-byte. Canonical
source, headers, manifest, compiler catalog, assets, references and toolchains
were not modified. Only this report is a tracked change.

## Delivery and gate

[EVIDENCE] `scratch/sol-treat/delivery/strict.patch` and
`non-strict-corrections.patch` are empty source patches; both pass
`git apply --check --allow-empty`. Newly strict bodies: none. Separately
evidenced non-strict source corrections: none. No failed experiment is retained.
`delivery/audit.json` records HEAD, source/header hashes, all 396 accepted
entries, per-run metrics, raw function hashes and the source-boundary audit.

[EVIDENCE] There were 17 serial verification runs: baseline, four TreatEventBase
controls, eleven DrawButtons controls, and the restored final baseline. All
exit zero. The 396 active entries require 393 STRICT and three FOLDED results;
the available gate observes 392 STRICT, three FOLDED and the expected
RELOC-UNVERIFIED sound CreateSoundBuffer (208-byte section). Sound's historical
`F:\Profil\sound.cpp` path cannot be mapped inside the sandbox; its accepted
code-byte checks pass. Reported strict/folded coverage is 107,232 / 141,488,
208 bytes below the handoff's authoritative-path figure. No new coverage is
claimed. Event reports 87 STRICT, one RELOC-UNVERIFIED and 21 MISMATCH records,
including inline/alias bodies.

```powershell
.pyenv\python.exe tools\verify.py --tu src/event.cpp=scratch\sol-treat\event.cpp --build-dir soltreat
```

[EVIDENCE] Every run uses pinned VC5 SP3 11.00.7022,
`/W3 /GX /O2 /ML /Gd /Gy`, manifest defines, the fresh canonical private header
set, no PCH, default job/compile-slot limits and repository TMP/TEMP through
verify.py/msvc.py. Shell invocations and campaigns run serially. The audit
proves that every trial changes only one of the two authorized bodies.
There are no header deltas, commits, game executions or remaining background
processes.

## True TreatEventBase extent

[EVIDENCE] The correct contiguous instruction extent is
`[0x1d230, 0x1ec08)`: **6,616 bytes**, consistent with both inventory files
and the Ghidra range. Its associated switch data/alignment occupies
`[0x1ec08, 0x1edd0)`: **456 bytes**. Thus the full function section is
**7,072 bytes**, exactly the candidate's extent.

| Relative interval (end exclusive) | Content | Bytes |
|---|---|---:|
| `+0x0000..+0x19d8` | Instructions through final return | 6,616 |
| `+0x19d8..+0x1a1c` | Keydown destinations, 17 pointers | 68 |
| `+0x1a1c..+0x1a83` | Keydown byte mapper | 103 |
| `+0x1a83..+0x1a84` | Alignment | 1 |
| `+0x1a84..+0x1aa4` | Keyup destinations, eight pointers | 32 |
| `+0x1aa4..+0x1abd` | Keyup byte mapper | 25 |
| `+0x1abd..+0x1ac0` | Alignment | 3 |
| `+0x1ac0..+0x1ad4` | Mouse destinations, five pointers | 20 |
| `+0x1ad4..+0x1af0` | Editor command destinations, seven pointers | 28 |
| `+0x1af0..+0x1b34` | Phase destinations, 17 pointers | 68 |
| `+0x1b34..+0x1b9b` | Phase byte mapper | 103 |
| `+0x1b9b..+0x1ba0` | Alignment | 5 |

[EVIDENCE] `asmscore.function_spans()` takes the maximum of inventory size
and distance to the next inventory start. That next entry is CJauge::Create
at `0x1edf0`, producing **7,104**, but the intervening
`[0x1edd0, 0x1edf0)` is the already accepted **CJauge constructor**, not part
of TreatEventBase. Its complete 32-byte COFF payload (21 instruction bytes,
11 NOPs) equals those oracle bytes. The constructor is absent from the Ghidra
function inventory but independently located STRICT in jauge.obj. No tool,
inventory or TU-map change is proposed within this worker's ownership.

[EVIDENCE] All **62 same-section relocation fields** independently resolve
to their oracle section-relative destinations: eight dispatch references and
54 destination pointers. The byte mappers and all alignment bytes agree.
`delivery/extent.json` records these targets and the constructor proof.
The 22-row cheat correction is already present at HEAD. Direct initialized
data validation proves the 330-byte array plus six alignment zeros at
VA `0x434480`; the loop's 330 addend reaches `0x4345ca` and validates as a
one-past reference into the matching alignment extent. No old 375 addend remains.

## Region map and remaining differences

[EVIDENCE] TreatEventBase has eight non-relocated byte differences. All code
outside the following sequences agrees at its original offset; this is a
diagnostic comparison, not a strict match:

- `+0x41a..+0x42f`: rank-zero toggle. Oracle input EDX/result ECX;
  candidate input ECX/result EDX. Six encoding bytes differ:
  `+0x41b/+0x421/+0x423/+0x426/+0x428/+0x42e`.
- `+0x6f3..+0x700`: fallback BYE PostMessage HWND load/push. Oracle EDX;
  candidate EAX. Two bytes differ: `+0x6f4/+0x6ff`.

[EVIDENCE] DrawButtons has 7,419 instruction bytes, a 7,424-byte section,
234 relocations and 65 non-relocated byte differences. Its first mismatch
is `+0x19c7`. The 63 other bytes belong to the split-title calls; differences
do not propagate to the rest of the function:

| Relative region | Oracle versus candidate |
|---|---|
| `+0x19c6..+0x19d0` | Single-line selected index: EAX versus ECX; two bytes |
| `+0x19f7..+0x1a5c` | Split copy, pointer arithmetic, selection compare: same fixed bytes |
| `+0x1a5c..+0x1a81` | First split draw: candidate hoists pixmap load before byte termination; oracle terminates then loads it; pointer/x argument staging differs |
| `+0x1a86..+0x1abc` | Second split draw: selected-index registers, pointer increment and pixmap-load scheduling differ |
| From `+0x1ac1` onward | Same fixed bytes through epilogue/alignment |

[EVIDENCE] A separate `resolve_sites` diagnostic on the final unmodified
objects verifies 218/273 TreatEventBase sites and 194/234 DrawButtons sites,
with zero hard relocation mismatches. The remaining 55/40 lack unique
run-wide symbol bindings because these callers are not located matches;
they remain RELOC-UNVERIFIED in that site-only diagnostic. The independent
cheat-data and internal-label checks above do not promote either function or
establish the remaining external bindings. Full function status is MISMATCH.
`scratch/sol-treat/relocation-audit.json` preserves the site reports.

## Previously disproven families excluded from this round

[EVIDENCE] Read both LAYOUT_ORDER rounds, SENIOR_SOL_EVENT, SOLSIB,
GERMAN_BUILD, PROVENANCE, WORKFLOW, MSVC5KB README and relevant EVENT_TU
findings. `kb.py hints TreatEventBase` and `hints DrawButtons` return keyword
matches only; VC5-CTX-01, VC5-ORD-01 and VC5-LOC-01 were consulted.
The following previous controls were not repeated:

- Broad top/before/end declaration-count and compiler-tree sweeps; real
  include/member/prototype/typedef/table order permutations; removed unused
  network declarations/unions; whole-definition reordering.
- Treat toggle polarity, ternaries, assignment chains, parallel/explicit
  outputs, cached input/result, BOOL/DWORD/byte width; shared character,
  rank, lives, phase and keyboard indices; cached HWND; sound POINT scope;
  swapped fallback stores; Escape/Return/action-sharing families.
- Draw font-expression masks, branch-based flags and comparison polarity;
  pointer casts/subtraction signedness; copy-return/alias-store forms;
  whole-row caching including pointer subtraction; newline increments;
  copy/termination/point statement permutations; loop-index identity/origin;
  copied title POINT and save-only POINT lifetime; unused gauge arrays.
- German HELP layout explains a different earlier region; it supplies no
  source correction for this shared save-title suffix.

## Compiler findings

All rows use the full freshly rebased event TU and the pinned context above.
Every accepted peer passes the available gate in every run. No catalog or
validation file was edited. Hashes in delivery/audit.json distinguish raw
section equality from a same-size/non-relocated-difference metric.

| Trigger and original construct | New controlled variation / effect | Result / rule / classification |
|---|---|---|
| Treat's two register residues; shared bEnable also carries setter inputs | Put !GetDrawSecret directly inside SetDrawSecret; separately do this for all four flag setters; third control uses distinct block-local BOOL setter inputs and retains bEnable only for the sound result. All three complete raw target sections are identical to baseline. | MISMATCH, eight bytes; VC5-LOC-01 / BLUPI-REG-01; **negative**. Runs t_direct_secret, t_direct_flags, t_setter_inputs_separate. |
| Treat fallback HWND selection in a single BYE if | Replace only this if with a single-case phase switch. Complete raw target section remains identical. | MISMATCH, eight bytes; VC5-CTL-01 / BLUPI-REG-01; **negative**. Run t_bye_switch. |
| Draw title-call selection/register graph; inline font arithmetic | Stage actual font values immediately at their draw sites using three independent locals, a single-line local plus shared split local, one shared root local, or three independent ternary locals. Unlike the older prefont controls, these stage at the call sites, after termination/point assignments. Reuse the actual split length scalar for the second font in a fifth control. All five complete raw target sections equal baseline. | MISMATCH, 65 bytes; VC5-LOC-01 / VC5-ORD-01 / BLUPI-REG-01; **negative**. Runs d_font_* and d_length_second_font. |
| Draw's constant x argument and text/pixmap staging | Pass POINT(330,pos.y) by value for first split call, both split calls, or all three title calls; remove replaced x assignments. First-only and all-three are raw-byte-inert. Both-split alone changes the scheduling family to 95 fixed differences at unchanged body/section size. | MISMATCH; VC5-LOC-01 / VC5-ORD-01; **negative**. Runs d_point_first/both/all. |
| Draw row-value identity without changing the target pointer subtraction | Introduce a used filename pointer for strchr alone, strchr plus single-line draw, or those uses plus strcpy; deliberately retain the original array expression in the measured length computation. All three complete raw target sections equal baseline. This differs from the older full-row substitution that also rewrote the subtraction and shrank the body. | MISMATCH, 65 bytes; VC5-LOC-01 / BLUPI-REG-01; **negative**. Runs d_row_alias_strchr/calls/copy. |

## Stopping point

[EVIDENCE] The only changed target raw payload among 15 new controls worsens
DrawButtons; all other target payloads equal baseline. Four Treat and eleven
Draw controls, combined with the extensive earlier negatives, reach diminishing
returns before the approximately 30-control ceiling per function. No unsupported
source change, artificial context declaration or forced-byte construct is kept.

[OPEN] A new positive compiler/source clue is needed for the rank-zero toggle,
fallback HWND choice and save-title call graph. Do not treat the 32-byte
asmscore overrun as missing TreatEventBase code or seek a table-size fix.
The next bounded experiment should first reproduce the precise alias/store/
font scheduling difference in a reduced ordinary C++ example; another broad
equivalent-expression/context sweep is unsupported by these results.
