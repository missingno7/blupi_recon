# Read the existing diagnostic dump; no raw struct/padding or host pointers.
function(sb2_field path field out)
    file(READ "${path}" dump)
    string(REGEX MATCH "${field} +[0-9]+ ([0-9a-f]+)" found "${dump}")
    if(NOT found)
        message(FATAL_ERROR "missing ${field} in ${path}")
    endif()
    set(${out} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()

function(sb2_phase trace frame phase)
    file(STRINGS "${trace}" lines REGEX "^${frame} ${phase} ")
    if(NOT lines)
        message(FATAL_ERROR "expected phase ${phase} at frame ${frame}: ${trace}")
    endif()
endfunction()

function(sb2_run)
    execute_process(COMMAND "${EXE}" --headless --data "${DATA}" --user "${WORK}/user"
        --color 8 --trace "${WORK}/trace.txt" --check-commands ${ARGN}
        RESULT_VARIABLE rc OUTPUT_VARIABLE output ERROR_VARIABLE errors TIMEOUT 60)
    file(WRITE "${WORK}/run.log" "${output}\n${errors}")
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "SB2 exited with ${rc}; see ${WORK}/run.log")
    endif()
endfunction()
