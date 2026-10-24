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
        for name in ('IsWorld', 'IsVentillo'):
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
    @staticmethod
    def relocation_receipt():
        return [dict(offset=7, oracle_offset=9, type='DIR32', symbol='table', addend=4,
                     resolved_symbol_va='0x00436000', effective_target_va='0x00436004',
                     target_va='0x00436004', object_section='.data', oracle_section='.data',
                     object_offset=120, data_size=16, verified=True, status='STRICT',
                     reason='initialized data bytes verified')]

    def test_relocation_binding_ignores_only_data_placement_without_mutating_receipt(self):
        before = self.relocation_receipt()
        after = self.relocation_receipt()
        after[0]['object_offset'] += 328
        self.assertEqual(equiv.relocation_binding(before), equiv.relocation_binding(after))
        self.assertEqual(before[0]['object_offset'], 120)
        self.assertEqual(after[0]['object_offset'], 448)
        # Companion data targets follow the same rule; their own relocation
        # sites and identities remain bound recursively.
        nested = [dict(companion_relocations=before)]
        shifted = [dict(companion_relocations=after)]
        self.assertEqual(equiv.relocation_binding(nested), equiv.relocation_binding(shifted))
        shifted[0]['companion_relocations'][0]['symbol'] = 'another_table'
        self.assertNotEqual(equiv.relocation_binding(nested), equiv.relocation_binding(shifted))
        with self.assertRaisesRegex(ValueError, 'unrecognized object_offset'):
            equiv.relocation_binding([dict(object_offset=1, symbol='unknown')])

    def test_relocation_binding_keeps_every_other_proof_fact(self):
        before = self.relocation_receipt()
        expected = equiv.hashed(equiv.relocation_binding(before))
        for field, value in before[0].items():
            if field == 'object_offset':
                continue
            with self.subTest(field=field):
                changed = self.relocation_receipt()
                changed[0][field] = not value if isinstance(value, bool) else (
                    value + 1 if isinstance(value, int) else value + '-changed')
                self.assertNotEqual(expected, equiv.hashed(equiv.relocation_binding(changed)))
        self.assertNotEqual(expected, equiv.hashed(equiv.relocation_binding([])))
        extended = self.relocation_receipt()
        extended[0]['future_proof_fact'] = 1
        self.assertNotEqual(expected, equiv.hashed(equiv.relocation_binding(extended)))

    def test_declarations_are_target_local_and_keep_absolute_rvas(self):
        for name in targets.names():
            target = targets.load(name)
            entries = equiv.declarations(target, target.manifest())
            if name.startswith('sb1/'):
                expected = 0 if target.primary or name == 'sb1/blupi/demo' else 3 if '/eggbert/' in name else 2
            else:
                expected = 4 if name == 'sb2/eggbert/en' else 13
            self.assertEqual(len(entries), expected)
            self.assertTrue(all(target.slug in e['certificate'] for e in entries))
            # Translation never applies to the namespaced target records.
            translated = replace(target, address_map={
                'code': [['0x400000', '0x500000', 0x10000]], 'data': []})
            self.assertEqual(equiv.declarations(translated, translated.manifest()), entries)

    def test_equivalent_cannot_satisfy_strict_acceptance(self):
        for name in targets.names():
            target = targets.load(name)
            for entry in equiv.declarations(target,target.manifest()):
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
        self.assertIn('13 functions', lines[1][-1])
        self.assertIn('51,664 bytes', lines[1][-1])

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

    def test_constant_analysis_meets_joins_and_preserves_only_abi_saved_values(self):
        # Both arms establish EBX=1; the call destroys EAX, but preserves EBX.
        ins = equiv.decode(bytes.fromhex('7407 bb01000000 eb05 bb01000000 b801000000 e800000000 c3'))
        constants = equiv.constant_states(ins)
        self.assertEqual(constants[0x13], {'ebx':1,'eax':1})
        self.assertEqual(constants[0x18], {'ebx':1})
        changed = equiv.decode(bytes.fromhex('7407 bb01000000 eb05 bb02000000 b801000000 e800000000 c3'))
        self.assertNotIn('ebx', equiv.constant_states(changed)[0xe])
        partial = equiv.decode(bytes.fromhex('bb01000000 b302 c3'))
        self.assertNotIn('ebx', equiv.constant_states(partial)[7])

    def test_constant_analysis_accounts_for_loop_predecessors_and_relocations(self):
        ins = equiv.decode(bytes.fromhex('bb01000000 43 75fd c3'))
        self.assertNotIn('ebx', equiv.constant_states(ins)[5])
        relocated = equiv.decode(bytes.fromhex('bb01000000 c3'))
        self.assertNotIn('ebx', equiv.constant_states(relocated,[1])[5])

    def test_constant_register_relation_and_live_flags(self):
        a = self.function('53 57 31db bff5050000 39d8 7500 5f 5b c3')
        b = bytes.fromhex('53 57 31ff bbf5050000 39f8 7500 5f 5b c3')
        _, proof = equiv.structure(a,b,[(2,11,2,11)],False,constant_invariants=True)
        self.assertIn('constant_invariants',proof)
        bad = bytes.fromhex('53 57 31ff bbf5050000 39d8 7500 5f 5b c3')
        with self.assertRaisesRegex(ValueError,'unequal symbolic effects'):
            equiv.structure(a,bad,[(2,11,2,11)],False,constant_invariants=True)

    def test_unchanged_uses_and_returns_cannot_observe_unequal_constants(self):
        a = self.function('53 57 31db bff5050000 53 5f 5b c3')
        b = bytes.fromhex('53 57 31ff bbf5050000 53 5f 5b c3')
        with self.assertRaisesRegex(ValueError,'reads unequal constants'):
            equiv.structure(a,b,[(2,9,2,9)],False,constant_invariants=True)
        a = self.function('31db c3')
        with self.assertRaisesRegex(ValueError,'reads unequal constants|unequal symbolic effects'):
            equiv.structure(a,bytes.fromhex('31ff c3'),[(0,2,0,2)],False,constant_invariants=True)

    def test_cmp_zero_test_identity_excludes_af_observation(self):
        a = self.function('31db 39d8 7500 c3')
        b = bytes.fromhex('31db 85c0 7500 c3')
        equiv.structure(a,b,[(2,4,2,4)],False,constant_invariants=True)
        with self.assertRaisesRegex(ValueError,'flags observer'):
            equiv.structure(self.function('31db 39d8 9f c3'),bytes.fromhex('31db 85c0 9f c3'),
                            [(2,4,2,4)],False,constant_invariants=True)

    def test_cfg_constant_reload_order_and_returns(self):
        a = self.function('53 57 bb01000000 bf02000000 8919 897904 5f 5b c3')
        b = bytes.fromhex('53 57 bf02000000 bb01000000 8919 897904 5f 5b c3')
        _,proof = equiv.cfg_structure(a,b,[],False)
        self.assertTrue(proof['constant_reload_proofs'])
        with self.assertRaisesRegex(ValueError,'unequal constant read'):
            equiv.cfg_structure(self.function('b801000000 c3'),bytes.fromhex('b802000000 c3'),[],True)

    def test_cfg_repeated_guard_and_je_jne_complement(self):
        for second in ('75','74'):
            a = self.function('83f801 7505 83f801 '+second+'00 c3')
            _,proof = equiv.cfg_structure(a,bytes.fromhex('83f801 7500 c3'),[],False)
            self.assertTrue(proof['repeated_guard_proofs'])
        with self.assertRaisesRegex(ValueError,'CFG opcode|unproved instruction'):
            equiv.cfg_structure(self.function('83f801 7505 83f802 7500 c3'),bytes.fromhex('83f801 7500 c3'),[],False)
        with self.assertRaisesRegex(ValueError,'CFG opcode|unproved instruction'):
            equiv.cfg_structure(self.function('83f801 7505 83f801 7f00 c3'),bytes.fromhex('83f801 7500 c3'),[],False)

    def test_cfg_silent_cycles_and_unproved_memory_writes_fail(self):
        with self.assertRaisesRegex(ValueError,'silent cycle'):
            equiv.cfg_structure(self.function('ebfe c3'),bytes.fromhex('c3'),[],False)
        with self.assertRaisesRegex(ValueError,'CFG opcode'):
            equiv.cfg_structure(self.function('8901 c3'),bytes.fromhex('c3'),[],False)
        # Even a paired ordinary write invalidates the remembered guard: an
        # extra comparison after it needs fresh proof, not the stale outcome.
        with self.assertRaisesRegex(ValueError,'CFG opcode'):
            equiv.cfg_structure(self.function('833901 750b c70102000000 833901 7500 c3'),
                                bytes.fromhex('833901 7506 c70102000000 c3'),[],False)

    def test_cfg_call_targets_and_complete_relocation_bijection_are_required(self):
        with self.assertRaisesRegex(ValueError,'unproved instruction'):
            equiv.cfg_structure(self.function('e800000000 c3'),bytes.fromhex('e801000000 c3'),[],False)

    def test_constant_fact_loss_cannot_strengthen_a_join_relation(self):
        lives=[{1:{'ebx':equiv.MASK}},{1:{'ebx':equiv.MASK}}]
        constants=[{0:{'ebx':0},1:{}},{0:{'ebx':1},1:{'ebx':1}}]
        with self.assertRaisesRegex(ValueError,'relation lost at join'):
            equiv.constant_join((0,0),(1,1),lives,constants)
        equiv.constant_join((0,0),(1,1),lives,constants,{'ebx'})
        self.assertNotIn('eax',equiv.full_register_writes(equiv.decode(bytes.fromhex('66b80100'))[0]))

    def test_unmodeled_control_transfer_is_rejected(self):
        with self.assertRaisesRegex(ValueError,'control transfers'):
            equiv.constant_states(equiv.decode(bytes.fromhex('e2fe c3')))


