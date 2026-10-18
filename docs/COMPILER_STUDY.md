# Compiler study: residues, reduced examples and the residue map (2026-10-17)

Method: start from where the bytes differ, never from blind runs.
1. `tools/residues.py` aligns every unmatched function with its oracle body and classifies each differing region
   (register / order / constant / insert / delete / mixed). Relocated operands, addend-0 spellings and switch
   tables are normalized, so the regions are the real differences.
2. For one region, `tools/fnprobe.py` compiles one TU (canonical or candidate) with the manifest settings in about
   a second and diffs one function against the oracle (`--region` prints both sequences). Diagnostic only;
   acceptance is always `tools/verify.py`.
3. When the cause is unclear, rebuild the function's shape in a micro TU and vary one property at a time.

In Git Bash, set `MSYS_NO_PATHCONV=1` before passing `/flags` (`fnprobe --extra-flag=/G5`): MSYS otherwise rewrites
`/G5` into a path and the compiler silently ignores it. An earlier flag sweep in this study was invalid for this
reason and was redone.

## Results ([EVIDENCE])

| Function | Residue class (map) | Cause | Fix |
|---|---|---|---|
| `CDecor::FileExist` 0x159f0 | cleanup block at end vs inline | reconstruction `goto success` arrangement | Planet Blupi decio form (BLUPI-CTL-02) |
| `CDecor::CurrentRead` 0x153d0 | same | same | same |
| `CButton::Create` 0x2a30 | store order | member-wise POINT copies; flag order | `m_pos = pos; m_dim = iconDim;`, `m_bDemo` before `m_bHide` |
| `CDecor::AdaptBorder` 0x144d0 | register-only (rotation) | POINT temporaries as call arguments | `AdaptMidBorder(int x, int y)` (BLUPI-REG-03) |
| `ChangeSprite` 0x1f210 | constant off by one + reload | SB2 `SPRITE_MAP` shifted SB1 numbering; default assignment | SB1 numbering; `hCursor` left uninitialized (BLUPI-SEM-02) |

Coverage 107,440 -> 110,144 / 141,488 bytes. All accepted functions stay strict.

### Struct temporaries

[EVIDENCE] Micro TU (`AdaptBorder` shape, 30 lines): five calls with `MAKE_POINT(...)` arguments use eax for every
temporary; the same calls with `(int x, int y)` parameters rotate eax/ecx/edx/eax, and the following index
calculation follows the rotation. Inline-helper form (initializer, member stores, derived-class constructor) and a
single extra struct-temporary call elsewhere are neutral. On SB1 the scalar signature made `AdaptBorder` STRICT while
`AdaptMidBorder`'s own body stayed byte-identical (its accepted record was renamed to `?AdaptMidBorder@CDecor@@QAEXHH@Z`).

### Error-block placement

[EVIDENCE] `if (bad) goto error; ... return TRUE; error: ...` (error label at the end of the source) is laid out
with the cleanup right after the last check, and earlier NULL checks jump into its middle. The reconstruction's
`if (ok) goto success; error: ...; success:` made VC5 move the cleanup to the end.

### Uninitialized local

[EVIDENCE] `ChangeSprite` without a default `hCursor` assignment reads the dead `sprite` parameter slot
(`mov eax,[esp+0xc]`), exactly as the oracle does.

## TreatEventBase (still open)

[EVIDENCE] Exactly 8 bytes differ, at two coupled free-choice sites (+0x41a toggle, +0x6f3 PostMessage hwnd); all
other code and the 62 jump-table entries match. Reduced examples show the choice is function-global: a load added
anywhere (even laid out after the sites) can flip it, with period 3 in the number of inserted loads.
[EVIDENCE] Inert with identical code elsewhere: declaration order, case-label order, case source order, copies,
dead stores, local types, renames, blank lines, declaration-count probes 0-32, stubbing all 96 preceding
functions, scalar signatures for its POINT callees, all VC5 trees, 25 flags (BLUPI-NEG-03).
[OPEN] The original differs in something not yet tested; the BLUPI-REG-03 class (argument/expression forms that
leave bytes identical but change temporaries) is the best remaining direction.

## Other map entries ([OPEN])

- `DrawMap` 0x14550 (STRICT, see "CDecor::DrawMap: field-assignment order"): tail register phase differed around the `Build(RECT, POINT)` / `DrawMap(8, RECT, RECT)` struct
  temporaries; local RECT variables enlarge the frame (rejected); temporary forms were neutral in the micro TU.
