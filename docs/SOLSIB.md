# Event SIB investigation (sol-sib)

**[EVIDENCE] Integration delivery follow-up.** One apply-checked patch is now
at `scratch/solsib/integration/integration.patch`. A fresh full copy of all
20 canonical TUs, sharing the revised event.h, passes every active accepted
entry: 330 STRICT and three FOLDED, with expected RVAs/sizes/owners preserved.
DrawButtons' 78 bytes were not closed by the additional controlled experiments;
TreatEventBase remains eight bytes away. The final delivery section below
supersedes the earlier warning that shared-header integration was untested.

**[EVIDENCE] Latest continuation after `010763d`, 2026-10-17.** Refreshed the
worker from canonical source and headers. TreatEventBase remains an eight-byte
register near match. Fifteen private header/context probes did not change it.
DrawButtons advances from a 4,886-byte fixed prefix to 6,599 bytes, with its
exact 7,419-byte body size and 78 remaining fixed-byte differences. The final
worker changes only DrawButtons and the private event.h name-array layout;
ChangeButtons remains STRICT. No new STRICT function or byte coverage is claimed.
See the final section for the retained header delta and controlled results.

## Initial ChangeButtons checkpoint

**[EVIDENCE] Partial completion, 2026-10-17.** `scratch/solsib/event.cpp`
and its private `event.h` make `CEvent::ChangeButtons` STRICT at `0x1b220`:
2,854 original function-body bytes, plus 10 matching alignment NOPs. All 83
relocations are proved. The verifier reports the complete 2,864-byte COFF
section as `code_size`; do not count the 10 NOPs as new original function code.
The other two large handlers remain mismatches. No canonical source, manifest,
catalog, toolchain, asset, or reference-repository file was changed.

## Reproduction and result

```powershell
.pyenv\python.exe tools\verify.py --tu src/event.cpp=scratch\solsib\event.cpp --build-dir solsib
```

**[EVIDENCE]** Compiler: pinned VC5 SP3; `/W3 /GX /O2 /ML /Gd /Gy`, manifest
defines, canonical headers except the two declarations below, no PCH.
`TMP`/`TEMP` are set by `msvc.py` to repository `build/tmp`.

| Function | RVA | Baseline -> final |
|---|---:|---|
| `ChangeButtons` | `0x1b220` | 3,024-byte candidate section, 1,531-byte fixed prefix -> STRICT, 2,854-byte body / 2,864-byte section, 83 proved relocations |
| `TreatEventBase` | `0x1d230` | Unchanged 6,944-byte candidate section versus 6,616-byte target; first fixed difference `+0x2da`, prefix 730 |
| `DrawButtons` | `0x16f70` | Unchanged 7,392-byte candidate section versus 7,419-byte target; first fixed difference `+0x1316`, prefix 4,886 |

**[EVIDENCE]** The baseline event TU had 68 STRICT symbols, 1
RELOC-UNVERIFIED and 40 MISMATCH; the final candidate has 69 STRICT, 1
RELOC-UNVERIFIED and 39 MISMATCH. Every baseline STRICT symbol remains STRICT;
the only added STRICT body is `ChangeButtons`. These counts include header
inline/COMDAT functions and are not counts of distinct original bodies.

Generated sources, micro objects, disassemblies and run snapshots are confined
to `scratch/solsib/`; the last verification is in `build/verify-solsib/`.
`runs/buttons_readcall/` preserves the first complete STRICT result.

## A testable SIB rule

**[EVIDENCE, follow-up clarification]** The deciding variable in the reduced
test below is **one index identity versus two index identities**, not block
nesting by itself. Two distinct indices both declared at function scope emit
the same complete 112-byte section as the two block-local indices. A single
shared index emits the commuted SIB. Thus "shared versus separate locals" is
more precise than "function scope versus block scope".

**[EVIDENCE; proposed extension to BLUPI-REG-01]** A source local's identity
and scope can select SIB base/index order even when its values, physical
registers, instruction scheduling, and generated body size do not change.
Before more address-spelling or TU-count sweeps, compare whether related
indexed blocks use two independent block locals or assignments to one shared
function-scope index. Test this with a real, used local; never add padding
declarations to the reconstructed source.

The decisive SB1 change is simply:

```cpp
// Before: each MULTI button group declares a separate playerId.
int playerId = message - WM_BUTTON1;
// ... and in the ready button group:
int playerId = message - WM_BUTTON11;

// After: one int playerId at ChangeButtons function scope;
// each group assigns its value at the same original point.
playerId = message - WM_BUTTON1;
playerId = message - WM_BUTTON11;
```

**[EVIDENCE]** With the other source corrections held fixed, the two complete
COFF function sections differ at exactly one byte: `+0x5fb`, `0e -> 31`.
Instruction `+0x5f9` changes from `8a 54 0e 25` (`[esi+ecx+25h]`) to
`8a 54 31 25` (`[ecx+esi+25h]`). Both toggle accesses at `+0x5b9/+0x5bd`,
every register load, and every other code byte stay unchanged. All 83
relocation offsets, types, symbols and addends stay unchanged. This is a
source-local identity effect, not evidence that ECX must always be the base.

### Independent reduced experiment

**[EVIDENCE]** The following stand-alone C++ file needs no headers or link.
Compile two versions with the exact flags above. In version A use the two
`int playerId = ...` definitions shown. In version B declare `int playerId;`
beside `data` and remove `int` from those two assignments.

```cpp
struct Player {
    char present, ready; short flags; int id;
    short team; char name[22];
};
struct Net { int state[8], id; Player players[4]; void send(void*, int, int); };
struct Event { Net* net; void f(int message); };
void Event::f(int message)
{
    char data[12];
    if (message >= 1 && message <= 4) {
        int playerId = message - 1;
        net->players[playerId].team++;
    }
    if (message >= 11 && message <= 14) {
        int playerId = message - 11;
        net->players[playerId].ready = !net->players[playerId].ready;
        data[8] = net->players[playerId].ready;
        net->send(data, 12, 1);
    }
}
```

**[EVIDENCE]** `micro_minimum.py` builds this pair as `micro/minimum_plain_*`.
Each section is 112 bytes (105 instructions/data bytes through `ret 4`, seven
alignment NOPs). Its only difference is byte `+0x51`: `08 -> 01`. Instruction
`+0x4f` changes `8a 54 08 25 -> 8a 54 01 25`; definitions of EAX and ECX,
the earlier ready toggle, and all other instructions remain identical. Removing
the unused Net prefix fields produces the same one-byte effect with displacement
`1`; independently reducing Player stride from 32 to 16 also preserves it.
These are three controlled pairs / six compiles, not three different rules.

