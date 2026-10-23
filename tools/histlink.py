#!/usr/bin/env python3
"""Run pinned VC5 link/resource tools with their historical CRT and a process-local clock.

The debugger only changes the running tool's IAT and clock API results. Link
and resource-object outputs are written naturally by the pinned tools and are
never patched afterward. Requires 64-bit Windows Python (the repo's PyPy works).
The private CRT copy is byte-identical to the pinned redistributable.
"""
from __future__ import annotations

import argparse
import ctypes as C
import ctypes.wintypes as W
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import time

import pefile
from c2trace.c2dbg import K, CTX, DEBUG_EVENT, SI, PI, BP_CODES, SS_CODES, DBG_CONTINUE, DBG_NOT_HANDLED

ROOT = Path(__file__).resolve().parents[1]
TOOLS = {
    'sp3-link': dict(tool=Path(r'C:\tools\msvc5_sp3\VC\BIN\LINK.EXE'),
                     tool_sha256='04c892989f4cc8076ef45e033215eb7aefbdf1459cbee9b13a81f8ddb718b579',
                     crt=Path(r'C:\tools\msvc5_sp3\VC\REDIST\MSVCRT.DLL'),
                     crt_sha256='e390b0529fb9ee1214bb68302a4658b578134d92ab76bc5eb48f3badd843d210',
                     crt_imports=111, label='VC5 SP3 LINK'),
    'rtm-link': dict(tool=Path(r'C:\tools\msvc5_rtm\VC\BIN\LINK.EXE'),
                     tool_sha256='279544809e6353c8e622eb88aea94e86b0258f3c4b8a976373bf4d68a41b63e1',
                     crt=Path(r'C:\tools\msvc5_rtm\VC\REDIST\MSVCRT.DLL'),
                     crt_sha256='d398de0da85572ae6e40254f93ffe0ab0bc4bfcb77681b4a0c358493552707bb',
                     crt_imports=97, label='VC5 RTM LINK'),
    'rtm-cvtres': dict(tool=Path(r'C:\tools\msvc5_rtm\VC\BIN\CVTRES.EXE'),
                       tool_sha256='0f82167cd888105224463feef5cadebaef970c12932597919e3033d77ed3de6c',
                       crt=Path(r'C:\tools\msvc5_rtm\VC\REDIST\MSVCRT.DLL'),
                       crt_sha256='d398de0da85572ae6e40254f93ffe0ab0bc4bfcb77681b4a0c358493552707bb',
                       crt_imports=45, label='VC5 RTM CVTRES'),
}
ORACLE_TIME = 0x36245013

for name, result, args in [
    ('CreateProcessW', W.BOOL, [W.LPCWSTR, W.LPWSTR, C.c_void_p, C.c_void_p, W.BOOL, W.DWORD,
                               C.c_void_p, W.LPCWSTR, C.POINTER(SI), C.POINTER(PI)]),
    ('TerminateProcess', W.BOOL, [W.HANDLE, W.UINT]),
    ('WaitForSingleObject', W.DWORD, [W.HANDLE, W.DWORD]),
    ('GetFinalPathNameByHandleW', W.DWORD, [W.HANDLE, W.LPWSTR, W.DWORD, W.DWORD]),
    ('VirtualAllocEx', C.c_void_p, [W.HANDLE, C.c_void_p, C.c_size_t, W.DWORD, W.DWORD]),
    ('VirtualProtectEx', W.BOOL, [W.HANDLE, C.c_void_p, C.c_size_t, W.DWORD, C.POINTER(W.DWORD)]),
    ('GetStdHandle', W.HANDLE, [W.DWORD]),
    ('SetHandleInformation', W.BOOL, [W.HANDLE, W.DWORD, W.DWORD]),
    ('CreateJobObjectW', W.HANDLE, [C.c_void_p, W.LPCWSTR]),
    ('SetInformationJobObject', W.BOOL, [W.HANDLE, C.c_int, C.c_void_p, W.DWORD]),
    ('QueryInformationJobObject', W.BOOL, [W.HANDLE, C.c_int, C.c_void_p, W.DWORD, C.c_void_p]),
    ('AssignProcessToJobObject', W.BOOL, [W.HANDLE, W.HANDLE]),
    ('TerminateJobObject', W.BOOL, [W.HANDLE, W.UINT]),
    ('ResumeThread', W.DWORD, [W.HANDLE]),
]:
    fn = getattr(K, name)
    fn.restype, fn.argtypes = result, args


