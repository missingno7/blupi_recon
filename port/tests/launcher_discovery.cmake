# Build a minimal installed-folder layout, without any original executables.
# Prefer hard links; restricted NTFS ACLs may require copies of these startup files.
# All game writes go to separate user directories.
file(MAKE_DIRECTORY "${WORK}/installed" "${WORK}/empty")
get_filename_component(exe_name "${EXE}" NAME)
configure_file("${EXE}" "${WORK}/installed/${exe_name}" COPYONLY)
if(SDL_RUNTIME)
    configure_file("${SDL_RUNTIME}" "${WORK}/installed/${SDL_NAME}" COPYONLY)
endif()
file(GLOB roots LIST_DIRECTORIES true RELATIVE "${DATA}" "${DATA}/*")
foreach(dir IN LISTS roots)
    string(TOLOWER "${dir}" lower)
    if(NOT lower MATCHES "^(data|image08)$")
        continue()
    endif()
    file(GLOB_RECURSE files LIST_DIRECTORIES false RELATIVE "${DATA}" "${DATA}/${dir}/*")
    foreach(file IN LISTS files)
        string(TOLOWER "${file}" lower_file)
        if(lower STREQUAL "image08" AND NOT lower_file MATCHES
                "/(init|object|blupi00[0-3]|temp|map|element|explo|decor000|button00|jauge|text|little)\\.blp$")
            continue()
        endif()
        get_filename_component(parent "${WORK}/installed/${file}" DIRECTORY)
        file(MAKE_DIRECTORY "${parent}")
        if(NOT EXISTS "${WORK}/installed/${file}")
            file(CREATE_LINK "${DATA}/${file}" "${WORK}/installed/${file}" COPY_ON_ERROR RESULT rc)
            if(NOT rc STREQUAL "0")
                message(FATAL_ERROR "Cannot hard-link fixture ${file}: ${rc}")
            endif()
        endif()
    endforeach()
endforeach()
set(flags --headless --frames 3 --color 8 --check-commands)
# Empty environment, empty CWD, no preferences: only the executable folder works.
execute_process(COMMAND "${CMAKE_COMMAND}" -E env "${ENVIRONMENT}="
    "${WORK}/installed/${exe_name}" ${flags} --user "${WORK}/discovered-user"
    --trace "${WORK}/discovered.trace"
    WORKING_DIRECTORY "${WORK}/empty"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 30)
file(WRITE "${WORK}/discovered.log" "${out}${err}")
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Executable-folder discovery failed: ${rc}: ${out}${err}")
endif()
# The same data selected explicitly must produce exactly the same state/pixels.
execute_process(COMMAND "${EXE}" ${flags} --data "${DATA}" --user "${WORK}/explicit-user"
    --trace "${WORK}/explicit.trace"
    WORKING_DIRECTORY "${WORK}/empty"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 30)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Explicit data failed: ${rc}: ${out}${err}")
endif()
file(READ "${WORK}/discovered.trace" discovered)
file(READ "${WORK}/explicit.trace" explicit)
if(NOT discovered STREQUAL explicit)
    message(FATAL_ERROR "Executable-folder data differs from explicit data")
endif()
# A valid environment root must not rescue an invalid explicit --data.
execute_process(COMMAND "${CMAKE_COMMAND}" -E env "${ENVIRONMENT}=${DATA}"
    "${WORK}/installed/${exe_name}" ${flags} --data "${WORK}/empty" --user "${WORK}/invalid-user"
    WORKING_DIRECTORY "${WORK}/empty"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 10)
if(NOT rc EQUAL 1)
    message(FATAL_ERROR "Invalid explicit root fell back to another root: ${rc}: ${out}${err}")
endif()
