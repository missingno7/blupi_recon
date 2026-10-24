"""Resolve a target's whole-file source overlays without changing logical TU names.

VC5 searches a quoted include beside the including file before /I directories.
An overlay /I alone therefore cannot override headers of a shared TU. Compile
overlaid targets from a private, complete tree; targets without overlays keep
their original paths. This module changes no source bytes or compiler options.
"""
from pathlib import Path
import shutil
import uuid

ROOT = Path(__file__).resolve().parents[1]


def base_root(manifest):
    return ROOT / manifest.get('source_root', 'src')


def roots(manifest):
    """Base first, then overlays in increasing precedence."""
    return [base_root(manifest)] + [ROOT / p for p in manifest.get('source_overlays', [])]


def resolve(manifest, canonical):
    """The canonical input supplying a logical TU/header (for provenance)."""
    path = Path(canonical)
    if not path.is_absolute():
        path = ROOT / path
    relative = path.relative_to(base_root(manifest))
    return next((root / relative for root in reversed(roots(manifest))
                 if (root / relative).is_file()), path)


def prepare(manifest, work):
    """A fresh effective include/source tree, under the caller's build output."""
    if not manifest.get('source_overlays'):
        return base_root(manifest)
    work = Path(work).resolve()
    if not any(work.is_relative_to(ROOT / p) for p in ('build', 'scratch')):
        raise ValueError('effective source tree must be inside build/ or scratch/')
    work.mkdir(parents=True, exist_ok=True)
    destination = work / ('sources-' + uuid.uuid4().hex)
    destination.mkdir()
    for root in roots(manifest):
        shutil.copytree(root, destination, dirs_exist_ok=True)
    return destination


def compile_source(manifest, tree, canonical, candidate=None):
    """Select the staged default, or preserve an explicit scratch substitution."""
    logical = ROOT / canonical
    if candidate is None or Path(candidate).resolve() in (logical.resolve(), resolve(manifest, canonical).resolve()):
        return Path(tree) / logical.relative_to(base_root(manifest))
    return Path(candidate)
