# VC5 SP3 front-end IL emission study

Worker sol-c1. Toolchain analysis only; no diagnostic alteration is a source match.

## Scope and stage distinction

[EVIDENCE] C1XX.DLL's SP3 preferred image base is `0x10400000`.
The front end emits a postfix expression stream, not the final scalar tuple
sequence seen by the temp-register allocator. C2's decoded-record entry is
`0x425587`; the assembled post-reader snapshot is at `0x42491e`.
The source-level IL and post-reader/pre-allocation tuples must be reported
separately. In particular, an op-1 copy in late C2 IR is not automatically an
op-1 C1 record.

[EVIDENCE, inherited] `scratch/sol-sched/round3/LOWERING.md` shows a real
`pos.x=330` comma operand serialized after font/text argument records, then
assembled before those arguments by C2. Therefore B cannot be diagnosed
only by inspecting C1 record order. Its opcode-0x55 argument handling and
side-effect-chain assembly matter as well.

[EVIDENCE] C1XX.DLL SHA-256:
`e27df3bb9a37058c85dad3fb7d1ed30b9e52d38fb6c0f66c8fdba0a11111d3d9`.
All addresses below are SP3 component VAs, not game RVAs.

## AST builders and serialization

[EVIDENCE] Unary-node allocator `10410187` reserves 24 bytes: AST operator
at +0, arity 1 at +1, type at +4, child at +10. Binary allocator `104065c0`
sets arity 2, left at +10 and right at +14. Offsets here are hexadecimal.
The low three bits of byte +1 describe arity; higher bits have other uses.

[EVIDENCE] Expression emitter `104048fb`, ordinary path `10404921`, emits
left, then right if present, then maps the AST operator through the dword
table `104cf390` and calls `10403296` (`1040493a..10404947`). The writer
appends the record opcode and serializes opcode-specific payloads using
the template table `104cf86c`. Buffer object: `104c0360`; cursor: `104c0364`.
`10403982` flushes the buffer with fwrite (`104039ce`).

[EVIDENCE] A hardware cursor watch found the writer. Software observations
then captured opcode, AST node/type and child trees at its entry. Evidence:
`emit-watch-c1.json`, `emit-ast-c1.json`, `emit-ast-summary.json`,
`boundaries-ast-c1.json`. Independent native payload comparisons pass
(`payload-audit.json`); no emitter decision was changed.

| C1 AST op | Record | Meaning in these controls |
|---:|---:|---|
| `3c` | `1a` | logical not |
| `3d` | `30` | typed value read; some contexts emit `41` |
| `64` | `2c` | conversion |
| `34` | `32` | scalar assignment |
| `74` | `32` | aggregate assignment with aggregate type/size |
| `37` | `44` | comma |
| `56`, `36` | `42`, `43` | conditional expression and arms |
| `40` | `35` | postfix increment |
| `60` | `55` | argument list node, emitted by the call loop |

[EVIDENCE] These mappings agree with the static operator table and real
emitter hits. **C1 record 01 is not C2's allocator copy opcode 01.** Searching
raw .ex bytes for 01 does not identify an allocator copy.

## A: logical-not emission versus retained copies

[EVIDENCE] Unary entry `10415738` tries class overload resolution through
`10414f0c` for appropriate operand types. Built-in builder `10410239`
allocates the unary AST at `10410270 -> 10410187`. The not case starts
at **1041060f**: validate scalar/pointer-like properties, set result type
from `104cc4ec`, set node +2 bit 1, try constant recognition (`10407a5d`),
then return the existing unary node for a nonconstant operand (`10410645`).
That normal path creates no assignment node, saved-value symbol or input copy.

[EVIDENCE] Direct not emits the member read followed by `1a`, then conversion
and assignment as needed. A named `int old=p->flag` does emit an additional
source assignment. C1 is therefore not deleting that named source copy.