- `CNetwork::Send` 0x20ec0: oracle holds dwFlags/dwDataSize in esi/edi and evaluates `dwFlags != 0` first; natural
  assignment/ternary forms are inert. The comma expression is an SB2-reconstruction artifact kept until a form matches.
- `DrawButtons` 0x16f70: split-title draw calls; the oracle loads `m_pPixmap` right before its push (strict
  right-to-left argument order), the candidate hoists it; ternary/pointer-store/POINT-temporary forms tested.
- Larger: `ChangePhase`, `Build`, `DoInit` (CDecor size), pixmap `Create`/`QuickIcon` (frame), `AddCDPath`.

## Round 2: tail functions (residue map driven)

### Results ([EVIDENCE])

| Function | Cause found | Result |
|---|---|---|
| `CPixmap::QuickIcon` 0x21b20 | generic path and clip/blit tail are Planet Blupi `DrawIcon` + `BltFast` code (nbx/nby, `limit`, `HRESULT ddrval`); the reconstruction's reused `num` produced the 0x10 vs 0x14 frame | STRICT |
| `CPixmap::Create` 0x21190 | `SetDisplayMode(..., bTrueColor ? 16 : 8)` uses the parameter (oracle reads its register), `m_dim = dim`; this also removed the global zero-register promotion | 597 -> 4 bytes (one scheduling swap) |
| `AddCDPath` 0x1f410 | SB1 prefix list from the oracle strings, no `daniel` case | 3 regions (copy-tail layout) |
| `CEvent::ChangePhase` 0x19310 | if-chain (not the SB2 `switch`), `WM_PHASE_DOQUIT`, `WM_PHASE_SETUPp`, demo worlds 200-219, `MESS_LEAVE`; Sol WIP integrated | 1,879/2,029 instructions; rest is the ebx/edi assignment of promoted 0 / `WM_PHASE_PLAY` |
| `CDecor::Build` 0x4350 | `type >= 8 && type <= 11`; cloud loop like the net-blupi loop | 1,155/1,237 instructions; per-branch blupi QuickIcon tails still open |

### Emission order (corrected)

[EVIDENCE] A free scratch-register choice follows the allocations emitted before it in layout order (BLUPI-REG-04).
Micro TU: an extra load in `case 0x102`/`case 0x150` (bodies emitted before the `case 0x100` body) flips the 0x100
case's choices regardless of source position; in `case 0xA0`/`case 0x50` (emitted after) it never does. Real
`TreatEventBase`: the same diagnostic load flips both residues in `WM_NCMOUSEMOVE` (emitted before `WM_KEYDOWN`) and
in no case emitted after it. An earlier draft of this section said "descending case value"; that was wrong.
Consequence: the original differs from our source in something emitted before the residue sites (function start,
network receive block, dispatch, `WM_NCMOUSEMOVE`), although all of that code is byte-identical, so the difference
must change free/forced allocation status without changing bytes.

### Zero promotion

[EVIDENCE] `CPixmap::Create`: removing any one of four early zero uses (diagnostic only) stops VC5 from keeping 0 in
a callee-saved register; the original had a different early register demand (the `bTrueColor` parameter kept live),
not fewer zeros. Use `fnprobe` on single-substitution diagnostics to find such thresholds, then look for a real
source difference nearby.

### Negative / open

- [EVIDENCE] Merged duplicate tails (`goto key_recorded` inlined) and all VC5 front-end/back-end mixes (42 pairs) leave
  `TreatEventBase`, `DrawMap`, `Send`, `DrawButtons` unchanged.
- [OPEN] `TreatEventBase` (8 bytes), `DrawButtons` (split-title scheduling), `DrawMap` (per-field registers for the
  RECT/POINT temporaries), `CNetwork::Send` (parameters cached in esi/edi), `ChangePhase` (constant register swap),
  `Build` (tail sharing in the blupi block), `CPixmap::Create` (vtable load order), `DoInit` (CDecor size, layout debt).

### DecorNextAction / GetBlupiInfo ([EVIDENCE], still open)

- The DecorNextAction body (scratch/sol-build) is byte-identical to the oracle; inserting it before GetBlupiInfo swaps
  both commutative OR operand pairs there (`m_blupiJeep|m_blupiTank`, `m_blupiSurf|m_blupiNage`).
