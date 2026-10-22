// Font metrics the port sets through its build switches (game/port_switches.h):
// the inverted exclamation mark has Speedy Eggbert's Spanish width (src/text.cpp,
// LANGUAGE_ES), and the other letters keep the original's.
// Usage: test_text

#include <windows.h>
#include <stdio.h>
#include <ddraw.h>
#include "def.h"
#include "pixmap.h"
#include "text.h"

static int failures = 0;

static void check(bool ok, const char* what)
{
	printf("%s %s\n", ok ? "ok  " : "FAIL", what);
	if (!ok) failures++;
}

int main()
{
	check(GetTextWidth((char*)"\xa1", FONTWHITE) == 4, "inverted exclamation mark 4 pixels (glyph 5, Spanish 1.1)");
	check(GetTextWidth((char*)"!", FONTWHITE) == 5, "exclamation mark unchanged");
	check(GetTextWidth((char*)"\xbf", FONTWHITE) == 9, "inverted question mark unchanged");
	check(GetTextWidth((char*)"Version 1.0", FONTLITTLE) > 0, "little font");
	return failures == 0 ? 0 : 1;
}
