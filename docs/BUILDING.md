# Building the historical executables

How to compile and link every reconstructed historical executable as an ordinary C++ project, with its
historical toolchain. This is the **ordinary build**. Two other operations are separate and documented
elsewhere:

| Operation | Reads | Answers | Command / document |
|---|---|---|---|
| Ordinary build | sources, resource scripts, `targets.json`, manifest build sections, `toolchains.json`, external VC5/DX5 | "does it compile and link?" | `tools/build.py` (this page) |
| Historical verification | the original executable, `oracle/`, certificates, matching tools | STRICT / FOLDED / EQUIVALENT, image layout | `tools/verify.py`, `tools/relink.py` (docs/WORKFLOW.md) |
| Runtime and port fidelity | game data, test infrastructure | "does it behave like the original?" | docs/RUNTIME.md, port/docs/FIDELITY.md |

A linked executable from `tools/build.py` is **not** claimed to be byte-identical to the original, and it
is **not** claimed to be behaviourally verified. The build never runs it.

## Requirements

All of these are external, and none is vendored.

| Dependency | Where | Used for |
|---|---|---|
| Visual C++ 5.0 SP3 | `C:\tools\msvc5_sp3` | compile (SB1 Blupi, Eggbert 1, Eggbert 2) and link (SB1 Blupi, Eggbert 2) |
| Visual C++ 5.0 RTM | `C:\tools\msvc5_rtm` | compile (Blupi 2, SB1 demo) and link (Eggbert 1, Blupi 2, demo) |
| DirectX 5 SDK | `C:\tools\dx5sdk` | `ddraw.h`, `dsound.h`, `dplay.h`, `dsound/ddraw/dplayx.lib` |
| Python 3.11 + pefile | `.pyenv\python.exe` (repo-local, ignored) | the build driver; pefile for the static image report |
| Windows | host | VC5 is Win32; any 64-bit Windows runs it |

Expected component hashes are pinned in `toolchains.json`:

- **compile:** CL, C1XX, C2, MSPDB50
- **resources:** RC, RCDLL, AFXRES.H
- **link:** LINK, CVTRES, MSPDB50, MSDIS100
- **libraries:** KERNEL32, USER32, GDI32, WINMM, LIBC, OLDNAMES
- **DX5:** the headers and import libraries listed above

[EVIDENCE] CL.EXE is the same file in both trees (11.00.7022). The compiler release is identified by
C1XX/C2: SP3 is 11.00.7303 and RTM is 11.00.7022. Never select a compiler by its banner.

The build refuses a component that differs from its pin. `--allow-unpinned-toolchain` overrides this and
records it as DEGRADED.

**Not needed:**

- `assets/` or any original executable;
- `oracle/`, `evidence/`, `external/`;
- Capstone, Ghidra;
- any research tool.

`tests/test_build_boundary.py` enforces this. It checks the import closure, and it plans every target
under an audit hook that records each file opened and each module imported.

**Historical `__FILE__` paths.** `sound.cpp` embeds its build path through `TRY_DS`
(docs/BUILD_PATH.md):

- SB1 builds use `F:\Profil\sound.cpp` (a temporary drive mapping), so drive F: must be free.
- Blupi 2 uses `D:\Blupi\Profil\sound.cpp`, and Eggbert 2 uses `D:\Blupi\Profil egames 2\sound.cpp`.
  `tools/buildpath.py` creates the folder for one compile and removes what it created.

If the path cannot be established, the TU compiles at its real path. The build then reports
`DEGRADED` and records `UNAVAILABLE` in `build.json`. That executable embeds different data, and it
never counts toward acceptance. Inside the Codex sandbox the mapping is denied.

## Commands

```bash
.pyenv/python.exe tools/build.py --target sb1/blupi/us
.pyenv/python.exe tools/build.py --target sb1/eggbert/en
.pyenv/python.exe tools/build.py --target sb2/blupi/en
.pyenv/python.exe tools/build.py --target sb2/eggbert/en
.pyenv/python.exe tools/build.py --all
.pyenv/python.exe tools/build.py --list
.pyenv/python.exe tools/build.py --plan --target sb2/blupi/de
.pyenv/python.exe tools/check_clean_build.py --compare build/standalone
```

- `--all` builds the 14 targets one after another.
- `--list` prints each target's configuration.
- `--plan` resolves inputs and hashes the toolchain without compiling.
- `check_clean_build.py` runs the clean-checkout test.

