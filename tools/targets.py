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
    status: str = "active"

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

    def manifest(self) -> dict[str, Any]:
        """The manifest with this target's defines and accepted-function overrides applied."""
        manifest = json.loads(self.manifest_path.read_text(encoding="utf-8"))
        if self.primary:
            return manifest
        manifest = copy.deepcopy(manifest)
        manifest["toolchain"]["defines"] = list(manifest["toolchain"].get("defines", [])) + self.defines
        for item in manifest.get("accepted_functions", []):
            override = self.accepted_overrides.get(item["symbol"])
            if override:
                for key in ("rva", "size", "status"):
                    if key in override:
                        item[key] = override[key]
                if override.get("exclude"):
                    item["withdrawn"] = {"reason": f"not part of target {self.name}"}
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
    return Target(name=name, exe=ROOT / spec["exe"], sha256=spec["sha256"],
                  manifest_path=ROOT / spec["manifest"], defines=list(spec.get("defines", [])),
                  game_text_end=int(end, 0) if isinstance(end, str) else int(end),
                  accepted_overrides=dict(spec.get("accepted_overrides", {})),
                  status=spec.get("status", "active"))


def add_argument(parser: argparse.ArgumentParser) -> None:
    parser.add_argument("--target", metavar="GAME/EDITION/LANGUAGE",
                        help=f"matching target from targets.json (default: {registry()['default']})")
