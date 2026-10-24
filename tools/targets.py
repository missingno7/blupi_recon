"""Matching targets (targets.json, docs/MULTI_GAME.md).

A target is one build of a game: game/edition/language, e.g. sb1/blupi/us
(the primary target, the one every acceptance number refers to). Tools take
--target and default to the primary target, so their behaviour without the
option is unchanged.

A target names its executable (path and SHA-256), the manifest it is verified
with, the defines its build adds to every TU, the end of its game code in
.text, and accepted-function overrides where its code differs from the
manifest's target (e.g. a language switch that changes a function's size).
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REGISTRY = ROOT / "targets.json"


@dataclass
class Target:
    name: str
    exe: Path
    sha256: str
    manifest_path: Path
    defines: list[str] = field(default_factory=list)
    game_text_end: int = 0x238B0
    accepted_overrides: dict[str, dict[str, Any]] = field(default_factory=dict)
    accepted_functions: list[dict[str, Any]] = field(default_factory=list)
    status: str = "active"
    oracle_dir: Path | None = None        # own inventory (tools/derive_target.py); None: oracle/
    address_map: dict[str, Any] | None = None
    toolchain_tree: str | None = None     # compiler release, if not the manifest's
    build_path: str | None = None         # the historical __FILE__ path of the TU that has one, if not the manifest's
    source_overlays: list[str] = field(default_factory=list)  # whole-file edition revisions, last wins

    @property
    def primary(self) -> bool:
        return self.status == "primary"

    @property
    def slug(self) -> str:
        return self.name.replace("/", "-")

    def check_exe(self) -> None:
        digest = hashlib.sha256(self.exe.read_bytes()).hexdigest()
        if digest != self.sha256:
            raise RuntimeError(f"{self.exe} is not the registered build of {self.name} (SHA-256 {digest})")

    def translate(self, va: int, kind: str) -> int | None:
        """A source-target VA at this target's address (address map), or None."""
        if not self.address_map:
            return va
        if kind == "code":
            for src, dst, size in self.address_map.get("functions", []):
                start = int(src, 16)
                if start <= va < start + max(size, 1):
                    return int(dst, 16) + (va - start)
        else:
            for src, dst in self.address_map.get("data_points", []):
                if int(src, 16) == va:
                    return int(dst, 16)
        runs = [(int(a, 16), int(b, 16), shift) for a, b, shift in self.address_map[kind]]
        for start, end, shift in runs:
            if start <= va < end:
                return va + shift
        # between two segments with the same shift (functions not located on their own)
        for (_, end, shift), (start, _, shift2) in zip(runs, runs[1:]):
            if end <= va < start and shift == shift2:
                return va + shift
        return None

    def manifest(self) -> dict[str, Any]:
        """The manifest with this target's defines, address map and overrides applied."""
        manifest = json.loads(self.manifest_path.read_text(encoding="utf-8"))
        if self.source_overlays:
            manifest["source_overlays"] = list(manifest.get("source_overlays", [])) + self.source_overlays
        if self.primary:
            return manifest
        manifest = copy.deepcopy(manifest)
        manifest["toolchain"]["defines"] = list(manifest["toolchain"].get("defines", [])) + self.defines
        if self.toolchain_tree:
            manifest["toolchain"]["tree"] = self.toolchain_tree
        if self.build_path:
            for unit in manifest["translation_units"]:
                if unit.get("build_path"):
                    unit["build_path"] = self.build_path
        if self.address_map:
            base = 0x400000

            def code_rva(value):
                va = self.translate(base + int(str(value), 0), "code")
                return None if va is None else va - base

            located = []
            for item in manifest.get("accepted_functions", []):
                # a function not located in the target is no expectation for it
                rva = code_rva(item["rva"])
                into = code_rva(item["into"]["rva"]) if item.get("into") else None
                if rva is None or (item.get("into") and into is None):
                    continue
                item["rva"] = rva
                if into is not None:
                    item["into"]["rva"] = into
                located.append(item)
            manifest["accepted_functions"] = located
            mappings = []
            for item in manifest.get("symbol_mappings", []):
                va = int(str(item["va"]), 0)
                new = self.translate(va, "code") if va < base + 0x2C000 else self.translate(va, "data")
                if new is not None:
                    mappings.append(dict(item, va=new))
            manifest["symbol_mappings"] = mappings
            forced = []
            for item in manifest.get("forced_locations", []):
                rva = code_rva(item["rva"])
                if rva is not None:
                    forced.append(dict(item, rva=rva))
            manifest["forced_locations"] = forced
            manifest["equivalent_functions"] = []   # proofs against the primary executable
        # Target-local proofs already use this executable's RVAs. They can cover
        # bodies absent, unlocated or still mismatching in the base target.
        local_keys = {(item["symbol"], item["tu"]) for item in self.accepted_functions}
        manifest["accepted_functions"] = [item for item in manifest.get("accepted_functions", [])
                                          if (item["symbol"], item["tu"]) not in local_keys]
        manifest["accepted_functions"].extend(copy.deepcopy(self.accepted_functions))
        for item in manifest.get("accepted_functions", []):
            override = self.accepted_overrides.get(item["symbol"])
            if override:
                # A sibling target can prove a body still withdrawn in the base manifest.
                # Its explicit withdrawn: null override reactivates only this target.
                for key in ("rva", "size", "status", "into", "withdrawn"):
                    if key in override:
                        item[key] = override[key]
                if override.get("status") == "strict":
                    item.pop("status", None)
                    item.pop("into", None)
                if override.get("exclude"):
                    item["exclude"] = True
        manifest["accepted_functions"] = [item for item in manifest.get("accepted_functions", [])
                                          if not item.get("exclude")]
        return manifest


def registry() -> dict[str, Any]:
    return json.loads(REGISTRY.read_text(encoding="utf-8"))


def names(active_only: bool = True) -> list[str]:
    targets = registry()["targets"]
    return [n for n, t in targets.items() if not active_only or t.get("status") in ("primary", "active")]


def load(name: str | None = None) -> Target:
    data = registry()
    name = name or data["default"]
    spec = data["targets"].get(name)
    if spec is None:
        raise ValueError(f"unknown target {name!r}; known: {', '.join(data['targets'])}")
    if spec.get("status") not in ("primary", "active"):
        raise ValueError(f"target {name} is {spec.get('status')!r}: not verifiable yet (docs/MULTI_GAME.md)")
    end = spec.get("game_text_end", "0x238b0")
    oracle_dir = ROOT / spec["oracle"] if spec.get("oracle") else None
    address_map = None
    if oracle_dir is not None and (oracle_dir / "address_map.json").is_file():
        address_map = json.loads((oracle_dir / "address_map.json").read_text(encoding="utf-8"))
    return Target(name=name, exe=ROOT / spec["exe"], sha256=spec["sha256"],
                  manifest_path=ROOT / spec["manifest"], defines=list(spec.get("defines", [])),
                  game_text_end=int(end, 0) if isinstance(end, str) else int(end),
                  accepted_overrides=dict(spec.get("accepted_overrides", {})),
                  accepted_functions=list(spec.get("accepted_functions", [])),
                  status=spec.get("status", "active"), oracle_dir=oracle_dir, address_map=address_map,
                  toolchain_tree=spec.get("toolchain_tree"), build_path=spec.get("build_path"),
                  source_overlays=list(spec.get("source_overlays", [])))


def add_argument(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--target", metavar="GAME/EDITION/LANGUAGE",
                        help=f"matching target from targets.json (default: {registry()['default']})")
