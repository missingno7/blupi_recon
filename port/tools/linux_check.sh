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
#   BLUPI_TEST_DATA_EGGBERT  Eggbert 1 folder (default: assets/eggbert/Game, if present)
#   BLUPI2_DATA       Eggbert 2 folder (default: assets/eggbert2, if present)
#   BLUPI2_TEST_DATA_BLUPI  Blupi 2 folder (default: assets/blupi2, if present)
#
# Needs CMake, Ninja, a C++17 compiler, rsync and the SDL3 development files
# (Ubuntu: sudo apt install libsdl3-dev ninja-build).
set -eu

REPO=$(cd "$(dirname "$0")/../.." && pwd)
WORK=${BLUPI_LINUX_WORK:-$HOME/blupi-linux}
DATA=${BLUPI_DATA:-$REPO/assets/Speedy Blupi}

mkdir -p "$WORK"
rsync -a --delete "$REPO/src" "$REPO/port" "$WORK/"
mkdir -p "$WORK/games/sb2/src"
rsync -a --delete "$REPO/games/sb2/src/" "$WORK/games/sb2/src/"
rsync -a --delete "$DATA/" "$WORK/data/"

# Keep Windows data paths out of the Linux cache. The SB2 tests are optional
# when their data is absent, just as in the desktop CMake configuration.
DATA2=${BLUPI2_DATA:-$REPO/assets/eggbert2}
BLUPI2=${BLUPI2_TEST_DATA_BLUPI:-$REPO/assets/blupi2}
EGGBERT1=${BLUPI_TEST_DATA_EGGBERT:-$REPO/assets/eggbert/Game}
EGGBERT2_TEST=
BLUPI2_TEST=
EGGBERT1_TEST=
if [ -d "$EGGBERT1" ]; then
    rsync -a --delete "$EGGBERT1/" "$WORK/eggbert-data/"
    EGGBERT1_TEST=$WORK/eggbert-data
fi
if [ -d "$DATA2" ]; then
    rsync -a --delete "$DATA2/" "$WORK/eggbert2-data/"
    EGGBERT2_TEST=$WORK/eggbert2-data
fi
if [ -d "$BLUPI2" ]; then
    rsync -a --delete "$BLUPI2/" "$WORK/blupi2-data/"
    BLUPI2_TEST=$WORK/blupi2-data
fi

cd "$WORK/port"
BLUPI_DATA="$WORK/data" cmake --preset linux -S . \
    -DBLUPI2_TEST_DATA="$EGGBERT2_TEST" -DBLUPI2_TEST_DATA_BLUPI="$BLUPI2_TEST" \
    -DBLUPI_TEST_DATA_EGGBERT="$EGGBERT1_TEST"
cmake --build "$WORK/build/port-linux" -j 4
# Both desktop entry points use common/app; Android still builds SB1 only.
"$WORK/build/port-linux/blupi-sdl3" --version
"$WORK/build/port-linux/blupi2-sdl3" --version
ctest --test-dir "$WORK/build/port-linux" -j 1 --output-on-failure "$@"
