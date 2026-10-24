# SB2 surface depth from config.def (no --color): the original CPixmap::Create
# selects a 16-bit display when TrueColorBack or TrueColorDecor is set
# (games/sb2/src/pixmap.cpp), so mixed settings must give 16-bit surfaces.
get_filename_component(work_absolute "${WORK}" ABSOLUTE)
get_filename_component(root_absolute "${BUILDROOT}" ABSOLUTE)
string(FIND "${work_absolute}/" "${root_absolute}/" prefix)
if(NOT prefix EQUAL 0 OR work_absolute STREQUAL root_absolute)
    message(FATAL_ERROR "mixed-depth test work directory must be below ${BUILDROOT}")
endif()
file(REMOVE_RECURSE "${work_absolute}")
foreach(combo "8;8;8" "16;8;16" "8;16;16" "16;16;16")
    list(GET combo 0 back)
    list(GET combo 1 decor)
    list(GET combo 2 expected)
    set(case "${WORK}/b${back}_d${decor}")
    file(MAKE_DIRECTORY "${case}/user/data")
    # Benchmark >= 3100 lets ReadConfig honour the two TrueColor keys.
    file(WRITE "${case}/user/data/config.def"
        "CD-Rom=.\r\nFullScreen=1\r\nBenchmark=6666\r\nTrueColorBack=${back}\r\nTrueColorDecor=${decor}\r\nMouseType=2\r\n")
    execute_process(COMMAND "${EXE}" --headless --data "${DATA}" --user "${case}/user"
        --frames 1 --display-log "${case}/display.log"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 30)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "back ${back} decor ${decor}: ${result}\n${output}\n${error}")
    endif()
    file(STRINGS "${case}/display.log" displays)
    list(GET displays 0 first)
    if(NOT first MATCHES "^0 ${expected} [0-9a-f]+$")
        message(FATAL_ERROR "back ${back} decor ${decor}: expected a ${expected}-bit display, got: ${first}")
    endif()
endforeach()
