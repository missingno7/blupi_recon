# Include from tests/CMakeLists.txt after the existing SB1 data options.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(save_tests "${CMAKE_CURRENT_LIST_DIR}")
    add_executable(fidelity_save_sb1 "${save_tests}/fidelity_save.cpp")
    target_link_libraries(fidelity_save_sb1 PRIVATE blupi_hist_context blupi_game)
    target_include_directories(fidelity_save_sb1 PRIVATE "${save_tests}/../games/sb1")
    target_link_options(fidelity_save_sb1 PRIVATE
        -Wl,--wrap=_ZN6CDecor12CurrentWriteEii -Wl,--wrap=_ZN6CDecor11CurrentReadEiiPiS0_)
    if(WIN32)
        add_custom_command(TARGET fidelity_save_sb1 POST_BUILD COMMAND ${CMAKE_COMMAND}
            -E copy_if_different $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:fidelity_save_sb1>)
    endif()
    foreach(edition blupi eggbert)
        if(edition STREQUAL "blupi")
            set(data "${BLUPI_TEST_DATA}")
        else()
            set(data "${BLUPI_TEST_DATA_EGGBERT}")
        endif()
        if(data)
            foreach(depth 8 16)
                add_test(NAME fidelity_save_sb1_${edition}_${depth}
                    COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:fidelity_save_sb1>
                    "-DDATA=${data}" "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fidelity_save_${edition}_${depth}"
                    "-DARGS=--color ${depth} --frames 200 --input ${save_tests}/fidelity_save.input --check-commands"
                    -P "${save_tests}/run_fresh.cmake")
                set_tests_properties(fidelity_save_sb1_${edition}_${depth} PROPERTIES TIMEOUT 90)
            endforeach()
        endif()
    endforeach()
endif()
