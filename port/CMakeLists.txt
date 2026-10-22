cmake_minimum_required(VERSION 3.20)

# Speedy Blupi 1 native SDL3 port. See port/README.md.
#
# Ownership:
#   ../src/*            historical reconstruction (read-only from here; never patched for the port)
#   compat/             declaration-only Win32/DirectX names + their small SDL-free implementations
#   game/               port-owned replacements of the historical platform TUs
#   platform/           SDL3 host (window, input, timing, audio, file system); no Win32 names
#   render/             software reference renderer (surfaces, blits); no SDL, no Win32 names
project(blupi_port LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED CONFIG)

set(BLUPI_HIST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

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
    file(WRITE ${BLUPI_CASE_DIR}/Windows.h "#include \"${CMAKE_CURRENT_SOURCE_DIR}/compat/include/windows.h\"\n")
    file(WRITE ${BLUPI_CASE_DIR}/DEF.H "#include \"${BLUPI_HIST_SRC}/def.h\"\n")
endif()

# Interface carrying the historical-source compile context.
add_library(blupi_hist_context INTERFACE)
target_include_directories(blupi_hist_context BEFORE INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/compat/include
    ${BLUPI_CASE_DIR}
    ${BLUPI_HIST_SRC})
if(MSVC)
    target_compile_options(blupi_hist_context INTERFACE
        /FI${CMAKE_CURRENT_SOURCE_DIR}/compat/include/port_forced.h
        /permissive /Zc:forScope- /W0)
else()
    target_compile_options(blupi_hist_context INTERFACE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/compat/include/port_forced.h
        -fpermissive -fno-strict-aliasing -fwrapv -w)
endif()

# The historical files are cp1252; keep their string bytes unchanged.
add_library(blupi_hist OBJECT ${BLUPI_SHARED_TUS})
target_link_libraries(blupi_hist PRIVATE blupi_hist_context)
# Their file I/O goes through BlupiFile (compat/include/port_hist_stdio.h).
if(MSVC)
    target_compile_options(blupi_hist PRIVATE /source-charset:.1252 /execution-charset:.1252
        /FI${CMAKE_CURRENT_SOURCE_DIR}/compat/include/port_hist_stdio.h)
else()
    target_compile_options(blupi_hist PRIVATE -finput-charset=CP1252 -fexec-charset=CP1252
        "SHELL:-include ${CMAKE_CURRENT_SOURCE_DIR}/compat/include/port_hist_stdio.h")
endif()

# Software reference renderer: plain C++, no SDL and no Win32 names.
add_library(blupi_render STATIC
    render/bmp.cpp
    render/device.cpp
    render/surface.cpp)
target_include_directories(blupi_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# SDL3 host. Must not see compat/include.
add_library(blupi_platform STATIC
    platform/host.cpp
    platform/vfs.cpp
    platform/audio.cpp
    platform/music.cpp
    platform/resources.cpp)
target_include_directories(blupi_platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(blupi_platform PUBLIC SDL3::SDL3 blupi_render)

# The historical string table, embedded verbatim and parsed at start-up.
set(BLUPI_RC ${BLUPI_HIST_SRC}/res/blupi.rc)
set(BLUPI_RC_INC ${CMAKE_CURRENT_BINARY_DIR}/generated/blupi_rc.inc)
add_custom_command(OUTPUT ${BLUPI_RC_INC}
    COMMAND ${CMAKE_COMMAND} -DIN=${BLUPI_RC} -DOUT=${BLUPI_RC_INC}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_bytes.cmake
    DEPENDS ${BLUPI_RC} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_bytes.cmake
    COMMENT "Embedding src/res/blupi.rc")
target_sources(blupi_platform PRIVATE ${BLUPI_RC_INC})
target_include_directories(blupi_platform PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

# Port-owned replacements of the historical platform TUs, plus the shared
# historical objects: everything that sees the Win32 compatibility names.
add_library(blupi_game STATIC
    compat/src/win32.cpp
    compat/src/hist_stdio.cpp
    game/main.cpp
    game/misc.cpp
    game/pixmap.cpp
    game/sound.cpp
    game/movie.cpp
    game/network.cpp
    game/state_hash.cpp
    game/scene.cpp
    game/probe.cpp
    game/end_of_tick.cpp
    game/editor_wide.cpp
    game/options_screen.cpp
    game/settings.cpp
    $<TARGET_OBJECTS:blupi_hist>)
target_link_libraries(blupi_game PRIVATE blupi_hist_context PUBLIC blupi_platform)
target_include_directories(blupi_game PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Entry point TU (SDL_main) without the compatibility include path.
add_executable(blupi platform/entry.cpp)
target_link_libraries(blupi PRIVATE blupi_game SDL3::SDL3)
set_target_properties(blupi PROPERTIES OUTPUT_NAME blupi-sdl3)
if(WIN32)
    set_target_properties(blupi PROPERTIES WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)
    if(MINGW)
        target_link_options(blupi PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()

# Windows: ship SDL3.dll next to the executable (elsewhere the system's SDL3).
if(WIN32)
    add_custom_command(TARGET blupi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:blupi>)
endif()

# GeneralUser GS SoundFont for the MIDI music: git submodule third_party/generaluser-gs
# (shallow, pinned; see third_party/README.md). Without it the game runs without music.
set(BLUPI_SOUNDFONT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/generaluser-gs/GeneralUser-GS.sf2)
option(BLUPI_BUNDLE_SOUNDFONT "Copy the GeneralUser GS SoundFont next to the executable" ON)
if(NOT EXISTS ${BLUPI_SOUNDFONT_FILE})
    message(WARNING "SoundFont submodule missing; the game will have no music. Run:\n"
                    "  git submodule update --init port/third_party/generaluser-gs")
    set(BLUPI_SOUNDFONT_FILE "")
endif()
if(BLUPI_BUNDLE_SOUNDFONT AND BLUPI_SOUNDFONT_FILE)
    add_custom_command(TARGET blupi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${BLUPI_SOUNDFONT_FILE} $<TARGET_FILE_DIR:blupi>/soundfont.sf2)
endif()

enable_testing()
add_subdirectory(tests)
