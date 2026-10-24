"""Canonical target wrapper, relocating only its generated inventory shadow."""
import sys
sys.dont_write_bytecode=True
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools'))
code=(ROOT/'tools/cmpfunc_target.py').read_text()
old='Path(__file__).resolve().parents[1] / "build" / "targets"'
assert old in code
code=code.replace(old,repr(ROOT/'build/w-sb1icf-inventory'))
# repr(Path) is WindowsPath(...); use a Path constructor literal instead.
code=code.replace(repr(ROOT/'build/w-sb1icf-inventory'), 'Path('+repr(str(ROOT/'build/w-sb1icf-inventory'))+')')
exec(compile(code,str(ROOT/'tools/cmpfunc_target.py'),'exec'),
    {'__name__':'__main__','__file__':str(ROOT/'tools/cmpfunc_target.py')})
