# Event SIB investigation (sol-sib)

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

## Escape action follow-up

**[EVIDENCE]** `runs/t3_no_intro` is the new best checkpoint: 7,072-byte COFF
section (6,616 instruction bytes plus 456 switch-table/alignment bytes), 273
relocations, exactly eight non-relocated byte differences at RVA `0x1d230`.
It remains MISMATCH. Escape owns its separate GAMER action at `+0x9bb`, STOP
at `+0x9d9`, Return starts at `+0x9f7`, and the dispatch jump now reaches the
exact target `+0xc17`. All branch spans and complete switch data match.
Remaining bytes: `41b,421,423,426,428,42e` (ECX/EDX toggle roles), `6f4,6ff`
(EAX/EDX HWND load/push). Fixed prefix: 1,051 bytes; no new STRICT function.

**[EVIDENCE]** The fix combines positive Return phase checks and shared
PLAYTEST action with the target keyboard behavior: assignment inside each
KeyChange call, direct TRUE returns for Pause/F1..F4/Home, default FALSE,
and shared event recording. Failed CurrentRead reaches a fresh GAMEWRITE
test. The rest of Return is then placed at target `+0x1870`.

**[EVIDENCE]** Other target-backed corrections: WM_CHAR normalizes BOOL;
WM_MOUSEMOVE omits ShowCursor; mouse down/up include PLAYTEST; 50 button
IDs share ChangeButtons with DONAME/DOCLEAR/DOWRITE/DOREAD; phase action order
is DEMO, DOPLAY, PRIVATE, general ChangePhase. PLAY/PRIVATE/PREV/NEXT phase
requests normalize BOOL (INSERT still forwards it). Multiplayer reads use
offset 100 and NEXT clamps to 10; NEXT's mission limit is build-enabled
220, otherwise 20, with selection based only on m_bPrivate. Separate used
private/official phase actions recover the target clamp blocks. Unsupported
INTRO1/INTRO2 labels accounted for the last two switch-map differences.

**[EVIDENCE] Header delta:** private def.h starts from canonical and adds only
WM_BUTTON40..49 = WM_USER+240..249 (`0x4f0..0x4f9`), proved by the original
dispatch interval `0x4c8..0x4f9` ending at its range test. No existing header
entry changed; event.h remains canonical. Only TreatEventBase changed in cpp.

### Compiler findings: recovered layout

**[EVIDENCE]** Same full TU / VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy` throughout.
Escape suffix ladder, negative suffix, explicit label (3) are byte-inert;
whole ladder adds unrelated blocks. Positive Return restores Escape's
30-byte action; the keyboard call-expression form is also required for
the complete layout. Eight Return polarity combinations (`t3_shapes_00..07`)
give exact `+0xc17` in shapes 00/05; none fixes the early registers. Explicit
Return joins/physical outlining do not fix them either. VC5-CTL-01 /
BLUPI-REG-01: positive layout transfer, negative scalar-register transfer.

**[EVIDENCE]** Later semantic/order corrections reduce fixed differences
888 -> 620 -> 10; removing INTRO gives eight. VC5-SEM-01 / VC5-CTL-01,
Blupi-specific reconstruction evidence. Four used-local controls (life counts
in i, NEXT bound in fwKeys, root j for lives, shared root j for lives/bound)
are inert in this recovered layout. Packet BYTE, DWORD bEnable, and duplicated
DemoRecEvent/TRUE instead of the recording goto are also inert. BYTE cheat
character regresses prefix to 48. BLUPI-REG-01 / VC5-LOC-01: negative here.

**[OPEN]** Exact layout does not determine the two remaining register choices.
The controlled shared-index SIB rule above remains valid only with its
stated conditions/counterexamples; it is not a universal allocation rule.

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
