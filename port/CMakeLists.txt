cmake_minimum_required(VERSION 3.20)

# Speedy Blupi 1 native SDL3 port. See port/README.md.
#
# Ownership:
#   ../src/*                historical reconstruction (read-only from here; never patched for the port)
#   common/compat/          declaration-only Win32/DirectX names + their small SDL-free implementations
#   common/platform/        SDL3 host (window, input, timing, audio, file system, languages); no Win32 names
#   common/render/          software reference renderer (surfaces, blits); no SDL, no Win32 names
#   games/sb1/game/         Speedy Blupi 1's port-owned replacements of the historical platform TUs
#
# The port's release version (port/VERSION.txt; also read by port/android).
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt BLUPI_PORT_VERSION LIMIT_COUNT 1)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/VERSION.txt)
# A pre-release suffix (1.2.0-rc1) stays in the shown version; CMake and the Windows resource take the number.
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" BLUPI_PORT_VERSION_NUMBER "${BLUPI_PORT_VERSION}")
project(blupi_port VERSION ${BLUPI_PORT_VERSION_NUMBER} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Android builds SDL3 from its source (BLUPI_SDL3_SOURCE_DIR, passed by
# port/android/app/build.gradle) as libSDL3.so; elsewhere an installed SDL3.
if(ANDROID)
    set(BLUPI_SDL3_SOURCE_DIR "" CACHE PATH "SDL3 source tree")
    if(NOT EXISTS "${BLUPI_SDL3_SOURCE_DIR}/CMakeLists.txt")
        message(FATAL_ERROR "BLUPI_SDL3_SOURCE_DIR must point to an SDL3 source tree (see port/android/README.md)")
    endif()
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    add_subdirectory(${BLUPI_SDL3_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/SDL3 EXCLUDE_FROM_ALL)
else()
    find_package(SDL3 REQUIRED CONFIG)
endif()

set(BLUPI_HIST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src)
# The port's game-independent layers (compat/, platform/, render/) and the
# game's own code (game/); each is an include root, so includes read
# "platform/host.h", "game/port_game.h" (docs/MULTI_GAME.md).
set(BLUPI_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(BLUPI_GAME_DIR ${CMAKE_CURRENT_SOURCE_DIR}/games/sb1)

# Pinned, unmodified ENet C sources. It never sees the game's compat headers.
add_library(blupi_enet STATIC
    third_party/enet/callbacks.c third_party/enet/compress.c
    third_party/enet/host.c third_party/enet/list.c third_party/enet/packet.c
    third_party/enet/peer.c third_party/enet/protocol.c
    third_party/enet/unix.c third_party/enet/win32.c)
target_include_directories(blupi_enet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/enet/include)
if(WIN32)
    target_link_libraries(blupi_enet PUBLIC ws2_32 winmm)
else()
    target_compile_definitions(blupi_enet PRIVATE HAS_FCNTL=1 HAS_POLL=1 HAS_GETADDRINFO=1
        HAS_GETNAMEINFO=1 HAS_INET_PTON=1 HAS_INET_NTOP=1 HAS_SOCKLEN_T=1)
endif()
add_library(blupi_net STATIC common/net/session.cpp)
target_include_directories(blupi_net PUBLIC ${BLUPI_COMMON})
target_link_libraries(blupi_net PRIVATE blupi_enet)
add_library(blupi_sb1_protocol STATIC games/sb1/game/network_protocol.cpp)
target_include_directories(blupi_sb1_protocol PUBLIC ${BLUPI_GAME_DIR})

# Historical TUs compiled unchanged. The other seven historical TUs (blupi, ddutil,
# misc, movie, network, pixmap, sound) talk to Windows/DirectX/MCI and are replaced
# by port-owned files in game/.
set(BLUPI_SHARED_TUS
    button.cpp decblock.cpp decblupi.cpp decdesign.cpp decmove.cpp decnet.cpp
    decor.cpp dectables.cpp event.cpp jauge.cpp menu.cpp text.cpp)
list(TRANSFORM BLUPI_SHARED_TUS PREPEND ${BLUPI_HIST_SRC}/)

# Case-sensitive hosts: forward the spellings the historical source uses. The
# aliases name the real headers by absolute path, so they cannot include
# themselves where the build directory is case-insensitive (e.g. a Windows
# drive under WSL).
set(BLUPI_CASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/compat_case)
file(MAKE_DIRECTORY ${BLUPI_CASE_DIR})
if(NOT CMAKE_HOST_WIN32)
    file(WRITE ${BLUPI_CASE_DIR}/Windows.h "#include \"${BLUPI_COMMON}/compat/include/windows.h\"\n")
    file(WRITE ${BLUPI_CASE_DIR}/DEF.H "#include \"${BLUPI_HIST_SRC}/def.h\"\n")
endif()

# Interface carrying the historical-source compile context.
add_library(blupi_hist_context INTERFACE)
target_include_directories(blupi_hist_context BEFORE INTERFACE
    ${BLUPI_COMMON}/compat/include
    ${BLUPI_CASE_DIR}
    ${BLUPI_HIST_SRC})
if(MSVC)
    target_compile_options(blupi_hist_context INTERFACE
        /FI${BLUPI_COMMON}/compat/include/port_forced.h
        /FI${BLUPI_GAME_DIR}/game/port_switches.h
        /permissive /Zc:forScope- /W0)
else()
    # -fsigned-char: char is signed in MSVC (and on x86) but unsigned on ARM;
    # the historical code relies on the former (cp1252 bytes, -1 in char tables).
    # GCC needs -fpermissive and Clang -fms-extensions for the same MSVC
    # leniencies (e.g. CEvent::LoadState's (int) cast of a pointer variable
    # used as int storage).
    target_compile_options(blupi_hist_context INTERFACE
        "SHELL:-include ${BLUPI_COMMON}/compat/include/port_forced.h"
        "SHELL:-include ${BLUPI_GAME_DIR}/game/port_switches.h"
        -fno-strict-aliasing -fwrapv -fsigned-char -w
        $<$<CXX_COMPILER_ID:GNU>:-fpermissive>
        $<$<CXX_COMPILER_ID:Clang>:-fms-extensions>)
endif()

# The historical files are cp1252; keep their string bytes unchanged (GCC is
# told the charset; Clang has no such option and keeps the bytes as they are).
add_library(blupi_hist OBJECT ${BLUPI_SHARED_TUS})
target_link_libraries(blupi_hist PRIVATE blupi_hist_context)
# Their file I/O goes through BlupiFile (compat/include/port_hist_stdio.h).
if(MSVC)
    target_compile_options(blupi_hist PRIVATE /source-charset:.1252 /execution-charset:.1252
        /FI${BLUPI_COMMON}/compat/include/port_hist_stdio.h)
else()
    target_compile_options(blupi_hist PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-finput-charset=CP1252>
        $<$<CXX_COMPILER_ID:GNU>:-fexec-charset=CP1252>
        "SHELL:-include ${BLUPI_COMMON}/compat/include/port_hist_stdio.h")
endif()

# Software reference renderer: plain C++, no SDL and no Win32 names.
add_library(blupi_render STATIC
    common/render/bmp.cpp
    common/render/display_capture.cpp
    common/render/device.cpp
    common/render/surface.cpp)
target_include_directories(blupi_render PUBLIC ${BLUPI_COMMON})

# SDL3 host. Must not see compat/include.
add_library(blupi_platform STATIC
    common/platform/host.cpp
    common/platform/touch.cpp
    common/platform/vfs.cpp
    common/platform/audio.cpp
    common/platform/music.cpp
    common/platform/language.cpp
    common/platform/icon.cpp)
target_include_directories(blupi_platform PUBLIC ${BLUPI_COMMON})
target_include_directories(blupi_platform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})   # third_party/
target_link_libraries(blupi_platform PUBLIC SDL3::SDL3 blupi_render)

# The port's icon (res/icon.png, tools/make_icon.py), for the window on
# systems without executable resources.
set(BLUPI_ICON_INC ${CMAKE_CURRENT_BINARY_DIR}/generated/icon_png.inc)
add_custom_command(OUTPUT ${BLUPI_ICON_INC}
    COMMAND ${CMAKE_COMMAND} -DIN=${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png -DOUT=${BLUPI_ICON_INC}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_bytes.cmake
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.png ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_bytes.cmake
    COMMENT "Embedding res/icon.png")
target_sources(blupi_platform PRIVATE ${BLUPI_ICON_INC})
target_include_directories(blupi_platform PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

# Launcher: no historical context or game headers. Compat declarations are
# needed only at its message boundary; platform/ still never sees them.
add_library(blupi_app STATIC common/app/launcher.cpp)
target_include_directories(blupi_app PUBLIC ${BLUPI_COMMON} PRIVATE ${BLUPI_COMMON}/compat/include)
target_compile_definitions(blupi_app PRIVATE BLUPI_PORT_VERSION="${BLUPI_PORT_VERSION}")
target_link_libraries(blupi_app PUBLIC blupi_platform blupi_net)

# Port-owned replacements of the historical platform TUs, plus the shared
# historical objects: everything that sees the Win32 compatibility names.
add_library(blupi_game STATIC
    common/compat/src/win32.cpp
    common/compat/src/hist_stdio.cpp
    games/sb1/game/main.cpp
    games/sb1/game/misc.cpp
    games/sb1/game/pixmap.cpp
    games/sb1/game/sound.cpp
    games/sb1/game/movie.cpp
    games/sb1/game/network.cpp
    games/sb1/game/state_hash.cpp
    games/sb1/game/scene.cpp
    games/sb1/game/probe.cpp
    games/sb1/game/end_of_tick.cpp
    games/sb1/game/editor_wide.cpp
    common/ui/options_screen.cpp
    common/ui/settings.cpp
    games/sb1/game/language.cpp
    $<TARGET_OBJECTS:blupi_hist>)
target_link_libraries(blupi_game PRIVATE blupi_hist_context PUBLIC blupi_platform blupi_app blupi_sb1_protocol)
target_compile_definitions(blupi_game PRIVATE BLUPI_PORT_VERSION="${BLUPI_PORT_VERSION}")
target_include_directories(blupi_game PRIVATE ${BLUPI_GAME_DIR})

# The built-in language packs (languages/*.txt, port/docs/LANGUAGES.md),
# embedded for game/language.cpp.
file(GLOB BLUPI_LANGUAGE_PACKS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/languages/*.txt)
list(SORT BLUPI_LANGUAGE_PACKS)
string(REPLACE ";" "|" BLUPI_LANGUAGE_PACK_ARG "${BLUPI_LANGUAGE_PACKS}")
set(BLUPI_LANGUAGE_INC ${CMAKE_CURRENT_BINARY_DIR}/generated/language_packs.inc)
add_custom_command(OUTPUT ${BLUPI_LANGUAGE_INC}
    COMMAND ${CMAKE_COMMAND} "-DFILES=${BLUPI_LANGUAGE_PACK_ARG}" -DOUT=${BLUPI_LANGUAGE_INC}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_packs.cmake
    DEPENDS ${BLUPI_LANGUAGE_PACKS} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_packs.cmake
    COMMENT "Embedding the language packs"
    VERBATIM)
target_sources(blupi_game PRIVATE ${BLUPI_LANGUAGE_INC})
target_include_directories(blupi_game PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

# Entry point TU (SDL_main) without the compatibility include path. On Android
# the game is libmain.so, which SDL's Java activity loads.
if(ANDROID)
    add_library(blupi SHARED common/platform/entry.cpp)
    set_target_properties(blupi PROPERTIES OUTPUT_NAME main)
else()
    add_executable(blupi common/platform/entry.cpp)
    set_target_properties(blupi PROPERTIES OUTPUT_NAME blupi-sdl3)
endif()
target_link_libraries(blupi PRIVATE blupi_game SDL3::SDL3)
if(WIN32)
    # The port's icon (res/icon.ico, which SDL also uses for the window) and
    # the version information.
    set(BLUPI_RC_ICON ${CMAKE_CURRENT_SOURCE_DIR}/res/icon.ico)
    set(BLUPI_RC_DESCRIPTION "Speedy Blupi (SDL3 port)")
    set(BLUPI_RC_NAME blupi-sdl3)
    set(BLUPI_RC_PRODUCT "Speedy Blupi SDL3 port")
    configure_file(common/platform/blupi_win.rc.in ${CMAKE_CURRENT_BINARY_DIR}/generated/blupi_win.rc @ONLY)
    target_sources(blupi PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated/blupi_win.rc)
endif()

option(BLUPI_PORTABLE "Linux: SDL3 next to the executable, static C++ runtime" OFF)
include(cmake/package_app.cmake)

# GeneralUser GS SoundFont for the MIDI music: git submodule third_party/generaluser-gs
# (shallow, pinned; see third_party/README.md). Without it the game runs without music.
set(BLUPI_SOUNDFONT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/generaluser-gs/GeneralUser-GS.sf2)
option(BLUPI_BUNDLE_SOUNDFONT "Copy the GeneralUser GS SoundFont next to the executable" ON)
if(NOT EXISTS ${BLUPI_SOUNDFONT_FILE})
    message(WARNING "SoundFont submodule missing; the game will have no music. Run:\n"
                    "  git submodule update --init port/third_party/generaluser-gs")
    set(BLUPI_SOUNDFONT_FILE "")
endif()
# (Android: the APK carries it as an asset, see port/android/app/build.gradle.)
blupi_package_app(blupi)

# Speedy Blupi 2 / Speedy Eggbert 2: blupi2-sdl3 on the desktop, libblupi2.so on Android.
add_subdirectory(games/sb2)
if(NOT ANDROID)
    enable_testing()
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME fidelity_map COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_fidelity.py --check)
    add_subdirectory(tests)
endif()
