# Round 3: POINT arguments and constant definitions

[EVIDENCE] STATUS: PARTIAL. No normal C++ source improvement. Eighteen
preregistered controls in three serial micro translation units produced no
pair of late x definitions. None justified a new full-function source trial.
The delivered candidate is unchanged: 63 fnprobe differences, 2288/2304
aligned instructions, eight regions. The ordinary regression gate exits 0;
sound.cpp has the expected historical-path INCOMPLETE gate.

## Prototype and oracle

[EVIDENCE] text.h declares:

```cpp
void DrawTextLeft(CPixmap *pPixmap, POINT pos, char *pText, int font);
```

The split calls actually target oracle VA **0x423740**, not 0x40c7d0.
Static disassembly of the wrapper at 0x423740 reads the five incoming
dwords in the corresponding layout and forwards them to 0x423770:

| Entry stack offset | Value |
|---:|---|
| +4 | pixmap |
| +8 | POINT.x |
| +12 | POINT.y |
| +16 | text |
| +20 | font |

[EVIDENCE] There is no prototype argument-order discrepancy to repair.
Changing the public prototype to scalar coordinates would also change C++
symbols and can allow immediate pushes; the supervisor's scalar-declaration
negative is not a source solution. No prototype or header was edited.

## Three different meanings of scalarization

[EVIDENCE] These are separate stages, not one transformation:

1. **Field descriptors during C2 IL reading.** 425d0f handles typed
   references; 40398b interns subranges by type, size and offset. The first
   actual pos field accesses at relative lines 55/56 already obtain parent
   descriptor 4 (size 8, type 0x5008), x descriptor 5 (offset 0, size 4),
   and y descriptor 6 (offset 4, size 4). This is C2's interpretation of
   the emitted C1 IL, not proof that C1 allocated two register variables.
2. **Outgoing aggregate expansion during machine lowering.** At 412d5c
   entry, each POINT argument remains an op 0x13e, type 0x5008, reading
   the whole parent. 41350c -> 4135da -> 44f44f expands the eight-byte
   argument. Its 0x13e case uses 45b7db, and the resulting wide 0x130
   uses the wide-argument path in 44f44f. 44fcc0 produces the high
   subrange, offset +4. The result is y-copy/push, x-copy/push. At these
   sites, outgoing space is filled by pushes, not by a retained memory
   POINT copy. The x-copy temps are t329 and t299 in the baseline capture.
3. **Global live ranges and register promotion.** 414a64/416a8d build the
   LR descriptors from the lowered uses. The two source-field intervals
   become LR142 and LR150, both originating from descriptor 5. Their
   definition/use pairs are the source assignment and x-argument read.
   Global allocation later chooses eax for each. These LR numbers are not
   C1 temp numbers or POINT field descriptor numbers.

[EVIDENCE] Read-only snapshots are in base.pipeline.json, base.reader.json
and base.coalescing.json. The full Round 2 colour/assignment evidence remains
in ../round2/. Every final replay reproduces every native function payload.

## Why the argument comma expression failed

[EVIDENCE] C1 already serializes the argument expression records in the
expected font, text, POINT, pixmap order. In the earlier argument_x candidate,
the decoded records for relative line 639 include:

```text
this + 0x6d0c; i; comparison; ternary; 55(type=0x1004)   font argument
text; pointer conversion; 55(type=0x3004)              text argument
pos + 0; literal 330; 32; pos; 30(type=0x5008); 44;
55(type=0x5008, size=8)                               POINT comma argument
this + 0x6d54; pointer read; 55(type=0x3004)           pixmap argument
```

[EVIDENCE] 55 is processed by 435338 to form an argument node. The 32/44
records describe the real assignment/comma expression. These are IL records,
not instructions or already executed pushes. 434e08 assembles the call,
425acb/4262d8 assemble its expression/statement chains. By the first
post-reader snapshot at 42491e, the pos.x assignment is ahead of the font
comparison and text argument nodes. No scheduler or late allocator pass
caused that reordering. It is visible before the optimization pipeline.

[EVIDENCE] argument_x.reader.json and argument_x.pipeline.json re-observe
the existing Round 2 source; they are not a repeated source compile. This
refines the former broad "hoists/normalizes" explanation: changing lexical
argument position did change C1's record position, but did not change the
assembled C2 definition interval. The const_positions replay likewise shows
ordinary early _firstPos/_secondPos field definitions, not argument literals.

[EVIDENCE / tracer format] Only opcode-specific fields in the decoded
eight-dword reader buffer are valid. Unwritten fields retain stale data.
reader.py names reference symbols from +0x18 and decodes line-marker data;
it does not interpret every raw dword as a fresh IL field.