**[EVIDENCE] Exact measured conditions and orientation.** In this model the
first guarded group mutates an indexed player field; the second computes
another index, toggles its ready byte, reloads `net` after that aliasing byte
store, copies the ready byte into a packet and calls `send`. At the final
ready load EAX is the scaled player offset and ECX is the reloaded network
pointer. Distinct indices select **base EAX, index ECX** (`SIB 08`); one shared
index selects **base ECX, index EAX** (`SIB 01`). The earlier ready-toggle
load/address computations have the same bytes in both forms. Neither the
physical registers nor their load order decides this pair: both are fixed
and identical. In the complete ChangeButtons transfer, the analogous scaled
offset is ESI, giving `SIB 0e -> 31`.

**[EVIDENCE] Boundary controls added in the follow-up.** Six further objects
(`micro_identity_boundaries.py`) test two root indices versus one root index,
pointer-addition spelling in each form, and unsigned-ready-byte type in each
form. Every outcome keeps the corresponding orientation above; the two-root
and two-block-local sections are byte-identical. Thirty-six scratch-only
local-count controls (`micro_symbol_count.py`: two identity forms, top/end
placement, unused-local counts 0-8) produce exactly one complete function
section per identity form. Unused local count does not explain this result.
These count controls are diagnostic micro inputs and are never candidate
reconstruction source.

**[EVIDENCE]** Reduction controls: the full 272-byte two-group model differs
only at `+0xe9` (`0f -> 39`); removing the first group's send, wrap test and
draw/adjust calls yields a 176-byte pair with the same phenomenon. Nine first
reduction pairs and six second reduction pairs were compiled. Removing the
ready toggle eliminates the scope sensitivity in that model; replacing the
first group's indexed mutation by an unrelated call also eliminates it.
Scope changes are therefore a diagnostic, not a guaranteed permutation switch.
`micro_player.py`, `micro_reduce.py`, `micro_reduce2.py`, and
`micro_minimum.py` retain the inputs and JSON instruction listings.

### What the experiments establish about the mechanism

**[EVIDENCE]** Neither textual operand order, numeric register order nor
runtime load order gives a universal SIB rule. Six direct spellings (pointer
index/addition, reversed addition, integer-address addition) produced the same
bytes. Swapping two non-volatile pointer/index load statements was also inert.
Sixty-five stand-alone variants with 0-64 later independent conditional calls
kept the same SIB roles; later block count alone does not predict the choice.

**[EVIDENCE]** Lowering context matters even with fixed incoming registers:
`char __fastcall f(char* p,int i,char* d) { return p[i+37]; }` starts
`8a 44 11 25`, but introducing the used local
`char c=p[i+37]; return c;` starts `8a 44 0a 25`. Both are otherwise identical
16-byte sections, including `ret 4`: ECX=p, EDX=i, AL=result in both. An `int`
return instead emits `movsx eax,[edx+ecx+25h]`. This is a ten-case lowering
experiment (`micro_lower.py`); the direct/local-return pair is the relevant
same-opcode, one-byte control.

**[HYPOTHESIS]** Local identity/scope survives into c2's lowered expression
representation and its commutative-address ordering. An optimizer value-number
or virtual-operand tie-break is consistent with all these results. The private
c2 ordering algorithm and the reason it prefers one orientation for an arbitrary
function are not proved. Calling every such difference a change of physical
register allocation is too specific: the reduced controls change no physical
allocation at all. BLUPI-REG-01's earlier positive control-flow result remains
valid, but does not by itself identify the compiler pass responsible.

## Header deltas requiring supervisor integration

**[EVIDENCE] Integrated in supervisor commit `34fb42e`.** The two deltas below
describe the first round. In the follow-up, `scratch/solsib/event.h` was
refreshed from canonical and is byte-identical to `src/event.h`; no new header
delta is requested.

| Declaration delta | Evidence and limit |
|---|---|
| `static BOOL CopyMission(char*, char*);` | Target `0x1b150` ends with plain `ret` at `+0xc2`; caller cleans two arguments, e.g. `ChangeButtons +0x85f`. |
| `static void AddUserExportPath(char*, char*);` | Target `0x1b090` ends with plain `ret` at `+0xb4`; caller cleans two arguments at `ChangeButtons +0x6b5` and `+0x823`. No this argument is passed. The separate LPCSTR overload is unchanged. |

**[EVIDENCE]** The instance `__thiscall` declarations emitted `ret 8` helpers
and omitted required caller cleanup. Static member declarations give the proved
two-argument cdecl ABI and let the caller verify strictly. **[OPEN]** Machine
code alone cannot distinguish static members from free cdecl helpers. The
private static spelling preserves existing source names and class structure;
it is not proof of original C++ ownership. No constants, offsets or members
were changed. The two helper bodies themselves remain MISMATCH.

## Other necessary ChangeButtons corrections

**[EVIDENCE]** The SIB fix was isolated after matching the rest of the body:

- Restore descendant-style READDESIGN/WRITEDESIGN local buffer scopes. Two
  256-byte buffers share slots across the sibling blocks; exported filename
  uses `esp+11c`, mission path `esp+1c`, matching target. Swapping two
  function-scope declarations had no effect.
- READDESIGN pagination uses `((m_nbFiles+5)/6)*6`, not
  `6*(m_nbFiles+5)/6`. Use the target's FALSE/conditional-TRUE hide variables
  and the visible-first loop arm, consistently with the earlier SERVICE block.
- GAMEWRITE uses the inclusive six-button range; the old condition
  `message == WM_BUTTON1 && WM_BUTTON6` tested only button one.
- Read-game confirmation calls `CurrentRead`, landing at `0x1af60`; write-game
  confirmation calls `CurrentWrite`, landing at `0x1afc0`. Using CurrentWrite
  at both sites caused a real relocation-binding conflict even when all fixed
  bytes matched. The final correction proves both distinct targets.
- Read audio/MIDI volume only in each selected button arm, as the target's
  calls show. Buttons 5/6 store FALSE/TRUE at `m_bMovie +0x98fc`; the SB2
  true-color background/decor calls are absent in SB1.
- Select joystick ID with `m_joyIDList[m_joyIndex-1]` when index >0. The final
  target access is `[this+eax*4+0x3f0]`, while the array starts at `+0x3f4`.

## Compiler findings