- Isolation: the action table alone is inert; a trivial `return m_posDecor;` body, or any free/static/member function
  defined before GetBlupiInfo, causes the same swap; the same function after GetBlupiInfo is inert. Removing the 14
  POINT operator helpers from def.h is inert. Operand spelling (natural, swapped, local-variable forms) is inert.
- Extern-declaration probes before GetBlupiInfo (with DecorNextAction) fix only the jeep pair (counts 3-6, 11-16);
  extra static functions give the same 2-byte state; no tested context reaches 0.
- Conclusion: the original TU differs in the number/kind of definitions before GetBlupiInfo in a way not yet
  identified. The non-inline POINT operators in def.h are a reconstruction convenience (they would also clash at
  link time) and are recorded for cleanup, but they are not the cause here.
- [EVIDENCE] Removing the 14 POINT operator helpers from def.h compiles every TU (none uses them) but regresses four
  accepted functions (IsDeepWater, MockeryDetect, MoveObjectSort, QuickIcon): their context currently depends on
  those definitions, which therefore stand in for some genuine original context. Kept until that is identified.
- [EVIDENCE] Declaring the same 14 helpers `inline` keeps every accepted function strict (clean gate, 110,928 bytes)
  and removes 14 of the 16 duplicate definitions that blocked the full link (tools/relink.py). The front-end
  context they supply survives; only their COMDAT emission goes away. Canonical def.h now uses `inline`.
- [EVIDENCE] IsDeepWater (decblupi.cpp) has a declaration-count threshold: removing the duplicate
  GetBlupiChannelActual/Standard bodies (owned by decblock.cpp) breaks it (x/y order), and 6 or more `extern int`
  probes before BlupiIsGround restore it (0-4 do not). [OPEN] which genuine SB1 declarations supplied that count.

## Round 3: allocator measurements (build/tmp/ra lab)

### Rotor ([EVIDENCE], BLUPI-REG-05)

Serialized loads (each followed by a call) take eax, ecx, edx, eax, ... - a per-function rotor that calls do not
reset. Free expression temporaries take the next register and advance it; forced registers (call results, cdq/idiv,
`cl` shift counts) do not; `a + b` advances it twice (ecx, edx), which looks like a reset. A plain-copy local
(`t = h[k]; V1(t);`) takes the first free register (eax) and leaves the rotor alone; a computed local (`t = h[k] + 1`)
behaves as a temporary; a local used twice goes to a callee-saved register.

### Counter domains ([EVIDENCE])

Using the Blood II phase model (one TU-wide symbol counter; front-end and back-end numbered decisions):
- TreatEventBase: no change for 0-300 `extern` probes at the end of event.cpp (back-end domain), 0-256 at the top,
  0-256 directly before the function. Its two residues are not symbol-number decisions.
- DrawButtons: no change for 0-128 probes directly before it or 0-64 at the top; a few large top counts make it worse
  (78). Not a small counter offset either.

### Negative ([EVIDENCE])

- Local/temporary conversions of single-use loads emitted before TreatEventBase's toggle (network block, prologue,
  text-input guard, WM_NCMOUSEMOVE) are inert in the full function.
- DrawButtons: selection/font locals at the split-title site are worse.
- The simple rotor model mispredicts one choice in TreatEventBase's cheat code (`lea ecx,[edi+1]`), so the real
  free-choice rule has more inputs than the micro corpus shows. [OPEN] Reading the selection routine in c2.exe
  (Ghidra) is the remaining route for these two functions.

### Rotor refinements ([EVIDENCE], build/tmp/ra lab, VC5 SP3 /O2)

- The rotor continues across if/else arms, join blocks and early-return arms in layout order (`if1`, `if2`,
  `ifelse`, `ret_in*`). A loop body is allocated before the loop's entry test (`loop`).
- A load used only by a compare/test (`if (g[9])`, `if (g[2] == 0)`) takes the first free register and does not
  advance the rotor.
- `g = !g` / `if (g) g = 0; else g = 1;` give identical code: the tested load is first-free and the sete result
  takes the rotor register. When the toggled value is reused afterwards through CSE (`V1(g[1])`), the roles swap:
  the load takes the rotor register and the result the first free one. This is the shape of TreatEventBase's
  oracle toggle at +0x41a (load edx, result ecx with eax live); the candidate has the non-swapped shape. [OPEN]
  which original use makes the swap; copying the result into a register local (`b = g[1]`, `b = g = !g`) does not.
- `add2` (`g[5] = g[6] + g[7]` between rotating loads) separates the rotor from an LRU model: only the rotor
  predicts it.

