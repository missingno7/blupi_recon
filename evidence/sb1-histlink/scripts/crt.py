"""Extract only historical library members for read-only section/alias inspection."""
import sys
sys.dont_write_bytecode=True
import json
from pathlib import Path
import pefile
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
sys.path.insert(0,str(ROOT/'tools'))
from cmpfunc import Coff
out=ROOT/'build/w-sb1icf-crt'
out.mkdir(exist_ok=True)
data=Path(r'C:\tools\msvc5_sp3\VC\LIB\LIBC.LIB').read_bytes()
assert data[:8]==b'!<arch>\n'
longnames=b''
pos=8
members={}
while pos+60<=len(data):
    header=data[pos:pos+60]
    assert header[58:60]==b'`\n'
    size=int(header[48:58])
    name=header[:16].decode().strip()
    body=data[pos+60:pos+60+size]
    if name=='//':
        longnames=body
    elif name not in ['/']:
        if name.startswith('/') and name[1:].isdigit():
            raw=longnames[int(name[1:]):]
            name=raw.split(b'\0')[0].split(b'\n')[0].decode().rstrip('/')
        stem=name.replace('\\','/').split('/')[-1].rstrip('/')
        if stem in ['aw_map.obj','aw_cmp.obj','trnsctrl.obj','chkstk.obj','ehvecdtr.obj','frame.obj']:
            p=out/stem
            p.write_bytes(body)
            members[stem]=p
    pos+=60+size+(size&1)
oracle=pefile.PE(str(ROOT/'assets/Speedy Blupi/BLUPI.EXE'))
text=next(s for s in oracle.sections if s.Name.rstrip(b'\0')==b'.text')
raw=text.get_data()[:text.Misc_VirtualSize]
report={}
for stem,p in members.items():
    c=Coff(p)
    wanted=[s for s in c.symbols if s.section>0 and (s.name in ['_strncnt','__chkstk','__alloca_probe'] or '_CallMemberFunction' in s.name or 'UnwindFilter' in s.name)]
    items=[]
    for s in wanted:
        section=c.sections[s.section-1]
        hits=[]
        start=0
        while section.raw and (idx:=raw.find(section.raw,start))>=0:
            hits.append(hex(text.VirtualAddress+idx));start=idx+1
        items.append(dict(name=s.name,section=s.section,value=s.value,symbol_index=s.index,
            raw_size=section.raw_size,characteristics=hex(section.characteristics),relocs=len(section.relocations),
            full_section_unmasked_oracle_hits=hits))
    report[stem]=items
(HERE/'crt.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
