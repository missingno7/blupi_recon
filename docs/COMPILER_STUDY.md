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
| `AddCDPath` 0x1f410 | SB1 prefix list from the oracle strings, no `daniel` case | STRICT with the constant `!bDaniel &&` term (see below) |
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

### AddCDPath: constant-folded outer condition ([EVIDENCE])

STRICT with Planet Blupi's `BOOL bDaniel = FALSE;` and `if ( !bDaniel && (five prefix tests) )`; SB1 dropped only
the `\daniel\blupi\` detection (no such string in the oracle), so `bDaniel` stays FALSE and folds away (KB
BLUPI-SRC-04, build/tmp/rx/s_acd.py). Predicted from the layout before the trial:
- c1 lowers a bare OR chain `if (A || ... || E) X else Y` with the last test jumping to X and Y falling through
  (measured: 2-term OR `je, je`; single condition and 2-term AND fall into X). The oracle ends with `jne Y` and X
  falling through, the shape of an OR chain inside an outer `&&`: `if E goto X; goto Y; X:` becomes `if !E goto Y`.
- `TRUE && (...)` gives the same bytes, so any constant term works; the bDaniel form is the one with ancestry.
- Inert before: arm swap with De Morgan, `!(...)`, common or duplicated tails, a relative-path BOOL, early returns.

## Round 6: Codex workers ([EVIDENCE])

Four gpt-6.1-sol workers, briefs in scratch/prompts/{evanc,evanc_r2,evanc_r3,build3,build3_r2,build3_r3,cphase,cphase_r2,mos2}.md,
findings in scratch/sol-{evanc,build3,cphase,mos}/FINDINGS.md (all trials with counts). Supervisor gate after
integration: 404 STRICT + 4 FOLDED, 113,344 / 141,488, zero regressions.

- **DX5 state integrated (sol-mos).** The last regression of the DX5 + authenticity state, MoveObjectSort, is fixed
  by moving decmove.cpp's existing `#include <decor.h>` below `misc.h` and `dectables.h` (137 counter numbers before
  decor.h, inside the measured 109-141 window); DecorNextAction becomes STRICT (+272). [HYPOTHESIS] The include order
  is plausible, not proven (docs/LINK_CONTEXT.md "DX5 state integrated").
- **ChangePhase (sol-cphase), integrated as best-known body.** The oracle computes the previous-page hide flag in both
  network page lists: `bHide = FALSE; if (m_filePageOffset == 0) bHide = TRUE; SetHide(WM_BUTTON10, bHide);` (and the
  same form for next-page) replaces `SetHide(WM_BUTTON10, TRUE)`. Right after the offset reset the test folds to TRUE,
  yet it changes later temps: 60 -> 28 regions, 1925 -> 1995 / 2029 aligned (BLUPI-SRC-04 family). Remaining: the
  early zero (21 refs, LR 99) and WM_PHASE_PLAY (8 refs, LR 127) webs are coloured ebx/edi instead of edi/ebx. The
  PLAY range loses its WriteInfo comparison (BB 92) to c2 rematerialisation; zero's late uses are the index test, the
  HideMouse argument and the Write flag. 117 forms tested, including 14 predicted from the trace; none swaps the webs.
- **TreatEventBase (sol-evanc), not integrated.** H1 confirmed: a rotor-assigned byte temp at the terminator test
  supplies the missing rotor step (BLUPI-REG-10). `i = m_posCheat++; if (cheat_code[m_rankCheat][i + 1] == 0)` gives
  8 -> 5 bytes but loads before the store and uses a table+1 addend, so it stays in scratch. Round 1 tested 49 ancestry
  forms from PB b0270ab event.cpp (direct `m_b = !m_b` toggles, PB declaration lists, default-TRUE feedback, completion
  flags), round 3 50 byte-temp shapes. DrawButtons unchanged (63 bytes); its esi loads are scratch picks.
