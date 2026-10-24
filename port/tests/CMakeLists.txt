# Port tests. Unit tests need nothing but the build; the replay tests need the
# original game data: configure with -DBLUPI_TEST_DATA=<folder with DATA/ ...>.

add_executable(test_render test_render.cpp)
target_link_libraries(test_render PRIVATE blupi_render)
add_test(NAME render COMMAND test_render)

# Historical-source context: layout of the structures written to disk and the
# C runtime generator the original linked.
add_executable(test_layout test_layout.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../common/compat/src/win32.cpp)
target_link_libraries(test_layout PRIVATE blupi_hist_context blupi_platform)
add_test(NAME layout COMMAND test_layout)

# Joystick (winmm) calls of the game answered from an SDL virtual gamepad.
add_executable(test_gamepad test_gamepad.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../common/compat/src/win32.cpp)
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
target_include_directories(test_music_volume PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../games/sb1)
if(BLUPI_TEST_DATA AND BLUPI_TEST_SOUNDFONT)
    add_test(NAME music_volume COMMAND test_music_volume ${BLUPI_TEST_DATA} ${BLUPI_TEST_SOUNDFONT}
             ${CMAKE_CURRENT_BINARY_DIR}/music_volume_user)
endif()

# Port settings file (game/settings.cpp).
add_executable(test_settings test_settings.cpp)
target_link_libraries(test_settings PRIVATE blupi_hist_context blupi_game)
target_include_directories(test_settings PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../games/sb1)
add_test(NAME settings COMMAND test_settings ${CMAKE_CURRENT_BINARY_DIR}/settings_test.json)

# Language packs (platform/language.cpp, the built-in packs in languages/),
# and the US pack against the reconstructed string table, embedded here only.
set(BLUPI_RC_INC ${CMAKE_CURRENT_BINARY_DIR}/generated/blupi_rc.inc)
add_custom_command(OUTPUT ${BLUPI_RC_INC}
    COMMAND ${CMAKE_COMMAND} -DIN=${BLUPI_HIST_SRC}/res/blupi.rc -DOUT=${BLUPI_RC_INC}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/embed_bytes.cmake
    DEPENDS ${BLUPI_HIST_SRC}/res/blupi.rc ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/embed_bytes.cmake
    COMMENT "Embedding src/res/blupi.rc")
add_executable(test_language test_language.cpp rc_strings.cpp ${BLUPI_RC_INC})
target_include_directories(test_language PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
target_link_libraries(test_language PRIVATE blupi_platform)
add_test(NAME language COMMAND test_language ${CMAKE_CURRENT_SOURCE_DIR}/../languages)

# Font metrics set by the port's build switches (game/port_switches.h).
add_executable(test_text test_text.cpp)
target_link_libraries(test_text PRIVATE blupi_hist_context blupi_game)
add_test(NAME text COMMAND test_text)

# Windows: the tests that link SDL3 find SDL3.dll next to them.
if(WIN32)
    add_custom_command(TARGET test_layout POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3-shared> ${CMAKE_CURRENT_BINARY_DIR})
endif()

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

# A second, isolated historical include context and a separate golden-update
# option: SB2 bring-up must never regenerate SB1's accepted traces.
add_executable(test_sb2_layout test_sb2_layout.cpp)
target_link_libraries(test_sb2_layout PRIVATE blupi2_hist_context)
add_test(NAME sb2_layout COMMAND test_sb2_layout)
add_executable(test_sb2_view test_sb2_view.cpp)
target_link_libraries(test_sb2_view PRIVATE blupi2_hist_context blupi2_game)
add_test(NAME sb2_view COMMAND test_sb2_view)
set(BLUPI2_TEST_DATA "$ENV{BLUPI2_DATA}" CACHE PATH "Eggbert 2 data root for SB2 golden tests")
set(BLUPI2_TEST_DATA_BLUPI "" CACHE PATH "Blupi 2 data root for SB2 golden tests")
foreach(game sb1 sb2_eggbert sb2_blupi)
    if(game STREQUAL "sb1")
        set(oracle_exe blupi)
        set(oracle_data "${BLUPI_TEST_DATA}")
    elseif(game STREQUAL "sb2_eggbert")
        set(oracle_exe blupi2)
        set(oracle_data "${BLUPI2_TEST_DATA}")
    else()
        set(oracle_exe blupi2)
        set(oracle_data "${BLUPI2_TEST_DATA_BLUPI}")
    endif()
    if(oracle_data)
        foreach(depth 8 16)
            add_test(NAME oracle_display_${game}_${depth}
                COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${oracle_exe}> -DDATA=${oracle_data}
                    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/oracle_${game}_${depth}
                    -DBUILDROOT=${CMAKE_CURRENT_BINARY_DIR} -DDEPTH=${depth}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/oracle_display.cmake)
        endforeach()
    endif()
endforeach()

# Both executables use the same launcher, with game-specific names and extras.
foreach(target blupi blupi2)
    if(target STREQUAL "blupi")
        set(app_name blupi-sdl3)
        set(data_env BLUPI_DATA)
        set(extra --export-language)
    else()
        set(app_name blupi2-sdl3)
        set(data_env BLUPI2_DATA)
        set(extra --demo-log)
    endif()
    add_test(NAME launcher_help_${target}
        COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${target}> -DNAME=${app_name}
            -DENVIRONMENT=${data_env} -DEXTRA=${extra}
            -DVERSION_FILE=${PROJECT_SOURCE_DIR}/VERSION.txt
            -P ${CMAKE_CURRENT_SOURCE_DIR}/launcher_help.cmake)
endforeach()
option(BLUPI2_UPDATE_GOLDEN "Rewrite only the SB2 golden traces" OFF)
# Optional presentation modes use the existing faithful traces as their oracle.
# Updating any golden is deliberately disabled for these variants.
function(sb2_mode_goldens name data every args script)
    foreach(mode reference rerender present interp wide)
        if(mode STREQUAL "reference")
            set(extra "--reference")
        elseif(mode STREQUAL "rerender")
            set(extra "--render-repeat 3")
        elseif(mode STREQUAL "present")
            set(extra "--present display --present-repeat 3")
        elseif(mode STREQUAL "interp")
            set(extra "--interpolate --check-view --present-repeat 3")
        else()
            set(extra "--wide 16:9 --interpolate --check-view --present-repeat 3")
            if(name MATCHES "attract|editor")
                string(APPEND extra " --check-wide")
            endif()
        endif()
        add_test(NAME golden_${name}_${mode}
            COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> -DDATA=${data}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}_${mode}
                -DGOLDEN=${CMAKE_CURRENT_SOURCE_DIR}/golden/${name}.trace -DEVERY=${every}
                "-DARGS=${args} ${extra}" -DSCENARIO=${scenario} -DEDITION=${edition}
                -DUPDATE=OFF -P ${CMAKE_CURRENT_SOURCE_DIR}/${script})
    endforeach()
