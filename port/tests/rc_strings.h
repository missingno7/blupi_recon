// The STRINGTABLE of the reconstructed src/res/blupi.rc, embedded at build
// time for test_language, which checks that the built-in US pack equals it.
// The game itself answers LoadString from the language packs only. Strings
// stay cp1252 bytes, as in the .rc.
#ifndef BLUPI_TESTS_RC_STRINGS_H
#define BLUPI_TESTS_RC_STRINGS_H

namespace blupi::rc_strings
{
    // nullptr when the id is not in the string table.
    const char* string(unsigned id);
    int count();
}

#endif