### c2.exe internals ([EVIDENCE], static only; project build/c2, export build/c2/out/all.c)

- c2.exe (SP3, 660,240 bytes) is basic-block reordered: functions are split into hot and cold chunks far apart,
  so Ghidra's function bodies are fragments. Assertion strings name the back-end sources (`P2egasg.c`,
  `P2\color.c`, `P2\code.c`, `lower.c`, `schedmd.c`, ...): register assignment is a graph-colouring
  allocator (color.c, chunks around 0x4676b8-0x46930a) plus regasg.c (0x46ee34, 0x478192, 0x478503).
- Register numbers: 0 noreg, 1 eax, 2 ecx, 3 edx, 4 ebx, 5 esp, 6 ebp, 7 esi, 8 edi (name table at 0x49d924
  downwards; regasg code loops over registers below 9 and skips 5). Hidden option table at 0x49a45a (`-Bd`,
  `-off#`, `-db#`, `-il#`, `-noblend`, ...) has no IR or allocation dump switch.
- [OPEN] The rotor's implementation was not located by text search of the decompilation; reading color.c needs
  chunk reassembly (or a dynamic trace of c2.exe, which is a toolchain binary, not game code).

## Round 4: c2.exe temp-register assignment, traced ([EVIDENCE])

Method: cl.exe `/B2` runs a stand-in pass 2 (build/c2/dyn/c2stub.c, VC6-built helper) that copies the IL files and
`MSC_CMD_FLAGS`, then calls the real SP3 c2.exe. A small WOW64 debugger (build/c2/dbg.py, ctypes) re-runs c2.exe on
the captured IL with breakpoints and hardware watchpoints. Nothing on disk is modified; the traced run writes the
same object as a normal compile. Differential hit counts over 4/5/6-load micro TUs located the state.

- The rotor is a pointer at 0x491120 into the register list at 0x491100 (eax, ecx, edx; slot 3 holds 4 and ends
  the scan). It is reset to eax at the start of every function (0x42a90e).
- `0x435c37 assign(temp, tuple)` gives a temp its register, in this order:
  1. preference: a copy tuple (op 1) from a real register (1-8, not esp) records that register; some op 0x12
     (address) tuples record one too. A free, non-interfering preferred register is taken without touching the rotor;
  2. rotor (when the flag at 0x48fb60 is set): the first register at or after the pointer that is not busy
     (0x491180[r]) and does not interfere (bitset 0x491140[r]); the pointer moves to the slot after it, wrapping;
  3. otherwise first free from eax.
  Only this rotor path writes 0x491120 (hardware watchpoint over all of event.cpp).
- `0x42aa3c` calls it for every temp a tuple defines; `0x42a99f` frees a register at the temp's last use. Loads that
  are folded into a compare (`mov dl,[m]; test dl,dl`, toggle loads) never pass through it: code generation picks
  their register later. That is why they look "first free" and never advance the rotor.
- Register numbering: 1 eax, 2 ecx, 3 edx, 4 ebx, 5 esp, 6 ebp, 7 esi, 8 edi.

### TreatEventBase, located by rotor perturbation

Shifting the rotor pointer in memory at one assignment of TreatEventBase (59 assignments; diagnostic only) and
diffing the function against the oracle: a shift of +1 or +2 slots at assignment 21 gives **0 differing bytes**;
every other position leaves differences. Assignment 20 is the `lParam` argument of `TreatTextInput` (+0x35f, ecx),
assignment 21 the toggle result at +0x41a (candidate edx, oracle ecx; the toggle load then follows by code-generation
choice). The PostMessage `m_hWnd` load (+0x6f3) is assignment 28; 22-27 take preferred registers and leave the rotor
alone. So the original has exactly one rotor step more (or, equivalently, one less) between the TreatTextInput
argument and the toggle result, with identical bytes. Consistent explanations: the terminator test
`cheat_code[m_rankCheat][m_posCheat] == 0` (the only value in that span that would get the same register, edx, from
the rotor) was a real temp in the original, or the `lParam` argument took a non-rotating path (still ecx). [OPEN]
the source form. Inert so far: c-variable / `!` / pre-increment / `+= 1` forms of the test, `message` vs
`WM_KEYDOWN`, local copies of `lParam` and of the call result.

### Preference pass and toggle forms ([EVIDENCE])

