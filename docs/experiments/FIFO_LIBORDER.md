# Experiment: FIFO input + alternative library order (sol-linkjoint r2, 2026-10-19)

Status: experimental evidence only; NOT the canonical historical link (user decision 2026-10-19).

Inputs: the 19 canonical game objects at commit fbd0ec0 (tools/verify.py build), in established order, plus
`fifo.cpp` / `fifo.h` byte-identical to `external/planetblupi` at commit b0270ab
(SHA-256 91631ecf06d5aab141272c056804c3e698ca115bcb98264739e36dd09d85084d /
47ddc722a7108b86af873aaf2f6b1a90bdcf00137fb866f3bec81643d2ad9061), compiled with tools/msvc.py and the manifest flags.
Libraries in this order: kernel32 user32 gdi32 winmm dsound LIBC OLDNAMES ddraw dplayx (DX5 SDK import libraries).
Linker: LINK 5.10.7303 through tools/relink.py --historical-host --diagnostic (historical MSVCRT, 1998 clock).
Exact LINK response file: docs/experiments/fifo-liborder-link.rsp (paths are the worker's private build directory).

Result (static comparison with the oracle): empty-body ICF owner CMenu::~CMenu at 0x1f010 and filter owner ehvecdtr
(both as the oracle); entry 0x24d30 (oracle 0x24d40); first CRT function 0x238a0 (oracle 0x238b0); Rich (0,269)
(oracle 270); oracle IAT positions 15/128; 40,288 differing bytes (.text 39,956, .rdata 315, .data 15, .rsrc 0; header
0x94 and 0xd8) versus 121,401 for the canonical link. Image SHA-256
6e3222622aa41bb2016348631367138efbd913dbd196aa7edd5cb143e4a296ef. Full gate: zero accepted regressions.

Full artefacts (ignored scratch): scratch/sol-linkjoint/r2/ (FINDINGS.md, actions.json, relink-report.json,
selected-config.json, model and ICF permutations). See docs/LINK_ICF.md 'Duplicate-copy survivors, bounded round'.
