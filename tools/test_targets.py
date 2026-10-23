"""Regression checks for sibling-target acceptance and coverage (no compiler run).

    .pyenv\python.exe tools/test_targets.py
"""
from contextlib import redirect_stdout
from dataclasses import replace
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

import target_coverage
import targets
import verify


class TargetAcceptanceTests(unittest.TestCase):
    def setUp(self):
        self.blupi = targets.load('sb2/blupi/en')
        self.eggbert = targets.load('sb2/eggbert/en')

    @staticmethod
    def accepted(target, symbol):
        return next(a for a in target.manifest()['accepted_functions'] if a['symbol'] == symbol)

    def test_checkworld1_has_distinct_blupi_body(self):
        symbol = '?CheckWorld1@CEvent@@IAEHXZ'
        blupi = self.accepted(self.blupi, symbol)
        eggbert = self.accepted(self.eggbert, symbol)
        self.assertEqual((verify.parse_number(blupi['rva']), blupi['size']), (0x1fbd0, 128))
        self.assertEqual(blupi.get('status', 'strict'), 'strict')
        self.assertNotIn('into', blupi)
        self.assertEqual(eggbert['status'], 'folded')
        self.assertEqual(eggbert['into']['symbol'], '?Flush@CPixmap@@QAEHXZ')

    def test_reactivated_bodies_are_enforced_only_on_blupi(self):
        for name in ('IsWorld', 'IsMoveTraj', 'IsVentillo'):
            symbol = f'?{name}@CDecor@@QAEHUtagPOINT@@@Z'
            with self.subTest(function=name):
                blupi = self.accepted(self.blupi, symbol)
                eggbert = self.accepted(self.eggbert, symbol)
                self.assertFalse(blupi.get('withdrawn'))
                self.assertTrue(eggbert.get('withdrawn'))
                function = dict(name=symbol, status='STRICT', code_size=blupi['size'],
                                matches=[dict(rva=hex(verify.parse_number(blupi['rva'])))])
                manifest = dict(accepted_functions=[blupi])
                by_unit = {blupi['tu']: [function]}
                self.assertEqual(verify.accepted_regressions(manifest, by_unit), [])
                function['status'] = 'MISMATCH'
                self.assertEqual(len(verify.accepted_regressions(manifest, by_unit)), 1)
                function['status'] = 'STRICT'
                function['matches'][0]['rva'] = '0x1000'
                self.assertEqual(len(verify.accepted_regressions(manifest, by_unit)), 1)
                self.assertEqual(verify.accepted_regressions(
                    dict(accepted_functions=[eggbert]), by_unit), [])

    def test_target_local_proofs_keep_their_rvas_and_enter_the_gate(self):
        target = replace(self.blupi, address_map={
            'code': [['0x400000', '0x500000', 0x10000]], 'data': []})
        for proof in target.accepted_functions:
            with self.subTest(function=proof['symbol']):
                accepted = self.accepted(target, proof['symbol'])
                self.assertEqual(accepted['rva'], proof['rva'])
                self.assertFalse(accepted.get('withdrawn'))
                function = dict(name=proof['symbol'], status='RELOC-UNVERIFIED',
                                code_size=proof['size'], matches=[dict(rva=proof['rva'])])
                self.assertEqual(len(verify.accepted_regressions(
                    dict(accepted_functions=[accepted]), {proof['tu']: [function]})), 1)

    def test_coverage_applies_location_size_and_exclusion_overrides(self):
        temp_root = targets.ROOT / 'build' / 'tmp'
        temp_root.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=temp_root, prefix='target-test-') as folder:
            manifest_path = Path(folder) / 'manifest.json'
            manifest_path.write_text(json.dumps(dict(
                toolchain={}, translation_units=[], accepted_functions=[
                    dict(symbol='retained', rva='0x1000', size=16, tu='game.cpp'),
                    dict(symbol='excluded', rva='0x1040', size=16, tu='game.cpp')
                ])), encoding='utf-8')
            target = replace(self.blupi, manifest_path=manifest_path, address_map=None,
                             game_text_end=0x1100, accepted_functions=[], accepted_overrides={
                                 'retained': dict(rva='0x10f0', size=32),
                                 'excluded': dict(exclude=True)})
            output = io.StringIO()
            with patch.object(targets, 'load', return_value=target), \
                    patch('sys.argv', ['target_coverage.py']), redirect_stdout(output):
                self.assertEqual(target_coverage.main(), 0)
            total = next(line.split() for line in output.getvalue().splitlines()
                         if line.strip().startswith('total'))
            self.assertEqual(total, ['total', '1', 'STRICT', '0', 'FOLDED', '16', 'bytes', '(6.2%)'])


if __name__ == '__main__':
    unittest.main()
