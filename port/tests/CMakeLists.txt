# Port tests. Unit tests need nothing but the build; the replay tests need the
# original game data: configure with -DBLUPI_TEST_DATA=<folder with DATA/ ...>.

add_executable(test_render test_render.cpp)
target_link_libraries(test_render PRIVATE blupi_render)
add_test(NAME render COMMAND test_render)

# Historical-source context: layout of the structures written to disk and the
# C runtime generator the original linked.
add_executable(test_layout test_layout.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../compat/src/win32.cpp)
target_link_libraries(test_layout PRIVATE blupi_hist_context blupi_platform)
add_test(NAME layout COMMAND test_layout)

set(BLUPI_TEST_DATA "" CACHE PATH "Game root (DATA/, IMAGE08/, ...) for replay tests")
option(BLUPI_UPDATE_GOLDEN "Golden tests rewrite tests/golden/*.trace instead of comparing" OFF)
if(BLUPI_TEST_DATA)
    add_test(NAME data_layout COMMAND test_layout ${BLUPI_TEST_DATA})

    # Golden traces (port/docs/ROADMAP.md stage 0.2): checkpoints of phase,
    # simulation state hash and screen hash. Scenarios:
    #   attract: 7500 idle frames; the original attract mode starts at frame
    #            ~992 and plays DEMO200..206 (about 6100 demo steps) in turn.
    #   tour:    tests/golden/tour.input, every screen reachable from the menus
    #            without gameplay, plus a short walk in the world-1 hub.
    set(golden ${CMAKE_CURRENT_SOURCE_DIR}/golden)
    function(blupi_golden name every args)
        add_test(NAME golden_${name}
            COMMAND ${CMAKE_COMMAND}
                -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}
                -DGOLDEN=${golden}/${name}.trace -DEVERY=${every} "-DARGS=${args}"
                -DUPDATE=${BLUPI_UPDATE_GOLDEN}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
    endfunction()
    blupi_golden(attract16 10 "--color 16 --frames 7500")
    blupi_golden(attract8  10 "--color 8 --frames 7500")
    blupi_golden(tour16    5  "--color 16 --frames 800 --input ${golden}/tour.input")
    blupi_golden(tour8     5  "--color 8 --frames 800 --input ${golden}/tour.input")
    blupi_golden(editor16  5  "--color 16 --frames 360 --input ${golden}/editor.input")
    blupi_golden(editor8   5  "--color 8 --frames 360 --input ${golden}/editor.input")
    # Stage-1 gate (ROADMAP.md): every frame's rendering commands, replayed on an
    # independent device, give the same back buffer, screen and palette.
    foreach(depth 16 8)
        add_test(NAME commands_attract${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/commands_a${depth}
                    --color ${depth} --frames 7500 --check-commands)
        add_test(NAME commands_tour${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/commands_t${depth}
                    --color ${depth} --frames 800 --input ${golden}/tour.input --check-commands)
        add_test(NAME commands_editor${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/commands_e${depth}
                    --color ${depth} --frames 360 --input ${golden}/editor.input --check-commands)
    endforeach()
    # Stage-3 gate (ROADMAP.md): the scene built before each CDecor::Build call
    # flattens to exactly Build's rendering commands.
    foreach(depth 16 8)
        add_test(NAME scene_attract${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/scene_a${depth}
                    --color ${depth} --frames 7500 --check-scene)
        add_test(NAME scene_tour${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/scene_t${depth}
                    --color ${depth} --frames 800 --input ${golden}/tour.input --check-scene)
        add_test(NAME scene_editor${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/scene_e${depth}
                    --color ${depth} --frames 360 --input ${golden}/editor.input --check-scene)
    endforeach()
    foreach(s attract tour editor)
        add_test(NAME golden_${s}_depth_independent
            COMMAND ${CMAKE_COMMAND} -DA=${golden}/${s}16.trace -DB=${golden}/${s}8.trace
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_states_equal.cmake)
    endforeach()
endif()

# MIDI player: needs the game data and a SoundFont (the bundled GeneralUser GS by default).
set(BLUPI_TEST_SOUNDFONT "${BLUPI_SOUNDFONT_FILE}" CACHE FILEPATH "SoundFont (.sf2) for the music tests (default: the bundled GeneralUser GS)")
add_executable(test_music test_music.cpp)
target_link_libraries(test_music PRIVATE blupi_platform)
if(BLUPI_TEST_DATA AND BLUPI_TEST_SOUNDFONT)
    add_test(NAME music COMMAND test_music ${BLUPI_TEST_DATA} ${BLUPI_TEST_SOUNDFONT})
endif()
if(BLUPI_TEST_DATA AND BLUPI_TEST_SOUNDFONT)
    add_test(NAME music_notify
        COMMAND ${CMAKE_COMMAND}
            -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA} -DSOUNDFONT=${BLUPI_TEST_SOUNDFONT}
            -DWORK=${CMAKE_CURRENT_BINARY_DIR}/music_notify
            -P ${CMAKE_CURRENT_SOURCE_DIR}/music_notify.cmake)
endif()

# Options-screen music volume through the historical CSound calls.
add_executable(test_music_volume test_music_volume.cpp)
target_link_libraries(test_music_volume PRIVATE blupi_hist_context blupi_game)
target_include_directories(test_music_volume PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
if(BLUPI_TEST_DATA AND BLUPI_TEST_SOUNDFONT)
    add_test(NAME music_volume COMMAND test_music_volume ${BLUPI_TEST_DATA} ${BLUPI_TEST_SOUNDFONT}
             ${CMAKE_CURRENT_BINARY_DIR}/music_volume_user)
endif()
