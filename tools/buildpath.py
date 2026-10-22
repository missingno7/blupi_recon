#!/usr/bin/env python3
"""Compile a translation unit under its historical absolute path (e.g. F:\\Profil\\sound.cpp).

MSVC 5 expands __FILE__ to the source path exactly as passed to cl.exe, so the original build path ends up in
the binary (BLUPI.EXE holds "F:\\Profil\\sound.cpp" at file offset 0x79c84 for TRY_DS). Reproducing that
string is a build-environment fact, not a source change: the reconstructed source stays untouched.

`historical_path()` maps the drive letter temporarily to a staging directory under build/histpath/<drive>/
with DefineDosDeviceW, which only affects the current logon session and is never persistent:
  * the drive must be unused; an existing mapping is never replaced or removed (the only exception is a stale
    mapping that points exactly at our own staging directory, left by a killed run);
  * removal uses DDD_EXACT_MATCH_ON_REMOVE with our own target, so only our definition can ever be deleted;
  * DDD_NO_BROADCAST_SYSTEM keeps Explorer and other windows from being notified;
  * a machine-wide lock (build/locks/drive-<X>.lock) serializes all users of the same drive letter;
  * the mapping is removed in `finally`, including on compile failure or KeyboardInterrupt.

Builds whose path lies on a drive in use (the SB2 builds' D:\\Blupi\\Profil\\ and D:\\Blupi\\Profil egames 2\\,
FOLDER_PATHS) use a folder instead, as the user allowed on 2026-10-22: the missing folders of the path are created for
one compile, under the same drive lock, and exactly what was created (the file and those folders) is removed again;
an existing file at the path is never overwritten.

Inspection / self-test:  .pyenv\\python.exe tools\\buildpath.py status F:
                         .pyenv\\python.exe tools\\buildpath.py selftest F:\\Profil\\sound.cpp
"""

from __future__ import annotations

import contextlib
import ctypes
from ctypes import wintypes
import os
from pathlib import Path, PureWindowsPath
import sys
import time

ROOT = Path(__file__).resolve().parents[1]

DDD_RAW_TARGET_PATH = 0x1
DDD_REMOVE_DEFINITION = 0x2
DDD_EXACT_MATCH_ON_REMOVE = 0x4
DDD_NO_BROADCAST_SYSTEM = 0x8
ERROR_FILE_NOT_FOUND = 2
ERROR_ACCESS_DENIED = 5
UNAVAILABLE_MARKER = "buildpath: UNAVAILABLE"

_kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
_kernel32.DefineDosDeviceW.argtypes = [wintypes.DWORD, wintypes.LPCWSTR, wintypes.LPCWSTR]
_kernel32.DefineDosDeviceW.restype = wintypes.BOOL
_kernel32.QueryDosDeviceW.argtypes = [wintypes.LPCWSTR, wintypes.LPWSTR, wintypes.DWORD]
_kernel32.QueryDosDeviceW.restype = wintypes.DWORD
_kernel32.GetLogicalDrives.restype = wintypes.DWORD


# Historical folders on a drive in use that may be created for one compile (user permission, 2026-10-22).
FOLDER_PATHS = (r"D:\Blupi\Profil", r"D:\Blupi\Profil egames 2")


class DriveInUse(RuntimeError):
    pass


class MappingDenied(RuntimeError):
    """The caller may not define DOS devices (e.g. the Codex restricted-token sandbox: ERROR_ACCESS_DENIED)."""


def query_drive(drive: str) -> list[str] | None:
    """Targets defined for `drive` (topmost first), or None when the letter is not defined at all."""
    buffer = ctypes.create_unicode_buffer(4096)
    length = _kernel32.QueryDosDeviceW(drive, buffer, len(buffer))
    if not length:
        error = ctypes.get_last_error()
        if error == ERROR_FILE_NOT_FOUND:
            return None
        raise OSError(error, f"QueryDosDeviceW({drive}) failed")
    return [item for item in buffer[:length].split("\0") if item]


def drive_is_free(drive: str) -> bool:
    bit = 1 << (ord(drive[0].upper()) - ord("A"))
    return query_drive(drive) is None and not (_kernel32.GetLogicalDrives() & bit) and not os.path.exists(drive + "\\")


def split_build_path(build_path: str) -> tuple[str, PureWindowsPath]:
    path = PureWindowsPath(build_path)
    if not path.drive or len(path.drive) != 2 or path.drive[1] != ":" or not path.root:
        raise ValueError(f"historical build path must be an absolute drive path like F:\\dir\\file.cpp: {build_path}")
    return path.drive.upper(), path.relative_to(path.anchor)


@contextlib.contextmanager
def _drive_lock(drive: str):
    import msvcrt
    lock_dir = ROOT / "build" / "locks"
    lock_dir.mkdir(parents=True, exist_ok=True)
    handle = open(lock_dir / f"drive-{drive[0]}.lock", "a+b")
    try:
        while True:
            try:
                msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
                break
            except OSError:
                time.sleep(0.1)
        try:
            yield
        finally:
            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
    finally:
        handle.close()


def _remove(drive: str, target: str) -> None:
    flags = DDD_REMOVE_DEFINITION | DDD_EXACT_MATCH_ON_REMOVE | DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM
    if not _kernel32.DefineDosDeviceW(flags, drive, target):
        raise OSError(ctypes.get_last_error(), f"could not remove temporary mapping {drive} -> {target}")


