#!/usr/bin/env python3
"""Small provenance/impact gate. No build tools, compiler, or semantic inference.

Exit 0: consistent/cleared; 1: review required; 2: invalid input or git failure.
Hashes are SHA-256 of bytes with CRLF converted to LF (checkout independent).
"""
import argparse
from collections import Counter, defaultdict
from functools import lru_cache
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
MAP = "port/provenance.json"
KINDS = {"DIRECT_SHARED", "VERIFIED_DERIVATIVE", "VERIFIED_PLATFORM_REPLACEMENT",
         "INTENTIONAL_DIFFERENCE", "UNVERIFIED"}
SOURCE = {".cpp", ".c", ".cc", ".h", ".hpp", ".rc", ".in"}


def digest(data):
    return hashlib.sha256(data.replace(b"\r\n", b"\n")).hexdigest()


def git(*args):
    result = subprocess.run(["git", *args], cwd=ROOT, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    if result.returncode:
        raise ValueError(result.stderr.decode("utf-8", "replace").strip())
    return result.stdout


class Tree:
    def __init__(self, revision=None):
        self.revision = revision

    @lru_cache(maxsize=None)
    def read(self, path):
        if self.revision is None:
            p = ROOT / path
            return p.read_bytes() if p.is_file() else None
        # git show also works for an absent path without requiring a worktree.
        result = subprocess.run(["git", "show", f"{self.revision}:{path}"], cwd=ROOT,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if result.returncode:
            error = result.stderr.decode("utf-8", "replace")
            if "does not exist in" in error or "exists on disk, but not in" in error:
                return None
            raise ValueError(error.strip())
        return result.stdout

    def files(self, root):
        if self.revision is None:
            return {p.relative_to(ROOT).as_posix() for p in (ROOT / root).rglob("*")
                    if p.is_file()}
        data = self.read(root)
        if data is None:
            return set()
        lines = data.decode("utf-8").splitlines()
        if not lines or not lines[0].startswith("tree "):
            raise ValueError(f"not a tree: {self.revision}:{root}")
        found = set()
        for name in lines[2:]:
            path = root + "/" + name.rstrip("/")
            found.update(self.files(path) if name.endswith("/") else {path})
        return found

    def json(self, path):
        data = self.read(path)
        if data is None:
            raise ValueError(f"missing {path} at {self.revision or 'working tree'}")
        return json.loads(data)


def target_policy(tree):
    """Same whole-file, last-overlay-wins rule as tools/source_tree.py.

    source_root comes from the manifest unless the target overrides it. In
    particular SB2's manifest supplies games/sb2/src, not targets.json's default.
    """
    result = {}
    for target, config in tree.json("targets.json")["targets"].items():
        manifest = tree.json(config.get("manifest", "recon.json"))
        policy = dict(manifest)
        policy.update(config)
        result[target] = (policy.get("source_root", "src"),
                          policy.get("source_overlays", []))
    return result


def effective(tree, policy, target, logical):
    root, overlays = policy[target]
    relative = PurePosixPath(logical).relative_to(root).as_posix()
    for overlay in reversed(overlays):
        candidate = overlay + "/" + relative
        if tree.read(candidate) is not None:
            return candidate
    return logical


def paths(mapping, edge):
    found = set(edge["dependency_paths"])
    for group in edge["dependency_groups"]:
        found.update(mapping["dependency_groups"][group])
    for item in edge["historical"]:
        found.add(item["logical_path"])
        found.update(item["effective_paths"].values())
    found.update(item["path"] for item in edge["port"])
    return found


def shares(tree):
    found = set()
    for path, variable, root in [
        ("port/CMakeLists.txt", "BLUPI_SHARED_TUS", "src"),
        ("port/games/sb2/CMakeLists.txt", "SB2_SHARED_TUS", "games/sb2/src"),
    ]:
        data = tree.read(path)
        if data is None:
            raise ValueError(f"missing direct-share CMake file: {path}")
        text = data.decode("utf-8")
        text = re.sub(r"#[^\n]*", "", text)
        match = re.search(r"set\(\s*" + variable + r"\s+([^)]*)\)", text)
        if not match:
            raise ValueError(f"cannot read {variable} in {path}")
        for name in match[1].split():
            if not re.fullmatch(r"[a-zA-Z0-9_]+(?:\.cpp)?", name):
                raise ValueError(f"unsupported CMake shared TU: {name}")
            found.add(root + "/" + name + ("" if name.endswith(".cpp") else ".cpp"))
    return found


def consistency(mapping, tree, hashes=True):
    errors = []
    if mapping["schema"] != 1:
        raise ValueError("unsupported provenance schema")
    policy = target_policy(tree)
    catalog = set(mapping["supported_targets"]) | set(mapping["excluded_targets"])
    if catalog != set(policy):
        errors.append("target catalog changed: review supported/excluded targets")
    for target, reason in mapping["excluded_targets"].items():
        if not reason:
            errors.append(f"excluded target lacks reason: {target}")
    owners = defaultdict(set)
    direct = set()
    ids = set()
    for edge in mapping["edges"]:
        eid = edge["id"]
        if eid in ids:
            errors.append(f"duplicate edge {eid}")
        ids.add(eid)
        if edge["kind"] not in KINDS:
            errors.append(f"{eid}: invalid kind")
        for field in ("game", "editions", "targets", "reason", "proof_scope", "tests", "evidence"):
            if not edge[field]:
                errors.append(f"{eid}: empty {field}")
        review = edge["review"]
        if not re.fullmatch(r"[0-9a-f]{40}", review["commit"]) or not review["disposition"]:
            errors.append(f"{eid}: missing immutable review commit/disposition")
        if edge["kind"] == "UNVERIFIED" and not edge.get("follow_up"):
            errors.append(f"{eid}: UNVERIFIED needs follow_up")
        for item in edge["historical"]:
            applicable = {t for t in edge["targets"] if t in policy
                          and item["logical_path"].startswith(policy[t][0] + "/")}
            if set(item["effective_paths"]) != applicable:
                errors.append(f"{eid}: effective target set differs")
            for target, recorded in item["effective_paths"].items():
                if target not in policy or effective(tree, policy, target, item["logical_path"]) != recorded:
                    errors.append(f"{eid}: effective overlay changed for {target}: {item['logical_path']}")
        if edge["kind"] == "DIRECT_SHARED":
            logical = [item["logical_path"] for item in edge["historical"]]
            physical = [item["path"] for item in edge["port"]]
            if len(logical) != 1 or physical != logical:
                errors.append(f"{eid}: DIRECT_SHARED must own one identical physical TU")
            direct.update(logical)
        for path in paths(mapping, edge):
            owners[path].add(eid)
    expected = shares(tree)
    if direct != expected:
        errors.append(f"CMake direct-share mismatch: missing={sorted(expected-direct)} extra={sorted(direct-expected)}")
    ignored = mapping["not_relevant"]
    for path, reason in ignored.items():
        if not reason or path in owners:
            errors.append(f"invalid not_relevant entry: {path}")
    roots = {root for root, _ in policy.values()}
    overlays = {p for _, ps in policy.values() for p in ps}
    required = set()
    for root in roots:
        required.update(p for p in tree.files(root) if PurePosixPath(p).suffix.lower() in SOURCE)
    for root in overlays:
        required.update(tree.files(root))
    for root in ("port/games", "port/common"):
        required.update(p for p in tree.files(root) if PurePosixPath(p).suffix.lower() in SOURCE)
    for path in sorted(required - owners.keys() - ignored.keys()):
        errors.append(f"UNOWNED source/header/overlay: {path}")
    referenced = set(owners) | set(ignored)
    hashed = set(owners) | {p for p in ignored if PurePosixPath(p).suffix.lower() != ".md"}
    if set(mapping["reviewed_files"]) != hashed:
        errors.append("reviewed_files must contain exactly owned paths and non-prose exceptions")
    for path in sorted(referenced):
        if PurePosixPath(path).is_absolute() or ".." in PurePosixPath(path).parts or "\\" in path:
            raise ValueError(f"non-repository path: {path}")
        data = tree.read(path)
        if data is None:
            errors.append(f"MISSING {path}: edges={','.join(sorted(owners[path])) or 'not_relevant'}")
        elif hashes and path in hashed and digest(data) != mapping["reviewed_files"].get(path):
            errors.append(f"UNREVIEWED {path}: edges={','.join(sorted(owners[path])) or 'not_relevant'}")
    return errors


def noncode_only(old, new, path):
    """Deliberately narrow lexical proof, never a C++ equivalence test.

    Keep internal code spacing, literals and each logical line's boundary.
    Only indentation, trailing spaces and comment contents are discarded.
    Reject raw strings, continuations, trigraphs, non-ASCII and __LINE__ uses.
    Directives must stay exact after trimming; blank lines remain significant.
    """
    if old is None or new is None or PurePosixPath(path).suffix.lower() not in {".cpp", ".h", ".c", ".hpp"}:
        return False
    def tokens(data):
        text = data.replace(b"\r\n", b"\n").decode("ascii")
        if any(x in text for x in ('R"', "\\\n", "??", "__LINE__", "__FILE__")):
            raise ValueError("ambiguous lexical context")
        directives = [line.strip() for line in text.split("\n") if line.lstrip().startswith("#")]
        # Strings/chars are indivisible; comments become spaces, retaining lines.
        pattern = r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'|//[^\n]*|/\*[\s\S]*?\*/'
        text = re.sub(pattern, lambda m: (" " + "\n" * m[0].count("\n"))
                      if m[0].startswith(("//", "/*")) else m[0], text)
        if "/*" in text or "*/" in text:
            raise ValueError("unterminated comment")
        return directives, [line.strip() for line in text.split("\n")]
    try:
        return tokens(old) == tokens(new)
    except (ValueError, UnicodeDecodeError):
        return False


def action(edge):
    return {"DIRECT_SHARED": "verify equivalence", "VERIFIED_DERIVATIVE": "update port / verify equivalence",
            "VERIFIED_PLATFORM_REPLACEMENT": "verify equivalence",
            "INTENTIONAL_DIFFERENCE": "intentional difference", "UNVERIFIED": "investigate"}[edge["kind"]]


def impact(base, head):
    before, after = Tree(base), Tree(head)
    # Validate revisions even for an empty diff; use only git diff and git show.
    changed = git("diff", "--name-only", "--no-renames", "-z", base, head, "--").decode("utf-8").split("\0")
    changed = sorted(p for p in changed if p)
    raw_head, raw_base = after.read(MAP), before.read(MAP)
    bootstrap = raw_head is None
    mapping = json.loads(raw_head if raw_head is not None else (ROOT / MAP).read_bytes())
    oldmap = json.loads(raw_base) if raw_base is not None else None
    oldedges = {e["id"]: e for e in oldmap["edges"]} if oldmap else {}
    print(f"Impact {base} -> {head}: {len(changed)} changed files")
    if bootstrap:
        print("Inventory: working-tree map (predates map installation; review acknowledgments disabled)")
    failures = 0
    if not bootstrap:
        # --check separately enforces exact snapshot hashes. Impact can clear a
        # lexical-only diff without pretending its old snapshot hash is current.
        errors = consistency(mapping, after, hashes=False)
        for error in errors:
            print("REVIEW:", error)
        failures += len(errors)
    all_edges = {e["id"]: (mapping, e) for e in mapping["edges"]}
    if oldmap:
        for edge in oldmap["edges"]:
            if edge["id"] not in all_edges:
                all_edges[edge["id"]] = (oldmap, edge)
    for path in changed:
        old, new = before.read(path), after.read(path)
        affected = [(m, e) for m, e in all_edges.values() if path in paths(m, e)
                    or (oldmap and e["id"] in oldedges and path in paths(oldmap, oldedges[e["id"]]))]
        transition = f"{path if old is not None else '(absent)'} -> {path if new is not None else '(deleted)'}"
        if path == MAP:
            print(f"MAP {transition}: reviewed dispositions/ownership (human review of this diff required)")
            continue
        harmless = old == new or (old is not None and new is not None and old.replace(b"\r\n", b"\n") == new.replace(b"\r\n", b"\n")) or noncode_only(old, new, path)
        if not affected:
            reason = mapping["not_relevant"].get(path)
            prose = PurePosixPath(path).suffix.lower() == ".md"
            recorded = (not bootstrap and reason and new is not None and
                        digest(new) == mapping["reviewed_files"].get(path) and
                        (oldmap is None or mapping["reviewed_files"].get(path) !=
                         oldmap["reviewed_files"].get(path)))
            if reason and (prose or harmless or recorded):
                print(f"NO ACTION {transition}: {reason}")
            else:
                print(f"REVIEW {transition}: unowned change; investigate / add explicit ownership or reason")
                failures += 1
            continue
        for m, edge in sorted(affected, key=lambda pair: pair[1]["id"]):
            eid = edge["id"]
            reviewed = False
            if not bootstrap and m is mapping:
                previous = oldedges.get(eid)
                # A shared hash refresh cannot silently approve all consumers.
                # Each affected relationship needs its own disposition update.
                changed_review = previous is None or edge["review"] != previous["review"]
                reviewed = changed_review and all(after.read(p) is not None and
                    digest(after.read(p)) == mapping["reviewed_files"].get(p) for p in paths(mapping, edge))
            label = "NO ACTION" if harmless else "REVIEWED" if reviewed else "REVIEW"
            required = "no action (identity/comments/whitespace/line endings)" if harmless else edge["review"]["disposition"] if reviewed else action(edge)
            print(f"{label} {eid} [{','.join(edge['targets'])}] {edge['kind']} {transition}; required: {required}; reason: {edge['reason']}; tests: {', '.join(edge['tests'])}")
            if edge["kind"] == "DIRECT_SHARED":
                print("  ownership: directly compiled physical TU; context/runtime review still required")
            if not harmless and not reviewed:
                failures += 1
    print(f"{'FAIL' if failures else 'PASS'}: {failures} unresolved impact/consistency items")
    return 1 if failures else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--base")
    parser.add_argument("--head")
    args = parser.parse_args()
    if args.check == bool(args.base or args.head) or bool(args.base) != bool(args.head):
        parser.error("use --check OR --base A --head B")
    try:
        if not args.check:
            return impact(args.base, args.head)
        mapping = Tree().json(MAP)
        errors = consistency(mapping, Tree())
        for error in errors:
            print("REVIEW:", error)
        counts = Counter(e["kind"] for e in mapping["edges"])
        print(f"{'FAIL' if errors else 'PASS'}: {len(mapping['edges'])} edges; " +
              ", ".join(f"{k}={v}" for k, v in sorted(counts.items())) +
              f"; {len(errors)} consistency issues")
        return 1 if errors else 0
    except (ValueError, KeyError, TypeError, OSError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
