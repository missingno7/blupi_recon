# Writes the built-in language packs (FILES, '|'-separated) to OUT as C arrays
# and the table k_builtinPacks[] of {file name, bytes, size}, for
# game/language.cpp. The packs are UTF-8 text, embedded byte for byte.
string(REPLACE "|" ";" files "${FILES}")
set(arrays "")
set(table "")
set(index 0)
foreach(file IN LISTS files)
    file(READ "${file}" hex HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")
    string(REGEX REPLACE "((0x..,){16})" "\\1\n" bytes "${bytes}")
    get_filename_component(name "${file}" NAME)
    string(APPEND arrays "const unsigned char k_pack${index}[] = {\n${bytes}\n};\n")
    string(APPEND table "    {\"${name}\", k_pack${index}, sizeof(k_pack${index})},\n")
    math(EXPR index "${index} + 1")
endforeach()
file(WRITE "${OUT}" "// Generated from port/languages; do not edit.\n${arrays}\nconst BuiltinPack k_builtinPacks[] = {\n${table}};\n")
