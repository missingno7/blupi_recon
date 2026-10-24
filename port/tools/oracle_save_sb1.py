"""Bounded SB1 save interchange using the existing Display proxy and oracle_diff.

The supervisor runs historical processes. --prepare-only starts neither game.
--port-only rehearses all transfers with SDL on both sides; NEVER certifies
historical interchange. No fuzzy alignment, discarded byte ranges or new golden.
"""
import argparse
import contextlib
import ctypes
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from datetime import datetime

REPO = Path(__file__).resolve().parents[2]
SAVE = 's001-000.blp'
LOAD_FIRST = 111
LOAD_DISPLAYS = 171


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def record(path):
    return {'path': str(path), 'sha256': sha(path)}


def require(ok, message):
    if not ok:
        raise RuntimeError(message)


def absolute(path):
    path = Path(path)
    return (path if path.is_absolute() else REPO / path).resolve(strict=True)


def validate(built, port, target, catalog):
    originals = {v['sha256'].lower() for v in catalog.values() if 'sha256' in v}
    # Hash installed originals too, so a renamed or locally changed original
    # cannot be admitted merely by making a gate.json next to it.
    for entry in catalog.values():
        original = REPO / entry['exe']
        if original.is_file():
            originals.add(sha(original))
    for path in (built, port):
        require(path.is_relative_to((REPO / 'build').resolve()), 'Both executables must be under build/')
        require(sha(path) not in originals, f'Refusing original executable: {path}')
    require(port.name.lower() == 'blupi-sdl3.exe', 'Expected SB1 port blupi-sdl3.exe')
    expected = Path(catalog[target]['exe']).name
    require(built.name.lower() == expected.lower(), f'Expected rebuilt {expected} for {target}')
    gate_path, build_path = built.parent / 'gate.json', built.parent / 'build.json'
    gate, build = json.loads(gate_path.read_text()), json.loads(build_path.read_text())
    require(gate.get('status') == 'PASS', 'Validated source build gate PASS required')
    require(build.get('exit_code') == 0, 'Successful source build.json required')
    # Older primary relinks predate build.target; their default target was US.
    require(build.get('target', 'sb1/blupi/us') == target, 'build.json target does not match requested edition')
    return {'historical': record(built), 'port': record(port),
            'gate': record(gate_path), 'build': record(build_path)}


def checked_input(path):
    for line in path.read_text().splitlines():
        command = line.split('#', 1)[0].strip()
        require(not command or re.fullmatch(r'\d+\s+(click\s+\d+\s+\d+|(?:key|keydown|keyup)\s+\d+)', command),
                f'Unsupported proxy input: {line}')


def copy_data(source, destination):
    destination.mkdir()
    for name in ('DATA', 'IMAGE08', 'IMAGE16', 'SOUND'):
        shutil.copytree(source / name, destination / name)
    # Only isolated copies are cleaned. Never import old player progress/saves.
    for path in (destination / 'DATA').iterdir():
        if re.fullmatch(r'(s\d{3}-\d{3}|info\d{3})\.blp', path.name, re.I):
            path.unlink()


