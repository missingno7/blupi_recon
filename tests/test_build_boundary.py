"""The ordinary build (tools/build.py) stays independent of verification and research.

Static: build.py's local import closure is the ordinary-build set. Runtime: planning every target (inputs,
overlays, resources, toolchain pins) opens no original executable, oracle inventory, certificate, reference
repository or scratch file, and imports no verification/research module. The full compile + link of all
targets is checked by tools/check_clean_build.py in a pruned clean export.
"""
import ast
import json
from pathlib import Path
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
TOOLS = ROOT / "tools"
ORDINARY = {"build", "targets", "source_tree", "msvc", "buildpath"}
FORBIDDEN_DIRS = ("assets", "oracle", "evidence", "external", "scratch", "steering", "msvc5kb")


def local_imports(path):
    names = set()
    for node in ast.walk(ast.parse(path.read_text(encoding="utf-8"))):
        if isinstance(node, ast.Import):
            names.update(alias.name.split(".")[0] for alias in node.names)
        elif isinstance(node, ast.ImportFrom) and node.module and not node.level:
            names.add(node.module.split(".")[0])
    return {name for name in names if (TOOLS / f"{name}.py").is_file()}


PROBE = r"""
import json, sys
from pathlib import Path
root = Path(sys.argv[1]).resolve()
opened, imported = [], []
def hook(event, args):
    if event == "open" and args and isinstance(args[0], str):
        opened.append(args[0])
    elif event == "import" and args:
        imported.append(args[0])
sys.addaudithook(hook)
sys.path.insert(0, str(root / "tools"))
import io, contextlib
import build
with contextlib.redirect_stdout(io.StringIO()) as plans:
    status = build.main(["--plan", "--all"])
print(json.dumps(dict(status=status, opened=opened, imported=imported, plans=plans.getvalue().splitlines())))
"""


class BuildBoundary(unittest.TestCase):
    def test_import_closure(self):
        seen, pending = set(), ["build"]
        while pending:
            name = pending.pop()
            if name not in seen:
                seen.add(name)
                pending.extend(local_imports(TOOLS / f"{name}.py") - seen)
        self.assertLessEqual(seen, ORDINARY, f"build.py reaches non-build modules: {sorted(seen - ORDINARY)}")

    @unittest.skipUnless(Path(r"C:\tools\msvc5_sp3").is_dir() and Path(r"C:\tools\msvc5_rtm").is_dir(),
                         "historical toolchain not installed")
    def test_plan_reads_no_verification_inputs(self):
        run = subprocess.run([sys.executable, "-c", PROBE, str(ROOT)], capture_output=True, text=True, cwd=ROOT)
        self.assertEqual(run.returncode, 0, run.stderr)
        result = json.loads(run.stdout.splitlines()[-1])
        self.assertEqual(result["status"], 0)
        plans = [json.loads(line) for line in result["plans"]]
        registered = json.loads((ROOT / "targets.json").read_text(encoding="utf-8"))["targets"]
        self.assertEqual({p["target"] for p in plans}, set(registered))
        for plan in plans:
            self.assertEqual(plan["unpinned"], [], plan["target"])
        self.assertTrue(any(Path(p).name == "targets.json" for p in result["opened"]), "audit hook saw no opens")
        forbidden = [ROOT / d for d in FORBIDDEN_DIRS]
        bad = sorted({p for p in result["opened"] for d in forbidden
                      if Path(p).is_absolute() and Path(p).resolve().is_relative_to(d)})
        self.assertEqual(bad, [], "ordinary build planning opened verification/research inputs")
        modules = {m.split(".")[0] for m in result["imported"]}
        local = {m for m in modules if (TOOLS / f"{m}.py").is_file() or (TOOLS / m).is_dir()}
        self.assertLessEqual(local, ORDINARY, f"imported: {sorted(local - ORDINARY)}")
        self.assertNotIn("capstone", modules)


if __name__ == "__main__":
    unittest.main()
