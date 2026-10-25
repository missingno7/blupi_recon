# These targets observe existing libraries, never recompile matching source
# with test instrumentation. GNU ld wrapping is limited to GNU/MinGW builds.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_executable(fidelity_sound_sb2 fidelity_sound.cpp)
    target_link_libraries(fidelity_sound_sb2 PRIVATE blupi2_hist_context blupi2_game)
    add_executable(fidelity_language_sb2 fidelity_language.cpp)
    target_link_libraries(fidelity_language_sb2 PRIVATE blupi2_hist_context blupi2_game)
    if(BLUPI_TEST_PYTHON_PEFILE AND BLUPI2_TEST_DATA AND BLUPI2_TEST_DATA_BLUPI AND BLUPI2_TEST_DATA_VARIANTS)
        add_test(NAME fidelity_sb2_resources COMMAND "${BLUPI_TEST_PYTHON}"
            "${CMAKE_CURRENT_SOURCE_DIR}/fidelity_resources.py"
            --eggbert2 "${BLUPI2_TEST_DATA}" --blupi2 "${BLUPI2_TEST_DATA_BLUPI}"
            --blupi2-variants "${BLUPI2_TEST_DATA_VARIANTS}"
            --languages "${PROJECT_SOURCE_DIR}/games/sb2/languages"
            --exe $<TARGET_FILE:fidelity_language_sb2>
            --work "${CMAKE_CURRENT_BINARY_DIR}/fidelity_resources")
    endif()
    add_executable(fidelity_editor_sb2 fidelity_editor.cpp)
    target_link_libraries(fidelity_editor_sb2 PRIVATE blupi2_hist_context blupi2_game)
    target_link_options(fidelity_editor_sb2 PRIVATE -Wl,--wrap=_Z9EndOfTickR6CDecor)
    foreach(game sb1 sb2)
        if(game STREQUAL "sb1")
            set(context blupi_hist_context)
            set(library blupi_game)
        else()
            set(context blupi2_hist_context)
            set(library blupi2_game)
        endif()
        add_executable(fidelity_stdio_${game} fidelity_stdio.cpp)
        target_link_libraries(fidelity_stdio_${game} PRIVATE ${context} ${library})
        foreach(symbol blupi_hist_fread blupi_hist_fwrite blupi_hist_fclose)
            target_link_options(fidelity_stdio_${game} PRIVATE "-Wl,--wrap=${symbol}")
        endforeach()
        add_test(NAME fidelity_stdio_${game} COMMAND fidelity_stdio_${game}
            "${CMAKE_CURRENT_BINARY_DIR}/fidelity_stdio_${game}_user")
        add_executable(fidelity_effects_${game} fidelity_effects.cpp)
        target_compile_definitions(fidelity_effects_${game} PRIVATE FIDELITY_SB2=$<STREQUAL:${game},sb2>)
        target_link_libraries(fidelity_effects_${game} PRIVATE ${context} ${library})
        target_include_directories(fidelity_effects_${game} PRIVATE ${PROJECT_SOURCE_DIR}/games/${game})
        foreach(symbol _ZN6CDecor5BuildE7tagRECT8tagPOINT _Z9EndOfTickR6CDecor
            _ZN6CSound9PlayImageEi8tagPOINTii _ZN6CSound9StopSoundEi
            _ZN6CSound12SuspendMusicEv _ZN6CSound12RestartMusicEv _ZN6CSound9StopMusicEv
            _ZN6CDecor11ObjectStartE8tagPOINTiii _ZN6CDecor12NetPlaySoundEs8tagPOINT)
            target_link_options(fidelity_effects_${game} PRIVATE "-Wl,--wrap=${symbol}")
        endforeach()
        if(game STREQUAL "sb2")
            target_link_options(fidelity_effects_${game} PRIVATE -Wl,--wrap=_ZN6CSound9PlayMusicEP11BlupiHWND__i)
        endif()
        add_executable(fidelity_cursor_${game} fidelity_cursor.cpp)
        target_link_libraries(fidelity_cursor_${game} PRIVATE ${context} ${library})
        target_include_directories(fidelity_cursor_${game} PRIVATE ${PROJECT_SOURCE_DIR}/games/${game})
        target_compile_definitions(fidelity_cursor_${game} PRIVATE FIDELITY_SB2=$<STREQUAL:${game},sb2>)
    endforeach()
    foreach(edition sb1 sb1_eggbert sb2_eggbert sb2_blupi)
        set(game sb2)
        if(edition STREQUAL "sb1")
            set(data "${BLUPI_TEST_DATA}")
            set(game sb1)
        elseif(edition STREQUAL "sb1_eggbert")
            set(data "${BLUPI_TEST_DATA_EGGBERT}")
            set(game sb1)
        elseif(edition STREQUAL "sb2_eggbert")
            set(data "${BLUPI2_TEST_DATA}")
        else()
            set(data "${BLUPI2_TEST_DATA_BLUPI}")
        endif()
        if(data)
            if(game STREQUAL "sb2")
                add_test(NAME fidelity_sound_${edition} COMMAND fidelity_sound_sb2
                    "${data}" "${CMAKE_CURRENT_BINARY_DIR}/fidelity_sound_${edition}")
                add_test(NAME fidelity_editor_${edition}
                    COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:fidelity_editor_sb2>
                    "-DDATA=${data}" "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fidelity_editor_${edition}"
                    "-DARGS=--color 16 --frames 180 --input ${CMAKE_CURRENT_SOURCE_DIR}/golden/sb2_editor.input"
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_fresh.cmake)
                if(edition STREQUAL "sb2_blupi")
                    foreach(language de fr)
                        add_test(NAME fidelity_language_menu_${language}
                            COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:blupi2> "-DDATA=${data}"
                            "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fidelity_language_menu_${language}"
                            "-DARGS=--language ${language} --color 16 --frames 260 --input ${CMAKE_CURRENT_SOURCE_DIR}/golden/sb2_tour.input"
                            -P ${CMAKE_CURRENT_SOURCE_DIR}/sb2_scene.cmake)
                    endforeach()
                endif()
            endif()
            add_test(NAME fidelity_cursor_${edition} COMMAND fidelity_cursor_${game}
                "${data}" "${CMAKE_CURRENT_BINARY_DIR}/fidelity_cursor_${edition}_work")
            foreach(scenario attract effects)
                if(game STREQUAL "sb1")
                    set(args "--frames 7500")
                    if(scenario STREQUAL "effects")
                        set(args "--frames 520 --input ${CMAKE_CURRENT_SOURCE_DIR}/golden/effects.input --probe ${CMAKE_CURRENT_SOURCE_DIR}/golden/effects.probe")
                    endif()
                else()
                    set(args "--frames 8000")
                    if(scenario STREQUAL "effects")
                        set(args "--frames 160 --input ${CMAKE_CURRENT_SOURCE_DIR}/sb2_scene.input --probe ${CMAKE_CURRENT_SOURCE_DIR}/sb2_scene.probe")
                    endif()
                endif()
                add_test(NAME fidelity_effects_${edition}_${scenario}
                    COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:fidelity_effects_${game}>
                    "-DDATA=${data}" "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fidelity_effects_${edition}_${scenario}"
                    "-DARGS=${args}" -DSB2=$<STREQUAL:${game},sb2>
                    -DEFFECTS=$<AND:$<STREQUAL:${game},sb2>,$<STREQUAL:${scenario},effects>>
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/fidelity_effects.cmake)
            endforeach()
        endif()
    endforeach()
endif()
