include(${CMAKE_CURRENT_LIST_DIR}/sb2_smoke_helpers.cmake)
if(SCENARIO STREQUAL "tour")
    set(ARGS "${ARGS} --state-dump 100:${WORK}/before.dump --state-dump 110:${WORK}/paused.dump --state-dump 130:${WORK}/resumed.dump --screenshot 210:${WORK}/bye.bmp")
endif()
set(ARGS "${ARGS} --check-commands")
include(${CMAKE_CURRENT_LIST_DIR}/golden_check.cmake)
if(SCENARIO STREQUAL "tour")
    sb2_phase("${WORK}/trace.txt" 45 1545) # gamer
    sb2_phase("${WORK}/trace.txt" 75 1525) # official world 1
    # Build still advances m_time while paused, as in the original. MoveStep
    # stops the actors, not that rendering counter.
    sb2_field("${WORK}/before.dump" CDecor::m_moveObject before)
    sb2_field("${WORK}/paused.dump" CDecor::m_moveObject paused)
    sb2_field("${WORK}/resumed.dump" CDecor::m_moveObject resumed)
    if(NOT before STREQUAL paused OR paused STREQUAL resumed)
        message(FATAL_ERROR "Pause must stop, and resume must advance, moving objects")
    endif()
    sb2_field("${WORK}/before.dump" CDecor::m_bPause before)
    sb2_field("${WORK}/paused.dump" CDecor::m_bPause paused)
    sb2_field("${WORK}/resumed.dump" CDecor::m_bPause resumed)
    if(before STREQUAL paused OR NOT before STREQUAL resumed)
        message(FATAL_ERROR "Pause flag must toggle and return to its starting value")
    endif()
    file(STRINGS "${WORK}/trace.txt" trace)
    list(GET trace -1 last)
    if(EDITION STREQUAL "blupi")
        sb2_phase("${WORK}/trace.txt" 210 1556) # goodbye drawn before dismissal
        if(NOT EXISTS "${WORK}/bye.bmp" OR NOT last MATCHES "^230 1556 ")
            message(FATAL_ERROR "Blupi must draw BYE and close on its dismissal at 230")
        endif()
    elseif(NOT last MATCHES "^190 1524 " OR EXISTS "${WORK}/bye.bmp")
        message(FATAL_ERROR "Eggbert must close directly on the title's quit button at 190")
    endif()
else()
    sb2_phase("${WORK}/trace.txt" 105 1526) # editor
    sb2_phase("${WORK}/trace.txt" 165 1529) # private mission info
    sb2_phase("${WORK}/trace.txt" 205 1530) # playtest
    sb2_phase("${WORK}/trace.txt" 225 1526) # back to editor
    if(NOT EXISTS "${WORK}/user/data/u001-001.blp")
        message(FATAL_ERROR "editor mission did not land in the user root")
    endif()
endif()