def checked(value, operation):
    if not value:
        raise C.WinError(C.get_last_error(), operation)
    return value


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


class Process:
    def __init__(self, handle):
        self.handle = handle
        self.breakpoints = {}

    def read(self, address, count):
        data = C.create_string_buffer(count)
        actual = C.c_size_t()
        checked(K.ReadProcessMemory(self.handle, address, data, count, C.byref(actual)), 'ReadProcessMemory')
        if actual.value != count:
            raise RuntimeError('short process memory read')
        return data.raw

    def u32(self, address):
        return struct.unpack('<I', self.read(address, 4))[0]

    def string(self, address):
        data = bytearray()
        if address:
            while len(data) < 1024:
                byte = self.read(address + len(data), 1)
                if byte == b'\0':
                    break
                data.extend(byte)
        return data.decode('cp1252', errors='replace')

    def write(self, address, data):
        actual = C.c_size_t()
        checked(K.WriteProcessMemory(self.handle, address, data, len(data), C.byref(actual)), 'WriteProcessMemory')
        if actual.value != len(data):
            raise RuntimeError('short process memory write')
        checked(K.FlushInstructionCache(self.handle, address, len(data)), 'FlushInstructionCache')

    def write_import(self, address, value):
        old = W.DWORD()
        checked(K.VirtualProtectEx(self.handle, address, 4, 0x04, C.byref(old)), 'VirtualProtectEx')
        try:
            self.write(address, struct.pack('<I', value))
        finally:
            restored = W.DWORD()
            checked(K.VirtualProtectEx(self.handle, address, 4, old.value, C.byref(restored)),
                    'VirtualProtectEx restore')

    def arm(self, address, kind, name):
        if address in self.breakpoints:
            return
        original = self.read(address, 1)
        if original == b'\xcc':
            raise RuntimeError(f'pre-existing breakpoint at {address:#x}')
        self.breakpoints[address] = (original, kind, name)
        self.write(address, b'\xcc')

    def disarm(self, address):
        original, _, _ = self.breakpoints.pop(address)
        self.write(address, original)


def thread_context(tid):
    handle = checked(K.OpenThread(0x1fffff, False, tid), 'OpenThread')
    context = CTX()
    context.ContextFlags = 0x1003f  # WOW64_CONTEXT_ALL, including floating-point/extended state
    try:
        checked(K.Wow64GetThreadContext(handle, C.byref(context)), 'Wow64GetThreadContext')
    except BaseException:
        K.CloseHandle(handle)
        raise
    return handle, context


