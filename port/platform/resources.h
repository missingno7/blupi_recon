// Game resources the original EXE carried: the STRINGTABLE of src/res/blupi.rc,
// embedded verbatim at build time and parsed once. Strings stay cp1252 bytes,
// which is what the game's bitmap font tables index.
#ifndef BLUPI_PLATFORM_RESOURCES_H
#define BLUPI_PLATFORM_RESOURCES_H

namespace blupi::resources
{
    // nullptr when the id is not in the string table.
    const char* string(unsigned id);
    int count();
}

#endif
