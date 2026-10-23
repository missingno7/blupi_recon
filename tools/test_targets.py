"""Regression checks for sibling-target acceptance and coverage (no compiler run).

    .pyenv\python.exe tools/test_targets.py
"""
from contextlib import redirect_stdout
from dataclasses import replace
import io
import json
from pathlib import Path
from types import SimpleNamespace
import unittest
from unittest.mock import patch

import target_coverage
import targets
import verify
import equivalence_target as equiv


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
        manifest_path = targets.ROOT / 'build' / 'w-equiv-coverage-test.json'
        contents = json.dumps(dict(
                toolchain={}, translation_units=[], accepted_functions=[
                    dict(symbol='retained', rva='0x1000', size=16, tu='game.cpp'),
                    dict(symbol='excluded', rva='0x1040', size=16, tu='game.cpp')
                ]))
        original_read = Path.read_text
        def read(path, *args, **kwargs):
            return contents if path == manifest_path else original_read(path, *args, **kwargs)
        with patch.object(Path, 'read_text', read):
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


class TargetEquivalenceTests(unittest.TestCase):
    def test_declarations_are_target_local_and_keep_absolute_rvas(self):
        for name in targets.names():
            target = targets.load(name)
            entries = equiv.declarations(target, target.manifest())
            if name.startswith('sb1/'):
                self.assertEqual(entries, [])
                continue
            expected = 5 if name == 'sb2/eggbert/en' else 9
            self.assertEqual(len(entries), expected)
            self.assertTrue(all(target.slug in e['certificate'] for e in entries))
            # Translation never applies to the namespaced target records.
            translated = replace(target, address_map={
                'code': [['0x400000', '0x500000', 0x10000]], 'data': []})
            self.assertEqual(equiv.declarations(translated, translated.manifest()), entries)

    def test_equivalent_cannot_satisfy_strict_acceptance(self):
        entry = equiv.declarations(targets.load('sb2/blupi/en'), targets.load('sb2/blupi/en').manifest())[0]
        for status in ('MISMATCH', 'EQUIVALENT'):
            row = dict(name=entry['symbol'], status=status, code_size=entry['size'], matches=[dict(rva=entry['rva'])])
            self.assertEqual(len(verify.accepted_regressions(
                dict(accepted_functions=[entry]), {entry['tu']: [row]})), 1)

    def test_coverage_keeps_strict_line_identical(self):
        target = targets.load('sb2/blupi/en')
        lines = []
        for present in (False, True):
            manifest = target.manifest()
            if not present:
                manifest.pop('target_equivalent_functions')
            output = io.StringIO()
            with patch.object(targets, 'load', return_value=target), \
                    patch.object(target, 'manifest', return_value=manifest), \
                    patch('sys.argv', ['target_coverage.py']), redirect_stdout(output):
                self.assertEqual(target_coverage.main(), 0)
            lines.append(output.getvalue().splitlines())
        self.assertEqual(lines[0], lines[1][:-1])
        self.assertIn('9 functions', lines[1][-1])
        self.assertIn('8,944 bytes', lines[1][-1])

    @staticmethod
    def function(payload):
        return SimpleNamespace(data=bytes.fromhex(payload), relocs=[], start=0)

    def test_address_associativity_and_dead_scratch_register(self):
        # (ECX + EDX) - EBX == (ECX - EBX) + EDX modulo 2**32.
        a = self.function('8d0411 2bc3 c3')
        b = bytes.fromhex('2bcb 8d0411 c3')
        _, receipt = equiv.structure(a, b, [(0,5,0,5)], True)
        self.assertEqual(receipt['regions'][0]['exhaustive_boolean_assignments'], 1)

    def test_live_scratch_register_difference_fails(self):
        a = self.function('8d0411 2bc3 51 58 c3')
        b = bytes.fromhex('2bcb 8d0411 51 58 c3')
        with self.assertRaisesRegex(ValueError, 'unequal symbolic effects'):
            equiv.structure(a, b, [(0,5,0,5)], True)

    def test_live_flags_difference_fails(self):
        a = self.function('8d0411 2bc3 7400 c3')
        b = bytes.fromhex('2bcb 8d0411 7400 c3')
        with self.assertRaisesRegex(ValueError, 'unequal symbolic effects'):
            equiv.structure(a, b, [(0,5,0,5)], True)

    def test_region_boundaries_and_unproved_instructions_fail(self):
        a = self.function('8d0411 2bc3 c3')
        with self.assertRaisesRegex(ValueError, 'non-instruction boundary'):
            equiv.structure(a, a.data, [(1,5,1,5)], True)
        with self.assertRaisesRegex(ValueError, 'opcode|instruction count'):
            equiv.structure(a, bytes.fromhex('8d0411 2bc3 90 c3'), [(0,5,0,5)], True)

    def test_reordered_outputs_require_disjoint_fields(self):
        pointer = ('input', 'pointer')
        writes = [(pointer,4,1), (equiv.add((pointer,1),(4,1)),4,2)]
        self.assertEqual(equiv.disjoint_writes(writes), equiv.disjoint_writes(list(reversed(writes))))
        with self.assertRaisesRegex(ValueError, 'non-alias proof'):
            equiv.disjoint_writes([(pointer,4,1), (equiv.add((pointer,1),(2,1)),4,2)])

    def test_unknown_or_impure_calls_are_rejected(self):
        a = self.function('e800000000 c3')
        with self.assertRaisesRegex(ValueError, 'purity theorem'):
            equiv.structure(a, a.data, [(0,5,0,5)], True)


if __name__ == '__main__':
    unittest.main()