| Trigger / original construct | Controlled variation, counts and effect | Strict result / rule / classification |
|---|---|---|
| ChangeButtons `+0x5fb` commuted SIB; two block-local `playerId`s | One shared, used function-scope index; otherwise one-byte-identical section. Independently reproduced by the three minimum pairs / six compiles and the larger reductions above. | Full caller STRICT after source/ABI/callee corrections, 83 proved relocations; BLUPI-REG-01 extension, **confirmed-transferable**. Exact internal c2 algorithm remains unverified. |
| Same SIB, byte-expression/type hypotheses | Ready field `unsigned char` / `bool` (2), packet `unsigned char` / BYTE (2), one-byte memcpy / unsigned cast / int local / access-only network local (4), unsigned index / recomputed index / shared index / whole-send-block cached network (4). | Shared index and whole-block network cache flip the desired SIB; access-only cache and the other seven width/access variants do not. Recomputed index and int local regress earlier code. Only shared-index result is retained. BLUPI-REG-01 / VC5-LOC-01; **positive source-identity result and negatives**, no universal caching rule. |
| Load-order and instruction-width theories | Six spellings, two load-order controls, 65 later-call counts; ten lowering cases with fixed fastcall inputs. | No original-game match claimed for micro objects. Address spelling/count controls negative; direct vs used-local byte return changes only SIB. VC5-NEG-01/02 and BLUPI-REG-01; **measured compiler controls**. |
| ChangeButtons later size/load/constant differences | Two ordinary source-form batches, settings correction, descendant scopes, joystick index; one-variable scope swap itself inert. | Fixed-byte mismatch count 1,052 -> 1 before the SIB fix; complete body then STRICT. VC5-SEM-01 / VC5-ORD-01 / VC5-LOC-03; **Blupi-specific reconstruction evidence**, not a new optimizer rule for each fix. |
| TreatEventBase `+0x41a`, `+0x6f3` register residues | Narrow `bEnable` to completed-cheat block; narrow `c` to its use block; reuse existing `i` for both life counts; replace cached VK_RETURN phase by direct member expressions (4 valid variants). | All retain 6,944-byte section and prefix 730; no improvement at either register site. BLUPI-REG-01 transfer **negative here**. One intermediate typo in the lives probe failed compilation and was corrected before measurement. |
| DrawButtons `+0x1316` page-index scheduling | Ternary / reversed compare / existing POINT local / conditional font local (4); reuse root loop `i` (1); root shared font / shared ternary font / shared font plus root i (3). | Best remains 7,392 bytes, prefix 4,886. Conditional font local regresses prefix to 4,245; shared i changes later bytes only. BLUPI-REG-01 / VC5-ORD-01 / transferred VC5-EXP-06; **negative here**. |

## Remaining work

**[OPEN]** TreatEventBase retains the branch-span and ECX/EDX, EAX/EDX
differences identified in EVENT_TU.md; DrawButtons retains the suffix-copy
scheduling difference. Their tested local-scope transfers did not improve
either function. Reconstructing those blocks further is a better next step
than repeating the disproven width, spelling, count, or compiler-tree sweeps.
The first round's ChangeButtons body and ABI declarations were subsequently
integrated in `34fb42e`; the follow-up below supersedes this integration step.

## TreatEventBase follow-up

**[EVIDENCE]** Checked `git log -1 -- src/event.cpp`; after ChangeButtons
integration appeared as `34fb42e`, refreshed `event.cpp`, `event.h`, and the
round's baseline `treat_start.cpp` from `src/`. The refreshed TreatEventBase
reproduces the prior 6,944-byte section, 730-byte fixed prefix, ECX-input/EDX-
result toggle at `+0x41a`, and EAX HWND load at `+0x6f3`. All subsequent
experiments changed only TreatEventBase. DrawButtons was not worked on in
this follow-up.

**[EVIDENCE] The refreshed branch-span residue is 13 bytes, not seven.** Both
WM_KEYDOWN entries are at `+0x32f`. The jump instruction at `+0x2d8` is
`0f 87 46 09 00 00`, reaching `+0xc24`; target is `0f 87 39 09 00 00`,
reaching `+0xc17`. Byte `+0x2da` is its displacement. This measurement is
retained as `t2_refreshed_baseline_metrics.json` and the paired disassemblies.
No run in this follow-up established a seven-byte residue in the refreshed
canonical shape.

### Compiler findings: transfer tests

**[EVIDENCE]** Thirty-six distinct valid TreatEventBase variants were measured
with the pinned flags, the complete TU and `--build-dir solsib`; none improves
the three requested sites while preserving the preceding target shape. The
following are negative transfers of BLUPI-REG-01's local-identity diagnostic,
not evidence that the original SIB observation was a physical-register rule.

| Family | Controlled inputs / count | Result |
|---|---|---|
| Toggle inputs and result form | Used cached BOOL input, cached result, assignment chain, ternary (4); shared BOOL input/result across cheats 0/1 (2); bool input, chained branch outputs, branch-local enable result, existing-i input (4); parallel outputs, explicit outputs in both branches, reversed assignment chain (3) | 13 variants. Cached input and shared input are code-inert at the sites. Result-first forms move the ESI copy before the store; explicit outputs retain extra branches. A larger branching form happens to choose EDX for the later HWND, but changes many preceding instructions and is not a fix. No target ECX-result toggle was recovered. |
| Other actual variable identities and lifetimes | Remove fwKeys alias; separate/direct sound point; separate cheat index; separate network indices; local/shared HWND (7); shared/separate completion character and folded preincrement (3); reuse i or fwKeys for enable, i for phase (3); narrow received-player ID alone or ID plus packet data to the MULTI block (2) | 15 variants. Scalar/index/HWND copies are mostly inert. Point separation and receive-block narrowing change the frame/stack map and regress the prefix to 2 bytes. Sharing enable/phase with existing root integers changes neither early register site. |
| Keyboard shared value / call-argument source form | Shared root i, fwKeys, or dedicated keyPress (3); direct member assignment inside each KeyChange call, with a shared DemoRecEvent tail (1) | 4 variants. First three choose EAX masks and regress the early SIB to prefix 172. The direct member-expression form preserves prefix 730 and reproduces the target's EDX mask arms, EDX-to-EAX argument copy, and shared member store/call, but shifted later in a 7,008-byte MISMATCH section. Its dispatch destination is `+0xc56`, still wrong. |
| VK_RETURN/QUITPLAYTEST shape | Put positive PLAYTEST action first with a shared label; duplicate ordinary calls; positive-first ordinary calls; positive-first plus the direct keyboard form (4) | All preserve the original early register residues and worsen or fail to improve the dispatch span. No such control-flow rewrite is retained. |

**[EVIDENCE]** One initial overlap between two probe batches reused the same
private build directory. Those ambiguous outputs were discarded; affected
identity tests were rerun sequentially. Four intermediate malformed probes
failed compilation and contributed no measured result. Source/object/result
snapshots for the valid runs are in `scratch/solsib/runs/t2_*/`.