[EVIDENCE] C2 dispatcher `4276b5` handles record `1a` through `43495f` outside
an existing conditional context. `43495f` extracts the value with `425f81`,
adds zero and constructs compare **151** with `428a6e`, then complements the
condition encoding. The direct-not post-reader tuples are:

```
151 flags = compare memory[p+0], 0
15e result = select flags, 1, 0
131 memory[p+0] = result
```

The Boolean zero-copy and `10f` update appear during later C2 lowering.
C1 does not serialize that zero/compare/sete instruction sequence.

[EVIDENCE] The if/else toggle takes a different C1 route: read flag;
`38` to the false-arm label; scalar `32` assigning zero; `3a` to the join;
`29` false label; scalar `32` assigning one; `29` join label. Scope/line
records surround these expressions. `a-if.il.txt` is the compact decoded
record list, while `core.json` retains every record. C2 later recognizes
the Boolean result pattern and reaches compare/zero/10f without an old copy.

[EVIDENCE] `a_old` reaches the post-reader boundary as
`131 old=memory;151 compare old,0`. `core-a_old-passes.json` keeps it through
the first `42127d` entry; the following `409c5c` entry within that pass has
lost it. By machine lowering the compare again reads memory. The deletion
is bounded to C2 early dataflow/loop optimization. Reusing old after the
store or across a real call retains a named source/global range.

[EVIDENCE] C2 finalizer `4262d8` can create a `131` copy into `44136d`'s temp
when the `403381` predicate requires preserving an operand. Value extraction
`425f81 -> 44422c` also handles pending definitions; `44422c` tests
`403381` / `4442e7` and can synthesize a copy. These are C2 expression/use-state
predicates, not a C1 logical-not flag exposed by an extra scalar cast.

[EVIDENCE] Positive boundaries, each still failing the requested target:

* `a_post` preserves an old-value copy, but also increment/store and
  branch-based Boolean formation. Its one-expression form has unsequenced
  updates; it is only a mechanism control. The well-sequenced version also
  retains the unwanted update.
* `a_condition`: `!(rank ? flag : flag)` leaves two copies into an unnamed
  conditional result plus a rank branch. The result is already globally
  bound to eax at `42a8aa`.
* `a_value_condition`, `a_conditional_if` and its guarded version: a selection
  `flag ? flag : 0` can leave one old-value copy before zero/compare/10f,
  without a surviving selection branch. The old value is nevertheless
  **globally allocated**, not available to the rotor.

[EVIDENCE] The direct if/else micro also retains the canonical store and
feedback memory reload, so one bounded full-function diagnostic was warranted.
`conditional-input-diagnostic.cpp` changes only the rank-0 condition to
`if (flag ? flag : FALSE)`. In the full pre-temp snapshot, `t1772` is the
width-4 load of `[this+6d40]` before the Boolean zero, but global allocation
has already bound it to **ecx**. It retains **8 masked differences**.
It does not reproduce the fresh uncoloured descriptor used by the successful
IR counterfactual. See `conditional-transfer.json` and its prediction file.
The redundant normalization is a diagnostic, not the delivered source.

[EVIDENCE] Normal fnprobe independently confirms the same eight offsets.
All **94 event native function payloads**, including TreatEventBase, equal
the unchanged baseline despite the explicit input copy in the observed IR.
`conditional-payload-audit.json` records this byte-inert result. Relocation
validation is not inferred from this object-payload equality.

[EVIDENCE] Dominating-rank and local-constant controls in `selections.cpp`
remove their selections and fold the old input back into the memory compare.
The conditional-result copy does not become a free rotor temp after its
selecting condition is proven away in these controls.

[OPEN] No measured source form supplies the requested unassigned old-value
copy with the right consumer/order and no extra code. This is a bounded
negative, not a proof that all possible C++ forms are incapable of doing so.
A new lead must explain survival through C2 dataflow and global allocation.

## B: whole aggregate arguments and extracted effects