- **Build (sol-build3), no change.** The hoisted zero is a pooled constant (BLUPI-REG-09): 9 references, score +1.
  The oracle keeps both network-loop zero initialisations and `m_blupiSec = 0`, so no statement is missing there;
  splitting the uses (shield-direct, 3+2+4) removes the web but worsens the function (74 regions). The background
  `rect.bottom` hold is bottom-definition cloning; the spill-slot order is a history-dependent tie. Remaining: the
  shared player-drawing tail (call count and register roles). 230+ trials, 10 micro labs.

## Round 7: compiler models ([EVIDENCE])

- Global register allocator (splitting, cost, queue, colouring, late demotion, stack packing): docs/ALLOCATOR.md, KB
  BLUPI-ALLOC-01..07, tool tools/c2trace/colour_trace.py. 2124/2124 queue and colour decisions predicted from IL.
- Declaration-count sensitivity (c1xx symbol IDs hashed into c2's operand-order keys): docs/SYMBOL_ORDER.md, KB
  BLUPI-CTX-03/04, tool tools/c2trace/ctxpredict.py. 60/60 held-out predictions; it corrects the measured
  MoveObjectSort and MockeryDetect windows and the declaration cost model above.


### TreatEventBase localized by the 1998 demo ([EVIDENCE], worker sol-teb + supervisor)

The June 1998 demo build (same compiler) has no rank-0 (build-official-missions) cheat block. Deleting only that block
from our TreatEventBase reproduces the demo's code exactly over +0x35f..+0x6b2 (238/238 normalized instructions, PostMessage
hwnd edx included). So all our cheat-matcher source outside the rank-0 block is right, and the original rank-0 block
differs from ours by one temp-rotor step modulo 3 (rotor diagnostics: +1 or +2 at the toggle both give 0 bytes).
`m_bBuildOfficialMissions ^= TRUE;` makes every later instruction align (1890/1894, one region), but sol-teb round 2
showed why: its load stays a scratch pick and the rotating result assignment disappears (one step fewer, which is
equivalent to two more), so it does not show a rotating operand. Inert or worse: chained assignments, the toggle via
bEnable or a local, ternaries, == FALSE, != FALSE, casts, 1 - x, !!x (sol-teb: 87 full captures + 87 micros; supervisor: 18).
[OPEN] a spelling of the rank-0 block whose rotor step count differs by one (mod 3) while still emitting
`mov edx,[m]; xor ecx,ecx; test edx,edx; sete cl; mov [m],ecx; mov esi,ecx`.

### Instruction scheduler ([EVIDENCE], worker sol-sched; docs/SCHEDULER.md)

c2's P5 list scheduler (driver 0x40d9be, graph 0x431ec0, select 0x43179a, U/V pairing 0x431c72) runs after register
assignment on dependence windows and orders ready nodes by eligibility, priority (reverse heights, critical path) and
the original tuple ordinal. Model validated on 25 windows / 599 decisions with zero discrepancies (KB BLUPI-SCHED-01;
tool tools/c2trace/sched/tracer.py). DrawButtons: the oracle's late m_pPixmap load and early line2 load follow from
register anti-dependences that the scheduler cannot break; an alias edge alone is insufficient. The residue therefore
goes back to temp-register assignment in the split-title block, not to scheduling.

### DrawButtons: the residue is two early pos.x definitions ([EVIDENCE], worker sol-sched rounds 2-3)

First divergent temp assignment: 310 (the first text address). The rotor already points at eax, but eax is globally
forbidden: the two scalarized pos.x ranges (LR142/LR150) bind eax and run from the early `pos.x = 330` statements to
their argument pushes. A diagnostic that only moves those two IR definitions to just before their x-argument pushes
(compiler memory, before temp assignment; never a source or output change) removes the whole 63-byte residue. Lowering
model: c2 assembles assignments early, deletes late copies at 0x443889, and rematerializes the 330 constant while keeping
the early definitions (docs/LOWERING.md). Inert or worse source forms: x inside the POINT argument, const/fresh POINT
aggregates, inline POINT-returning helpers, font/line2+1 precomputed into locals before `pos.x = 330`, and the 18
micro controls in round 3. [OPEN] the C++ construct whose IR defines x after the text/font arguments.

