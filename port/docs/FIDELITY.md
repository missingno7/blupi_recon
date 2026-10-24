# Historical source and SDL3 fidelity

The reviewed [provenance map](../provenance.json) connects the historical logical
sources to the port. It records an inventory and bounded evidence, not matching
acceptance or universal semantic approval. The initial review is the 472-row
`scratch/w-fidelity` audit. Its open obligations remain open. F297 was subsequently
fixed by `c68e167` (both SB2 editions, all four back/decor depth combinations).

Compiler-only or equivalent historical changes need no port edit, but need a
recorded review explaining why. Gameplay algorithms and newly discovered edition
differences must reach the port. Platform changes must preserve the used contract
in the replacement, or record a justified intentional difference and its scope.
Direct compilation removes copy synchronization; it still needs context/runtime
regression review. Do not move compiler steering into SDL code without behavioral
evidence. Goldens cannot be regenerated in place of an explanation.

The supported catalog is the nine SB1 retail builds and four SB2 retail builds
listed in the map. The June 1998 SB1 demo is unsupported. SB1 Eggbert deliberately
uses Blupi's INSERT/retry recovery for **both background and mission read/cache
failures**, rather than Eggbert's INIT recovery. Unavailable network/AVI/CD audio,
modern device/presentation policy, and remaining verification gaps retain their
individual map classifications. Historical STRICT/EQUIVALENT certificates do not
certify SDL behavior.

## Reviewing a change

1. Run `python port/tools/check_fidelity.py --base BASE --head HEAD` to identify
   edges, editions, required action and gates. Also inspect both sides of the diff.
   Physical direct shares are distinguished from historical effective overlay
   files. Target manifest `source_root` and last-overlay-wins resolution follow
   `tools/source_tree.py`, without importing it or any acceptance tools.
2. Update the port for behavior changes, verify equivalence for compiler/context
   changes, explain an intentional difference, or keep a concrete investigation
   follow-up. Review shared headers, tables, switches and common services for both
   game consumers. Dependency groups intentionally fan out conservatively.
3. Edit affected edges in `port/provenance.json`. Keep stable IDs; new groups get
   new IDs. `ledger_ids` preserve all 472 initial audit rows after aggregation.
   Use one TU per DIRECT_SHARED edge, function groups for derivatives and service
   contracts for replacements/omissions. Keep symbols and per-target effective
   paths accurate. Dependency paths/groups also own their context files; new
   sources, overlays and headers must get ownership or an exact-file exclusion
   with a reason. No directory-wide exclusions or mutable approval booleans.
4. Record the inspected source/port revision in each affected `review.commit`
   (full immutable commit ID), a concrete `review.disposition`, updated reason,
   scoped evidence, tests and remaining follow-ups. `reviewed_files` is the shared
   content-hash registry referenced by each edge's expanded paths, avoiding many
   duplicate hashes. Update SHA-256 values **only after review**. Hash bytes after
   converting CRLF to LF, e.g. Python's
   `hashlib.sha256(Path(path).read_bytes().replace(b'\r\n', b'\n')).hexdigest()`.
   The review commit may precede the map-update commit; it identifies the inspected
   tree. Do not claim tests ran merely because they appear in `tests`.
5. Run `--check` and the named gates. Source/port hash changes invalidate every
   edge using that file, including dependency consumers. Update each affected
   edge's review commit/disposition; a registry hash refresh alone cannot clear
   consumers in impact mode. Exact-file prose exclusions need an existing path
   and reason, without implementation hashes; non-prose tooling exceptions are
   hashed and need a recorded hash update after tooling review.

New semantic receipts name target, source and port revision, configuration,
scenario and proof level. Asset-dependent captures remain separate supervisor
receipts; the cheap gate needs no game data. Never execute an original executable.

## Gate behavior

`--check` validates ownership, existing paths, hashes, catalog/overlay resolution
and the two CMake direct-share lists. CTest registers it as `fidelity_map`.
`--base A --head B` uses only `git diff --name-only` and `git show`; it reads the
map at B and validates that snapshot. It considers removed ownership from A too.
Renames appear as old deletion plus new addition. Changes clear only through
identity, a conservative lexical comment/whitespace check, explicit prose-only
exclusions, or updated map hashes/review dispositions matching B. Direct-share
ownership alone never clears a semantic change. Code, tables, function order and
used-local changes require review; the checker does not infer equivalence.

The lexical check retains literals, internal code spacing and line boundaries;
it clears indentation/trailing-space and comment-content edits and rejects
ambiguous constructs or directive edits. Its narrow scope may request review
for harmless edits. Hash normalization only accommodates checkout line endings;
it is not a historical matching proof. When B predates map installation, impact
mode labels the working-tree map as inventory only and disables review clearing.
This permits historical demonstrations without retroactive approval.

CI checks every push and pull request with full git history. PRs also run impact
mode from their merge base to HEAD. Exit 0 means consistent/cleared, 1 means review
is needed, and 2 means malformed inputs or a git/tool error. Inspect the JSON diff
in human review: hash updates and dispositions are review records, not signatures.
Impact mode can clear a lexical-only edit while `--check` still requires refreshing
its snapshot hash; such an edit needs no semantic equivalence disposition.
