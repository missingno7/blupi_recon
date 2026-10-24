// Shared options-screen event/presentation API. Game object setup stays per game.
#ifndef BLUPI_UI_OPTIONS_EVENTS_H
#define BLUPI_UI_OPTIONS_EVENTS_H
#include <cstdint>
void     PortOptionsPhase(unsigned int phase);  // every frame: shows or closes it
bool     PortOptionsVisible();   // the entry button or the screen is shown
bool     PortOptionsOpen();      // the screen is shown
// Input for it; true when the event is not for the game.
bool     PortOptionsMouse(unsigned int message, uintptr_t wParam, intptr_t lParam);
bool     PortOptionsKey(unsigned int message, uintptr_t wParam);
void     PortOptionsHover(intptr_t lParam);   // after the game's WM_MOUSEMOVE: the cursor sprite
bool     PortOptionsCommand(unsigned int message);   // its buttons' posted messages
void     PortOptionsDraw();      // into the back buffer, after the frame
void     PortOptionsSync();      // settings changed elsewhere (Alt+Enter)

#endif
