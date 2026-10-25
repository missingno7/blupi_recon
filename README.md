# Speedy Blupi and Speedy Eggbert: matching reconstruction and SDL3 port

This repository reconstructs the C++ source of Epsitec's **Speedy Blupi 1** (1998) and **Speedy Blupi 2**
(2001), and of their eGames editions **Speedy Eggbert 1** (1999) and **Speedy Eggbert 2** (2004). The
goal is source that the historical Visual C++ 5 toolchain compiles back into the original machine
code. The same sources also drive a native SDL3 port for Windows, Linux and Android.

- **Historical matching.** There are 14 executables (SB1 US/German/French, Eggbert 1 in six
  languages, the June 1998 SB1 demo as an evidence-only target, Blupi 2 English/German/French and
  Eggbert 2). Each is matched independently and verified byte by byte against its original:
  STRICT, FOLDED, or a separately certified EQUIVALENT tier.
- **Ordinary builds.** Every executable builds from its reconstructed source with its historical
  compiler and linker. No original executable or reverse-engineering tool is needed.
- **SDL3 port.** [port/README.md](port/README.md): the faithful 640×480 games, plus optional
  widescreen and smooth motion. They compile the historical game sources unchanged. Players use their
  own copy of the game data. Multiplayer is not available yet; it is a required upcoming milestone.

| Read | For |
|---|---|
| [docs/STATUS.md](docs/STATUS.md) | Current status of every executable and of the port: STRICT, EQUIVALENT, whole image, ordinary build, port fidelity |
| [docs/BUILDING.md](docs/BUILDING.md) | Building the historical executables (`tools/build.py --target …`) |
| [docs/VERIFYING.md](docs/VERIFYING.md) | Verifying against the originals, and the regression gate |
| [docs/research/README.md](docs/research/README.md) | Index of the preserved research (compiler studies, TU reports, linker work) |
| [AGENTS.md](AGENTS.md) | Binding rules for contributors and agents (port rules: [port/AGENTS.md](port/AGENTS.md)) |

## Quick start

```bash
.pyenv/python.exe tools/build.py --target sb1/blupi/us
.pyenv/python.exe tools/verify.py --target sb2/eggbert/en
```

- **`build.py`** builds one executable from its reconstructed source. It needs VC5 SP3/RTM and the
  DirectX 5 SDK under `C:\tools`, plus Python with pefile.
- **`verify.py`** verifies one target against its original. It also needs the original under
  `assets/` and Capstone.

## Repository layout

| Path | Contents |
|---|---|
| `src/` | Reconstructed historical source of the ten SB1 targets: Blupi, Eggbert 1 and the demo, with build-wide edition/language switches |
| `games/sb2/src/`, `games/sb2/blupi/` | SB2 source (Eggbert 2) and the Blupi 2 whole-file overlay; `games/sb2/res/` holds the SB2 resource scripts |
| `targets.json`, `recon.json`, `games/sb2/recon.json`, `toolchains.json` | Target registry, manifests (build settings and acceptance records), pinned toolchain hashes |
| `tools/` | `build.py` (ordinary build), `verify.py`/`relink.py`/`regression_gate.py` (verification), and research tools ([docs/research/README.md](docs/research/README.md)) |
| `oracle/`, `evidence/`, `tests/` | Oracle inventories, equivalence certificates, tests |
| `msvc5kb/`, `steering/` | MSVC 5 code-generation knowledge base; a steering demonstration (never canonical source) |
| `port/` | The SDL3 port: `common/`, `games/sb1/`, `games/sb2/`, its own build, docs and provenance map |

**Not in the repository:**

- `assets/`, the original games, which are immutable;
- `external/`, read-only reference repositories;
- `C:\tools`, the historical toolchains;
- `build/` and `scratch/`, generated output.

## Licence

The source in this repository is free software under the GNU General Public License, version 3
([LICENSE](LICENSE)). The reconstruction derives from Planet Blupi (GPLv3 or later) and the
Speedy Eggbert 2 reconstruction (GPLv3); see [src/PROVENANCE.md](src/PROVENANCE.md) and
[games/sb2/src/PROVENANCE.md](games/sb2/src/PROVENANCE.md). The port's third-party components keep their
own licences ([port/third_party/README.md](port/third_party/README.md)).

Speedy Blupi, Speedy Eggbert and their data (graphics, sounds, music, levels) are © Epsitec SA. They are
not part of this repository or of its licence; the port's icon (`port/res/`) is cut from the game's own
Blupi sprite. This is a fan project, not affiliated with Epsitec or eGames.
