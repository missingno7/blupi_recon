# The simulation must not depend on the colour depth: the state-hash column of
# two golden traces (8 bpp and 16 bpp runs of the same scenario) must match.
file(STRINGS ${A} a)
file(STRINGS ${B} b)
list(TRANSFORM a REPLACE "^([0-9]+ [0-9]+ [0-9a-f]+) .*$" "\\1")
list(TRANSFORM b REPLACE "^([0-9]+ [0-9]+ [0-9a-f]+) .*$" "\\1")
if(NOT a STREQUAL b)
    message(FATAL_ERROR "state differs between ${A} and ${B}")
endif()
list(LENGTH a n)
message(STATUS "${n} checkpoints: identical simulation state in both colour depths")