### An independent register-choice control

**[EVIDENCE]** `micro_assignment.py` (three objects) shows that
`int x=*p|k; *p=x; call(x);` and `int x=*p|k; call(*p=x);` are byte-identical,
whereas `call(*p=*p|k);` selects a different register path and adds a copy into
EAX before the push. `micro_keyboard.py` (three objects) confirms the same
source distinction with two switch arms: separate member update followed by
the common call uses ECX; placing the member assignment inside each call uses
EDX and a common `mov eax,edx; push eax` tail. Expanded versus compound
assignment inside the call is byte-identical. These are measured compiler
controls; the related TreatEventBase variant matches that target sub-sequence
but remains MISMATCH as a whole. It is not a new STRICT function or proof of
the original source spelling.

### Static location of the remaining layout differences

**[EVIDENCE]** `treat_switches.py` decodes the actual oracle switch mapper and
COFF switch-table relocations. It does not normalize them into a match.

| Case entry | Target | Refreshed baseline | Direct-keyboard experiment |
|---|---:|---:|---:|
| Escape | `+0x736` | `+0x736` | `+0x736` |
| Return | `+0x9f7` | `+0x9d9` | `+0x9d9` |
| Space | `+0xaab` | `+0xb68` | `+0xaea` |
| Pause | `+0xadf` | `+0xafd` | `+0xb1e` |
| Left / Right | `+0xb0b / +0xb19` | `+0xb1c / +0xb42` | `+0xb4a / +0xb58` |
| F1 / F4 | `+0xb5f / +0xbe9` | `+0xba8 / +0xbe1` | `+0xb9e / +0xc28` |

**[EVIDENCE]** With the target-shaped direct keyboard tail, all entries from
Space through F4 have the same +63-byte displacement. Escape's region is 30
bytes short and Return's region is 93 bytes too large relative to the target.
The target INFO/LOSTMULTI Escape action owns a separate 30-byte GAMER action at
`+0x9bb`; the current candidate merges it with the later Return GAMER action.
This is a concrete block-sharing/source-form lead for further work; it is
not explained by changing an arbitrary SIB byte or by the net 13-byte span
alone. No proposed way of recovering that layout has yet been verified.

**[OPEN]** The exact internal c2 ordering key remains unknown. The catalog
rule should specify the tested index-identity pattern and its counterexamples,
not promise a universal mapping from local scope to any register choice.
The best retained TreatEventBase at that checkpoint was the refreshed canonical body; the failed
variants remain evidence only. ChangeButtons remains STRICT. Next investigate
the differing Escape/Return action sharing using the decoded switch/label
map, then retest the two early scalar-register residues in that closer shape.

## Escape action and complete TreatEventBase layout recovery

**[EVIDENCE]** Follow-up to the separate Escape GAMER-action lead. The retained
candidate is `scratch/solsib/event.cpp`; the reference checkpoint is
`scratch/solsib/runs/t3_no_intro/`. Only TreatEventBase differs from the
`34fb42e` event.cpp baseline. It has a 7,072-byte COFF function section:
6,616 bytes of instructions through the final `ret`, then 456 bytes of switch
tables/alignment. Comparing that complete section with the original at
`0x1d230` leaves **eight non-relocated byte differences**, all register
encodings. This is **MISMATCH**, not STRICT or a new byte-coverage claim.
The section has 273 relocations; strict relocation proof remains required.

**[EVIDENCE]** The retained layout has the target's separate Escape
INFO/LOSTMULTI -> GAMER call/TRUE-return action at `+0x9bb`, STOP action at
`+0x9d9`, and Return entry at `+0x9f7`. Return's PLAYTEST action is at `+0xa34`,
its shared GAMER action at `+0xa8d`, and its other phase handling starts out
of line at `+0x1870`. The jump at `+0x2d8` is now exactly
`0f 87 39 09 00 00`, reaching `+0xc17`. Space, Pause, all six movement/fire
arms, and F1..F4 have their exact target entries. Generic TRUE/FALSE exits
are at `+0x1820` / `+0x1861`, and the keyboard mask/store/call tail is at
`+0xcfd`. Both earlier branch-span and later block-placement residues are
therefore resolved without padding or unused source constructs.

**[EVIDENCE]** A positive sequence of Return phase checks, with a shared
PLAYTEST action, first restored the Escape action, but did not by itself
restore the whole layout. Combining it with the target keyboard behavior
and call-expression form puts the remaining Return checks out of line.
The keyboard source calls `KeyChange(m_keyPress = m_keyPress | mask)` (and
the corresponding `&` expressions on keyup), then records the event. Pause
and F1..F4 return TRUE directly; Home returns TRUE; unhandled keys return
FALSE. The source also lets an unsuccessful CurrentRead reach a fresh
GAMEWRITE member test. These behaviors and the shared call tails follow
the actual target CFG; the old post-switch phase guard was not present.

**[EVIDENCE]** Continuing through the same function recovered these additional
source differences from the original instructions/mapper:

- WM_CHAR tests TreatTextInput's result and returns TRUE on success rather
  than forwarding an arbitrary nonzero BOOL value.
- WM_MOUSEMOVE has no ShowCursor call. Mouse-down and mouse-up include
  PLAYTEST as well as PLAY.
- The ChangeButtons dispatcher covers `0x4c8..0x4f9` (50 button IDs). The
  six DONAME/DOCLEAR/DOWRITE/DOREAD cases share that same source action.
- The leading phase actions are DEMO, DOPLAY, PRIVATE, then the general
  ChangePhase group. Successful PLAY/PRIVATE/previous/next phase requests
  normalize their result to TRUE; the INSERT return still forwards its BOOL.
- Previous/next multiplayer image reads use mission offset 100; NEXT clamps
  the multiplayer mission to 10. Its private and official mission limit is
  `m_bBuildOfficialMissions ? 220 : 20`; the private selection tests only
  m_bPrivate. Keeping the used private/official phase requests in their
  respective branches also recovers the target's duplicated clamp actions.
- INTRO1/INTRO2 are not handled by this dispatcher: their target mapper
  entries select the default FALSE exit. Removing those two labels fixes
  the final two table-byte differences (`+0x1b75`, `+0x1b76`).

**[EVIDENCE] Header delta.** Private `scratch/solsib/def.h` starts from the
canonical header and adds only WM_BUTTON40..WM_BUTTON49, with values
`WM_USER+240..249` (`0x4f0..0x4f9`). Evidence is the original outer range
test ending at `0x4f9` and its complete ChangeButtons dispatch interval.
No existing constant, member, offset, declaration, or canonical header was
changed. Private event.h remains identical to canonical event.h.