### Build: exact CFG recovered ([EVIDENCE], worker sol-build4)

A static CFG comparison (scratch/sol-build4/cfg.py) showed that the original draws the blupi with a QuickIcon call in
each shield/power/cloud/hide branch, plus QuickIcon(CHTEMP, 0, tinyPoint) on the hide path with one mask origin, and
jumps from the final else into the normal-draw call. The reconstruction's shared `num`/`channel` assignments followed by
one QuickIcon came from the decompiler-derived SB2 code. The canonical source now has the oracle's exact ordered CFG and
player/mask register roles (1172/1237 aligned, previously 1162). The `goto blupi_normal` is kept: the goto-free
duplicated form gives the same CFG but swaps register roles and regresses accepted GetBlupiInfo (context). Remaining:
allocation regions (58), e.g. a floor-register exchange traced to a 1400-point EDI penalty from two cache ranges
(scratch/sol-build4/round2/FINDINGS.md).
Round 3: the background loop reuses `rect.bottom` as the height (`rect.bottom -= rect.top; tinyPoint.y += rect.bottom;`
then resets it to DIMDECORY): that is the oracle's eax hold of 0x1e0 that the earlier placement sweeps could not
reproduce. The background code now matches; 2,328 differing bytes, 1177/1237 aligned, first difference +0x4b1;
remaining body excess 38 bytes (lift block 26, cloud 8, moving objects 4).
Round 5 (in-memory IR diagnostic, never source or output): a singleton-1 cut, moving the cloud block's Y definition
and coalescing the initial Y copy together give 8 masked bytes and 1229/1237 aligned instructions; the lift, cloud and
moving-object blocks and B47 then match. None of 20 C++ translations passed the IR gate (scratch/sol-build4/FINDINGS.md).

### C1XX IL emission ([EVIDENCE], worker sol-c1; docs/C1_IL.md)

C1XX (SP3) emits scalar reads, logical-not, typed assignments, comma expressions and whole aggregate arguments (unary entry
0x10415738, built-in builder 0x10410239, argument prepend 0x10412057, expression emitter 0x104048fb, record writer
0x10403296). The temp copies and definition placements that TreatEventBase and DrawButtons need are NOT C1 records: c2
materializes values, deletes source copies, extracts effects and expands aggregate arguments. So C1 record order alone
cannot produce either target; the missing source property must change what c2 sees (lifetimes, uses, blocks). 68 micros;
bounded negatives: bitfields, enums, constructors, copy constructors, conversion/assignment operators, inline draw
wrappers, varargs aggregates.

### Differential decompilation ([EVIDENCE], negative; worker sol-diffdec)

Ghidra 12.1 projects of the oracle and of our historical-host relink (579/579 seeded roots, identical signatures) were
decompiled and diffed for the four open functions. TreatEventBase and ChangePhase decompile identically despite their byte
differences: the decompiler normalizes away the register-lifetime differences that remain. The few DrawButtons/Build
pseudocode differences gave four candidates, all failed or inert (scratch/sol-diffdec/FINDINGS.md).

### ChangePhase: PLAY-first priority from condition order ([EVIDENCE], worker sol-alloc ir1-ir4)

An in-memory zero-only cut at the first late zero consumer gives 0 masked bytes (2029/2029). The source lever is allocator
priority: when WM_PHASE_PLAY's range is selected before zero's, the index's pressure cuts zero and excludes the committed
PLAY range. Reordering condition terms does this: `(m_phase == WM_PHASE_PLAY || phase != WM_PHASE_PLAY) && ... &&
!m_bDemoPlay` and `phase == WM_PHASE_PLAY || phase == WM_PHASE_GAMER` (canonical since this round): 4,668 -> 32 bytes,
2021/2029 aligned, 7 regions; zero in EDI, PLAY in EBX through WriteInfo, oldPhase late, in both release and eGames.
Remaining: the oracle tests m_bDemoPlay first in that guard ([ebp+0x9918] at +0xc5) and orders one 0x609 compare
differently. [OPEN] keep PLAY-first recosting with the oracle's guard order.