def prepare(args, run, built, port, data, receipt):
    proxy = REPO / 'build/oracle/ddraw.dll'
    proxy_source = REPO / 'port/oracle/ddraw_proxy.cpp'
    require(proxy.is_file() and proxy.stat().st_mtime >= proxy_source.stat().st_mtime,
            'Build a current 32-bit ddraw proxy first (see port/docs/ORACLE.md)')
    # The common wrapper is also used for Eggbert installations without cnc-ddraw.
    ddraw_data = REPO / 'assets/Speedy Blupi'
    receipt['proxy'] = record(proxy)
    receipt['proxy_source'] = record(proxy_source)
    receipt['cnc_ddraw'] = record(ddraw_data / 'ddraw.dll')
    scripts = {}
    for kind in ('save', 'load'):
        source = REPO / f'port/oracle/sb1_{kind}.input'
        checked_input(source)
        scripts[kind] = run / f'{kind}.input'
        shutil.copyfile(source, scripts[kind])
    receipt['inputs'] = {k: record(v) for k, v in scripts.items()}
    base = run / 'data'
    copy_data(data, base)
    (base / 'ENGLISH').mkdir()
    # Short enough for the historical 199-byte ReadConfig buffer. ENGLISH's
    # parent contains DATA for Blupi's CD check; Eggbert directly uses DATA.
    config = (f'CD-Rom=ENGLISH\r\nFullScreen=1\r\nBenchmark=0\r\n'
              f'TrueColor={args.color}\r\nMouseType=2\r\nTimer=50\r\nSpeedRate=1\r\n').encode('ascii')
    require(len(config) <= 199, 'Config too long')
    (base / 'DATA/config.def').write_bytes(config)
    receipt['config'] = record(base / 'DATA/config.def')
    ini = (ddraw_data / 'ddraw.ini').read_text()
    for key, value in dict(width=640, height=480, windowed='true', fullscreen='false',
                           renderer='gdi', devmode='true', maxfps=60, posX=100,
                           posY=100, savesettings=0, adjmouse='false').items():
        ini = re.sub(rf'(?m)^{key}=.*$', f'{key}={value}', ini)
    plans = {}
    for name, hist, kind in (('hist-save', True, 'save'), ('port-save', False, 'save'),
                             ('hist-from-hist', True, 'load'), ('port-from-hist', False, 'load'),
                             ('hist-from-port', True, 'load'), ('port-from-port', False, 'load')):
        folder = run / name
        folder.mkdir()
        user = folder / 'user'
        (user / 'data').mkdir(parents=True)
        (user / 'data/config.def').write_bytes(config)
        game = folder / 'game'
        if hist:
            shutil.copytree(base, game)
            shutil.copyfile(built, game / built.name)
            for dll in built.parent.glob('msvcrt*.dll'):
                shutil.copyfile(dll, game / dll.name)
            shutil.copyfile(proxy, game / 'ddraw.dll')
            shutil.copyfile(ddraw_data / 'ddraw.dll', game / 'cnc-ddraw.dll')
            (game / 'ddraw.ini').write_text(ini)
        dumps = folder / 'frames'
        dumps.mkdir()
        ticks = 130 if kind == 'save' else 160
        # Port-only is a transfer/capture rehearsal, with no historical launch.
        use_hist = hist and not args.port_only
        command = ([str(game / built.name), '-nocd'] if use_hist else
                   [str(port), '--headless', '--reference', '--data', str(base), '--user', str(user),
                    '--frames', str(ticks), '--input', str(scripts[kind]),
                    '--display-log', str(folder / 'display.log'),
                    '--display-dump', f'{dumps}:1', '--trace', str(folder / 'trace.txt'),
                    '--trace-every', '1', '--check-commands'])
        plans[name] = {'historical_process': use_hist, 'command': command,
                       'cwd': str(game if use_hist else folder), 'ticks': ticks,
                       'script': str(scripts[kind]), 'folder': str(folder),
                       'save': str((game / 'DATA' if use_hist else user / 'data') / SAVE)}
    receipt['plans'] = plans
    receipt['staged_ddraw_ini'] = record(run / 'hist-save/game/ddraw.ini')
    return plans