### Compiler findings: layout versus remaining register choices

| Trigger / controlled family | Flags, variation, measured effect | Rule / classification |
|---|---|---|
| Escape's missing 30-byte action | Complete event TU, VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy`. Escape suffix ladder, negative suffix, and explicit Escape label (3) are byte-inert; the whole Escape ladder adds unrelated blocks. Positive Return checks restore the independent action at `+0x9bb`. | VC5-CTL-01, BLUPI-REG-01; **SB1 layout evidence**, not a general no-merging guarantee. |
| Return layout with the target keyboard suffix | Eight controlled combinations of positive/negative first Return phase checks (`t3_shapes_00..07`). Shapes 00 and 05 restore the `+0xc17` dispatch destination; the other six do not. Shape 00 is retained. All eight leave the two scalar-register residues unchanged. Explicit Return joins and physically outlining its remaining checks also fail to improve them. | VC5-CTL-01; **positive source-shape transfer for layout, negative for these registers**. |
| Later loads, constants, and branch spans | Restored target behaviors listed above, phase action order, and shared ChangeButtons dispatch. Separate private/official actions reduce fixed differences 888 -> 620 -> 10; removing unsupported INTRO cases gives eight. The first fixed mismatch advances to `+0x41b` (1,051-byte prefix). | VC5-SEM-01 / VC5-CTL-01; **Blupi-specific reconstruction evidence**, no separate optimizer rule for each semantic correction. |
| Local identities in the recovered layout | Four valid near-match controls: reuse i for both life counts; reuse fwKeys for the NEXT limit; one used root j for life counts alone; one shared root j for life counts plus NEXT limit. All retain exactly the same eight register bytes (plus the two INTRO table bytes in that checkpoint). | BLUPI-REG-01, VC5-LOC-01; **negative transfer in this pattern**. |
| Byte/BOOL types and recording tail | In the eight-byte checkpoint, unsigned packet BYTE and DWORD bEnable are byte-inert. Replacing the shared recording goto with actual DemoRecEvent/TRUE returns in each mask arm is also byte-inert. Changing the signed cheat character to BYTE regresses the prefix to 48 and changes the frame/use graph. | BLUPI-REG-01 / VC5-SEM-01; **negative controls**, no type change retained. |

**[OPEN]** The residual bytes are `+0x41b`, `+0x421`, `+0x423`, `+0x426`,
`+0x428`, `+0x42e` (ECX input / EDX result instead of EDX input / ECX result)
and `+0x6f4`, `+0x6ff` (EAX HWND load/push instead of EDX). Identical block
layout has not made those physical-register choices match. The earlier
controlled shared-index SIB rule remains valid under its stated conditions;
it is not a universal register-allocation predictor.

## Post-restart verification and remaining scalar registers

**[EVIDENCE]** After the desktop-heap host failure, checked the surviving
sources, refreshed scratch event.cpp/event.h/def.h from canonical, and reapplied
only the measured TreatEventBase body. Latest event.cpp commit was still
`34fb42e`. Both `r4_rebased` and the final `r4_final` verification reproduce
the eight-byte near match above. The working `scratch/solsib/event.cpp` is now
restored and verified; the earlier restoration uncertainty is resolved.
Its SHA-256 is `f44f94cae8252d1326b9adc7c3291655fc3b90dd518902335c26a08aa2d9ad7f`.
Only TreatEventBase differs from current canonical source, event.h is identical,
and def.h has only the ten added button macros already documented.

**[EVIDENCE]** `round4_audit.json` records the final 69 STRICT / 1
RELOC-UNVERIFIED / 39 MISMATCH event results. ChangeButtons stays STRICT at
`0x1b220`, 2,854 original instruction bytes. No new STRICT function or unique
byte coverage is claimed. TreatEventBase still has 273 relocations requiring
strict proof; eight equal-address/register encodings cannot be wildcarded
into a match. The comparison covers all 7,072 section bytes, including the
456 table/alignment bytes following the 6,616 instruction bytes.

**[EVIDENCE]** Decoded all 16 keyboard cases again: every entry equals the
target, including Home/default TRUE at `+0x1820`. The separate Escape GAMER
action is `+0x9bb`, STOP is `+0x9d9`, Return is `+0x9f7`, and the dispatch
instruction is exactly `0f 87 39 09 00 00` at `+0x2d8`. The remaining first
fixed difference is `+0x41b`, a register field in the instruction at `+0x41a`.

### Compiler findings: completed interrupted controls

All nine valid probes below used the complete TU and pinned SP3 flags with
`verify.py --build-dir solsib`. The worker helper sets `BLUPI_JOBS=4` and
awaits each verification before starting another. No permuter campaign was run.

| Trigger / actual source variation | Count and measured effect | Rule / classification |
|---|---|---|
| Toggle input local identity | Assign the original member input to existing bEnable or existing i inside the condition before toggling/storing it. | 2; byte-inert, same eight residues. BLUPI-REG-01 / VC5-LOC-01, **negative**. |
| Escape/Return phase identity | Cache phase separately in the two cases versus one used shared root phase; preserve the fresh GAMEWRITE member read after CurrentRead. | 2; byte-inert, same eight residues. BLUPI-REG-01, **negative**. |
| Cheat rank identity | Cache the rank used by the first two cheat conditions in existing i versus a distinct used rank local. | 2; byte-inert, same eight residues. BLUPI-REG-01 / VC5-LOC-01, **negative**. |
| Return-only phase cache | Complete the previously unrun cached-Return probe from its older ten-byte checkpoint. | 1; preserves its eight register residues and its two pre-existing INTRO mapper differences. No transfer to the scalar-register sites. VC5-LOC-01, **negative**. |
| Toggle arm ordering / result expression | Negative condition with swapped TRUE/FALSE arms versus negative ternary assignment followed by member reload. | 2; swapped arms are byte-inert. The ternary moves the ESI copy ahead of the member store, raising fixed differences 8 -> 14, while retaining the wrong register roles. VC5-ORD-01 / VC5-CTL-01, **negative**. |

**[OPEN]** The two scalar-register choices remain unexplained. All instruction
sizes, branch spans, switch-case entries, and non-relocated bytes outside those
two instructions/sequences now agree. More equivalent access spellings or
repeating already negative count/compiler sweeps has diminishing returns.
The next useful input is a new source-identity or c2 operand-order hypothesis
with a controlled positive example for these scalar loads, rather than treating
the confirmed shared-index SIB rule as a universal allocation rule. All worker
verification commands have completed; no background campaign is left running.

## Canonical refresh, header context, and DrawButtons continuation