[EVIDENCE] Argument builder **10412057** uses
`104065c0(60, expression, previous_head, type)`. The expression is at +10,
previous head at +14; parsing successive arguments prepends them. The +8
links support another traversal and are not the emission chain. Constructor
observations locate ordinary builder call sites `1043126c` / `10431286`.

[EVIDENCE] Call emission at **10404a25..10404a41** recursively emits the
head expression, calls the writer with `55`, and follows +14. The draw
records therefore appear **font, text, POINT, pixmap**. This loop does not
choose individual POINT fields relative to font/text.

[EVIDENCE] Type writer `104162d9` serializes the observed eight-byte POINT
as `30(type=5008,size=8)` and `55(type=5008,size=8)`. **C1 does not emit the
outgoing x/y field copies for this trivial by-value argument.** C2 `435338`
constructs whole aggregate op `13e` on its small-size path (size <= fff).
Field copy/push expansion occurs later in machine lowering, as documented
in the inherited Round 3 SB1 evidence.

| Source form | C1 records | Observed C2 consequence |
|---|---|---|
| field assignments | two scalar `32` before call | early x/y definitions |
| `{330,y}` initializer | separate field assignments | same early definitions |
| `Draw(...,p=q,...)` | whole `30/32/55`, type 5008 size 8 | p copy disappears; q fields remain early |
| POINT comma assignment | scalar `32`, whole `30`, comma `44`, aggregate `55`, after font/text | assignment extracted into early prefix |
| temporary POINT-derived constructor | real nested constructor call, aggregate read/copy | constructor inlines; fields early |
| conversion or assignment operator | real C++ calls | calls inline; fields early |
| temporary destructor | cleanup/EH records as well | early fields plus EH/stack effects |
| complete inline draw wrapper, scalar x/y | scalar wrapper arguments; POINT in callee | same early inlined fields |
| varargs / variadic pointer cast | `55` can supply inferred type zero | C2 infers 5008/8; same 13e |

[EVIDENCE] C2's comma case in `4276b5` calls `4287e6`, `425acb` and splices
the side-effect chain into the expression prefix. Assembly `4264d2` emits
the +14 prefix chain before the main expression, and +18 chain afterward.
The `b_comma` post-reader snapshot already puts x=330 before font tuples.
Neither scheduling nor register allocation first causes this move.

[EVIDENCE] The zero-type path in `435338` infers the actual value type and
then constructs the same aggregate node. Four held-out variadic controls
confirmed the predicted early definitions. Overloaded assignment/conversion
controls likewise confirmed the ordinary-call/inlining prediction.

[OPEN] No tested construction provides a rematerializable outgoing x literal
without the early source-field interval. A useful source lead must change
that value/definition relationship; lexical record position alone is insufficient.

## Verification limits

[EVIDENCE] Seven serial micro TUs contain 68 target controls (45 A, 23 B).
The ledger separates globally coloured positive copy controls from embedded
compares. No control qualifies for the exact rotor-input or late-x target.

[EVIDENCE] The first long Ghidra import was interrupted/reaped. The successful
private import disabled Decompiler Parameter ID / Switch Analysis and used
a 75-second analysis limit. Auto-analysis timed out but saved the program.
Selected pseudocode, focused disassembly, the raw table and dynamic hits
jointly support the addresses; the function map is not claimed complete.

[EVIDENCE] Only opcode-defined reader fields are interpreted: other buffer
words contain stale data. Only operand-bearing tuple kinds are used for
lifetime conclusions; marker tuples reuse their storage for other metadata.

## Reproduction and ownership

`ghidra.py` uses Ghidra 12.1 and the private `build/sol-c1/project/c1xx.gpr`
project. `ExportC1.java` exports selected functions and the static call map.
`probe.py` uses a private /B2 stub per uniquely named capture, normal repo
compiler settings, and synchronous owned/reaped c2 replays. It checks every
function payload against the native object after observation. Captures,
decoded IL and scalar tuples remain under this directory.