endfunction()
foreach(edition eggbert blupi)
    if(edition STREQUAL "eggbert")
        set(sb2_data "${BLUPI2_TEST_DATA}")
    else()
        set(sb2_data "${BLUPI2_TEST_DATA_BLUPI}")
    endif()
    if(sb2_data)
        foreach(depth 8 16)
            # SB1's scene / command / faithful-view seams, extended for SB2.
            # Compare full traces every frame; never regenerate a golden file.
            foreach(scenario attract tour editor effects)
                if(scenario STREQUAL "attract")
                    set(scene_args "--frames 8000")
                elseif(scenario STREQUAL "effects")
                    set(scene_args "--frames 160 --input ${CMAKE_CURRENT_SOURCE_DIR}/sb2_scene.input --probe ${CMAKE_CURRENT_SOURCE_DIR}/sb2_scene.probe")
                else()
                    set(scene_args "--frames 260 --input ${CMAKE_CURRENT_SOURCE_DIR}/golden/sb2_${scenario}.input")
                endif()
                set(scene_name scene_sb2_${edition}_${scenario}${depth})
                add_test(NAME ${scene_name}
                    COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> -DDATA=${sb2_data}
                        -DWORK=${CMAKE_CURRENT_BINARY_DIR}/${scene_name}
                        "-DARGS=--color ${depth} ${scene_args}"
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/sb2_scene.cmake)
            endforeach()
            set(name sb2_${edition}_attract${depth})
            sb2_mode_goldens(${name} "${sb2_data}" 10 "--color ${depth} --frames 8000" sb2_golden.cmake)
            add_test(NAME golden_${name}
                COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> -DDATA=${sb2_data}
                    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}
                    -DGOLDEN=${CMAKE_CURRENT_SOURCE_DIR}/golden/${name}.trace -DEVERY=10
                    "-DARGS=--color ${depth} --frames 8000" -DUPDATE=${BLUPI2_UPDATE_GOLDEN}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/sb2_golden.cmake)
            foreach(scenario tour editor)
                set(name sb2_${edition}_${scenario}${depth})
                sb2_mode_goldens(${name} "${sb2_data}" 5
                    "--color ${depth} --frames 260 --input ${CMAKE_CURRENT_SOURCE_DIR}/golden/sb2_${scenario}.input"
                    sb2_menu.cmake)
                add_test(NAME golden_${name}
                    COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> -DDATA=${sb2_data}
                        -DWORK=${CMAKE_CURRENT_BINARY_DIR}/golden_${name}
                        -DGOLDEN=${CMAKE_CURRENT_SOURCE_DIR}/golden/${name}.trace -DEVERY=5
                        "-DARGS=--color ${depth} --frames 260 --input ${CMAKE_CURRENT_SOURCE_DIR}/golden/sb2_${scenario}.input"
                        -DSCENARIO=${scenario} -DEDITION=${edition} -DUPDATE=${BLUPI2_UPDATE_GOLDEN}
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/sb2_menu.cmake)
            endforeach()
        endforeach()
        foreach(depth 8 16)
            add_test(NAME presentation_sb2_${edition}_${depth}
                COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> -DDATA=${sb2_data}
                    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/presentation_sb2_${edition}_${depth}
                    -DDEPTH=${depth} -P ${CMAKE_CURRENT_SOURCE_DIR}/sb2_presentation.cmake)
        endforeach()
        add_test(NAME preferences_sb2_${edition}
            COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> -DDATA=${sb2_data}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/preferences_sb2_${edition}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/sb2_preferences.cmake)
        add_test(NAME sb2_${edition}_save
            COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> -DDATA=${sb2_data}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/sb2_${edition}_save
                -P ${CMAKE_CURRENT_SOURCE_DIR}/sb2_save.cmake)
        foreach(case intro read cache)
            add_test(NAME sb2_${edition}_${case}
                COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> -DDATA=${sb2_data}
                    -DWORK=${CMAKE_CURRENT_BINARY_DIR}/sb2_${edition}_${case}
                    -DCASE=${case} -DEDITION=${edition}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/sb2_editions.cmake)
        endforeach()
    endif()
endforeach()
include(launcher_tests.cmake)
