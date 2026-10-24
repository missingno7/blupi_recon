# SB2 has its own historical headers/layouts. No SB1 include context here.
set(SB2_SRC ${CMAKE_CURRENT_SOURCE_DIR}/../../../games/sb2/src)
set(SB2_GAME ${CMAKE_CURRENT_SOURCE_DIR})
set(SB2_CASE ${CMAKE_CURRENT_BINARY_DIR}/compat_case)
file(MAKE_DIRECTORY ${SB2_CASE})
if(NOT CMAKE_HOST_WIN32)
    file(WRITE ${SB2_CASE}/Windows.h "#include \"${BLUPI_COMMON}/compat/include/windows.h\"\n")
    file(WRITE ${SB2_CASE}/DEF.H "#include \"${SB2_SRC}/def.h\"\n")
endif()
add_library(blupi2_hist_context INTERFACE)
target_include_directories(blupi2_hist_context BEFORE INTERFACE
    ${BLUPI_COMMON}/compat/include ${SB2_CASE} ${SB2_SRC} ${SB2_GAME})
if(MSVC)
    target_compile_options(blupi2_hist_context INTERFACE
        /FI${BLUPI_COMMON}/compat/include/port_forced.h
        /FI${SB2_GAME}/game/port_switches.h /permissive /Zc:forScope- /W0)
else()
    target_compile_options(blupi2_hist_context INTERFACE
        "SHELL:-include ${BLUPI_COMMON}/compat/include/port_forced.h"
        "SHELL:-include ${SB2_GAME}/game/port_switches.h"
        -fno-strict-aliasing -fwrapv -fsigned-char -w
        $<$<CXX_COMPILER_ID:GNU>:-fpermissive>
        $<$<CXX_COMPILER_ID:Clang>:-fms-extensions>)
endif()
set(SB2_SHARED_TUS button decblock decblupi decdesign decmove decnet decor dectables jauge menu text)
list(TRANSFORM SB2_SHARED_TUS APPEND .cpp)
list(TRANSFORM SB2_SHARED_TUS PREPEND ${SB2_SRC}/)
add_library(blupi2_hist OBJECT ${SB2_SHARED_TUS})
# CEvent's two phase tables/edition flows need runtime selection. This port
# derivative retains the historical encoding and VFS stdio context below.
target_sources(blupi2_hist PRIVATE game/event.cpp)
target_include_directories(blupi2_hist PRIVATE ${BLUPI_COMMON})
target_link_libraries(blupi2_hist PRIVATE blupi2_hist_context)
if(MSVC)
    target_compile_options(blupi2_hist PRIVATE /source-charset:.1252 /execution-charset:.1252
        /FI${BLUPI_COMMON}/compat/include/port_hist_stdio.h)
else()
    target_compile_options(blupi2_hist PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-finput-charset=CP1252>
        $<$<CXX_COMPILER_ID:GNU>:-fexec-charset=CP1252>
        "SHELL:-include ${BLUPI_COMMON}/compat/include/port_hist_stdio.h")
endif()
add_library(blupi2_game STATIC
    ${BLUPI_COMMON}/compat/src/win32.cpp ${BLUPI_COMMON}/compat/src/hist_stdio.cpp
    ${BLUPI_COMMON}/ui/options_screen.cpp ${BLUPI_COMMON}/ui/settings.cpp
    game/editor_wide.cpp game/scene.cpp game/end_of_tick.cpp
    game/main.cpp game/pixmap.cpp game/sound.cpp game/misc.cpp
    game/movie.cpp game/network.cpp game/state_hash.cpp game/language.cpp game/probe.cpp
    game/icon.cpp
    $<TARGET_OBJECTS:blupi2_hist>)
target_link_libraries(blupi2_game PRIVATE blupi2_hist_context PUBLIC blupi_platform blupi_app)
target_compile_definitions(blupi2_game PRIVATE BLUPI_PORT_VERSION="${PROJECT_VERSION}")

# blupi2's icon (res/icon.png, tools/make_icon.py --game sb2), for the window on
# systems without executable resources (game/icon.cpp).
set(SB2_ICON_INC ${CMAKE_CURRENT_BINARY_DIR}/generated/sb2_icon_png.inc)
add_custom_command(OUTPUT ${SB2_ICON_INC}
    COMMAND ${CMAKE_COMMAND} -DIN=${SB2_GAME}/res/icon.png -DOUT=${SB2_ICON_INC}
            -P ${PROJECT_SOURCE_DIR}/cmake/embed_bytes.cmake
    DEPENDS ${SB2_GAME}/res/icon.png ${PROJECT_SOURCE_DIR}/cmake/embed_bytes.cmake
    COMMENT "Embedding games/sb2/res/icon.png")
target_sources(blupi2_game PRIVATE ${SB2_ICON_INC})

file(GLOB SB2_PACKS CONFIGURE_DEPENDS ${SB2_GAME}/languages/*.txt)
list(SORT SB2_PACKS)
string(REPLACE ";" "|" SB2_PACK_ARGS "${SB2_PACKS}")
set(SB2_PACK_INC ${CMAKE_CURRENT_BINARY_DIR}/generated/sb2_language_packs.inc)
add_custom_command(OUTPUT ${SB2_PACK_INC}
    COMMAND ${CMAKE_COMMAND} "-DFILES=${SB2_PACK_ARGS}" -DOUT=${SB2_PACK_INC}
        -P ${PROJECT_SOURCE_DIR}/cmake/embed_packs.cmake
    DEPENDS ${SB2_PACKS} ${PROJECT_SOURCE_DIR}/cmake/embed_packs.cmake VERBATIM)
target_sources(blupi2_game PRIVATE ${SB2_PACK_INC})
target_include_directories(blupi2_game PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)

# Desktop bring-up; Android's single libmain continues to be SB1.
add_executable(blupi2 ${BLUPI_COMMON}/platform/entry.cpp)
target_link_libraries(blupi2 PRIVATE blupi2_game SDL3::SDL3)
set_target_properties(blupi2 PROPERTIES OUTPUT_NAME blupi2-sdl3 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
if(WIN32)
    # blupi2's icon (res/icon.ico, which SDL also uses for the window) and version.
    set(BLUPI_RC_ICON ${SB2_GAME}/res/icon.ico)
    set(BLUPI_RC_DESCRIPTION "Speedy Blupi 2 / Speedy Eggbert 2 (SDL3 port)")
    set(BLUPI_RC_NAME blupi2-sdl3)
    set(BLUPI_RC_PRODUCT "Speedy Blupi 2 SDL3 port")
    configure_file(${BLUPI_COMMON}/platform/blupi_win.rc.in ${CMAKE_CURRENT_BINARY_DIR}/generated/blupi2_win.rc @ONLY)
    target_sources(blupi2 PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated/blupi2_win.rc)
endif()
blupi_package_app(blupi2)
