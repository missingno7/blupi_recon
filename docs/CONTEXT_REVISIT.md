# Broader-context residual checks (2026-10-20)

[EVIDENCE] No new STRICT match. Canonical sources, accepted entries and the two
EQUIVALENT certificates are unchanged. Coverage remains 127,456 / 141,488 bytes.

[EVIDENCE] The later user-supplied old-dos.ru link resolves the SP1 compiler
inventory gap: full SP1 C1XX/C2, LINK and LIBC are byte-identical to the tested
SP2 components. See "Full SP1 obtained" below. This yields no new compiler build
to test and no new STRICT match.

## Why these controls

[EVIDENCE] Earlier studies tested 1,211 declaration/type variants, 103 selected
pairs, C1 symbol ordering, preceding-function order, per-function reset state,
allocator costs and several compiler builds. Those general families should not
be repeated. A compiler intervention giving oracle bytes is sufficient diagnostic
evidence, not a unique identification of the historical source or an exhaustive
proof that all alternative causes have been excluded.

[HYPOTHESIS] Ordinary network/message source forms or duplicated source tails
could change temporary identities before final code merging. Assignment chains
and reference lifetimes could change the phase dispatcher's pooling graph.

## Forty full-event-TU source controls

[EVIDENCE] `scratch/context-20261020/probe.py` compiles serially through
`tools/msvc.py`, using manifest flags, defines and SDK, one fixed candidate path,
and canonical headers. Its source control reproduces all 94 existing function
payloads and symbolic relocations. Forty successful controls comprise three
baseline/seed controls and 37 variants:

| Family | Controls and result |
|---|---|
| Receive/network prefix | Array decay, named result, nested/committed guard, packet casts, cached message/id, payload pointers, while/break/independent-slot loop forms: unchanged residue or additional differences |
| Input context | Earlier fwKeys definition, explicit input/result locals, saved key, unsigned fwKeys, compound key-bit assignments: unchanged; boolean-addition cheat character adds code |
| Shared source tails | Duplicate all 13 DemoRecEvent tails, keydown-only or keyup-only tails, quit-playtest tail, and both together: identical function bytes |
| Normalized BOOL feedback | Unsigned cast is inert; positive comparison, bit mask and double comparison add differences |
| ChangePhase, two seeds | Chained zero stores, shared zero local, assignment in the index test, phase reference; repeat with canonical and oracle guard/WriteInfo orders: no useful gain |

Canonical/oracle phase controls stay at 32/4,668 non-relocation byte differences.
The reference variant gives 279/4,664 respectively: a small change within a badly
mismatching function, not a useful partial match. It is not promoted. Other
zero/index forms are byte-inert. TreatEventBase controls retaining its current
code retain the same eight register-byte differences at the same offsets.

Some controls renumber generated COFF symbols without changing function bytes;
the ledger distinguishes these. Results, exact candidate patches and objects are
in `build/context-20261020/`. Masked byte scores never grant STRICT acceptance.
Two candidate-generator errors were corrected before counting their successful
compiles. CL initially failed in the sandbox with error 623; successful compiles
ran outside it. No game executable was run.

## VC4.2 fingerprint