- A pass before temp assignment (0x42ac33, store at 0x42b051) sets a temp's preferred register when the temp is
  copied into something that already has one (descriptors at 0x494758 + 0x50*reg). Example: in TreatEventBase the
  toggle result of `m_b = !m_b; bEnable = m_b;` prefers bEnable's esi; esi is busy, so the rotor decides (edx).
- The micro copy of the cheat matcher (build/c2/cheatlab.py) reproduces the candidate's toggle exactly. There,
  `m_b = !m_b;` (also `m_b ? 0 : 1`, `m_b == 0`) yields the oracle shape `mov edx,[m]; xor ecx,ecx; test edx,edx;
  sete cl` through a preference for ecx. Adding any later `bEnable = <call result>` handler (TreatEventBase has
  several) turns the preference into bEnable's register, and the shape returns to the candidate's.
- Remaining hypotheses for the one TreatEventBase rotor step: H1 the terminator byte load was a real temp (edx);
  H2 the TreatTextInput `lParam` argument took a non-rotating path (ecx); H3 the toggle load itself was a rotating temp
  allocated before the result (edx, then the result gets ecx with eax live). Inert in the micro copy and/or the real
  function: unsigned/int/pointer/c-variable/`<= 0` spellings of the terminator test; `!= 0` and `msg` forms of the
  TreatTextInput call; `== 1`, `!= 0`, braces and reversed if/else forms of the toggle; `int c`, `>= 0` rank test
  and dropping `bEnable = FALSE` (these change code).

### DrawButtons ([EVIDENCE], tools/c2trace)

341 temp assignments. Rotor shifts at assignment 234 or later change nothing in the function. The first residue
(+0x19c6, `mov ecx,[m_fileIndex]; cmp ecx,esi` vs oracle eax) is a compare-only load, so code generation's scratch
choice decides it, not the rotor. [OPEN] Locate that scratch picker in c2.exe (it also chooses TreatEventBase's toggle
load). The rest of the residue is split-title scheduling (`m_pPixmap` load placement), i.e. source form.

## Round 5: code-generation scratch picker ([EVIDENCE], tools/c2trace)

Found by diffing (function, return value) streams of two traced runs (tools/c2trace/retlog.py) that differ only in the
toggle registers: `0x42b2c4 pick(liveset=ecx, byteonly=edx)` returned 2 vs 3 for the toggle load.

- Second rotor: index of the last scratch pick at 0x4911a8, list length at 0x4911a4 (8: the list at 0x491100 extended
  with callee-saved registers). Reset to n-1 at the start of every basic block (0x42b70f), so each block's first pick
  scans from eax.
- Scan from last+1, wrapping. Pass 1 takes only registers already used in the function (descriptor byte +5 flag 0x10,
  set when a temp is given that register), pass 2 any register; registers in the live set are skipped, and with
  `byteonly` anything above ebx. The pick stores its index back. Fallback (0x42b2fa): first free from the start.
- Users: loads folded into a compare (`mov r,[m]; test/cmp r`) and similar code-generation temporaries. They never
  touch the temp rotor (0x491120) or the busy table (read/write watchpoints).
- Consequence: a compare-only load takes the first non-live register in its block, unless an earlier scratch pick
  in the same block moved the index. A value whose live range covers the compare in IR order (for example a constant
  assigned by an earlier statement) pushes the load to the next register.

### Validated on DrawButtons +0x19c6 (prediction confirmed)

The block's first scratch pick (`mov r,[m_fileIndex]; cmp r,esi`) should start at eax; the oracle has eax, the candidate
ecx, so eax had to be live in the candidate's IR. The constant 330 of `pos.x = 330;`, written as a statement inside the
NULL branch right before the call, was holding it. Writing `pos.x = 330;` once before `strchr` (the else branch keeps
its own assignments) removes exactly the +0x19c6/+0x19ce regions: 65 -> 63 differing bytes, regions 10 -> 8, no other
change. Removing the else branch's second `pos.x = 330` aligns the split-title region further (2295/2304 equal) but
keeps 330 in esi instead of rematerialising it; the title region is the source worker's (scratch/sol-srcres).

### TreatEventBase hypotheses, checked against the mechanisms

- H3 (toggle load as a rotating temp) is excluded: the toggle load is a scratch pick, the block's first, so it takes
  the first non-live register; eax holds m_rankCheat and the result's register is live, so the load simply follows the
  toggle result (assignment 21). It cannot carry the missing step.
