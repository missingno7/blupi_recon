# Runs the game headless with ARGS and a fresh, empty user directory in WORK
# (sessions that save, like the editor's, would otherwise start from the last
# run's files). Fails when the game exits with a non-zero code.
separate_arguments(args NATIVE_COMMAND "${ARGS}")
file(REMOVE_RECURSE ${WORK})
file(MAKE_DIRECTORY ${WORK})
execute_process(
    COMMAND ${EXE} --headless --data ${DATA} --user ${WORK}/user ${args}
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_QUIET TIMEOUT 900)
message(STATUS "${out}")
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "game exited with ${rc}")
endif()
