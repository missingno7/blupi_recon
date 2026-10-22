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

# Joystick (winmm) calls of the game answered from an SDL virtual gamepad.
add_executable(test_gamepad test_gamepad.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../compat/src/win32.cpp)
target_link_libraries(test_gamepad PRIVATE blupi_hist_context blupi_platform)
add_test(NAME gamepad COMMAND test_gamepad)

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
    # Each scenario runs in native mode (scene + EndOfTick + captured UI, the
    # default) and in reference mode (--reference, the historical SetDecor body)
    # against the same golden file: stage-4 gate (ROADMAP.md).
    function(blupi_golden name every args)
        add_test(NAME golden_${name}
            COMMAND ${CMAKE_COMMAND}
                -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}
                -DGOLDEN=${golden}/${name}.trace -DEVERY=${every} "-DARGS=${args}"
                -DUPDATE=${BLUPI_UPDATE_GOLDEN}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
        add_test(NAME golden_${name}_reference
            COMMAND ${CMAKE_COMMAND}
                -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}_reference
                -DGOLDEN=${golden}/${name}.trace -DEVERY=${every} "-DARGS=--reference ${args}"
                -DUPDATE=OFF
                -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
        # Rendering reads no game state: rendering every frame 3 extra times
        # must give the same trace (stage-4 gate).
        add_test(NAME golden_${name}_rerender
            COMMAND ${CMAKE_COMMAND}
                -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}_rerender
                -DGOLDEN=${golden}/${name}.trace -DEVERY=${every} "-DARGS=--render-repeat 3 ${args}"
                -DUPDATE=OFF
                -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
        # Display-rate presentation (stage 5): presenting the latest frame and
        # the cursor 3 extra times per tick must give the same trace.
        add_test(NAME golden_${name}_present
            COMMAND ${CMAKE_COMMAND}
                -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}_present
                -DGOLDEN=${golden}/${name}.trace -DEVERY=${every} "-DARGS=--present-repeat 3 ${args}"
                -DUPDATE=OFF
                -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
        # Interpolation (stage 7): presenting views between ticks (alpha 1/3,
        # 2/3, 1) changes no state, and the view renderer at alpha 1 issues the
        # faithful frame's commands (--check-view, exit code 5).
        add_test(NAME golden_${name}_interp
            COMMAND ${CMAKE_COMMAND}
                -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}_interp
                -DGOLDEN=${golden}/${name}.trace -DEVERY=${every}
                "-DARGS=--interpolate --check-view --present-repeat 3 ${args}"
                -DUPDATE=OFF
                -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
        # Widescreen (stage 8): the same with a 16:9 view presented 3 times per
        # tick; the wide view changes no state.
        add_test(NAME golden_${name}_wide
            COMMAND ${CMAKE_COMMAND}
                -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}_wide
                -DGOLDEN=${golden}/${name}.trace -DEVERY=${every}
                "-DARGS=--wide 16:9 --interpolate --check-view --present-repeat 3 ${args}"
                -DUPDATE=OFF
                -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
    endfunction()
    blupi_golden(attract16 10 "--color 16 --frames 7500")
    blupi_golden(attract8  10 "--color 8 --frames 7500")
    blupi_golden(tour16    5  "--color 16 --frames 800 --input ${golden}/tour.input")
    blupi_golden(tour8     5  "--color 8 --frames 800 --input ${golden}/tour.input")
    blupi_golden(editor16  5  "--color 16 --frames 360 --input ${golden}/editor.input")
    blupi_golden(editor8   5  "--color 8 --frames 360 --input ${golden}/editor.input")
    # effects: test-only state probes for paths the demos and menus never reach
    # (shield, cloud, hide/CHTEMP, notifications, mission-10 clamp, network Blupis).
    set(effects "--input ${golden}/effects.input --probe ${golden}/effects.probe")
    blupi_golden(effects16 5  "--color 16 --frames 520 ${effects}")
    blupi_golden(effects8  5  "--color 8 --frames 520 ${effects}")
    # Languages (docs/LANGUAGES.md): the tour in German. Its state hashes
    # differ from the US tour's only through CEvent's tooltip text buffer.
    blupi_golden(tour16_de 5  "--color 16 --frames 800 --input ${golden}/tour.input --language de")
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
        add_test(NAME commands_effects${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/commands_f${depth}
                    --color ${depth} --frames 520 --input ${golden}/effects.input
                    --probe ${golden}/effects.probe --check-commands)
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
        add_test(NAME scene_effects${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/scene_f${depth}
                    --color ${depth} --frames 520 --input ${golden}/effects.input
                    --probe ${golden}/effects.probe --check-scene)
    endforeach()
    # Stage-8 gate: where a 16:9 view is centred on the frame, its centre equals
    # the faithful frame (items chosen by Build's rules at the frame). The
    # attract demos give ~6000 such frames; the world-1 hub of the other
    # scenarios lies against the world's left edge (clamped, not compared).
    foreach(depth 16 8)
        add_test(NAME wide_attract${depth}
            COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/wide_a${depth}
                    --color ${depth} --frames 7500 --wide 16:9 --check-wide)
        # The editor's wide view is always centred on its frame (its mouse
        # mapping goes through the frame), so every editor frame is compared.
        add_test(NAME wide_editor${depth}
            COMMAND ${CMAKE_COMMAND}
                -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/wide_e${depth}
                "-DARGS=--color ${depth} --frames 360 --input ${golden}/editor.input --wide 21:9 --check-wide"
                -P ${CMAKE_CURRENT_SOURCE_DIR}/run_fresh.cmake)
    endforeach()
    # Display options screen (stage 9): opening it on the settings screen,
    # changing settings and closing it changes no game state (the cursor, part
    # of the screen hash, follows the clicks on that screen). The golden
    # options16.trace is the tour plus the one mouse move of options.input that
    # reaches the game (options_baseline.input), without the screen.
    add_test(NAME golden_options16
        COMMAND ${CMAKE_COMMAND}
            -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
            -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_options16
            -DGOLDEN=${golden}/options16.trace -DEVERY=5
            "-DARGS=--color 16 --frames 800 --input ${golden}/tour.input --input ${golden}/options_baseline.input"
            -DUPDATE=${BLUPI_UPDATE_GOLDEN}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
    add_test(NAME golden_options16_screen
        COMMAND ${CMAKE_COMMAND}
            -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA}
            -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_options16_screen
            -DGOLDEN=${golden}/options16.trace -DEVERY=5
            "-DARGS=--color 16 --frames 800 --input ${golden}/tour.input --input ${golden}/options.input --present-repeat 1"
            -DUPDATE=OFF -DSTATE_ONLY=ON
            -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
    # The same with the interpolation coverage: the extra items recorded around
    # the view leave Build's own items and order unchanged.
    add_test(NAME scene_attract16_interp
        COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/scene_ai
                --color 16 --frames 7500 --interpolate --check-scene)
    add_test(NAME scene_tour16_interp
        COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/scene_ti
                --color 16 --frames 800 --input ${golden}/tour.input --interpolate --check-scene)
    add_test(NAME scene_editor16_interp
        COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/scene_ei
                --color 16 --frames 360 --input ${golden}/editor.input --interpolate --check-scene)
    add_test(NAME scene_effects16_interp
        COMMAND blupi --headless --data ${BLUPI_TEST_DATA} --user ${CMAKE_CURRENT_BINARY_DIR}/scene_fi
                --color 16 --frames 520 --input ${golden}/effects.input
                --probe ${golden}/effects.probe --interpolate --check-scene)
    foreach(s attract tour editor effects)
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

# Port settings file (game/settings.cpp).
add_executable(test_settings test_settings.cpp)
target_link_libraries(test_settings PRIVATE blupi_hist_context blupi_game)
target_include_directories(test_settings PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
add_test(NAME settings COMMAND test_settings ${CMAKE_CURRENT_BINARY_DIR}/settings_test.json)

# Language packs (platform/language.cpp, the built-in packs in languages/).
add_executable(test_language test_language.cpp)
target_link_libraries(test_language PRIVATE blupi_platform)
add_test(NAME language COMMAND test_language ${CMAKE_CURRENT_SOURCE_DIR}/../languages)

# Speedy Eggbert 1 (docs/MULTI_GAME.md): its attract mode with the port's
# code, a regression gate. Configure with -DBLUPI_TEST_DATA_EGGBERT=<its game root>.
set(BLUPI_TEST_DATA_EGGBERT "" CACHE PATH "Speedy Eggbert 1 game root, for its replay test")
if(BLUPI_TEST_DATA_EGGBERT)
    add_test(NAME golden_eggbert_attract16
        COMMAND ${CMAKE_COMMAND}
            -DEXE=$<TARGET_FILE:blupi> -DDATA=${BLUPI_TEST_DATA_EGGBERT}
            -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_eggbert_attract16
            -DGOLDEN=${CMAKE_CURRENT_SOURCE_DIR}/golden/eggbert_attract16.trace -DEVERY=10
            "-DARGS=--color 16 --frames 7500"
            -DUPDATE=${BLUPI_UPDATE_GOLDEN}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/golden_check.cmake)
endif()