**[EVIDENCE]** Refreshed from src after `010763d` (BlupiStep/layout recovery),
including the integrated ChangeButtons and TreatEventBase bodies. The frozen
starting files are `scratch/solsib/ctx_base/`; `runs/ctx_canonical/` records
the baseline. The final candidate is `scratch/solsib/event.cpp`, with private
headers beside it and results in `build/verify-solsib/results.json` and
`scratch/solsib/runs/solsib_final/`. Its SHA-256 is
`e74d30addb493ae23ad06aaa78b9af64ac76c91fad070d59629a15eb1c0e31ad`.
Only DrawButtons differs from that canonical cpp; all private headers except
event.h are restored byte-for-byte from the refresh. Source outside DrawButtons,
including its original newline bytes, is preserved.

**[EVIDENCE]** Every compile in this continuation used verify.py -> msvc.py,
the pinned SP3 `/W3 /GX /O2 /ML /Gd /Gy` context, manifest defines, and
`--build-dir solsib`. Shell commands and candidate verifications ran serially;
BLUPI_JOBS and BLUPI_COMPILE_SLOTS were left at their defaults. No direct cl.exe
invocation or permuter campaign was used. No process is left running.

| Function | Final result |
|---|---|
| ChangeButtons, `0x1b220` | Still STRICT: 2,854 original code bytes, 2,864-byte section, 83 proved relocations; already integrated, no new coverage. |
| TreatEventBase, `0x1d230` | Still MISMATCH: 6,616-byte body, 7,072-byte section including tables/alignment, 273 relocations; exactly the same eight fixed-byte differences at `+0x41b/+0x421/+0x423/+0x426/+0x428/+0x42e/+0x6f4/+0x6ff`. The final raw function-section bytes equal the refresh baseline. Internal static/label symbol numbers changed after editing the earlier DrawButtons body; this is not strict relocation proof. |
| DrawButtons, `0x16f70` | Still MISMATCH: body 7,386 -> target 7,419 bytes; section 7,392 -> 7,424 bytes (five trailing NOPs). Fixed differences 1,525 -> 78; prefix 4,886 -> 6,599 (`+0x19c7`). All residual fixed bytes are between `+0x19c7` and `+0x1ab7`; the epilogue ends at the target `+0x1cfb`. There are 234 relocations, not yet all proved. |

**[EVIDENCE]** Final event counts remain 69 STRICT / 1 RELOC-UNVERIFIED /
39 MISMATCH, with no baseline STRICT symbol lost. The full verification gate
exits zero. `final_audit.json` lists exact remaining offsets and status counts;
`final_event.diff` and `final_header.diff` are reviewable source deltas.
The small register-insensitive disassembly alignment and relocation-masked
progress counts are diagnostics only. A separate same-offset comparison of
known mapped call targets finds no incorrect mapped call in final DrawButtons;
this does not establish proof for all 234 relocations.

### Header delta and offset evidence

**[EVIDENCE] Retained private event.h only.** Replace the reconstruction's
CEventNameRow wrapper (four-byte prefix, 96-byte text, conversion operator)
with ordinary `char m_names[10][100]`, enlarge `m_name[96]` to `m_name[100]`,
and remove `int m_900`. No constants or method declarations change.

| Member | Refreshed canonical layout | Private recovered layout / oracle evidence |
|---|---|---|
| m_name | Starts `0x490`, 96 bytes | Starts `0x490`, 100 bytes. |
| m_names | Wrapper array starts `0x4f0`; each text starts four bytes later | Plain row zero starts `0x4f4`, stride 100, preserving every row text address in DrawButtons. |
| m_bNamesExist | Starts `0x8d8` | Starts `0x8dc`. Target DrawButtons `+0x1af3` reads `[this + fileIndex*4 + 0x8dc]`; the same canonical discrepancy was independently recorded for ReadAll in docs/SWEEP_SWEEPA.md. |
| m_900 / m_filenameBuffer | Unused scalar at `0x900`; filenames at `0x904` | Scalar removed; the longer presence array ends at `0x904`, so filenames and all subsequent members retain their offsets. |

**[EVIDENCE]** The GAMER clear-button access uses the one-based gamer number:
target `[this + gamer*4 + 0x8d8]` is preserved by changing its source index to
`m_bNamesExist[m_gamer - 1]` under the recovered `0x8dc` base. The save-slot
access remains `m_bNamesExist[m_fileIndex]`. Changing the header without that
gamer correction exposes an earlier `+0x24c` field-offset mismatch. This is a
real layout/source correction (VC5-SEM-01), not a context adjustment. The target
proves addresses and stride, not the unique original declaration spelling.
No private header deletion/union experiment below is retained. The verify gate
compiles this event candidate against private headers and other TUs against
canonical headers; shared-header integration still requires the supervisor's
whole-tree check. ReadAll is not claimed STRICT.

### DrawButtons source recovery

**[EVIDENCE]** Target disassembly supports the retained read-design loop's
six-row head guard, selected font, x=240/y=122 setup, and absence of an input
box there. The write-design block uses heading y=31, white content text,
the text buffer rather than res, ellipsis at text+32, DrawTextLeft, and the
input box at (320,333) after that block. Target `+0x1826` addresses text+32;
the earlier trial using +26 was corrected. Save/load draws exactly six slots,
uses text for the number and first title line, draws the original filename
when no newline is present, and places the sheet-8 icon at (33,149). The
BYE strings are white and use `LXIMAGE/2 - lg/2` centering. These are grouped
VC5-SEM-01/VC5-CTL-01 reconstruction corrections, not separate optimizer rules.

### Compiler findings: header negatives and point live ranges

All controlled variants below use the full event TU and flags above. Candidate
names identify snapshots and logs under scratch/solsib. No new STRICT result.

