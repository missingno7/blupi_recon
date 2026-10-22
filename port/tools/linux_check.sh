#!/bin/sh
# Linux build and test of the port (port/docs/ROADMAP.md stage 0.4).
#
# Copies src/, port/ and the game data to a directory on a case-sensitive file
# system, then configures (preset "linux"), builds and runs the whole ctest
# suite there. A copy is used because a Windows drive mounted in WSL is
# case-insensitive, which would hide exactly the bugs this check is for.
#
#   port/tools/linux_check.sh [ctest arguments...]
#
#   BLUPI_LINUX_WORK  work directory (default: ~/blupi-linux)
#   BLUPI_DATA        game folder (default: assets/Speedy Blupi of this repo)
#
# Needs CMake, Ninja, a C++17 compiler, rsync and the SDL3 development files
# (Ubuntu: sudo apt install libsdl3-dev ninja-build).
set -eu

REPO=$(cd "$(dirname "$0")/../.." && pwd)
WORK=${BLUPI_LINUX_WORK:-$HOME/blupi-linux}
DATA=${BLUPI_DATA:-$REPO/assets/Speedy Blupi}

mkdir -p "$WORK"
rsync -a --delete "$REPO/src" "$REPO/port" "$WORK/"
rsync -a --delete "$DATA/" "$WORK/data/"

cd "$WORK/port"
BLUPI_DATA="$WORK/data" cmake --preset linux -S .
cmake --build "$WORK/build/port-linux" -j 4
ctest --test-dir "$WORK/build/port-linux" -j 3 --output-on-failure "$@"
