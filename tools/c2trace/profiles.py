"""Pinned VC5 expression tracing profiles; SP3 by default, target context on request."""
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
import targets


@dataclass(frozen=True)
class Profile:
    name: str
    sha256: str
    compare: int
    construct: int
    function: int
    current: int
    frame: int
    next_symbol: int
    store: int
    load: int
    swap_return: int
    sort: int
    merge: int
    merge_return: int
    key: int
    hash: int
    hash_end: int
    load_signature: str

    @property
    def c2(self):
        return Path('C:/tools') / ('msvc5_' + self.name) / 'VC/BIN/C2.EXE'

    @property
    def work(self):
        return ROOT / ('build/c2trace' if self.name == 'sp3' else 'build/c2trace-rtm')

    def verify(self):
        import pefile
        data = self.c2.read_bytes()
        if hashlib.sha256(data).hexdigest() != self.sha256:
            raise RuntimeError('Unsupported C2 image for profile ' + self.name)
        pe = pefile.PE(data=data)
        signatures = [(self.compare, '8b410c8b4a0c3bc17207'),
                      (self.construct, '51535556578bf9'),
                      (self.function, '53568bf157b904000000'),
                      (self.store, '890d' + self.current.to_bytes(4, 'little').hex()),
                      (self.load, self.load_signature)]
        for va, signature in signatures:
            expected = bytes.fromhex(signature)
            if pe.get_data(va - pe.OPTIONAL_HEADER.ImageBase, len(expected)) != expected:
                raise RuntimeError('C2 signature mismatch at %#x (%s)' % (va, self.name))
        return self.sha256


PROFILES = {
    'sp3': Profile('sp3', 'e75aecaf4073b0817ffb638cae5fff636b2e2d1a090daabf8f68dbc954515fae',
        0x408d58, 0x4280e8, 0x42491e, 0x495f1c, 0x48fb48, 0x491050,
        0x453391, 0x4533c4, 0x4058e1, 0x40922a, 0x409306, 0x409324,
        0x405301, 0x4053bf, 0x405508, '8b0d1c5f4900e8194dfdff'),
    'rtm': Profile('rtm', '6d1406af5f62647347cfa7175cd7e108b838d6841a546396f29252198a14d73d',
        0x4046cc, 0x43fe5a, 0x40f391, 0x4970e4, 0x4970ec, 0x499784,
        0x447f0f, 0x447f42, 0x40853d, 0x40ad4b, 0x40ae27, 0x40ae45,
        0x407f5c, 0x40801a, 0x408164, '8b0de4704900e80d7fffff'),
}
_active = PROFILES['sp3']
_target = None


def get():
    return _active


def target():
    return _target


def add_arguments(parser):
    parser.add_argument('--compiler', choices=tuple(PROFILES), help='C2 profile (default SP3; target toolchain when supplied)')
    targets.add_argument(parser)


def configure(compiler=None, target_name=None):
    global _active, _target
    _target = targets.load(target_name) if target_name else None
    tree = _target.manifest()['toolchain']['tree'] if _target else 'sp3'
    name = compiler or tree
    if name not in PROFILES:
        raise ValueError('No tracing profile for compiler ' + name)
    _active = PROFILES[name]
    _active.verify()
    return _active


def cli_args():
    args = ['--compiler', get().name]
    if _target:
        args += ['--target', _target.name]
    return args


def stamp(cap):
    (cap / 'compiler.json').write_text(json.dumps(dict(compiler=get().name, c2_sha256=get().sha256,
        target=_target.name if _target else None), indent=2) + '\n')


def check_capture(cap, check_target=True):
    for name in ['compiler.json', 'capture.json']:
        path = cap / name
        if path.exists():
            meta = json.loads(path.read_text())
            if meta.get('c2_sha256') != get().sha256:
                raise RuntimeError('Capture compiler disagrees with selected profile: ' + str(cap))
            if check_target and 'target' in meta and meta['target'] != (_target.name if _target else None):
                raise RuntimeError('Capture target disagrees with selected target: ' + str(cap))


def require_sp3(feature):
    if get().name != 'sp3':
        raise NotImplementedError(feature + ' is calibrated only for SP3')