| Trigger / original versus candidate | Exact controls and measured code effect | Rule / classification |
|---|---|---|
| TreatEventBase's eight register bytes; suspect unused header declarations | Ten probes: remove NetPacket; remove four unused NetMessage aliases; remove GetDemo; remove the const export-path overload; remove network callback prototypes; remove DescFile+DescGameFile; combine declaration removals; omit redundant explicit network include; recover flat name rows; combine flat rows with cleanup. Every probe retains all eight TreatEventBase bytes. The first eight leave DrawButtons' prefix 4,886; the two uncorrected flat-row probes expose the separate gamer-offset issue at 588. | BLUPI-CTX-02 / VC5-CTX-01, **negative for these residues**, not a refutation of the proven BlupiStep context case. |
| Anonymous-union type presence rather than scalar count | Five further probes: flatten each of NetMessage's three anonymous unions separately; flatten all four NetMessage/NetPacket unions to their first short member; retain only the first member inside each union. Same sizes and residue sites: TreatEventBase eight, DrawButtons 97 in the then-current checkpoint. All five pass the existing acceptance gate. They are private context probes; aliases needed by other TUs are restored. | BLUPI-CTX-02, **negative under these exact header controls**. |
| Session highlight member load hoisted before the inline ellipsis copy at `+0x1316` | Before: `pos.y=122`, construct POINT(170,pos.y) at each draw. After: `pos.x=170; pos.y=122` before the loop and pass existing pos at the draw. On the corrected later-body checkpoint this fixes the complete `+0x1316..+0x1375` sequence: xor EDX at +1316, text LEA before copy tail, page/index loads after the copy, and call arguments in target registers. Prefix 4,886 -> 6,599, differences 144 -> 97, size unchanged. Counter-example: assigning pos.x=170 inside the loop immediately before the draw was inert in the earlier draw_pos control. | VC5-LOC-01 / VC5-ORD-01, **Blupi-specific positive point-lifetime evidence**. It does not prove a universal register preference or SIB algorithm. |
| Multiplayer x reloaded as an immediate inside the player loop | Replace separate `int x=130, y=110` and POINT(x,y) with existing pos.x/pos.y initialized before the loop and pass pos. Target keeps x=130 in EBX across the loop. Differences 180 -> 144 at unchanged size and first prefix. | VC5-LOC-01, **Blupi-specific positive aggregate/local identity evidence**. |
| Save-title x=330 kept in ESI across two calls; five-byte body shortfall | Add the ordinary second-line `pos.x=330` assignment after incrementing y, before the second DrawTextLeft. This recovers the target's second immediate x load, restores body size 7,414 -> 7,419, and reduces differences 696 -> 180 at that checkpoint. | VC5-ORD-01 / VC5-LOC-01, **Blupi-specific positive source-order evidence**. |
| Remaining save-title address arithmetic and register choices | Five order/identity controls: y-before-x, termination after position assignments, root line2, int cast of pointer difference, block length local. First/root/cast retain 97; moving termination gives 108; length local gives 78. Five further controls: shared lg length, root length, end pointer, shared bEnable selection, shared lg selection: four give 78, end pointer 102. Five expression controls: selected-font arithmetic gives 78, number BOOL alone 97, both 78, integer-pointer and long-pointer casts both 97. The final retains selected-font arithmetic; pointer casts and extra locals are discarded. | BLUPI-REG-01 / VC5-LOC-01 / VC5-ORD-01, **mixed narrow evidence**, no general allocation rule. |
| Branch-based font assignments / loop continuation / unused gauge arrays | Two valid if-based font variants (separate used locals versus shared lg) regress prefix to 452 and section to 7,440. One initial separate-local source failed compilation through a duplicate declaration; corrected before measuring. A no-newline continue form retains 78. Removing levels[2], types[2], or both (three controls) retains 78. None is retained. | VC5-CTL-01 / BLUPI-CTX-02, **negative**. |

**[OPEN]** TreatEventBase still needs a new explanation for the EDX/ECX toggle
and EDX HWND load. The tested unused header/type/union families do not select
those registers. DrawButtons now has only a two-byte ECX/EAX choice in the
single-line arm (`+0x19c7/+0x19cf`) and the `+0x1a3d..+0x1ab7` arithmetic,
scheduling, and register family in its split-title arm. Further equivalent
spellings and already-tested declaration-count sweeps have diminishing returns.
The next useful experiment is a reduced positive control for that pointer-
difference/zero-store/conditional-font graph, while preserving the recovered
point lifetimes and checking shared-header integration separately.

## Integration delivery and pointer/font controls

**[EVIDENCE] Deliverable:**
`scratch/solsib/integration/integration.patch` (7,639 bytes, SHA-256
`de3d28984a92063fa59db932a2df2a668c7e708c27952c5c96ebf9ad0063adc5`).
It changes only src/event.cpp and src/event.h. Both `git apply --check` against
the canonical working tree and application to a clean scratch copy succeed;
the applied files equal the compiled candidate byte-for-byte. Their line
endings are CRLF. No canonical file, manifest, catalog, or tool was edited by
this worker. The working scratch event.cpp and headers now equal the delivered
copies; event.cpp SHA-256 is
`9f9b23a209b432712b0996902dea7c2dc469da2f11bd8a0b94d3f23c5c601692`.

**[EVIDENCE] Whole-tree verification:** The final base is canonical commit
`e11d936e120941018fb7c57bfd9e3db957803af6`; frozen recon.json SHA-256 is
`a0b938d22ed7f88baf09f2cbdbdf526e76cace44ac41244981b8d5087c10ec47`.
All src files were copied to `scratch/solsib/integration/src/`, and all 20
manifest TUs were substituted into one verify.py run with `--build-dir solsib`.
Quoted includes resolve to this one copied header set for every TU. The gate
exits zero: all 330 active accepted STRICT entries and three accepted FOLDED
entries retain status, RVA, size, and folded owner. The full comparator reports
335 STRICT, 3 FOLDED, 6 RELOC-UNVERIFIED, and 407 MISMATCH records. The five
other STRICT records are not new worker gains; no unique-byte coverage is added.
Event remains 69 STRICT / 1 RELOC-UNVERIFIED / 39 MISMATCH.

The full command is in `integration/verify_command.json`, and the snapshot,
per-file hashes, gate results, object files, raw comparator results, and log
are in `integration/base/`, `verification_summary.json`, `accepted_gate.json`,
`objects/`, `results.json`, and `verify.log`. `runs/whole_tree_final/` also
preserves the final event object/source/headers. The former pre-normalization
whole-tree success is archived under `integration/history/pre_normalization/`.
The shared verifier briefly had an invalid new line-ending warning; its owner
fixed it in 886b97d. The final gate uses that repaired verifier. A lone inherited
LF at the DrawButtons/method boundary was normalized without changing logical
line counts before the final build and patch check.

**[EVIDENCE] Handler scope:** The full delivered event.cpp contains both
DrawButtons and TreatEventBase. TreatEventBase is already identical to the
canonical source body integrated in f778a36, as checked in
`integration/body_state.json`; it therefore needs no additional patch hunk.
It remains MISMATCH at 0x1d230, 6,616 original instruction bytes / 7,072-byte
section, with eight fixed differences and 273 relocations. DrawButtons remains
MISMATCH at 0x16f70, 7,419 original instruction bytes / 7,424-byte section,
with 78 fixed differences and 234 relocations. ChangeButtons remains STRICT
at 0x1b220, 2,854 original instruction bytes, 83 proved relocations. Its coverage
was already integrated and is not counted again.

### Why the previously integrated event.h layout is revised

