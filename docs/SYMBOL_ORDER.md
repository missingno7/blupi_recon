# Declaration context and VC5 expression ordering

Why adding or moving unrelated declarations changes code in VC5 SP3, and how to predict it. Worker sol-symnum, 2026-10-18;
full model, captures and every prediction: scratch/sol-symnum/{MODEL.md,PREDICTIONS.md} (ignored; reproduction commands
there). Tool: `tools/c2trace/ctxpredict.py`. KB rules BLUPI-CTX-03 and BLUPI-CTX-04.

## Mechanism ([EVIDENCE])

1. **Producer.** C1XX.DLL (SP3, image base 0x10400000) keeps a next-symbol-ID counter at VA `0x104ccb58`. Hardware
   watchpoints show each new front-end entity taking the old value into the low 24 bits of its record (+0x18), e.g. at
   0x1040a74d, 0x10421a2f, 0x104242a2 and, for local statics, 0x1041b8d4. The IDs travel in the `sy/gl/ex` IL records;
   c2 reads them at 0x420250 and keeps them at symbol +0x28. c2 continues numbering its own symbols from the
   serialized front-end end (0x452f1c, 0x491050).
2. **Consumer.** c2 gives every expression a 32-bit key (cached at tuple +0x0c): `K(E) = (cost(E) << 16) | H(E)`
   (0x405301). The 16-bit hash (0x4053bf) is
   ```text
   F(s)                 = (s & 0xffff) XOR (s >> 16)
   H(symbol leaf)       = F(symbol ID)
   H(integer constant)  = F(low32 XOR high32)
   H(address)           = F(displacement) XOR opcode XOR H(base)
   H(operator)          = opcode XOR H(input_1) XOR ... XOR H(input_n)
   H(copy of E)         = H(E);  H(temp without E) = temp descriptor number
   ```
   Paired operands are ordered by an unsigned descending comparison of their keys (0x408d58, used by the stable merge
   sort at 0x40922a/0x409306 and the operand-swap path at 0x4058dc).
3. **Windows.** For two equal-cost expressions sharing a symbol contribution `P(N) = F(S+N)` (N = declarations added
   before the shared symbols), `H(A) = P XOR C_A`, `H(B) = P XOR C_B`. Only bit `b = highest_set_bit(C_A XOR C_B)` of
   `P(N)` decides the order, so the code flips in windows of width `2^b` with period `2^(b+1)`. That is why probe
   sweeps showed contiguous windows with sharp edges.
4. **Several decisions per function.** A later pass can re-sort with different keys (fixed temp numbers replace
   symbols). MoveObjectSort's first comparison predicts false holes at N=114-116; the emitted order follows the later
   key pair `F(S)^0x1fe` vs `F(S)^0x111` (bit 0x80 of `F(SortGetType ID)`).

## Validation ([EVIDENCE])

60/60 held-out predictions (56 function decisions, 4 static suffixes), each registered before compiling:
- MoveObjectSort (old include order, DX5): good at N = 109-236 and 365-492; the earlier 109-141 / 464-480 sweep
  only covered parts of these windows (N=237, 364, 493, 496 fail as predicted).
- MockeryDetect (`tinyPoint`, bit 0x100): good through 141, bad 142-397, good from 398.
- IsDeepWater (bit 0x40) and IsVentillo (bit 0x100) windows in the DX6/sol-macros states; IsVentillo ends after
  105 and the 44-extern removal extends it.
- CButton::Create's local static `ttypes` suffix is the front-end ID itself (34693; front-end end 34838). Removing the
  enum, operators and helpers subtracts 443/62/12 from both, as predicted.
- Two independent micro TUs (msvc5kb/micros/symnum/micro_grid.cpp, micro_calls.cpp) with held-out transition points.
[OPEN] One older fixture (IsDeepWater "six externs") predicts five; its exact historical source state is not
identified.

## Declaration costs ([EVIDENCE], replaces the Blood II approximation for SP3 /GX /O2)

extern / global / typedef 1; prototype with no parameters 1, with two parameters (named or unnamed) 3; enum with three
enumerators 4; empty or two-field struct 7; opening an empty guarded header 1; unused local 1; unused inline
`int f(){return 0;}` 2, with one parameter 3. Initial transfer of the Blood II costs hit 12/15.

## Use

- `tools/c2trace/ctxpredict.py src/X.cpp Function [--capture NAME] [--delta N]` makes one capture and one c2 replay
  and prints each context-sensitive comparison with the current N window in both directions. It replaces probe sweeps
  for "is this residue a declaration-count effect, and how far am I from the edge?".
- It assumes unchanged expression topology, cost bits and allocation history; a source edit that changes expression
  shape needs a new capture. It predicts comparator decisions, not strict bytes.
- To fix a context residue authentically, look for real declarations or include orders that move the decisive symbol's
  ID into the good window (the MoveObjectSort include move was 137 numbers, inside 109-236). Never add probes or dummy
  declarations.
- Portable to other VC5 SP3 projects (same c1xx/c2 build); VC6 needs re-verification of the addresses.

## Force diagnostic ([EVIDENCE])

`tools/c2trace/keyforce.py` (worker sol-keyforce; method from blood2_recon's keys/force) overrides one 0x408d58 comparison
inside the c2 child and compares the forced object with the oracle (diagnostic only, never source). It classifies residues
(KB BLUPI-CTX-05). On the open SB1 functions, 283 forces:
- TreatEventBase: all 53 comparisons keep the 8 bytes, so the residue is not a key-order decision.
- ChangePhase: 64 residue-local comparisons are byte-inert.
- Build: only collateral gains, requiring ID shifts of about 31,600, so no genuine source.
- DrawButtons: comparison #1310 needs F(line2) < F(text); moving the real `line2` declaration before `text` reproduces
  that order, but the function gets worse (63 -> 755 bytes, frame 0xe4 -> 0xe0), so it is rejected.
blood2_recon reached the same hash mechanism independently (its VC5-RA-06, VC5-CTX-10) and adds VC5-RA-08: inline
expansion renumbers the callee's symbols through the 1024-bucket table (id & 1023), so inline history before a site matters.
