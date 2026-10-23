// SB2 port values, forced after common/compat/include/port_forced.h.
// The shared layout/phase table uses Eggbert 2's superset (_EGAMES=1).
// Edition strings come from the data; CD checks are omitted because VFS
// reads a supplied game folder (ARCHITECTURE.md section 9).
#ifndef SB2_PORT_SWITCHES_H
#define SB2_PORT_SWITCHES_H
extern "C" const char* blupi_version_text(void);
#define _EGAMES 1
#define VERSION_TEXT (blupi_version_text())
// SB2 en/de/fr differ only in resources, never in help layout or glyphs.
#define LANGUAGE_DE 0
#define LANGUAGE_ACCENTS 0
#define LANGUAGE_ES 0
#define LANGUAGE_PT 0
#include "def.h"
#undef REQUIRE_CD
#define REQUIRE_CD 0
#endif