**[EVIDENCE]** The previous 96-byte m_name plus CEventNameRow wrapper preserved
the observed first name string at 0x4f4 and the 100-byte row stride, but put the
presence array at 0x8d8. The GAMER clear-button access alone appeared consistent
with that base because its source used the one-based m_gamer without subtracting
one. Two independent zero-based save-slot accesses contradict that inference:

| Oracle instruction | Proven address |
|---|---|
| DrawButtons `+0x214`, `8d bd f4 04 00 00` | Name row zero starts at this+0x4f4; the loop advances by 100. |
| DrawButtons `+0x249`, `8b 84 95 d8 08 00 00` | Presence at this+0x8d8+4*gamer, equivalent to this+0x8dc+4*(gamer-1). |
| DrawButtons `+0x1af3`, `8b 8c 85 dc 08 00 00` | Presence at this+0x8dc+4*fileIndex. |
| ReadAll `+0x10` at RVA 0x1aec0, `8b 8c 86 dc 08 00 00` | The same zero-based presence base 0x8dc, independently of DrawButtons' register residues. |

`integration/layout_evidence.txt` contains these disassemblies. Growing m_name
from 96 to 100 shifts the following array once, to 0x4f4. Plain 100-byte name
rows preserve all row-text addresses and stride while moving their end to
0x8dc. Ten BOOLs then end at 0x904, so the invented m_900 gap is unnecessary;
the filename buffer and every later member retain their offsets. The GAMER
source index becomes m_gamer-1. No constants or method declarations change.
The read-only SB2 header independently has m_name[100]/m_names[10][100], which
supports the declaration form as ancestry, not as proof of SB1 layout. Its
different filename dimensions are not transferred; SB1 keeps [256][100].
The new whole-tree gate proves that this revision preserves the currently
accepted code even in the other TUs that include event.h.

### Compiler findings: controlled pointer/font continuation

**[EVIDENCE] Reduced experiment:** `pointer_micro.py` emits eight independent
C++ files under scratch/solsib/micro: baseline plus unsigned pointer difference,
reversed subtraction, one shared row pointer, negative-condition font ternary,
reversed-operand font ternary, a length local, and a pixmap local. Each uses a
six-row loop, strcpy, a newline pointer difference/zero store, and three drawing
calls, with only ordinary scalars/arrays and no project headers or padding.
`pointer_return_micro.py` adds five independent copy-return/alias controls.
All 13 files use msvc.py with SP3 `/W3 /GX /O2 /ML /Gd /Gy`, default compile
slots and repository TMP/TEMP. Results/disassemblies are in the corresponding
micro .json/.obj/.log files and pointer_micro_results.json/pointer_return_results.json.

| Trigger / controlled family | Count and measured effect | Rule / classification |
|---|---|---|
| Pointer subtraction signedness, polarity, and local values | Reduced baseline section is 368 bytes. Unsigned difference, reversed subtraction, separate length local, and explicit pixmap local have identical raw function bytes. A shared row pointer shortens the section to 352 and removes redundant base arithmetic. Negative font ternaries generate SUB/NEG/SBB/AND/add rather than the target SETNE/DEC/AND; reversing operands also changes frame/register allocation. | VC5-SEM-01 / VC5-LOC-01 / BLUPI-REG-01; **unverified SB1 transfer**, measured micro evidence rather than oracle matches. |
| Copy return/alias identity | Five reduced controls: index the strcpy return directly; store its return in dest; also draw from dest; alias text in dest for the store; also draw from that alias. Direct return indexing emits a 352-byte section; the other four emit 368 but change raw bytes/register scheduling. Both plain-alias controls emit identical bytes to each other. Return values and aliases are therefore not universally interchangeable for VC5 allocation, even though their addresses agree. | VC5-LOC-01 / VC5-ORD-01; **unverified generalization, positive micro evidence**, no universal base/index prediction. |
| Apply reduced hypotheses to the full event TU | One shared-row candidate shrinks the section to 7,392 and regresses prefix to 2. Unsigned subtraction retains 78 differences. Three precomputed-font variants (new shared font, existing lg, existing bEnable) also retain 78. Five copy-return/alias candidates regress: sections 7,408 or 7,424, prefixes 2 or 6,310, 629..755 differences. None is retained. | BLUPI-REG-01 / VC5-LOC-01; **negative transfer to this target**. |
| Newline pointer value flow | Four valid controls advance line2 before the call, before y, before the first split draw, or in the final draw argument; all retain 78. One initially generated early-increment candidate accidentally omitted the increment because its replacement assumed CRCRLF. That semantically invalid result is discarded; the corrected source was rerun and retains 78. | VC5-LOC-01 / VC5-ORD-01; **negative**, with the invalid control excluded. |
| Independent statement order | Nine additional permutations of copy C, termination T, x assignment X, and y adjustment Y, constrained to C before T: CXTY, CYTX, CYXT, XCTY, XCYT, XYCT, YCTX, YCXT, YXCT. None improves 78: fixed differences 87..662; sections remain 7,424; prefixes 6,599 or 6,310. A shared x=330 assignment above the two branches also regresses to prefix 6,310 / 718 differences. Computing a length local before strcpy retains 78. | VC5-ORD-01; **negative for these exact orderings**. |
| Font-expression identity across the three selected-title calls | Test all six mixed combinations between the previous all-ternary and all-arithmetic controls. Bits 0/1/2 select arithmetic at single-line/first split/second split calls. Masks 1..6 all emit the same 78-difference family; all arithmetic is also 78, all ternary was 97. This narrows the 19-byte improvement to the presence of an arithmetic comparison in this graph, but does not explain the target's remaining choices. | VC5-CTL-01 / BLUPI-REG-01; **blupi-specific source-form evidence, negative closure**. |
| Induction local identity and origin | Four full-TU controls: a new block-local game index; a distinct function-root index; reuse existing lg; one-based loop with matching row/selection adjustment. The first three retain 78. One-based control regresses to prefix 452 / 7,440-byte section / 974 differences. No local-identity form closes the remaining bytes. | VC5-LOC-01 / BLUPI-REG-01; **negative transfer from the shared-index SIB rule**. |

**[OPEN]** No new STRICT body was recovered. The additional 35 valid full-TU
controls and 13 reduced sources have diminishing returns for the current
hypothesis family. The target still uses EAX instead of ECX for the single-line
selected-index load, and differs in reassociation, scheduling, and register
choice in the split-title block. A new source/IR/liveness clue is needed before
more equivalent-expression sweeps. The integration patch deliberately preserves
the best 78-byte checkpoint and the already integrated eight-byte TreatEventBase
checkpoint. All verification commands and compile campaigns have completed;
no background process remains.