Without `--target`, the primary `sb1/blupi/us` is built. Each target writes only to its own directory,
`build/standalone/<target-slug>/`, which the build recreates; there are no cached objects. The
directory holds:

| File | Contents |
|---|---|
| image | the linked executable |
| `.map`, `link.rsp`, `link.log` | link outputs and inputs |
| `objects/`, `logs/` | compiled objects and per-TU compile logs |
| `.res` | compiled resources |
| `sources-<id>/` | the staged effective source tree, for overlay targets only |
| `build.json` | the build record |

`build.json` records the following:

- the registry and manifest sections read;
- every source input and its hash, and the compile commands;
- the toolchain components actually loaded, with their hashes and pin status;
- the resource script and how it was chosen;
- the link order and its provenance, the arguments and the library hashes;
- the historical-path status;
- the image hash, with and without timestamp fields, and its timestamp offsets;
- any degradation.

## Targets

These are fresh results from 2026-10-25, on commit `a4a2352`.

| Target | Compiler / linker | Source root + overlay | Resources | Output | Image, digest w/o timestamps |
|---|---|---|---|---|---|
| sb1/blupi/us | SP3 / SP3 5.10 | `src` | `src/res/blupi.rc` | `build/standalone/sb1-blupi-us/BLUPI.EXE` | 535,552 B, `4dc0a26fdac5…` |
| sb1/blupi/de | SP3 / SP3 | `src` | `src/res/sb1-blupi-de.rc` | `…/sb1-blupi-de/BLUPI.EXE` | 538,112 B, `81ac12da19bc…` |
| sb1/blupi/fr | SP3 / SP3 | `src` | `src/res/sb1-blupi-fr.rc` | `…/sb1-blupi-fr/BLUPI.EXE` | 538,112 B, `b46c229f5259…` |
| sb1/eggbert/en | SP3 / RTM 5.00 + RTM libs | `src` | `src/res/sb1-eggbert-en.rc` | `…/sb1-eggbert-en/egames.exe` | 534,528 B, `e9f1ed17058b…` |
| sb1/eggbert/de | SP3 / RTM | `src` | `src/res/sb1-eggbert-de.rc` | `…/sb1-eggbert-de/EGAMES.EXE` | 537,088 B, `24bb3764378c…` |
| sb1/eggbert/es | SP3 / RTM | `src` | `src/res/sb1-eggbert-es.rc` | `…/sb1-eggbert-es/EGAMES.EXE` | 537,088 B, `dcebd0440ca3…` |
| sb1/eggbert/fr | SP3 / RTM | `src` | `src/res/sb1-eggbert-fr.rc` | `…/sb1-eggbert-fr/EGAMES.EXE` | 537,088 B, `478c849704cc…` |
| sb1/eggbert/it | SP3 / RTM | `src` | `src/res/sb1-eggbert-it.rc` | `…/sb1-eggbert-it/EGAMES.EXE` | 536,576 B, `09e374d42562…` |
| sb1/eggbert/pt | SP3 / RTM | `src` | `src/res/sb1-eggbert-pt.rc` | `…/sb1-eggbert-pt/EGAMES.EXE` | 536,576 B, `04669c836830…` |
| sb1/blupi/demo (evidence only) | RTM / RTM | `src` (`_DEMO`) | `src/res/blupi.rc`, **PARTIAL** | `…/sb1-blupi-demo/Blupi.exe` | 534,016 B, `8c747a876493…` |
| sb2/blupi/en | RTM / RTM | `games/sb2/src` + `games/sb2/blupi` | `games/sb2/res/sb2-blupi-en.rc` | `…/sb2-blupi-en/BLUPI.EXE` | 587,264 B, `9fb816dc9659…` |
| sb2/blupi/de | RTM / RTM | `games/sb2/src` + `games/sb2/blupi` | `games/sb2/res/sb2-blupi-de.rc` | `…/sb2-blupi-de/Blupi.exe` | 589,824 B, `91b0908a5483…` |
| sb2/blupi/fr | RTM / RTM | `games/sb2/src` + `games/sb2/blupi` | `games/sb2/res/sb2-blupi-fr.rc` | `…/sb2-blupi-fr/Blupi.exe` | 589,312 B, `e3e06462b1f0…` |
| sb2/eggbert/en | SP3 / SP3 | `games/sb2/src` | `games/sb2/res/sb2-eggbert-en.rc` | `…/sb2-eggbert-en/Eggbert2.exe` | 602,624 B, `6014656da4cf…` |

