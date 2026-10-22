#!/bin/sh
# Linux release package of the port.
#
# Builds SDL 3.4.16 from source (its video and audio backends are loaded at run
# time, so one build runs under X11 or Wayland, with PulseAudio, PipeWire's
# Pulse server or ALSA), then the game with BLUPI_PORTABLE (libSDL3.so.0 next
# to the executable, GCC runtimes linked in), runs the whole test suite on
# that build and packs build/release/speedy-blupi-sdl3-<version>-linux-x86_64.tar.gz.
# Like linux_check.sh it works on a copy in a case-sensitive directory.
#
# Run it on the oldest system the package should support: its glibc is the
# minimum (releases: WSL Ubuntu 22.04, glibc 2.35; port/docs/RELEASING.md).
#
#   wsl -d Ubuntu-22.04 -u root -e sh port/tools/linux_release.sh
#
#   BLUPI_LINUX_WORK  work directory (default: ~/blupi-release)
#   BLUPI_DATA        game folder for the tests (default: assets/Speedy Blupi of this repo)
#   BLUPI_SDL3_SRC    SDL3 source tree (default: build/third_party/SDL3-3.4.16 of this repo)
#
# Needs what linux_check.sh needs, and the development files of SDL's
# backends, which decide what the SDL build supports (Ubuntu: libx11-dev
# libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxss-dev libxtst-dev
# libxfixes-dev libwayland-dev libxkbcommon-dev libdecor-0-dev libegl-dev
# libgles-dev libdrm-dev libgbm-dev libasound2-dev libpulse-dev libudev-dev
# libdbus-1-dev).
set -eu

REPO=$(cd "$(dirname "$0")/../.." && pwd)
WORK=${BLUPI_LINUX_WORK:-$HOME/blupi-release}
DATA=${BLUPI_DATA:-$REPO/assets/Speedy Blupi}
SDL=${BLUPI_SDL3_SRC:-$REPO/build/third_party/SDL3-3.4.16}
VERSION=$(head -n 1 "$REPO/port/VERSION.txt" | tr -d '\r')
NAME=speedy-blupi-sdl3-$VERSION-linux-x86_64

mkdir -p "$WORK"
rsync -a --delete "$REPO/src" "$REPO/port" "$WORK/"
rsync -a --delete "$DATA/" "$WORK/data/"
rsync -a --delete "$SDL/" "$WORK/sdl3-src/"

cmake -S "$WORK/sdl3-src" -B "$WORK/build/sdl3" -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$WORK/sdl3" -DCMAKE_INSTALL_LIBDIR=lib \
    -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
cmake --build "$WORK/build/sdl3" -j 4
cmake --install "$WORK/build/sdl3"

B="$WORK/build/release"
cmake -S "$WORK/port" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_PREFIX_PATH="$WORK/sdl3" -DBLUPI_PORTABLE=ON -DBLUPI_TEST_DATA="$WORK/data"
cmake --build "$B" -j 4
ctest --test-dir "$B" -j 3 --output-on-failure

OUT="$WORK/package/$NAME"
rm -rf "$WORK/package"
mkdir -p "$OUT"
cp "$B/blupi-sdl3" "$B/libSDL3.so.0" "$B/soundfont.sf2" "$OUT/"
strip --strip-unneeded "$OUT/blupi-sdl3" "$OUT/libSDL3.so.0"
sed -e 's/\r$//' -e "s/@VERSION@/$VERSION/" "$WORK/port/release/README.txt" > "$OUT/README.txt"
sed 's/\r$//' "$WORK/port/release/THIRD_PARTY.txt" > "$OUT/THIRD_PARTY.txt"
sed 's/\r$//' "$WORK/port/release/COPYING.txt" > "$OUT/COPYING.txt"
sed 's/\r$//' "$WORK/sdl3-src/LICENSE.txt" > "$OUT/LICENSE-SDL3.txt"
sed 's/\r$//' "$WORK/port/third_party/generaluser-gs/documentation/LICENSE.txt" > "$OUT/LICENSE-GeneralUser-GS.txt"
chmod 0644 "$OUT"/*.txt "$OUT/soundfont.sf2" "$OUT/libSDL3.so.0"
chmod 0755 "$OUT/blupi-sdl3"
mkdir -p "$REPO/build/release"
tar -C "$WORK/package" --owner=0 --group=0 --numeric-owner -czf "$REPO/build/release/$NAME.tar.gz" "$NAME"

echo "package: build/release/$NAME.tar.gz"
"$OUT/blupi-sdl3" --version
echo "needs glibc: $(objdump -T "$OUT/blupi-sdl3" "$OUT/libSDL3.so.0" | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -n 1)"
echo "libraries:"
objdump -p "$OUT/blupi-sdl3" "$OUT/libSDL3.so.0" | grep -E 'NEEDED|RUNPATH' | sort -u
