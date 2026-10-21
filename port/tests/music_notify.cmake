# In-game MCI notification flow, headless (the synthesizer advances one timer
# interval per frame). The attract mode starts DEMO200 with music003 (180 s =
# 3600 frames) at frame ~992; by frame 5000 the song has ended, MM_MCINOTIFY
# (MCI_NOTIFY_SUCCESSFUL) has reached the window procedure, which suspended and
# restarted the music. A second run interrupts the demo with Escape: the
# still-playing song is closed and answered with MCI_NOTIFY_ABORTED (wParam=4).
file(REMOVE_RECURSE ${WORK})
file(MAKE_DIRECTORY ${WORK})
execute_process(
    COMMAND ${EXE} --headless --verbose --soundfont ${SOUNDFONT} --data ${DATA}
            --user ${WORK}/user --frames 5000
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc TIMEOUT 600)
set(log "${out}${err}")
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "end-of-song run exited with ${rc}")
endif()
if(NOT log MATCHES "MIDI open\\+play sound.music003.blp \\(device 1\\)[^\n]*\n(.*\n)?Event MM_MCINOTIFY\nMIDI close \\(device 1\\)\nEvent MCI_NOTIFY_SUCCESSFUL\nMIDI open\\+play sound.music003.blp \\(device 2\\)")
    message(FATAL_ERROR "end-of-song notification did not restart the music:\n${log}")
endif()

file(WRITE ${WORK}/escape.txt "1200 key 27\n")
execute_process(
    COMMAND ${EXE} --headless --verbose --soundfont ${SOUNDFONT} --data ${DATA}
            --user ${WORK}/user2 --frames 1300 --input ${WORK}/escape.txt
    OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc TIMEOUT 600)
set(log "${out}${err}")
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "escape run exited with ${rc}")
endif()
if(NOT log MATCHES "MIDI close \\(device 1\\)\nEvent MM_MCINOTIFY\nwParam=4\n")
    message(FATAL_ERROR "closing a playing song did not post MCI_NOTIFY_ABORTED:\n${log}")
endif()
if(log MATCHES "device 2")
    message(FATAL_ERROR "an aborted song must not be restarted:\n${log}")
endif()
message(STATUS "MCI notify/restart flow verified")