## Copy deletion and literal rematerialization

[EVIDENCE] The early field definitions and the late argument-copy temps
coexist before global allocation:

```text
L638: pos.x[LR142] = constant-pool-330[LR168]
L640: ... font push; text push; y copy/push;
      t329 = pos.x[LR142]; push t329
L642: pos.x[LR150] = constant-pool-330[LR168]
L643: ... font push; text push; y copy/push;
      t299 = pos.x[LR150]; push t299
```

[EVIDENCE] The copy simplifiers 41c72e and 41ca9b leave these x definitions
and argument copies intact. 41c72e's global-copy candidate predicate excludes
source descriptor kind 13 (the constant pool); the local x assignments are
not a generic constant-sinking case. 41cd63's rematerializable-expression
predicate recognizes selected LEA address forms, not arbitrary MOV literals.

[EVIDENCE] During global allocation, the copy-fusion routine **443552**
deletes t329/t299 (and the analogous y-copy temps) through **443889**.
The late argument read is redirected to the earlier source-field value.
That is the precise point at which the tempting late copy disappears.
It does not move the source's constant definition to that copy's site.

[EVIDENCE] The literal rematerializer **40ec8b**, called from **438c18**,
replaces the LR168 operand in the existing L638/L642 assignments with literal
330. It preserves their tuple positions. After 42aadd installs physical
bindings, those same tuples are `eax = 330`, still before the font tuples.
Rematerializing the *constant-pool operand* is not rematerializing the entire
source-field interval at its last read.

[EVIDENCE / correction] The final coalescing tracer follows stock
colour_trace: the third argument to 40ec8b is the original descriptor;
the LR node comes from the operand's +0x18 binding. An initial instrumentation
version confused these and printed LR0. It was corrected and replayed;
LR0 is not used as evidence. Final x rematerializations report LR168.

## Bounded controls and transfer decisions

[EVIDENCE] See LEDGER.md and ledger.json for every prediction, source,
capture, packaged size, comparison to plain and observed IR result. The
controls cover nonescaping pointer/reference aliases, a real address escape,
an escape on another arm, whole copies after font computation, array POINTs,
argument copies, one x value spanning two draws, constant aggregate/local
origin/static origin initialization, a late constant-origin copy, register
storage class, scalar font inlining, POINT inlining, their combination, and
the real x assignment in a POINT comma argument with a scalar inline call.

[EVIDENCE] Pointer/reference aliases, constant initialization with y
overwritten, register POINT, scalar font helper, and scalar helper plus
POINT comma all have the plain native micro payload. Escapes add actual
field memory traffic. A static const origin produces memory reads and
relocations. Forwarded copies and the one-reset form keep early x values,
often across the first call in a saved register. Both inline helpers still
prepare the POINT early. No control has the required two definition sites.

[EVIDENCE / limit] These are bounded micro negatives with matching compiler
and flags, not proof of full-function equivalence for every context. No
new full target variant was compiled after its micro failed the required
IR condition. The supervisor's POINT-helper fields/aggregate tests at both
calls or only the second (baseline payload), and font/line2 locals before
the coordinate statements (93/130 differences), are inherited negatives;
they were not repeated as target trials.

[OPEN] No qualifying source construct has been found. The remaining useful
question is how a real source expression can make the outgoing x read use
a rematerializable value without first creating the ordinary early field
interval which 443552 preserves. Merely giving the POINT another name,
scope, nonescaping alias, helper return, constant initializer or copied
origin did not supply that condition in these controls. Adding unrelated
uses, fake calls, forced registers or fabricated byte-oriented constructs
is not a justified next experiment.

## Reproduce

```text
.pyenv\python.exe scratch/sol-sched/round3/micros.py
.pyenv\python.exe scratch/sol-sched/round3/constants.py
.pyenv\python.exe scratch/sol-sched/round3/inline_args.py
.pyenv\python.exe scratch/sol-sched/round3/pipeline.py CAP OUT.json
.pyenv\python.exe scratch/sol-sched/round3/reader.py CAP OUT.json
.pyenv\python.exe scratch/sol-sched/round3/coalescing.py CAP OUT.json
.pyenv\python.exe scratch/sol-sched/round3/finish.py
```

[EVIDENCE] Run serially. Captures/replays use unique private directories.
The three controls compile only through tools/msvc.py; the final normal
candidate uses fnprobe and verify.py --build-dir sol-sched. All c2 replays
use colour_trace.owned_run and explicitly await/reap the child. No compiler
IR was perturbed in Round 3. No canonical source, header, tool, catalog,
asset, external reference or toolchain file was modified; no game execution,
commit or background process was introduced.
