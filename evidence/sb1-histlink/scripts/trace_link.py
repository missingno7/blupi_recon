"""Read-only supplemental telemetry in a private copy of canonical histlink.

Added breakpoints observe ICF folds; they do not select/change a contribution.
Existing histlink alone performs the documented CRT and clock setup.
"""
import sys
sys.dont_write_bytecode = True
import os
os.environ['PYTHONDONTWRITEBYTECODE'] = '1'
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
label=sys.argv[1]
out=ROOT/'build'/('w-sb1icf-trace-'+label)
out.mkdir(exist_ok=True)
prior=(ROOT/sys.argv[2]).resolve() if len(sys.argv)>2 else ROOT/'build'/('w-sb1icf-link-'+label)
rsp=(prior/'link.rsp').read_text(encoding='cp1252')
rsp=rsp.replace(str(prior/'BLUPI.EXE'),str(out/'BLUPI.EXE')).replace(str(prior/'BLUPI.map'),str(out/'BLUPI.map'))
(out/'link.rsp').write_text(rsp,encoding='cp1252')
code=(ROOT/'tools/histlink.py').read_text()
code=code.replace("ROOT = Path(__file__).resolve().parents[1]", "ROOT = Path("+repr(str(ROOT))+")")
code=code.replace("        if trace_rich:\n            process.arm", "        process.arm(0x432a4b, 'fold', 'ICF fold')\n        if trace_rich:\n            process.arm",1)
needle="                    item.update(size=process.u32(con+8)-process.u32(con+0x14),"
assert needle in code
code=code.replace(needle,"""                    module = process.u32(con+0x18)
                    item.update(con=hex(con), module=process.string(process.u32(module+4)),
                        module_index=process.u32(module+8), flags=hex(process.u32(con+4)),
                        con_fields=[hex(process.u32(con+i)) for i in range(0,0x40,4)])
"""+needle)
needle="                            elif kind == 'rich':"
assert needle in code
code=code.replace(needle,"""                            elif kind == 'fold':
                                report.setdefault('folds', []).append(dict(
                                    keeper=hex(context.Edi), folded=hex(context.Esi)))
"""+needle)
(HERE/'histlink-telemetry.diff').write_text(''.join(__import__('difflib').unified_diff(
    (ROOT/'tools/histlink.py').read_text().splitlines(True), code.splitlines(True),
    fromfile='tools/histlink.py',tofile='scratch/w-sb1icf/histlink-in-memory.py')))
sys.argv=['tools/histlink.py','--trace-rich','--report',str(out/'historical-host.json'),'--','@'+str(out/'link.rsp')]
namespace={'__name__':'__main__','__file__':str(ROOT/'tools/histlink.py')}
exec(compile(code,str(ROOT/'tools/histlink.py'),'exec'),namespace)