[EVIDENCE] The earlier VC5-versus-VC6 fingerprint omitted VC4.2. Four original
compiler files were retrieved individually from Microsoft's
[VC4.2 media](https://archive.org/details/msft-visual-cplusplus-42), without an
installer or full ISO download. Media SHA-1 from archive metadata:
`e8b64d5f25d3d58891a62722cfbec371f870823c`. Member paths are `MSDEV/BIN/`;
hashes/imports are in `build/context-20261020/vc42/provenance.json`.

| Component | SHA-256 |
|---|---|
| CL.EXE | `c5bf7ad84482e8a54d5753fcbd3e648d8a1192f5ca8b8cf1f5d23b651750585f` |
| C1XX.EXE | `9e0782ec157b30a387ca855374bc4c1b8a605dfb12364425497ba431541a5bf9` |
| C2.EXE | `674fb9e410481378c6980c3f21914e513128c85001e8337aca73b587b6273ae9` |
| MSPDB41.DLL | `6cab17cfcbc5a6317ab030a0db99164cafdfd1f360baa36186849237ffb25858` |

The full event TU compiles with CL 10.20, pinned SP3/DX5 headers and manifest
flags. TreatEventBase has a 6,785-byte section instead of 7,072, 5,189 differing
non-relocation bytes, first difference at entry. ChangePhase's section is 7,011
instead of 6,928. Neither target improves with these inputs. This does not exclude
every VC4 release or hypothetical source/header combination.

## LINK 7351 and the host CRT

[EVIDENCE] CE 200 LINK, file version 5.11.7351, was inventoried but absent from the
recorded controlled matrix. SHA-256:
`41dca4f3638aa843591a89c172737c5a22289eb7ed08c6fe868a0deb3848109e`.
It emits PE linker version 5.10 and a Rich header. Therefore only SP3 among local
tools fitting those coarse properties is no longer a supported claim.

With frozen verified objects and the system CRT, SP3 and 7351 produce identical
payloads for all four PE sections. The measured 7351 link has 26,296 whole-file
differences versus the original; the retained historical-CRT SP3 baseline has
121,401. This apparent gain is a host-CRT/ICF effect, not a linker-version gain.
The original IAT order and ICF choices remain unmatched. This round did not run
7351 under the historical CRT; SP3-specific debugger offsets require checking
before reuse on it. These links do not change compiler-body matching.

## Initial SP1 lead

[EVIDENCE, initial search] VC5 SP1 was absent from the identified compiler matrix. Microsoft's
[Q167967](https://ftp.zx.net.nz/pub/mirror/ftp.microsoft.com/MISC/KB/en-us/167/967.HTM)
records an optimizer fix in SP1. Authenticated SP1 C1/C2 files are worth
fingerprinting if obtained. This does not prove their bytes differ from tested
RTM/SP2 components or that SP1 explains either residue. No authenticated SP1
compiler package was located in the initial round; linker metadata is not a substitute.

## SP1 acquisition follow-up

[EVIDENCE] A public, Microsoft-signed **SP1 Core** package was subsequently found
on [io Programmo 8](https://discmaster.textfiles.com/browse/30619/IOPROG_8.ISO/soft/vssp1.exe).
Only the 14,033,840-byte member was downloaded, without its entire CD. Windows
`Get-AuthenticodeSignature` reports **Valid**, signer Microsoft Corporation,
certificate thumbprint `205E4843ABAD547771BD8D1A3CE0E59DF5BD25F9` (checked 2026-10-20).
The SFX was parsed statically; no installer or package executable ran.

| Input | SHA-256 |
|---|---|
| Signed `vssp1.exe` | `1ca459a484085b4303fcbd119e02c2368558a26eb30fb405bdecd70266a41425` |
| Embedded cabinet, 13,926,908 bytes | `52e2a03a0ff18807ef607c1f1ac0e9c68d84a653fd9182364ff1678f0de53f3d` |
| `VS97_SP1/setup.inf` | `49fc40a9534ea98903ee773ec42386016a58f0d5227c935bafa77fee09f62ca3` |

[EVIDENCE] The cabinet has 124 files and contains **none** of `cl.exe`, `c1.dll`,
`c1xx.dll`, `c2.exe` or `link.exe`. The `[vc]` install manifest likewise omits
them. Core supplies shared IDE/runtime updates, so it cannot supply the missing
compiler control. The readme describes compiler fixes also available in the full
distribution; that description alone does not establish this download's contents.
An initial [PatchWork 3/97 copy](https://archive.org/details/pw-397-all) has a
different SFX and Windows reports `HashMismatch`; its embedded cabinet is identical.
Use the independently verified io Programmo package as the retained source.

[EVIDENCE] Existing SP2 C1XX/C2 PE timestamps are respectively
`1997-05-30 00:23:29 UTC` and `1997-06-05 01:37:44 UTC`, earlier than the verified
Core SFX's July 2 timestamp. [HYPOTHESIS] The tested SP2 pair may already contain the full
SP1 compiler components. Dates alone did not prove component equality; full SP1
was still unverified at this stage, with no new compile or STRICT gain.
Public searches and period-CD inventories did not locate its compiler payload.

Local reproduction: `scratch/context-20261020/inventory_sp1.py`; signed download,
signature receipt and all 124 member records: `build/context-20261020/sp1/`.
The inventory reads CAB headers directly and asserts the pinned hash and missing
components. Downloads and search results remain ignored; installed tools and
canonical game sources are unchanged. Search receipts are compacted in
`build/context-20261020/sp1-search.zip` (1.26 MiB); duplicate cabinets and the
unverified SFX were moved to `to_delete/recovery-sp1-20261020/`.

## VS97 Professional retail-disc check

[EVIDENCE] The user-supplied [Disc 1 archive](https://archive.org/details/microsoft-visual-studio-97-professional-edition-disc-1)
and its Disc 2/3 companions were inspected through their full ISO member indexes.
Disc 1 contains the original Visual Basic tools; Visual C++ is on
[Disc 3](https://archive.org/details/microsoft-visual-studio-97-professional-edition-disc-3),
ISO SHA-1 `a3ade9b0681ffab34f058a7991f6b7aa25d342e1`. None of the three indexes
lists an SP1 update package. The four downloaded members under
`DEVSTUDIO/VC/BIN/` (`CL.EXE`, `C1XX.DLL`, `C2.EXE`, `LINK.EXE`) are byte-for-byte
identical to the installed VC5 RTM tools, including the hashes in
[TOOLCHAIN.md](TOOLCHAIN.md). This supplies no new compiler control; no compile
was needed and no downloaded program was executed. Metadata, member indexes
and per-file download URLs/hashes are retained under `build/sp1-hunt-20261020/`.

[EVIDENCE, earlier search] A [full SP1 catalogue entry](https://www.betaarchive.com/database/view_release.php?uuid=5085ccaa-890c-46f5-b25d-42b32418d5fb)
lists `media_iso.rar` (214.10 MB), with download requiring login and original-media
provenance marked unknown. It has not been obtained or authenticated. Public
alternative-name/date-filtered file-index searches did not supply the compiler
payload. Microsoft's original download-page lookup was rate-limited; that is an
unavailable search route, not evidence that the package does not exist.

## Full SP1 obtained

[EVIDENCE] The user supplied [old-dos.ru file 2693](http://old-dos.ru/files/file_2693.html).
Its `VS97_SP1_1997_05` entry, version `5.0 SP 1`, downloads from
[`dl.php?id=7415`](http://old-dos.ru/dl.php?id=7415). The response supplies filename
`VS97_SP1_1997_05.ver.5.0 SP 1.English.rar`, 75,069,165 bytes, SHA-256
`c24ad1b63aa7464d01d0f06c80038b8120e3710c0a237df543684b1dcb78e91c`.
Unlike Core, this archive includes `ENU/VC/BIN/C1XX.DLL`, `C2.EXE`, `LINK.EXE`,
CRT libraries, headers and the full install manifest. The manifest explicitly
installs these compiler components. Its English readme is byte-identical to the
readme in the independently signature-verified SP1 Core package.

| Full SP1 component | File version | SHA-256 | Comparison |
|---|---|---|---|
| C1XX.DLL | 11.0.0.7149 | `8b66d3f14035bfa228e79d45481318457594f65ed91f87319d23932372857d8b` | identical to installed SP2 |
| C2.EXE | 11.0.0.7153 | `592c65eea2e159a8b7bf61fb20ed12fc0dfdb3c5b7179267d34634aa7a2dc6e4` | identical to installed SP2 |
| LINK.EXE | 5.2.0.7132 | `e28424d3eefcdd96ecc8c3fe38d0fad3d33077c62026f7774eda90784d0eb4d9` | identical to installed SP2 |
| LIBC.LIB | not applicable | `f81440dd014ca4ce7ef8b4d60ed1ee4ad544601b07fbdb4fca54a1dfca31f9b0` | identical to installed SP2 |

[EVIDENCE] C1.DLL and MSPDB50.DLL also match installed SP2 exactly. Thus the
previously hypothesized SP1/SP2 compiler-component identity is now established
by complete-file comparisons for this English package. The already recorded
SP2 compiler controls exercise these exact C1XX/C2 bytes; recompiling identical
components would add no new compiler-version evidence. This closes the SP1
compiler inventory gap without changing either EQUIVALENT function or strict
coverage. It does not establish every SP1/SP2 file or header as identical.

[EVIDENCE] The extracted compiler/linker files have no Authenticode signature;
the outer RAR is a repackaged directory tree, not an authenticated original CD
image. Its evidence is the package manifest, matching signed-Core readme and
exact identity to the independently installed SP2 files. The page's month label
is not used as a component build date. No installer or downloaded executable was
run. Installed toolchains are unchanged.

Download receipt, the archive, ten selectively extracted members, hashes, PE
versions/imports and signature results are under
`build/sp1-hunt-20261020/old-dos/`. `components.json` records exact installed-file
comparisons. Full manifest SHA-256:
`c2329ea11dfc9676d1112ef5993d2c2744b1d49db6467f20e15503c0f847a34b`.

## Remaining source lead

[OPEN] Original event source/project inputs or a source witness with the actual
rotor-neutral BOOL shape offer stronger evidence than further spelling sweeps.
Old-flag materialization and phase cost constraints remain the best local
explanations, but their historical source form is not recovered. An unmodelled
lowering/cost input remains possible.
