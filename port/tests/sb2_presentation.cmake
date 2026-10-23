# SB2 M4b: intermediate presentation and wider coverage must leave the
# faithful state/screen trace equal to historical Build, even on effect paths.
set(ARGS "--color ${DEPTH} --frames 160 --input ${CMAKE_CURRENT_LIST_DIR}/sb2_scene.input --probe ${CMAKE_CURRENT_LIST_DIR}/sb2_scene.probe --wide 21:9 --interpolate --present-repeat 3")
include(${CMAKE_CURRENT_LIST_DIR}/sb2_scene.cmake)

# Preserve M4a's standalone --reference --check-view combination too, without
# requiring --check-scene. This still runs historical Build for state changes.
separate_arguments(args NATIVE_COMMAND "${ARGS}")
execute_process(COMMAND "${EXE}" --headless --data "${DATA}" --user "${WORK}/view-reference"
    ${args} --reference --check-view --trace "${WORK}/view-reference.trace"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 30)
file(WRITE "${WORK}/view-reference.log" "${out}${err}")
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Standalone reference view check failed: ${rc}: ${out}${err}")
endif()
file(READ "${WORK}/view-reference.trace" view_reference)
if(NOT native STREQUAL view_reference)
    message(FATAL_ERROR "Standalone reference view path changed the faithful trace")
endif()

# Options are drawn only for presentation; input is swallowed by the overlay.
# The baseline has only the first hover that reaches the historical handler.
file(WRITE "${WORK}/options.probe" "1 phase 1534\n")
file(WRITE "${WORK}/baseline.input" "5 move 90 430\n")
file(WRITE "${WORK}/options.input" "5 click 90 430\n10 click 130 130\n15 click 130 240\n20 click 90 350\n25 click 130 350\n30 click 390 300\n35 click 560 430\n")
foreach(mode baseline options)
    execute_process(COMMAND "${EXE}" --headless --data "${DATA}" --user "${WORK}/${mode}"
        --color ${DEPTH} --frames 45 --probe "${WORK}/options.probe" --input "${WORK}/${mode}.input"
        --present-repeat 3 --trace "${WORK}/${mode}-options.trace" --check-commands --check-presentation
        --state-dump "6:${WORK}/${mode}-options.dump"
        --screenshot "18:${WORK}/${mode}-options.bmp"
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 30)
    file(WRITE "${WORK}/${mode}-options.log" "${out}${err}")
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "${mode} options run failed: ${rc}: ${out}${err}")
    endif()
    file(READ "${WORK}/${mode}-options.trace" states)
    string(REGEX REPLACE " [0-9a-f]+\n" "\n" states "${states}")
    set(${mode}_states "${states}")
    if(EXISTS "${WORK}/${mode}/blupi2-sdl3.json")
        message(FATAL_ERROR "Headless presentation must not save settings")
    endif()
endforeach()
if(NOT baseline_states STREQUAL options_states)
    message(FATAL_ERROR "Display options changed simulation state")
endif()
file(SHA256 "${WORK}/baseline-options.bmp" baseline_image)
file(SHA256 "${WORK}/options-options.bmp" options_image)
if(baseline_image STREQUAL options_image)
    message(FATAL_ERROR "Display options did not change the presented image")
endif()

# A right click on the extra world, beyond the original 640-wide input area,
# must reach the editor's delete tool. The fixture seeds only via --probe.
file(WRITE "${WORK}/editor.probe" "100 @draw clear\n100 @draw camera 0 0\n100 @draw tile 12 2 211\n")
file(WRITE "${WORK}/delete.input" "110 viewrightclick 800 160\n")
foreach(mode before after)
    set(extra)
    if(mode STREQUAL "after")
        set(extra --input "${WORK}/delete.input")
    endif()
    execute_process(COMMAND "${EXE}" --headless --data "${DATA}" --user "${WORK}/editor-${mode}"
        --color ${DEPTH} --frames 120 --wide 16:9 --check-view --check-commands
        --input "${CMAKE_CURRENT_LIST_DIR}/golden/sb2_editor.input" --probe "${WORK}/editor.probe"
        --state-dump "115:${WORK}/${mode}.dump" ${extra}
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 30)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Wide editor ${mode} failed: ${rc}: ${out}${err}")
    endif()
endforeach()
include(${CMAKE_CURRENT_LIST_DIR}/sb2_smoke_helpers.cmake)
sb2_field("${WORK}/before.dump" CDecor::m_decor before)
sb2_field("${WORK}/after.dump" CDecor::m_decor after)
if(before STREQUAL after)
    message(FATAL_ERROR "Wide editor side click did not delete its world cell")
endif()

# Interpolation must actually move pixels between ticks, including the camera
# and parallax; all three alpha values retain the same faithful trace.
file(WRITE "${WORK}/interpolation.probe" "75-103 pause 1\n100 @draw clear\n100 @draw camera 200 64\n100 @draw position 400 128\n100 @draw tile 10 2 211\n101 @draw camera 216 64\n101 @draw position 416 128\n")
foreach(alpha 0 0.5 1)
    execute_process(COMMAND "${EXE}" --headless --data "${DATA}" --user "${WORK}/alpha-${alpha}"
        --color ${DEPTH} --frames 104 --wide 21:9 --interpolate --check-view --check-commands
        --input "${CMAKE_CURRENT_LIST_DIR}/sb2_scene.input" --probe "${WORK}/interpolation.probe"
        --trace "${WORK}/alpha-${alpha}.trace" --screenshot "102:${WORK}/alpha-${alpha}.bmp" --screenshot-alpha ${alpha}
        RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 30)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Interpolation alpha ${alpha} failed: ${rc}: ${out}${err}")
    endif()
    file(READ "${WORK}/alpha-${alpha}.trace" trace)
    file(SHA256 "${WORK}/alpha-${alpha}.bmp" pixels)
    if(DEFINED last_trace AND (NOT last_trace STREQUAL trace OR last_pixels STREQUAL pixels))
        message(FATAL_ERROR "Interpolation must change presented pixels without changing faithful traces")
    endif()
    set(last_trace "${trace}")
    set(last_pixels "${pixels}")
endforeach()
