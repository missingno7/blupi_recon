# Scheduler micro ledger

[EVIDENCE] All use the pinned SP3 `/O2 /GX /ML /Gd /Gy` environment through
`tools/msvc.py`. Each `NAME.prediction.json` is written before compilation;
`NAME.cpp`, `.sched.json`, `.asm.json`, `.result.json` and
`.sched.json.validation.json` preserve source, input DAG, ready decisions,
emitted code and validation. Capture/replay names are unique and private.

| Micro | Registered source hypothesis / measured outcome | Complete simulation |
|---|---|---|
| stack_array | Direct stack store needs no store/member-read memory edge; confirmed | 1 window, 25 choices |
| unknown_pointer | Unknown char pointer store aliases both member reads; confirmed RAW 20 edges | 1 / 21 |
| member_array | Variable index may reach adjacent members; both member reads get RAW 20 edges | 1 / 19 |
| named_first | A single-use address name folds; same code as stack_array | 1 / 25 |
| shared_first | Real pointer used for store and draw changes alias metadata; store -> member RAW edges appear | 1 / 25 |
| reference_cut | Byte reference changes alias metadata; same code as shared_first | 1 / 25 |
| tie_loads | Ready ties use original tuple ordinal subject to resource/latency eligibility; confirmed | 1 / 10 |
| split_plain | Full split/copy control; input-DAG scheduling reproduced | 6 / 148 |
| split_early_advance | Cursor step before coordinate changes folds back; same code as split_plain | 6 / 148 |
| split_shadow_buffer | Scoped real copy buffer adds a frame slot: e0 -> 144; rejects source transfer | 5 / 89, 1 excluded |

[EVIDENCE] Totals: 10 sources, 24 complete windows, 535 exactly predicted
choices. The excluded scoped-buffer window has address-rewrite dependence
categories not covered by complete simulation. Local priority/queue/pipe
checks pass there too; it is not counted as a complete-schedule success.

[EVIDENCE] tie_loads, unknown_pointer and stack_array were replayed after
adding persistent pre-execution predictions; their `.predictions.jsonl`
files were flushed while c2 was still stopped at graph entry. The final
target trace has the same pre-execution registration. No compiler decisions
were altered. All replay function payloads equal the normal compilation.

[EVIDENCE] Full target application: early-advance.cpp retains 7424 packaged
bytes / 63 unmasked differences / 2288 of 2304 aligned instructions.
All 94 TU function payloads equal baseline; not retained as a change.
Final event.cpp is baseline, and the full regression gate exits 0 with the
expected sound path INCOMPLETE note. None of these source trials is STRICT.
