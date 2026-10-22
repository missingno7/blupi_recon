// Forced include (after common/compat/include/port_forced.h) for every TU
// compiled with the historical-source context: the values the port gives
// Speedy Blupi 1's build switches (src/def.h; the matching targets set them
// per executable in targets.json).
//
// One port binary serves both editions and every language, so a switch whose
// value depends on them is a run-time call, answered from the recognised
// edition and the chosen language pack (game/language.cpp). The matching
// source uses those switches only in expressions, never in #if (root
// AGENTS.md, Targets). Every other switch keeps its def.h default, except as
// set here.
#ifndef BLUPI_GAME_PORT_SWITCHES_H
#define BLUPI_GAME_PORT_SWITCHES_H

#ifdef __cplusplus
extern "C" {
#endif
int         blupi_language_de(void);    // the pack's variant is "de"
const char* blupi_version_text(void);   // the edition's title-screen version
#ifdef __cplusplus
}
#endif

// The German help-screen layout (src/event.cpp DrawButtons).
#define LANGUAGE_DE (blupi_language_de())
// "Version 1.0" (Speedy Blupi) or "Version 1.1" (Speedy Eggbert).
#define VERSION_TEXT (blupi_version_text())
// Speedy Eggbert's Spanish width for glyph 5 (src/text.cpp): the inverted
// exclamation mark, the only character drawn with that glyph, which only
// Spanish texts use. No other language's text changes.
#define LANGUAGE_ES 1

#endif
