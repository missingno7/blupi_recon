"""Same fresh candidate objects, substituting only the attested sound input."""
import sys
sys.dont_write_bytecode=True
import json,shutil,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
HERE=ROOT/'scratch/w-sb1icf'
source=ROOT/'build/w-sb1icf-solver-us-tail8'
dest=ROOT/'build/w-sb1icf-solver-tail8-guard'
for name in ['objects','logs']:
    shutil.copytree(source/name,dest/name,dirs_exist_ok=True)
guard=ROOT/'build/w-sb1icf-guard-base'
for name in ['objects/08_sound.obj','logs/08_sound.log']:
    shutil.copyfile(guard/name,dest/name)
(HERE/'solver-guard-provenance.json').write_text(json.dumps(dict(
    candidate_source=str(source),sound_source=str(guard/'objects/08_sound.obj'),
    sound_sha256=hashlib.sha256((guard/'objects/08_sound.obj').read_bytes()).hexdigest()),indent=2))
print('Preserved-sound candidate objects ready; other 18 objects are from the fresh tail8 build.')
