# SB2 resource strings

These four packs contain the original string resources, read statically from:

| Pack | Source |
|---|---|
| eggbert-en | `assets/eggbert2/Eggbert2.exe` |
| blupi-en | `assets/blupi2/BLUPI.EXE` |
| blupi-de | `assets/blupi2_variants/deutsch/Blupi.exe` |
| blupi-fr | `assets/blupi2_variants/francais/Blupi.exe` |

Extraction uses `strings()` and `quote()` from `port/tools/language_pack.py` (the same
resource parser as SB1); executables are never run. The equivalent CLI is:

```powershell
.\.pyenv\python.exe port/tools/language_pack.py assets/blupi2/BLUPI.EXE --game sb2 --edition blupi --code en --name English --variant en -o build/sb2-blupi-en.txt
```

[EVIDENCE] SB2 Blupi en/de/fr have identical code and character tables (docs/MULTI_GAME.md).
All packs therefore use `variant=en`, with no character overrides. Edition and installed
language fingerprints (file size and FNV-1a only) live in `game/language.cpp`; the title
image identifies the edition even when the original executable is absent. Built-in
strings retain the originals' bytes through the common Windows-1252 pack encoder.
