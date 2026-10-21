# Writes the bytes of IN as a comma-separated C initializer list to OUT.
# Used to embed src/res/blupi.rc verbatim (cp1252 bytes, no transcoding);
# platform/resources.cpp parses its STRINGTABLE at start-up.
file(READ "${IN}" hex HEX)
string(LENGTH "${hex}" len)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
string(REGEX REPLACE "(0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,0x..,)" "\\1\n" bytes "${bytes}")
file(WRITE "${OUT}" "// Generated from ${IN}; do not edit.\n${bytes}\n")
