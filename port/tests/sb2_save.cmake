include(${CMAKE_CURRENT_LIST_DIR}/sb2_smoke_helpers.cmake)
file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
sb2_run(--frames 200 --input "${CMAKE_CURRENT_LIST_DIR}/golden/sb2_save.input"
    --state-dump "122:${WORK}/saved.dump" --state-dump "159:${WORK}/walked.dump"
    --state-dump "182:${WORK}/loaded.dump")
sb2_phase("${WORK}/trace.txt" 101 1584) # save dialog
sb2_phase("${WORK}/trace.txt" 161 1585) # load dialog
sb2_phase("${WORK}/trace.txt" 182 1525) # playing, paused after load
set(save "${WORK}/user/data/s001-000.blp")
if(NOT EXISTS "${save}")
    message(FATAL_ERROR "save did not land in the user root")
endif()
file(SIZE "${save}" size)
if(NOT size EQUAL 57008) # historical SB2 DescGameFile, 0xdeb0
    message(FATAL_ERROR "save size ${size}, expected SB2's 57008 bytes")
endif()
sb2_field("${WORK}/saved.dump" CDecor::m_blupiPos saved)
sb2_field("${WORK}/walked.dump" CDecor::m_blupiPos walked)
if(saved STREQUAL walked)
    message(FATAL_ERROR "the short walk must move away from the saved position")
endif()
# Check actual restored simulation fields, rather than transient dialog/tooltip
# and notification state (a successful save itself posts a notification).
foreach(field m_blupiPos m_blupiAction m_blupiDir m_blupiPhase m_blupiVitesse
    m_blupiShield m_blupiPower m_blupiCle m_nbVies m_nbTresor m_time m_region
    m_decor m_bigDecor m_moveObject m_balleTraj m_moveTraj)
    sb2_field("${WORK}/saved.dump" "CDecor::${field}" saved)
    sb2_field("${WORK}/loaded.dump" "CDecor::${field}" loaded)
    if(NOT saved STREQUAL loaded)
        message(FATAL_ERROR "save/load did not restore ${field}")
    endif()
endforeach()
