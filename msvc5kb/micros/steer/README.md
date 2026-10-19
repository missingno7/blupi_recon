# Steering lifetime micros (diagnostic only)

Each file is one translation unit; every function is a variant of `V00` with one construct inserted.
Capture with `tools/c2trace/c2trace.py capture-micro NAME file.cpp`, read with `tools/c2trace/lifetime.py NAME V00`.
Results and classification: docs/STEERING.md (KB BLUPI-STEER-01, BLUPI-NEG-05).

- lifetime_m1/m2: unreachable branches, unused calculations, self-assignments, declarations, dead calls.
- lifetime_m3: store-first cheat-window shape; duplicated terminator and toggle tests.
- lifetime_m4/m5: named select destinations (forms and widths/types).
- lifetime_m6: identical arms, phi merges, ternary, one-pass loop, goto, switch.
- lifetime_m7: dead calls to extern and dllimport functions (object symbols).