def module_from_event(event):
    handle = C.c_void_p.from_buffer(event, 16).value
    base = C.c_void_p.from_buffer(event, 24).value
    try:
        if not handle or not base or base > 0xffffffff:
            return None
        buf = C.create_unicode_buffer(32768)
        checked(K.GetFinalPathNameByHandleW(handle, buf, len(buf), 0), 'GetFinalPathNameByHandleW')
        path = Path(buf.value.removeprefix('\\\\?\\'))
        pe = pefile.PE(str(path), fast_load=True)
        pe.parse_data_directories(directories=[pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_EXPORT']])
        return dict(path=str(path), base=base, pe=pe,
                    exports={s.name.decode('ascii'): s for s in getattr(pe, 'DIRECTORY_ENTRY_EXPORT', ()).symbols
                             if s.name} if hasattr(pe, 'DIRECTORY_ENTRY_EXPORT') else {})
    finally:
        if handle:
            K.CloseHandle(handle)


def resolve(modules, module, name):
    item = modules.get(module.lower())
    if not item or name not in item['exports']:
        return None
    symbol = item['exports'][name]
    if symbol.forwarder:
        dll, forwarded = symbol.forwarder.decode('ascii').rsplit('.', 1)
        return resolve(modules, dll + '.dll', forwarded)
    return item['base'] + symbol.address


def launch(arguments, report_path, *, tool_kind='sp3-link', historical_crt=True,
           timestamp=ORACLE_TIME, timeout=120, trace_rich=False):
    """Block until the selected tool exits; return its code and write an audit report."""
    if os.name != 'nt' or C.sizeof(C.c_void_p) != 8:
        raise RuntimeError('histlink requires 64-bit Windows Python')
    if tool_kind not in TOOLS:
        raise ValueError('unsupported historical tool profile: ' + tool_kind)
    profile = TOOLS[tool_kind]
    tool_path, tool_hash = profile['tool'], profile['tool_sha256']
    crt_path, crt_hash = profile['crt'], profile['crt_sha256']
    if trace_rich and tool_kind != 'sp3-link':
        raise ValueError('--trace-rich is supported only for the pinned SP3 LINK')
    if timestamp is not None and not 0 <= timestamp <= 0x7fffffff:
        raise ValueError('timestamp must fit VC5 time_t')
    if sha(tool_path) != tool_hash or sha(crt_path) != crt_hash:
        raise RuntimeError('pinned historical tool/VC5 MSVCRT.DLL hash mismatch')
    report_path = report_path.resolve()
    if not any(report_path.is_relative_to(ROOT / d) for d in ('build', 'scratch')):
        raise ValueError('report must be under build/ or scratch/')
    report_path.parent.mkdir(parents=True, exist_ok=True)
    private = report_path.parent / ('msvcrt-vc5-' + crt_hash[:16] + '.dll')
    if historical_crt:
        if private.exists() and sha(private) != crt_hash:
            raise RuntimeError('private CRT copy has unexpected contents')
        if not private.exists():
            private.write_bytes(crt_path.read_bytes())
        if sha(private) != crt_hash:
            raise RuntimeError('private CRT copy verification failed')
    image = pefile.PE(str(tool_path))
    crt_imports = [i for t in image.DIRECTORY_ENTRY_IMPORT if t.dll.lower() == b'msvcrt.dll' for i in t.imports]
    if len(crt_imports) != profile['crt_imports']:
        raise RuntimeError('unexpected ' + profile['label'] + ' MSVCRT import table')
    if not any(i.name == b'time' for i in crt_imports):
        raise RuntimeError('historical tool does not import CRT time()')
    report = dict(tool_kind=tool_kind, tool=str(tool_path), tool_sha256=tool_hash,
                  link=str(tool_path), linker_sha256=tool_hash, historical_crt=historical_crt,
                  crt_source=str(crt_path), crt_sha256=crt_hash, private_crt=str(private) if historical_crt else None,
                  timestamp=timestamp, utc=datetime.fromtimestamp(timestamp, timezone.utc).isoformat() if timestamp is not None else None,
                  command=[str(tool_path), *arguments], modules=[], bindings=[], calls={}, time_returns=[],
                  configured_before_entry=False, exit_code=None, sorts=[])
    env = dict(os.environ)
    temp = ROOT / 'build/tmp'
    temp.mkdir(parents=True, exist_ok=True)
    env.update(TMP=str(temp), TEMP=str(temp), PATH=os.pathsep.join(map(str, [tool_path.parent,
        tool_path.parents[2] / 'SHAREDIDE/BIN', Path(env.get('SystemRoot', r'C:\Windows')) / 'System32'])))
    if timestamp is not None:
        env['TZ'] = 'UTC0'
    report['environment'] = {k: env.get(k) for k in ('TMP', 'TEMP', 'PATH', 'TZ')}
    block = C.create_unicode_buffer(''.join(f'{k}={v}\0' for k, v in sorted(env.items())) + '\0')
    startup, pi = SI(), PI()
    startup.cb = C.sizeof(startup)
    startup.x[7] = 0x100 | 1  # STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW
    for field, number in [('i', -10), ('o', -11), ('e', -12)]:
        handle = K.GetStdHandle(number & 0xffffffff)
        if handle and handle != C.c_void_p(-1).value:
            checked(K.SetHandleInformation(handle, 1, 1), 'SetHandleInformation')
        setattr(startup, field, handle)
    command = C.create_unicode_buffer(subprocess.list2cmdline([str(tool_path), *arguments]))
    process = None
    job = None
    event = DEBUG_EVENT()
    modules, rearm, returns = {}, {}, {}
    entry = image.OPTIONAL_HEADER.ImageBase + image.OPTIONAL_HEADER.AddressOfEntryPoint
    image_end = image.OPTIONAL_HEADER.ImageBase + image.OPTIONAL_HEADER.SizeOfImage
    saved = saved_stack = None
    pending = done = False
    started = time.monotonic()
    if timestamp is not None:
        dt = datetime.fromtimestamp(timestamp, timezone.utc)
        systemtime = struct.pack('<8H', dt.year, dt.month, (dt.weekday()+1) % 7, dt.day,
                                 dt.hour, dt.minute, dt.second, 0)
        filetime = struct.pack('<Q', (timestamp + 11644473600) * 10000000)
        zone = bytearray(172)
        zone[4:12] = 'UTC\0'.encode('utf-16le')
        zone[88:96] = 'UTC\0'.encode('utf-16le')
        wall_apis = dict(GetSystemTime=systemtime, GetLocalTime=systemtime,
                         GetSystemTimeAsFileTime=filetime, GetSystemTimePreciseAsFileTime=filetime,
                         GetTimeZoneInformation=bytes(zone))
    else:
        wall_apis = {}

    def install_hooks():
        for dll in ('kernel32.dll', 'kernelbase.dll'):
            for name in (*wall_apis, 'GetTickCount'):
                address = resolve(modules, dll, name)
                if address:
                    process.arm(address, 'wall' if name in wall_apis else 'observe', name)

    def record_call(name, caller):
        item = report['calls'].setdefault(name, dict(count=0, link_callers={}, callers={}))
        item['count'] += 1
        for field in ('callers', 'link_callers') if 0x400000 <= caller < image_end else ('callers',):
            key = hex(caller)
            item[field][key] = item[field].get(key, 0) + 1

    def configure():
        nonlocal saved
        if historical_crt:
            module = modules.get(private.name.lower())
            if not module:
                raise RuntimeError('historical CRT not present in debug module list')
            if report['crt_base'] != hex(module['base']):
                raise RuntimeError('loaded CRT handle does not identify the private DLL')
            if sha(Path(module['path'])) != crt_hash:
                raise RuntimeError('loaded historical CRT has unexpected contents')
            for imp in crt_imports:
                name = imp.name.decode('ascii')
                target = resolve(modules, private.name, name)
                if target is None:
                    raise RuntimeError('missing historical export: ' + name)
                before = process.u32(imp.address)
                process.write_import(imp.address, target)
                actual = process.u32(imp.address)
                if actual != target:
                    raise RuntimeError('IAT verification failed: ' + name)
                report['bindings'].append(dict(name=name, slot=hex(imp.address), original=hex(before),
                                               expected=hex(target), actual=hex(actual)))
        for name in ('time', 'ctime', 'qsort'):
            imp = next((i for i in crt_imports if i.name == name.encode('ascii')), None)
            if imp:
                process.arm(process.u32(imp.address), 'observe', 'MSVCRT!' + name)
        if trace_rich:
            process.arm(0x43e3af, 'rich', 'Rich module census')
        process.disarm(entry)
        report['configured_before_entry'] = True

    def sort_snapshot(base, count, comparator):
        items = []
        for index in range(count):
            pointer = process.u32(base + index*4)
            if comparator == 0x432130:  # ICF supplemental records
                con = process.u32(pointer+0x20) if pointer else 0
                item = dict(pointer=hex(pointer), name=process.string(process.u32(pointer+0x10)) if pointer else '')
                if con:
                    item.update(size=process.u32(con+8)-process.u32(con+0x14),
                                relocs=struct.unpack('<H', process.read(con+0x24, 2))[0])
            else:  # 0x40f8d0: contributions ordered by their module's object name
                module = process.u32(pointer+0x18)
                item = dict(pointer=hex(pointer), name=process.string(process.u32(module+4)),
                            module_index=process.u32(module+8))
            items.append(item)
        return items

    try:
        job = checked(K.CreateJobObjectW(None, None), 'CreateJobObjectW')
        # Native x64 JOBOBJECT_EXTENDED_LIMIT_INFORMATION: 144 bytes;
        # BasicLimitInformation.LimitFlags at +16. Kill the entire tool tree
        # on timeout/error/launcher exit, including a possible CVTRES child.
        limits = C.create_string_buffer(144)
        struct.pack_into('<I', limits, 16, 0x2000)  # JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE
        checked(K.SetInformationJobObject(job, 9, limits, 144), 'SetInformationJobObject')
        checked(K.CreateProcessW(str(tool_path), command, None, None, True, 0x406, block, str(ROOT),
                                C.byref(startup), C.byref(pi)), 'CreateProcessW')
        process = Process(pi.hProcess)
        checked(K.AssignProcessToJobObject(job, pi.hProcess), 'AssignProcessToJobObject')
        report['process_id'] = pi.pid
        report['tool_tree_job'] = True
        if K.ResumeThread(pi.hThread) == 0xffffffff:
            raise C.WinError(C.get_last_error(), 'ResumeThread')
        while not done:
            if time.monotonic() - started > timeout:
                raise TimeoutError(f'LINK exceeded {timeout}s')
            if not K.WaitForDebugEvent(C.byref(event), 1000):
                if C.get_last_error() == 121:  # ERROR_SEM_TIMEOUT
                    continue
                raise C.WinError(C.get_last_error(), 'WaitForDebugEvent')
            pending = True
            status = DBG_CONTINUE
            if event.Code == 5:
                report['exit_code'] = C.c_uint32.from_buffer(event, 16).value
                done = True
            elif event.Code == 3:
                base = C.c_void_p.from_buffer(event, 40).value
                if base != image.OPTIONAL_HEADER.ImageBase:
                    raise RuntimeError('unexpected ' + profile['label'] + ' image base')
                file_handle = C.c_void_p.from_buffer(event, 16).value
                if file_handle:
                    K.CloseHandle(file_handle)
                for offset, own in ((24, pi.hProcess), (32, pi.hThread)):
                    handle = C.c_void_p.from_buffer(event, offset).value
                    if handle and handle != own:
                        K.CloseHandle(handle)
                process.arm(entry, 'entry', profile['label'] + ' entry')
            elif event.Code == 6:
                module = module_from_event(event)
                if module:
                    modules[Path(module['path']).name.lower()] = module
                    report['modules'].append(dict(path=module['path'], base=hex(module['base'])))
                    install_hooks()
            elif event.Code == 2:
                handle = C.c_void_p.from_buffer(event, 16).value
                if handle and handle != pi.hThread:
                    K.CloseHandle(handle)
            elif event.Code == 1:
                code, address = event.Exc.Code, event.Exc.Addr or 0
                if code in BP_CODES and address in process.breakpoints:
                    ht, context = thread_context(event.Tid)
                    try:
                        context.Eip = address
                        original, kind, name = process.breakpoints[address]
                        if kind == 'entry':
                            if saved is None and historical_crt:
                                saved = CTX.from_buffer_copy(context)
                                saved_stack = process.read(context.Esp-8, 8)
                                path = (str(private) + '\0').encode('utf-16le')
                                remote = checked(K.VirtualAllocEx(process.handle, None, len(path), 0x3000, 4), 'VirtualAllocEx')
                                process.write(remote, path)
                                context.Esp -= 8
                                process.write(context.Esp, struct.pack('<II', entry, remote))
                                context.Eip = resolve(modules, 'kernel32.dll', 'LoadLibraryW')
                                if not context.Eip:
                                    raise RuntimeError('LoadLibraryW unavailable')
                            else:
                                if historical_crt:
                                    if not context.Eax:
                                        raise RuntimeError('LoadLibraryW failed for historical CRT')
                                    report['crt_base'] = hex(context.Eax)
                                    process.write(saved.Esp-8, saved_stack)
                                    context = CTX.from_buffer_copy(saved)
                                configure()
                        elif kind == 'wall':
                            caller, pointer = process.u32(context.Esp), process.u32(context.Esp+4)
                            record_call(name, caller)
                            process.write(pointer, wall_apis[name])
                            context.Eip, context.Esp = caller, context.Esp + 8  # stdcall, one argument
                            context.Eax = 0  # TIME_ZONE_ID_UNKNOWN; other wall APIs return void
                            context.EFlags &= ~0x100
                        else:
                            if kind == 'return':
                                for action in returns.pop((event.Tid, address), []):
                                    if action['kind'] == 'time':
                                        report['time_returns'].append(dict(caller=hex(address), value=hex(context.Eax)))
                                        if timestamp is not None and context.Eax != timestamp:
                                            raise RuntimeError('CRT time() did not return the emulated timestamp')
                                    else:
                                        action['after'] = sort_snapshot(action['base'], action['count'], action['comparator'])
                                        report['sorts'].append(action)
                            elif kind == 'rich':
                                # SP3 LINK's Rich writer (0x43e340) increments
                                # one count for the module at [esp+0x1c]. Its
                                # outer iterator's library is [esp+0x2c].
                                module = process.u32(context.Esp+0x1c)
                                library = process.u32(context.Esp+0x2c)
                                report.setdefault('rich_modules', []).append(dict(
                                    name=process.string(process.u32(module+4)),
                                    library=process.string(process.u32(library)),
                                    module_index=process.u32(module+8),
                                    archive_data_offset=process.u32(module),
                                    compiler_stamp=process.u32(module+0x30)))
                            else:
                                caller = process.u32(context.Esp)
                                record_call(name, caller)
                                if name == 'MSVCRT!time' and 0x400000 <= caller < image_end:
                                    returns.setdefault((event.Tid, caller), []).append(dict(kind='time'))
                                    process.arm(caller, 'return', 'CRT return')
                                elif name == 'MSVCRT!qsort':
                                    base, count, width, comparator = struct.unpack('<4I', process.read(context.Esp+4, 16))
                                    if comparator in (0x432130, 0x40f8d0) and width == 4:
                                        action = dict(kind='qsort', caller=hex(caller), base=base,
                                                      count=count, comparator=comparator)
                                        action['before'] = sort_snapshot(base, count, comparator)
                                        returns.setdefault((event.Tid, caller), []).append(action)
                                        process.arm(caller, 'return', 'CRT return')
                            process.write(address, original)
                            context.EFlags |= 0x100
                            rearm[event.Tid] = address
                        checked(K.Wow64SetThreadContext(ht, C.byref(context)), 'Wow64SetThreadContext')
                    finally:
                        K.CloseHandle(ht)
                elif code in SS_CODES and event.Tid in rearm:
                    process.write(rearm.pop(event.Tid), b'\xcc')
                elif code not in BP_CODES:
                    status = DBG_NOT_HANDLED
                elif report['configured_before_entry']:
                    status = DBG_NOT_HANDLED  # only pre-entry loader breakpoints are swallowed
            checked(K.ContinueDebugEvent(event.Pid, event.Tid, status), 'ContinueDebugEvent')
            pending = False
        if not report['configured_before_entry']:
            raise RuntimeError(profile['label'] + ' exited before host configuration completed')
        return report['exit_code']
    except BaseException as error:
        report['error'] = str(error)
        raise
    finally:
        if pi.hProcess and not done:
            if job:
                K.TerminateJobObject(job, 1)
            K.TerminateProcess(pi.hProcess, 1)
            if pending:
                K.ContinueDebugEvent(event.Pid, event.Tid, DBG_CONTINUE)
            # Drain the debuggee synchronously, including its exit event.
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline:
                if K.WaitForDebugEvent(C.byref(event), 1000):
                    K.ContinueDebugEvent(event.Pid, event.Tid, DBG_CONTINUE)
                    if event.Code == 5:
                        break
        if pi.hProcess:
            report['process_wait_at_exit'] = K.WaitForSingleObject(pi.hProcess, 1000)
        if job:
            accounting = C.create_string_buffer(48)  # JOBOBJECT_BASIC_ACCOUNTING_INFORMATION, x64
            deadline = time.monotonic() + 10
            while True:
                checked(K.QueryInformationJobObject(job, 1, accounting, 48, None), 'QueryInformationJobObject')
                active = struct.unpack_from('<I', accounting, 40)[0]
                if not active or time.monotonic() >= deadline:
                    break
                K.TerminateJobObject(job, 1)
                time.sleep(0.01)
            report['job_active_processes_at_exit'] = active
        K.CloseHandle(pi.hThread)
        K.CloseHandle(pi.hProcess)
        if job:
            K.CloseHandle(job)
        report['tool_files_unchanged'] = sha(tool_path) == tool_hash and sha(crt_path) == crt_hash
        report['elapsed_seconds'] = round(time.monotonic()-started, 3)
        report_path.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--tool', choices=sorted(TOOLS), default='sp3-link',
                        help='pinned VC5 tool profile')
    parser.add_argument('--report', type=Path, default=ROOT/'build/histlink/host.json')
    parser.add_argument('--timestamp', type=lambda s: int(s, 0), default=ORACLE_TIME)
    parser.add_argument('--real-clock', action='store_true', help='control: retain the native wall clock')
    parser.add_argument('--system-crt', action='store_true', help='control: retain the tool\'s original system-CRT bindings')
    parser.add_argument('--timeout', type=int, default=120)
    parser.add_argument('--trace-rich', action='store_true', help='observe every module counted by the Rich writer')
    parser.add_argument('arguments', nargs=argparse.REMAINDER, help='-- followed by tool arguments')
    args = parser.parse_args()
    arguments = args.arguments[1:] if args.arguments[:1] == ['--'] else args.arguments
    if not arguments:
        parser.error('supply tool arguments after --')
    if not 0 <= args.timestamp <= 0x7fffffff or args.timeout <= 0:
        parser.error('timestamp must fit VC5 time_t; timeout must be positive')
    try:
        return launch(arguments, args.report, tool_kind=args.tool, historical_crt=not args.system_crt,
                      timestamp=None if args.real_clock else args.timestamp, timeout=args.timeout, trace_rich=args.trace_rich)
    except (OSError, RuntimeError, ValueError) as error:
        print('histlink: ' + str(error), file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