- H2 (TreatTextInput `lParam` not rotating) is excluded: only a preference could do that, and the preference pass needs
  the temp to be copied into a register-holding value; it is only pushed. With eax busy at that load, the rotor state
  after assignment 20 is edx whatever came before, so the missing step lies strictly between assignments 20 and 21.
- H1 remains: the only value there that a rotor would place in the oracle's register is the terminator byte load (edx).
  c2 folded it in every tested form (comparison, `!`, casts, pointer, named local, assignment inside the condition,
  repeated test), so the original form is still [OPEN].

### Argument aggregates: CDecor::DrawMap ([EVIDENCE])

- Planet-Blupi-style local aggregates for `Build(clip, pos)` match the oracle's argument construction (fields promoted
  to registers, constants loaded before the stores) without growing the 0x28 frame; the reconstruction's
  `RECT(...)`/`POINT(...)` helper macros (inline MAKE_RECT) do not (integrated, 120 -> 66 bytes).
- For `m_pPixmap->DrawMap(8, src, dst)`: a local `src` (522x400) is promoted like the oracle but c2 also reserves a
  dead 16-byte frame slot (0x38); a local `dst` alone, or all four aggregates in one block, folds the constants into
  the copies instead (frame 0x28). Tested: separate/one block, declaration order, function scope, field assignment,
  reuse of `clip`, copy-then-modify, a RECT array (0x48). Resolved below.

### CDecor::DrawMap: field-assignment order ([EVIDENCE])

STRICT with `RECT src, dst;` assigned field by field, `src` first, both in top, bottom, left, right order (KB
BLUPI-SRC-03). Measured with 72 order variants (build/tmp/rx/s_dmr.py, `vary.py`):
- Initialising `src` (pushed second) before `dst` makes c2 preload src's four constants into registers before dst's
  stores, as in the oracle. Initialising in push order (dst first), `MAKE_RECT` temporaries and a local `dst` alone
  store each constant directly at its push site.
- The dead slot comes from a zero shared by two fields: whenever `dst.left = 0` is assigned before `dst.top = 0`,
  one store survives into a 16-byte frame slot (frame 0x38). All 12 dst orders with `top` before `left` avoid it, and
  so does `dst.left = dst.top = 0` (top assigned first). With distinct values (`{0, 1, ...}`, `{3, 4, ...}`) there is
  no dead store in any order. `= { 0, 0, ... }` and MAKE_RECT behave like left first.
- With the slot gone, src's preload registers follow its field statement order (temp rotor, BLUPI-REG-06). Of the 24
  src orders only top, bottom, left, right gives the oracle's edx/eax/edi/ecx; the others differ by 4-15 bytes.
- [HYPOTHESIS] The field order is a fitted choice. The author usually writes left, right, top, bottom (Planet Blupi,
  most SB1 functions), but this is the only order that matches, and using it for both rects keeps them consistent.

### CPixmap::Create +0x12d ([EVIDENCE], negative)

The 4-byte residue is a Pentium scheduling swap (`sbb edi,edi` vs the vtable load). Inert: `!= 0`, reversed and
`== FALSE` ternaries, parentheses, a local `depth`; an if/else with two calls changes the code.

### Source worker round 2 (sol-srcres, negatives; scratch/sol-srcres/round2/FINDINGS.md)

85 source trials and 24 rotor diagnostics on DrawButtons' split title, ChangePhase and Build; none retained.
- Removing the split branch's second `pos.x = 330` fixes the pixmap-load schedule but keeps 330 in esi across both
  calls (oracle rematerialises it in eax); moving the remaining assignment, or removing both, does not help.
- Rotor shifts at assignments 303-314 of DrawButtons do not close the region (best: 311 +1, 63 -> 51 fixed bytes):
  the residue needs a different temp structure, not only a different rotor state.
- Aggregate POINT locals, POINT copies, literal/`pos.x` POINT arguments, pointer and control-flow variants of the
  split draw: identical to HEAD or worse.

### CNetwork::Send: if/else normalisation of a parameter ([EVIDENCE])

`if (dwFlags) dwFlags = DPSEND_GUARANTEED; else dwFlags = 0;` before the call makes the parameter a register variable
(esi) and c2 turns the if/else into `xor edx,edx; test esi,esi; setne dl` - the oracle's code (STRICT). Branchless
`dwFlags = dwFlags != 0`, `!!dwFlags`, ternary arguments and a local flag fold away (old ebx/esi allocation); the
if-only form keeps a branch. Lesson: a setcc whose source stays in a callee-saved register points to an if/else
assignment of a variable, not to an expression.
