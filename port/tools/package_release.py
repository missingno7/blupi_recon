"""Packs the Windows and Android release files of the port into build/release/.

    python port/tools/package_release.py windows   # from build/port (MinGW, tested)
    python port/tools/package_release.py android   # the signed APK from build/port-android
    python port/tools/package_release.py sums      # SHA256SUMS.txt over build/release

The Linux package is made by port/tools/linux_release.sh. The whole procedure,
with the builds and tests that come first, is in port/docs/RELEASING.md.
"""
import hashlib
import os
import re
import shutil
import subprocess
import sys
import zipfile

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
PORT = os.path.join(ROOT, 'port')
OUT = os.path.join(ROOT, 'build', 'release')
VERSION = open(os.path.join(PORT, 'VERSION.txt')).readline().strip()
SDL_SOURCE = os.path.join(ROOT, 'build', 'third_party', 'SDL3-3.4.16')


def text(path, crlf):
    data = open(path, encoding='utf-8').read().replace('\r\n', '\n').replace('@VERSION@', VERSION)
    return data.replace('\n', '\r\n') if crlf else data


def cache_value(build, name):
    for line in open(os.path.join(build, 'CMakeCache.txt'), encoding='utf-8'):
        m = re.match(r'%s:[A-Z]+=(.*)' % name, line)
        if m:
            return m.group(1)
    return ''


def windows():
    build = os.path.join(ROOT, 'build', 'port')
    name = 'speedy-blupi-sdl3-%s-windows-x64' % VERSION
    apps = ('blupi-sdl3', 'blupi2-sdl3')
    for app in apps:
        out = subprocess.run([os.path.join(build, app + '.exe'), '--version'],
                             capture_output=True, text=True, check=True).stdout.strip()
        if out != app + ' ' + VERSION:
            sys.exit('%s is not version %s (%r): rebuild first' % (app, VERSION, out))
    staging = os.path.join(OUT, 'staging')
    shutil.rmtree(staging, ignore_errors=True)
    os.makedirs(staging)
    files = {
        'SDL3.dll': os.path.join(build, 'SDL3.dll'),
        'soundfont.sf2': os.path.join(build, 'soundfont.sf2'),
    }
    for app in apps:
        exe = app + '.exe'
        stripped = os.path.join(staging, exe)
        subprocess.run([cache_value(build, 'CMAKE_STRIP') or 'strip', '-o', stripped,
                        os.path.join(build, exe)], check=True)
        files[exe] = stripped
    texts = {
        'README.txt': os.path.join(PORT, 'release', 'README.txt'),
        'THIRD_PARTY.txt': os.path.join(PORT, 'release', 'THIRD_PARTY.txt'),
        'COPYING.txt': os.path.join(PORT, 'release', 'COPYING.txt'),
        'LICENSE-SDL3.txt': os.path.join(SDL_SOURCE, 'LICENSE.txt'),
        'LICENSE-GeneralUser-GS.txt': os.path.join(PORT, 'third_party', 'generaluser-gs', 'documentation', 'LICENSE.txt'),
    }
    path = os.path.join(OUT, name + '.zip')
    with zipfile.ZipFile(path, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for arc, src in files.items():
            z.write(src, name + '/' + arc)
        for arc, src in texts.items():
            z.writestr(name + '/' + arc, text(src, crlf=True))
    shutil.rmtree(staging)
    print('package: build/release/%s.zip' % name)


def android():
    apk = os.path.join(ROOT, 'build', 'port-android', 'app', 'outputs', 'apk', 'release', 'app-release.apk')
    if not os.path.exists(apk):
        sys.exit('no signed release APK: run gradlew assembleRelease (port/docs/RELEASING.md)')
    name = 'speedy-blupi-sdl3-%s-android.apk' % VERSION
    shutil.copyfile(apk, os.path.join(OUT, name))
    print('package: build/release/' + name)


def sums():
    lines = []
    for f in sorted(os.listdir(OUT)):
        # Exactly this version's packages: 1.2.0 must not also take 1.2.0-rc2's.
        if re.fullmatch(r'speedy-blupi-sdl3-%s-(windows-x64\.zip|linux-x86_64\.tar\.gz|android\.apk)'
                        % re.escape(VERSION), f):
            digest = hashlib.sha256(open(os.path.join(OUT, f), 'rb').read()).hexdigest()
            lines.append('%s  %s\n' % (digest, f))
    open(os.path.join(OUT, 'SHA256SUMS.txt'), 'w', newline='\n').write(''.join(lines))
    print(''.join(lines), end='')


def main():
    os.makedirs(OUT, exist_ok=True)
    actions = {'windows': windows, 'android': android, 'sums': sums}
    if len(sys.argv) < 2 or any(a not in actions for a in sys.argv[1:]):
        sys.exit(__doc__)
    for a in sys.argv[1:]:
        actions[a]()


if __name__ == '__main__':
    main()