### TreatEventBase / DrawButtons: condition order is inert ([EVIDENCE], worker sol-irsearch round 2)

Function-wide search of real condition-order rewrites (adjacent pure operand swaps, rotations, comparison flips, negated
if/else arm swaps; 349 singles + 250 disjoint pairs, 599 full-TU captures) gated at 0x42a8aa. No candidate moved either
target signature or any tracked partial signature (TreatEventBase flag copy still absent, rank/position bindings eax/edi;
DrawButtons split-title x definitions still eax/eax with 11-tuple gaps). Unlike ChangePhase, these two residues are not
driven by allocator priority. Ledger: scratch/sol-irsearch/round2/.

### Open functions: toolchain, flag, line-layout and edition negatives ([EVIDENCE], supervisor + sol-build5, sol-cp2)

- Compiler trees: TreatEventBase/DrawButtons/ChangePhase/Build give 8/63/32/2328 differing bytes with SP3, SP2, NT5-1592,
  NT5-1691, SP3-deu and SP3-jpn c1xx/c2 (all distinct binaries); RTM is worse (TreatEventBase 11). Not a compiler build issue.
- Extra flags /FD, /Zi, /Z7, /YX, /G5, /Gf: identical residues; /Ob2 breaks all four. Project flags are not the cause.
- Source line layout (rank-0 block on one line, 7 blank lines inserted, whole file shifted 3 lines): identical residues.
- Editions (sol-build5): Build, TreatEventBase and DrawButtons have identical normalized instructions and registers in the
  retail, demo and eGames images, so edition differences cannot localize them.
- ChangePhase (sol-cp2, 27 captures): with the oracle's guard order, a conditional SetMission argument adds the one
  missing PLAY priority point (tie -66/-66, both edition allocation gates pass) but emits different code (5,576 bytes).
  Switch/else-if/recursion/snapshot/chained-reset forms are inert. Ledger: scratch/sol-cp2/.

### C1-record-level search ([EVIDENCE], worker sol-ilsearch)

New tool: scratch/sol-ilsearch/tools parses and re-serializes c1xx's postfix record stream (event.cpp: 7,950/7,309 records
for TreatEventBase/DrawButtons) and replays c2 alone on it; an unmodified replay reproduces the whole COFF object
byte-exactly. 1,769 record mutations (commuted operands, char/short/int/unsigned/bool conversions, not/equality/conditional
shapes, comma/assignment chaining, a written/read local, result reuse, statement order, x definitions per argument):
TreatEventBase best 8 (if/else seed) / 14 (expression seed), DrawButtons 63. Deleting both inner x=330 definitions gives
the exact first-call run but 897 bytes elsewhere. Local front-end changes in these blocks do not reach the targets.

### Negatives after SRC-03/SRC-04 ([EVIDENCE])

- Build background loop (first region: the oracle keeps `rect.bottom` = 0x1e0 in eax across the inner `DrawPart`
  loop and rematerialises it after the call): all 31 placements of `tinyPoint.y = 0`, `rect.top`, `rect.bottom`,
  `rect.right` in the outer loop are byte-identical (c2 normalises loop-invariant fields); `rect.bottom` inside the
  inner loop moves `rect.right` to ecx as in the oracle (3,273 bytes) but not the eax hold; `rect.bottom - rect.top`
  and `rect.right - rect.left` spellings are inert (build/tmp/rx/s_bbg.py).
- DrawButtons split title: font spellings (`i == m_fileIndex`, the NULL branch's `((m_fileIndex != i) - 1) & 2` in
  all three draws) and pos.x/pos.y statement orders are inert; placements of the `pos.x = 330` assignments either
  keep the m_pPixmap schedule difference or hold 330 in esi (build/tmp/rx/s_dbx.py).

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
