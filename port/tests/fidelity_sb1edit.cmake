# Historical handlers remain in the existing unchanged game library.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(sb1edit_tests "${CMAKE_CURRENT_LIST_DIR}")
    add_executable(fidelity_editor_sb1 "${sb1edit_tests}/fidelity_sb1edit.cpp")
    target_link_libraries(fidelity_editor_sb1 PRIVATE blupi_hist_context blupi_game)
    target_include_directories(fidelity_editor_sb1 PRIVATE "${sb1edit_tests}/../games/sb1")
    target_link_options(fidelity_editor_sb1 PRIVATE -Wl,--wrap=_Z9EndOfTickR6CDecor)
    if(WIN32)
        add_custom_command(TARGET fidelity_editor_sb1 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3-shared>
                $<TARGET_FILE_DIR:fidelity_editor_sb1>)
    endif()
    foreach(edition blupi eggbert)
        if(edition STREQUAL "blupi")
            set(data "${BLUPI_TEST_DATA}")
        else()
            set(data "${BLUPI_TEST_DATA_EGGBERT}")
        endif()
        if(data)
            add_test(NAME fidelity_editor_sb1_${edition}
                COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:fidelity_editor_sb1>
                "-DDATA=${data}" "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fidelity_editor_sb1_${edition}"
                "-DARGS=--color 16 --frames 120 --input ${sb1edit_tests}/golden/editor.input"
                -P "${sb1edit_tests}/run_fresh.cmake")
            set_tests_properties(fidelity_editor_sb1_${edition} PROPERTIES TIMEOUT 900)
        endif()
    endforeach()
endif()
