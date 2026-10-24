"""Read-only validation of saved results; regenerate only the unapplied diff."""
import sys
sys.dont_write_bytecode = True
import difflib, hashlib, json, subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HERE = ROOT / 'scratch/w-sb1icf'
load = lambda name: json.loads((HERE / name).read_text())
receipt = {'suites': {}}
for name in ['solver-suite.json', 'solver-suite-sb2forms.json']:
    suite = load(name)
    assert len(suite) == 9 and len({r['target'] for r in suite}) == 9
    brief = []
    for row in suite:
        eggbert = '/eggbert/' in row['target']
        assert row['exit'] == 1
        assert (row['old_function_count'], row['new_function_count']) == (460, 468)
        assert len(row['added_records']) == 8
        assert all(r['status'] == 'MISMATCH' for r in row['added_records'].values())
        assert not row['raw_payload_changes']
        assert len(row['symbolic_relocation_changes']) == 16
        assert len(row['changed_existing_records']) == (12 if eggbert else 13)
        assert len(row['accepted_regressions']) == int(eggbert)
        brief.append(dict(target=row['target'], unchanged_raw_payloads=460,
            added_unlocated=8, changed_existing_records=len(row['changed_existing_records']),
            accepted_regressions=len(row['accepted_regressions']), official_exit=row['exit']))
    receipt['suites'][name] = brief

proposal = load('solver-owner-proposal.json')
assert len(proposal) == 6
for row in proposal:
    assert not row['accepted_regressions'] and row['semantic_proofs_completed']
    assert len(row['differing_records']) == 2
    assert all(p['differing_top_level_fields'] == ['inventory_sha256']
        for p in row['certificate_comparisons'])
receipt['owner_proposal'] = dict(targets=6, semantic_proofs_completed=True,
    changed_getter_records_per_target=2, accepted_regressions=0,
    acceptance='FAIL_PIN_PROPOSAL_ONLY', canonical_base_inventory_edits_proposed=1)

# The six private copies were validated independently, but all inherit this
# one canonical inventory. A reviewable patch must not repeat the same hunk.
original = ROOT / 'oracle/sb1-eggbert-en/tu_map.csv'
private = [HERE / 'proposed-inventory' / ('sb1-eggbert-' + language) / 'tu_map.csv'
    for language in ['en', 'de', 'es', 'fr', 'it', 'pt']]
new = private[0].read_text(encoding='utf-8-sig')
assert all(p.read_text(encoding='utf-8-sig') == new for p in private)
relative = original.relative_to(ROOT).as_posix()
patch = ''.join(difflib.unified_diff(
    original.read_text(encoding='utf-8-sig').splitlines(True), new.splitlines(True),
    fromfile=relative, tofile=relative))
(HERE / 'solver-eggbert-inventory.diff').write_text(patch)

receipt['diff_checks'] = {}
for name in ['solver-tail8.diff', 'solver-tail8-sb2forms.diff', 'solver-eggbert-inventory.diff']:
    check = subprocess.run(['git', 'apply', '--check', '-p0', str(HERE / name)],
        cwd=ROOT, text=True, capture_output=True)
    assert check.returncode == 0, (name, check.stderr)
    receipt['diff_checks'][name] = 'APPLIES_UNAPPLIED'

initial = load('facts.json')['canonical_hashes']
sources = {name: digest for name, digest in initial.items() if name.startswith('src\\')}
assert sources
assert all(hashlib.sha256((ROOT / name).read_bytes()).hexdigest() == digest
    for name, digest in sources.items())
receipt['canonical_source_hashes_unchanged'] = len(sources)
receipt['git_head'] = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
receipt['git_status'] = subprocess.check_output(['git', 'status', '--short'], cwd=ROOT, text=True)
assert receipt['git_status'] == ''
(HERE / 'solver-final-receipt.json').write_text(json.dumps(receipt, indent=2))
print('Both nine-target suites verified; six owner proposals retain failed inventory pins.')
print('Three unapplied diffs pass git apply --check; all', len(sources), 'initial source hashes unchanged.')
print('Tracked status clean at', receipt['git_head'])
