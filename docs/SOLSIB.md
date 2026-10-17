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

Only `scratch/solsib/event.h` differs from canonical headers:

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
The next integration step is the two ABI declarations and the focused
ChangeButtons body from this candidate, followed by canonical verification.
