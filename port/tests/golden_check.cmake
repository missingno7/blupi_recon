# Golden-trace gate (port/docs/ROADMAP.md stage 0.2).
# Runs the game headless with ARGS and a checkpoint trace (every EVERY frames:
# frame, phase, state hash, screen hash) and compares it with GOLDEN.
#   -DUPDATE=ON rewrites GOLDEN instead (only when a change is intended and
#   explained; faithful-mode changes are regressions by definition).
# On a mismatch it reports the first differing checkpoint; find the field with
#   blupi-sdl3 <same args> --state-dump <frame>:dump.txt
# on both builds and diff the dumps.
separate_arguments(args NATIVE_COMMAND "${ARGS}")
file(REMOVE_RECURSE ${WORK})
file(MAKE_DIRECTORY ${WORK})
execute_process(
    COMMAND ${EXE} --headless --data ${DATA} --user ${WORK}/user ${args}
            --trace ${WORK}/trace.txt --trace-every ${EVERY}
    RESULT_VARIABLE rc OUTPUT_QUIET ERROR_QUIET TIMEOUT 900)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "game exited with ${rc}")
endif()
if(UPDATE)
    file(COPY_FILE ${WORK}/trace.txt ${GOLDEN})
    message(STATUS "updated ${GOLDEN}")
    return()
endif()
file(STRINGS ${WORK}/trace.txt got)
file(STRINGS ${GOLDEN} want)
list(LENGTH got ngot)
list(LENGTH want nwant)
if(NOT ngot EQUAL nwant)
    message(FATAL_ERROR "trace has ${ngot} checkpoints, golden has ${nwant}")
endif()
math(EXPR last "${nwant} - 1")
foreach(i RANGE ${last})
    list(GET got ${i} g)
    list(GET want ${i} w)
    if(NOT g STREQUAL w)
        message(FATAL_ERROR "first difference at checkpoint ${i}:\n  golden: ${w}\n  now:    ${g}\n"
                            "(columns: frame phase state-hash screen-hash; trace in ${WORK})")
    endif()
endforeach()
message(STATUS "${nwant} checkpoints identical to ${GOLDEN}")