Every target's historical `sound.cpp` path was established, and no target was degraded. The
component hashes, abbreviated:

| Release | C1XX | C2 | LINK | CVTRES | LIBC |
|---|---|---|---|---|---|
| SP3 | `e27df3bb` | `e75aecaf` | `04c89298` | `16a45886` | `3b5f43e9` |
| RTM | `fea1e9fb` | `6d1406af` | `27954480` | `0f82167c` | `4ae22c6f` |

Full values are in `toolchains.json` and in each `build.json`.

[EVIDENCE] With the RTM linker, Eggbert 1 links against RTM's libraries. It loads MSPDB50/MSDIS100
from the SP3 `SHAREDIDE\BIN`, which comes first on PATH. This is the current reproducing link
environment, shared with `tools/relink.py`; `build.json` records the DLLs actually resolved.

**What each target's configuration means:**

- **Defines and the compiler tree** come from `targets.json` (for SB1, docs/EGGBERT.md).
- **The linker tree** is `link_tree` in `targets.json`. It is set for the six Eggbert 1 targets and
  otherwise follows the compiler tree.
- **Base, alignment and object order** are each manifest's `standalone_link`. They are chosen
  ordinary-build settings: the object order is the text order of the originals' TU anchors.
  [EVIDENCE] They are not a claim of the original linker inputs, TU population or source layout
  (docs/SB1_CLOSURE.md). Eggbert 1's originals place `menu` before `jauge`; the shared SB1 order is
  kept for all SB1 targets.
- **Blupi 2's overlay** is staged by `tools/source_tree.py`, so that quoted includes resolve to the
  edition's headers. The logical TU names stay in `games/sb2/src/`.
- **The SB1 June demo** builds with the **retail** SB1 resource script. Its resource section is
  therefore not the demo's own. `build.json` marks it PARTIAL, and the target stays evidence-only
  (docs/SB1_DEMO.md).

## Reproducibility

[EVIDENCE] 2026-10-25, `tools/check_clean_build.py --compare build/standalone`, on commit `a4a2352`.

- **The export.** `git archive` exported the commit, pruned to `src/`, `games/`, `targets.json`,
  `recon.json`, `toolchains.json` and five tool modules: build, targets, source_tree, msvc, buildpath.
  The export had no `assets/`, `oracle/`, `evidence/`, `build/`, `scratch/` or `.pyenv` copy.
- **All 14 targets built** with the documented `tools/build.py --all`. Every toolchain component
  matched its pin, and every historical path was established.
- **No output named a path of the main checkout,** apart from the interpreter.
- **Every image equals the main checkout's independent build apart from its timestamp fields.**
  The compiled section contents of every object are equal too.
- **Report:** `build/clean-export/report.json`.

What varies between builds:

| Difference | Cause | Handling |
|---|---|---|
| PE `TimeDateStamp` | LINK 5.x writes the current time and has no option to fix it | Recorded. `build.json` gives a digest with the PE and resource-directory timestamp fields read as zero; the file itself is never modified |
| Resource-directory timestamps (RTM CVTRES) | Same: the current time | Same digest; SP3 CVTRES writes 0 |
| Object `TimeDateStamp`, and the COFF `.file` record (absolute source path) | VC5 | `sections_sha256` per object covers code and data only. The linked image does not contain them |
| Unavailable historical `__FILE__` path | Host or sandbox | `DEGRADED`; different `.data`; never accepted |
| Original linker inputs, TU population, link order | Underdetermined by the evidence (docs/SB1_CLOSURE.md) | Not modelled by the ordinary build; it is not the original image |

**A fixed timestamp.** A historical timestamp is reproduced only on the verification side:
`tools/relink.py --historical-host` freezes the linker's clock at the original executable's own
timestamp, through a debugger launcher (docs/HISTLINK.md). The ordinary build does not do this,
because that needs the original's timestamp and research tooling, and it never patches the timestamp
afterwards.

## Remaining limitations

- [OPEN] **No byte-identical whole image is claimed for any target.**
  - SB1 US: the verification relink of the same sources is `557ca92e…`, which still differs from the
    original (docs/SB1_CLOSURE.md).
  - SB2: `.data` virtual size +32 (docs/SB2.md).
- **The toolchain roots are fixed under `C:\tools`.** There is no override yet.
- **The compile locks live in each checkout's `build/locks`.** Two checkouts building at once are not
  serialized against each other, so build one at a time.
- **The demo's resources** are partial (see above).
