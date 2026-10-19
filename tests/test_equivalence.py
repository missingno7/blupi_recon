"""Acceptance-boundary tests; no native game code is executed."""
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'tools'))
import equivalence as eq
import relink


class AcceptanceTests(unittest.TestCase):
    def test_certificate_changes_fail_closed(self):
        saved = json.loads((ROOT/'evidence/equivalence/ChangePhase.json').read_text())
        eq.validate_certificate(saved, copy.deepcopy(saved), 'ChangePhase')
        for field in ('payload_sha256', 'oracle_sha256', 'checker_sha256',
                      'comparator_sha256', 'assumptions', 'checks', 'scope'):
            with self.subTest(field=field):
                current = copy.deepcopy(saved)
                current[field] = 'changed'
                with self.assertRaises(ValueError):
                    eq.validate_certificate(saved, current, 'ChangePhase')

    def test_unknown_guard_instruction_rejected(self):
        instructions = {i.address:i for i in eq.md.disasm(bytes.fromhex('90'),0)}
        with self.assertRaises(AssertionError):
            eq.guard(instructions,0,{1},{})

    def test_dirty_register_read_rejected(self):
        # mov eax,ecx: an observable use before ECX is overwritten.
        instructions = {i.address:i for i in eq.md.disasm(bytes.fromhex('8bc1'),0)}
        with self.assertRaises(AssertionError):
            eq.dead_before_use(instructions,0,{'ecx'})

    def test_strict_upgrade_does_not_need_equivalence(self):
        manifest = json.loads((ROOT/'recon.json').read_text())
        comparison = [{'functions':[dict(name=e['symbol'], status='STRICT')
                       for e in manifest['equivalent_functions']]}]
        with tempfile.TemporaryDirectory(dir=ROOT/'build') as out:
            with patch.object(eq,'audit',side_effect=AssertionError('unexpected audit')):
                result = eq.check(manifest,comparison,[],out)
        self.assertTrue(all(r['status']=='STRICT' for r in result.values()))
        self.assertTrue(all(f['status']=='STRICT' for f in comparison[0]['functions']))

    def test_recorded_order_requires_every_unit_once(self):
        units = [dict(source='a'),dict(source='b')]
        self.assertEqual(relink.recorded_order(units,dict(object_order=['b','a'])), units[::-1])
        for names in (['a'],['a','a'],['a','b','c']):
            with self.assertRaises(ValueError):
                relink.recorded_order(units,dict(object_order=names))


if __name__ == '__main__':
    unittest.main()