class SB1EquivalenceTests(unittest.TestCase):
    @staticmethod
    def switch_function():
        # Every input either takes JA to RET or uses one of two checked slots.
        # The three bytes between RET and the table are unreachable padding.
        payload = bytes.fromhex('83f801 7707 ff248500000000 c3 8d4900 00000000 00000000')
        symbols = {1:SimpleNamespace(name='$Ltable',storage=6,section=1,value=16),
                   2:SimpleNamespace(name='$Lret',storage=6,section=1,value=12)}
        relocs = [SimpleNamespace(offset=off,typ=6,symbol_index=s) for off,s in ((8,1),(16,2),(20,2))]
        function = SimpleNamespace(data=payload,start=0,relocs=relocs,section=SimpleNamespace(index=1))
        coff = SimpleNamespace(sym_by_index=symbols)
        raw = bytearray(payload)
        for off,value in ((8,0x401010),(16,0x40100c),(20,0x40100c)):
            raw[off:off+4] = value.to_bytes(4,'little')
        raw[13:16] = bytes.fromhex('908bff')
        entry = dict(name='WindowProc',code_end='0xd',regions=[],padding=[['0xd','0x10']],
                     switches=[dict(jump='0x5',compare='0x0',table='0x10',count=2)])
        return function,bytes(raw),coff,entry

    def test_padding_has_complete_cfg_and_independent_table_bindings(self):
        f,raw,c,entry = self.switch_function()
        sites,proof = equiv.sb1_structure(f,raw,c,entry,0x1000,0x400000)
        self.assertEqual(sites,{8:8,16:16,20:20})
        self.assertEqual(proof['differing_bytes'],3)
        self.assertEqual(proof['cfg']['sides'][0]['indirect_successors'],{'0x5':['0xc']})

    def test_direct_branch_into_mutated_padding_is_rejected(self):
        f,raw,c,entry = self.switch_function()
        data,original = bytearray(f.data),bytearray(raw)
        data[4] = original[4] = 8  # JA +0xd: the padding is now executable.
        f.data = bytes(data)
        with self.assertRaisesRegex(ValueError,'control flow reaches padding'):
            equiv.sb1_structure(f,bytes(original),c,entry,0x1000,0x400000)

    def test_jump_table_cannot_target_padding_even_with_consistent_relocation(self):
        f,raw,c,entry = self.switch_function()
        c.sym_by_index[2].value = 13
        original = bytearray(raw)
        for off in (16,20):
            original[off:off+4] = (0x40100d).to_bytes(4,'little')
        with self.assertRaisesRegex(ValueError,'table destination is not a code boundary'):
            equiv.sb1_structure(f,bytes(original),c,entry,0x1000,0x400000)

    def test_retargeted_local_relocation_is_rejected(self):
        f,raw,c,entry = self.switch_function()
        c.sym_by_index[2].value = 0
        with self.assertRaisesRegex(ValueError,'retargeted local relocation'):
            equiv.sb1_structure(f,raw,c,entry,0x1000,0x400000)

    def test_valid_padding_change_still_invalidates_complete_payload_certificate(self):
        f,raw,c,entry = self.switch_function()
        saved = dict(payload_sha256=equiv.hashed(f.data.hex()))
        data = bytearray(f.data)
        data[13:16] = bytes.fromhex('909090')
        f.data = bytes(data)
        equiv.sb1_structure(f,raw,c,entry,0x1000,0x400000) # Reachability proof still holds.
        with self.assertRaisesRegex(ValueError,'certificate changed'):
            equiv.validate_certificate(saved,dict(payload_sha256=equiv.hashed(f.data.hex())),'WindowProc')

    def test_outside_window_mutation_and_incorrect_table_bound_fail(self):
        f,raw,c,entry = self.switch_function()
        data = bytearray(f.data); data[2] = 2; f.data = bytes(data)
        with self.assertRaisesRegex(ValueError,'bound/count'):
            equiv.sb1_structure(f,raw,c,entry,0x1000,0x400000)
        f,raw,c,entry = self.switch_function()
        data = bytearray(f.data); data[12] = 0x90; f.data = bytes(data)
        with self.assertRaisesRegex(ValueError,'end at RET'):
            equiv.sb1_structure(f,raw,c,entry,0x1000,0x400000)

    def test_aliasing_load_is_sensitive_to_prior_store_trace(self):
        # Moving the load across an arbitrary pointed-to store is not proved.
        a = equiv.decode(bytes.fromhex('8b01 8902 c3'))
        b = equiv.decode(bytes.fromhex('8902 8b01 c3'))
        with self.assertRaisesRegex(ValueError,'unequal effects'):
            equiv.sb1_schedule([a,b],0,4,[equiv.live_masks(x,True) for x in (a,b)])

    def test_rep_copy_requires_equal_counts_and_preserves_length_register_renaming(self):
        a = equiv.decode(bytes.fromhex('8bd1 f3a5 8bca c3'))
        b = equiv.decode(bytes.fromhex('8bc1 f3a5 8bc8 c3'))
        # EAX is an ABI result: the differing saved value cannot escape.
        with self.assertRaisesRegex(ValueError,'unequal effects'):
            equiv.sb1_schedule([a,b],0,6,[equiv.live_masks(x,True) for x in (a,b)])
        a = equiv.decode(bytes.fromhex('8bd1 f3a5 8bca 33c0 c3'))
        b = equiv.decode(bytes.fromhex('8bc1 f3a5 8bc8 33c0 c3'))
        equiv.sb1_schedule([a,b],0,6,[equiv.live_masks(x,True) for x in (a,b)])

    def test_sb1_declarations_exclude_primary_and_doinit(self):
        for name in targets.names():
            if not name.startswith('sb1/'):
                continue
            t = targets.load(name)
            entries = equiv.declarations(t,t.manifest())
            self.assertFalse(any(e['name']=='DoInit' for e in entries))
            self.assertEqual(len(entries),0 if t.primary or name == 'sb1/blupi/demo' else 3 if '/eggbert/' in name else 2)
            for entry in entries:
                self.assertEqual(entry['proof_method'],'sb1-local')
                self.assertTrue(entry['certificate'].startswith('evidence/equivalence/'+t.slug+'/'))


if __name__ == '__main__':
    unittest.main()
