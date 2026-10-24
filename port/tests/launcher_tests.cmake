foreach(game sb1 sb2_eggbert sb2_blupi)
    if(game STREQUAL "sb1")
        set(target blupi)
        set(data "${BLUPI_TEST_DATA}")
        set(data_env BLUPI_DATA)
    elseif(game STREQUAL "sb2_eggbert")
        set(target blupi2)
        set(data "${BLUPI2_TEST_DATA}")
        set(data_env BLUPI2_DATA)
    else()
        set(target blupi2)
        set(data "${BLUPI2_TEST_DATA_BLUPI}")
        set(data_env BLUPI2_DATA)
    endif()
    if(data)
        set(runtime)
        set(runtime_name)
        if(WIN32)
            set(runtime $<TARGET_FILE:SDL3::SDL3-shared>)
            set(runtime_name $<TARGET_FILE_NAME:SDL3::SDL3-shared>)
        elseif(BLUPI_PORTABLE AND UNIX AND NOT APPLE)
            set(runtime $<TARGET_FILE:SDL3::SDL3-shared>)
            set(runtime_name $<TARGET_SONAME_FILE_NAME:SDL3::SDL3-shared>)
        endif()
        add_test(NAME launcher_discovery_${game}
            COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${target}> -DDATA=${data}
                -DENVIRONMENT=${data_env} -DSDL_RUNTIME=${runtime} -DSDL_NAME=${runtime_name}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/launcher_${game}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/launcher_discovery.cmake)
        if(game STREQUAL "sb1")
            set(entry_y 350)
        else()
            set(entry_y 431)
        endif()
        add_test(NAME launcher_options_keys_${game}
            COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${target}> -DDATA=${data}
                -DBUILDROOT=${CMAKE_CURRENT_BINARY_DIR}
                -DENTRY_Y=${entry_y} -DWORK=${CMAKE_CURRENT_BINARY_DIR}/launcher_keys_${game}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/launcher_options_keys.cmake)
    endif()
endforeach()

# Exercise each installed edition in both directions. The game-independent
# launcher must apply the adapter's recognizer to explicit and discovered roots.
foreach(case sb1_eggbert2 sb1_blupi2 sb2_blupi1 sb2_eggbert1)
    if(case MATCHES "^sb1_")
        set(target blupi)
        set(data_env BLUPI_DATA)
        set(data_title "Speedy Blupi 1 / Speedy Eggbert 1")
        set(right_data "${BLUPI_TEST_DATA}")
        if(case STREQUAL "sb1_eggbert2")
            set(wrong_data "${BLUPI2_TEST_DATA}")
        else()
            set(wrong_data "${BLUPI2_TEST_DATA_BLUPI}")
        endif()
    else()
        set(target blupi2)
        set(data_env BLUPI2_DATA)
        set(data_title "Speedy Blupi 2 / Speedy Eggbert 2")
        set(right_data "${BLUPI2_TEST_DATA}")
        if(case STREQUAL "sb2_blupi1")
            set(wrong_data "${BLUPI_TEST_DATA}")
        else()
            set(wrong_data "${BLUPI_TEST_DATA_EGGBERT}")
        endif()
    endif()
    if(right_data AND wrong_data)
        add_test(NAME launcher_wrong_game_${case}
            COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:${target}>
                -DRIGHT_DATA=${right_data} -DWRONG_DATA=${wrong_data}
                -DENVIRONMENT=${data_env} -DDATA_TITLE=${data_title}
                -DSDL_RUNTIME=${runtime} -DSDL_NAME=${runtime_name}
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/launcher_wrong_${case}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/launcher_wrong_game.cmake)
    endif()
endforeach()
