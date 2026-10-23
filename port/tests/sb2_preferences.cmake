# Exercise the real desktop settings path with SDL's window/audio dummy
# drivers. The game loop remains bounded; every write stays in this test root.
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/user")
file(WRITE "${WORK}/user/blupi2-sdl3.json"
    "{\"widescreen\":true,\"interpolate\":false,\"displayRate\":true,\"scale\":1,\"dataFolder\":\"${DATA}\"}\n")
file(WRITE "${WORK}/options.probe" "1 phase 1534\n")
file(WRITE "${WORK}/options.input" "5 click 90 430\n10 click 130 240\n15 click 90 350\n20 click 130 350\n25 click 390 300\n30 key 27\n")
execute_process(COMMAND "${CMAKE_COMMAND}" -E env SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy
    "${EXE}" --data "${DATA}" --user "${WORK}/user" --frames 35 --color 16
    --probe "${WORK}/options.probe" --input "${WORK}/options.input" --check-commands
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 20)
file(WRITE "${WORK}/options.log" "${out}${err}")
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Desktop preferences failed: ${rc}: ${out}${err}")
endif()
file(READ "${WORK}/user/blupi2-sdl3.json" settings)
foreach(key widescreen interpolate displayRate integerScale)
    string(JSON value GET "${settings}" ${key})
    if(NOT value)
        message(FATAL_ERROR "Options did not persist ${key}: ${settings}")
    endif()
endforeach()
# A later launch reads the same file; explicit flags override it for this run.
file(SHA256 "${WORK}/user/blupi2-sdl3.json" saved)
execute_process(COMMAND "${CMAKE_COMMAND}" -E env SDL_VIDEODRIVER=dummy SDL_RENDER_DRIVER=software SDL_AUDIODRIVER=dummy BLUPI2_DATA=
    "${EXE}" --user "${WORK}/user" --frames 3 --wide 16:9 --present tick --color 16
    --screenshot "2:${WORK}/override.bmp"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out ERROR_VARIABLE err TIMEOUT 20)
file(WRITE "${WORK}/reload.log" "${out}${err}")
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "Settings data-folder reload failed: ${rc}: ${out}${err}")
endif()
file(SHA256 "${WORK}/user/blupi2-sdl3.json" reloaded)
if(NOT saved STREQUAL reloaded)
    message(FATAL_ERROR "One-run command-line overrides overwrote saved preferences")
endif()
# The BMP width is a little-endian 32-bit integer at byte 18: 854 = 0x356.
file(READ "${WORK}/override.bmp" width OFFSET 18 LIMIT 4 HEX)
if(NOT width STREQUAL "56030000")
    message(FATAL_ERROR "The fixed CLI width did not override saved automatic widescreen: ${width}")
endif()
if(EXISTS "${WORK}/user/blupi-sdl3.json")
    message(FATAL_ERROR "SB2 wrote SB1's preferences file")
endif()