def launch(plan, receipt, built_hash, port_hash):
    folder = Path(plan['folder'])
    env = {k: v for k, v in os.environ.items() if not k.startswith('BLUPI_ORACLE_')}
    if plan['historical_process']:
        require(sha(plan['command'][0]) == built_hash, 'Staged rebuild hash changed')
        env.update(BLUPI_ORACLE_LOG=str(folder / 'display.log'),
                   BLUPI_ORACLE_DUMP_DIR=str(folder / 'frames'), BLUPI_ORACLE_DUMP_EVERY='1',
                   BLUPI_ORACLE_MAX_FRAMES=str(plan['ticks'] + 11),
                   BLUPI_ORACLE_INPUT=plan['script'], BLUPI_ORACLE_DEBUG=str(folder / 'debug.txt'))
        ctypes.windll.user32.SetCursorPos(0, 0)
    else:
        require(sha(plan['command'][0]) == port_hash, 'Port executable hash changed')
    # run(timeout) kills and reaps this direct child; no shell or background job.
    result = subprocess.run(plan['command'], cwd=plan['cwd'], env=env, timeout=60,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    (folder / 'run.log').write_bytes(result.stdout)
    receipt.setdefault('runs', {})[folder.name] = {'exit_code': result.returncode}
    require(result.returncode == 0, f'{folder.name}: process failed')
    lines = (folder / 'display.log').read_text().splitlines()
    require(len(lines) == plan['ticks'] + 11, f'{folder.name}: incomplete capture ({len(lines)} displays)')
    require(all(line.split()[1] == str(receipt['color_depth']) for line in lines), 'Wrong pixel depth')
    require(all(int(line.split()[0]) == i for i, line in enumerate(lines)), 'Nonsequential display indices')
    receipt['runs'][folder.name].update(displays=len(lines), display_log=record(folder / 'display.log'))
    if not plan['historical_process'] and plan['ticks'] == 160:
        phases = {int(line.split()[0]): int(line.split()[1])
                  for line in (folder / 'trace.txt').read_text().splitlines() if line.strip() and not line.startswith('#')}
        require(phases.get(101) == 1525, 'Port did not return to PLAY after loading')


def valid_save(path):
    import struct
    raw = Path(path).read_bytes()
    require(len(raw) == 54416 and struct.unpack_from('<iii', raw) == (54416, 1, 2),
            f'Not a retail SB1 v1.2 save: {path}')
    return raw


def compare(run, plans, source):
    # Existing oracle_diff.py owns raw-frame decoding and pixel diagnostics.
    import oracle_diff
    pair = run / f'compare-from-{source}'
    pair.mkdir()
    hist = Path(plans[f'hist-from-{source}']['folder'])
    port = Path(plans[f'port-from-{source}']['folder'])
    a = (hist / 'display.log').read_text().splitlines()
    b = (port / 'display.log').read_text().splitlines()
    unequal = []
    for index in range(LOAD_FIRST, LOAD_DISPLAYS):
        if a[index] != b[index]:
            unequal.append(index)
    # Hard links avoid duplicating hundreds of MB of raw frames. Inputs stay
    # in this run's isolated directories; no links into assets or toolchains.
    for label, folder in (('hist-frames', hist), ('port-frames', port)):
        (pair / label).mkdir()
        for index in range(LOAD_FIRST, LOAD_DISPLAYS):
            name = f'frame_{index:06d}.raw'
            os.link(folder / 'frames' / name, pair / label / name)
    raw_equal = True
    with (pair / 'oracle_diff.txt').open('w') as log, contextlib.redirect_stdout(log):
        for index in range(LOAD_FIRST, LOAD_DISPLAYS):
            oracle_diff.compare(str(pair), index)
            name = f'frame_{index:06d}.raw'
            # Include 8-bit palettes; hash logs alone do not include palettes.
            raw_equal &= (pair / 'hist-frames' / name).read_bytes() == (pair / 'port-frames' / name).read_bytes()
    return {'first_display': LOAD_FIRST, 'last_display': LOAD_DISPLAYS - 1,
            'compared': LOAD_DISPLAYS - LOAD_FIRST, 'identical_hashes': LOAD_DISPLAYS - LOAD_FIRST - len(unequal),
            'first_difference': unequal[0] if unequal else None, 'raw_equal_including_palette': raw_equal,
            'oracle_diff': str(pair / 'oracle_diff.txt'), 'match': not unequal and raw_equal}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--edition', choices=['blupi', 'eggbert'], default='blupi')
    parser.add_argument('--color', type=int, choices=[8, 16], default=16)
    parser.add_argument('--built-exe', default='build/tmp/relink-us/BLUPI.EXE')
    parser.add_argument('--port-exe', default='build/port/blupi-sdl3.exe')
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--port-only', action='store_true')
    args = parser.parse_args()
    catalog = json.loads((REPO / 'targets.json').read_text())['targets']
    target = 'sb1/blupi/us' if args.edition == 'blupi' else 'sb1/eggbert/en'
    built, port = absolute(args.built_exe), absolute(args.port_exe)
    receipt = validate(built, port, target, catalog)
    receipt['git_head'] = subprocess.check_output(
        ['git', '-C', str(REPO), 'rev-parse', 'HEAD'], text=True, timeout=10).strip()
    receipt['tracked_worktree_changes'] = subprocess.check_output(
        ['git', '-C', str(REPO), 'diff', '--name-only'], text=True, timeout=10).splitlines()
    receipt['source_context'] = [record(REPO / name) for name in (
        'src/decor.cpp', 'src/decor.h', 'src/event.cpp', 'port/common/platform/vfs.cpp',
        'port/common/compat/src/hist_stdio.cpp', 'port/games/sb1/game/main.cpp',
        'port/tools/oracle_save_sb1.py', 'port/tools/oracle_diff.py')]
    run = REPO / 'build/oracle' / ('save-sb1-' + args.edition + '-' + datetime.now().strftime('%Y%m%d-%H%M%S-%f'))
    run.mkdir(parents=True)
    print(run, flush=True)
    receipt.update(status='PREPARING', target=target, color_depth=args.color, port_only=args.port_only,
                   scenario='player 1, world-1 hub, ten-tick right walk, slot 0, F3/F4, bounded continuation',
                   proof_level='port-only rehearsal' if args.port_only else 'rebuilt/port saved-game runtime interchange',
                   scope='Bounded runtime restore evidence; not STRICT/equivalence or complete-state fidelity')
    try:
        data = (REPO / catalog[target]['exe']).parent
        plans = prepare(args, run, built, port, data, receipt)
        receipt['status'] = 'PREPARED'
        if args.prepare_only:
            return 0
        receipt['status'] = 'RUNNING'
        launch(plans['hist-save'], receipt, sha(built), sha(port))
        launch(plans['port-save'], receipt, sha(built), sha(port))
        hist_save = valid_save(plans['hist-save']['save'])
        port_save = valid_save(plans['port-save']['save'])
        receipt['saves'] = {'historical': record(plans['hist-save']['save']),
                            'port': record(plans['port-save']['save']), 'byte_equal': hist_save == port_save,
                            'different_bytes': sum(a != b for a, b in zip(hist_save, port_save)),
                            'first_difference_offset': next((i for i, (a, b) in enumerate(zip(hist_save, port_save)) if a != b), None)}
        for source in ('hist', 'port'):
            for consumer in ('hist', 'port'):
                plan = plans[f'{consumer}-from-{source}']
                shutil.copyfile(plans[f'{source}-save']['save'], plan['save'])
                require(sha(plan['save']) == sha(plans[f'{source}-save']['save']), 'Save transfer hash differs')
                receipt.setdefault('transfers', []).append({'from': source, 'to': consumer, **record(plan['save'])})
                launch(plan, receipt, sha(built), sha(port))
        receipt['restores'] = {source: compare(run, plans, source) for source in ('hist', 'port')}
        passed = receipt['saves']['byte_equal'] and all(r['match'] for r in receipt['restores'].values())
        receipt['status'] = ('PORT_ONLY_MATCH' if args.port_only else 'MATCH') if passed else 'DIFFERENT'
        return 0 if passed else 1
    except Exception as error:
        receipt.update(status='INCOMPLETE', error=str(error))
        print(str(error), file=sys.stderr)
        return 1
    finally:
        (run / 'compare.json').write_text(json.dumps(receipt, indent=2) + '\n')
        print(receipt['status'], flush=True)


if __name__ == '__main__':
    sys.exit(main())
