include(${CMAKE_CURRENT_LIST_DIR}/sb2_smoke_helpers.cmake)
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/user/data" "${WORK}/user/image08")
if(CASE STREQUAL "intro")
    # Shipped _INTRO=0 folders omit publisher images. Supply valid placeholder
    # art in the test user root to exercise the timer, without inventing assets
    # or claiming a historical pixel comparison for these optional phases.
    file(GLOB title "${DATA}/[Ii][Mm][Aa][Gg][Ee]08/[Ii][Nn][Ii][Tt].[Bb][Ll][Pp]")
    file(COPY_FILE "${title}" "${WORK}/user/image08/intro1.blp")
    file(COPY_FILE "${title}" "${WORK}/user/image08/intro2.blp")
    file(WRITE "${WORK}/intro.probe" "20 phase 1589\n")
    sb2_run(--frames 160 --probe "${WORK}/intro.probe")
    if(EDITION STREQUAL "eggbert")
        sb2_phase("${WORK}/trace.txt" 21 1589)
        sb2_phase("${WORK}/trace.txt" 81 1590)
        sb2_phase("${WORK}/trace.txt" 142 1524)
    else()
        foreach(frame 21 81 142)
            sb2_phase("${WORK}/trace.txt" ${frame} 1524)
        endforeach()
    endif()
else()
    # User files shadow read-only data: failure fixtures never alter assets/.
    if(CASE STREQUAL "read")
        file(WRITE "${WORK}/user/data/world001.blp" "")
        set(trigger 70)
        set(extra "")
    else()
        file(WRITE "${WORK}/user/image08/help.blp" "")
        set(trigger 100)
        set(extra "100 key 112\n") # F1: help background cache error
    endif()
    file(WRITE "${WORK}/error.input"
        "40 key 13\n70 click 85 430\n${extra}200 click 36 444\n")
    math(EXPR first "${trigger}+1")
    math(EXPR retry "${trigger}+41")
    math(EXPR after "${trigger}+42")
    sb2_run(--frames 220 --input "${WORK}/error.input"
        --state-dump "${first}:${WORK}/first.dump"
        --state-dump "${retry}:${WORK}/retry.dump"
        --state-dump "${after}:${WORK}/after.dump")
    if(EDITION STREQUAL "blupi")
        sb2_phase("${WORK}/trace.txt" ${first} 1543) # INSERT
        sb2_phase("${WORK}/trace.txt" ${retry} 1543)
        sb2_phase("${WORK}/trace.txt" 201 1545) # Back -> gamer
        sb2_field("${WORK}/first.dump" CEvent::m_tryInsertCount firstCount)
        sb2_field("${WORK}/retry.dump" CEvent::m_tryInsertCount retryCount)
        sb2_field("${WORK}/after.dump" CEvent::m_tryInsertCount afterCount)
        if(firstCount STREQUAL retryCount OR NOT firstCount STREQUAL afterCount)
            message(FATAL_ERROR "Blupi must retry the supplied folder every 41 ticks")
        endif()
    else()
        sb2_phase("${WORK}/trace.txt" ${first} 1524) # title immediately
        sb2_phase("${WORK}/trace.txt" 201 1524)
    endif()
endif()