@contextlib.contextmanager
def _historical_folder(source: Path, build_path: str):
    """Create the missing folders of `build_path` and a copy of `source` there; remove exactly those afterwards."""
    path = Path(build_path)
    if path.exists():
        raise DriveInUse(f"{build_path} already exists; refusing to overwrite it")
    created: list[Path] = []
    folder = path.parent
    missing = []
    while not folder.exists():
        missing.append(folder)
        folder = folder.parent
    data = source.read_bytes()
    try:
        try:
            for item in reversed(missing):
                item.mkdir()
                created.append(item)
            with open(path, "xb") as stream:
                stream.write(data)
        except PermissionError as exc:
            raise MappingDenied(f"this process may not create {build_path} (sandboxed?): {exc}") from exc
        try:
            if path.read_bytes() != data:
                raise RuntimeError(f"{build_path} is not the copy of {source}")
            yield build_path
        finally:
            if path.read_bytes() == data:
                path.unlink()
    finally:
        for item in reversed(created):
            with contextlib.suppress(OSError):
                item.rmdir()   # only when empty: never removes anything we did not create
        left = [str(item) for item in created if item.exists()]
        if left:
            raise RuntimeError(f"temporary build folders left behind: {left}")


@contextlib.contextmanager
def historical_path(source: Path, build_path: str):
    """Yield `build_path` (a str) while it temporarily resolves to a byte-identical copy of `source`."""
    drive, relative = split_build_path(build_path)
    if str(PureWindowsPath(build_path).parent).lower() in (item.lower() for item in FOLDER_PATHS):
        with _drive_lock(drive):
            with _historical_folder(source, build_path) as path:
                yield path
        return
    staging = ROOT / "build" / "histpath" / drive[0]
    target = "\\??\\" + str(staging.resolve())
    with _drive_lock(drive):
        current = query_drive(drive)
        if current is not None:
            if current and current[0].lower() == target.lower():
                # Stale definition from a run that was killed before its finally block: ours, safe to drop.
                print(f"buildpath: removing stale temporary mapping {drive} -> {current[0]}")
                _remove(drive, current[0])
                current = query_drive(drive)
            if current is not None:
                raise DriveInUse(f"{drive} is already in use ({current}); refusing to touch it. Free the drive "
                                 f"letter to compile {source.name} under its historical path {build_path}.")
        if not drive_is_free(drive):
            raise DriveInUse(f"{drive} is in use (logical drive or network mapping); refusing to touch it.")
        staged = staging / relative
        staged.parent.mkdir(parents=True, exist_ok=True)
        data = source.read_bytes()
        staged.write_bytes(data)
        if not _kernel32.DefineDosDeviceW(DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM, drive, target):
            error = ctypes.get_last_error()
            with contextlib.suppress(OSError):
                staged.unlink()
            if error == ERROR_ACCESS_DENIED:
                raise MappingDenied(f"this process may not define {drive} (access denied; sandboxed?)")
            raise OSError(error, f"could not map {drive} -> {target}")
        try:
            mapped = query_drive(drive)
            if not mapped or mapped[0].lower() != target.lower():
                raise RuntimeError(f"{drive} does not resolve to the staging directory after mapping: {mapped}")
            if Path(build_path).read_bytes() != data:
                raise RuntimeError(f"{build_path} is not the staged copy of {source}")
            yield build_path
        finally:
            _remove(drive, target)
            remaining = query_drive(drive)
            if remaining and remaining[0].lower() == target.lower():
                raise RuntimeError(f"temporary mapping {drive} -> {target} is still present")
            with contextlib.suppress(OSError):
                staged.unlink()


def selftest(build_path: str) -> int:
    """Exercise the safety paths without compiling: success, failure inside, foreign mapping, stale mapping."""
    drive, _relative = split_build_path(build_path)
    if not drive_is_free(drive):
        print(f"skip: {drive} is in use on this machine")
        return 0
    scratch = ROOT / "build" / "tmp" / "buildpath-selftest"
    scratch.mkdir(parents=True, exist_ok=True)
    source = scratch / "probe.cpp"
    source.write_bytes(b"int probe;\r\n")
    ours = "\\??\\" + str((ROOT / "build" / "histpath" / drive[0]).resolve())

    try:
        with historical_path(source, build_path) as path:
            assert Path(path).read_bytes() == source.read_bytes()
    except MappingDenied as exc:
        print(f"skip: {exc}; compiles fall back to the real path and verify.py reports the gate as incomplete")
        assert drive_is_free(drive)
        return 0
    assert drive_is_free(drive), "mapping left after success"
    print("ok: mapped, staged copy readable, removed")

    try:
        with historical_path(source, build_path):
            raise KeyboardInterrupt
    except KeyboardInterrupt:
        pass
    assert drive_is_free(drive), "mapping left after failure"
    print("ok: removed after an exception inside the block")

    foreign = "\\??\\" + str(scratch.resolve())
    assert _kernel32.DefineDosDeviceW(DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM, drive, foreign)
    try:
        try:
            with historical_path(source, build_path):
                raise AssertionError("foreign mapping was not detected")
        except DriveInUse:
            pass
        assert query_drive(drive) == [foreign], f"foreign mapping changed: {query_drive(drive)}"
        print("ok: foreign mapping refused and left intact")
    finally:
        _remove(drive, foreign)

    assert _kernel32.DefineDosDeviceW(DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM, drive, ours)
    with historical_path(source, build_path):
        pass
    assert drive_is_free(drive), "stale mapping not cleaned"
    print("ok: stale mapping of our own staging directory removed")
    return 0


def main(argv: list[str]) -> int:
    if len(argv) == 3 and argv[1] == "status":
        drive = argv[2].upper().rstrip("\\")
        print(f"{drive} defined: {query_drive(drive)}; free: {drive_is_free(drive)}")
        return 0
    if len(argv) == 3 and argv[1] == "selftest":
        return selftest(argv[2])
    print(__doc__)
    return 2


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
