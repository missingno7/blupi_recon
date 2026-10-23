// Port-only glue between the port's game-side replacements (pixmap, sound,
// misc, main). Not part of the historical interface.
#ifndef BLUPI_PORT_GAME_H
#define BLUPI_PORT_GAME_H

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

#include "render/device.h"

class CPixmap;
class CSound;
class CEvent;
class CDecor;
struct PortSettings;

// Presentation of the CPixmap primary surface (the original "screen").
void     PortPixmapPresentIfDirty();
bool     PortPixmapHasPrimary();
// 0xAARRGGBB copy of what is on screen now (logical size, tightly packed).
bool     PortPixmapCopyScreen(std::vector<uint32_t>& argb, int& width, int& height);
// FNV-1a of the visible frame as displayed colours (independent of bpp).
uint64_t PortPixmapScreenHash();
bool     PortPixmapWriteScreenBmp(const char* path);
// Cross-build oracle: log the raw back-buffer hash of every CPixmap::Display
// (and dump every Nth frame), in the format of port/oracle/ddraw_proxy.cpp.
void     PortPixmapSetDisplayCapture(FILE* log, const char* dumpDir, long dumpEvery);
// Stage-1 gate: record every rendering command and replay each frame on an
// independent device; the result must equal direct rendering at every Display.
void     PortPixmapEnableCommandCheck();
bool     PortPixmapCommandCheckResult(long& frames, long& mismatches, long& firstMismatch);
// Collect the commands submitted until the matching End (execute=false: do not draw).
void     PortPixmapCaptureBegin(bool execute);
void     PortPixmapCaptureEnd(std::vector<blupi::render::Command>& out);
void     PortPixmapSubmit(const std::vector<blupi::render::Command>& commands);
// Copies the back buffer to the screen for display-rate presentation.
void     PortPixmapPresentBack();

// Touch screens: the game's cursor sprite is not drawn (a tap is the click).
void     PortPixmapHideCursor(bool hidden);
// Touch screens: a game button (released or pressed) with an icon of the
// button sheet on it, as a 40x40 0xAARRGGBB image (transparent outside).
bool     PortPixmapButtonImage(CPixmap* pixmap, int iconMenu, bool pressed, std::vector<uint32_t>& argb);

// Widescreen view (ROADMAP.md stage 8). With a view width set, the host shows
// a target of that width instead of the 640x480 screen; the game's own back
// buffer and screen stay as they are.
void     PortPixmapSetWide(int width);
// Starts drawing a fresh (black) view target: CPixmap's drawing goes there,
// with the clip on all of it, until WideEnd.
bool     PortPixmapWideBegin(CPixmap* pixmap);
void     PortPixmapWideEnd(CPixmap* pixmap);
// Back-buffer commands (the captured UI) drawn shifted by dx into the target.
void     PortPixmapWideSubmit(const std::vector<blupi::render::Command>& commands, int dx);
// The 640x480 back buffer drawn at dx into the target (centred 4:3 frames).
void     PortPixmapWideBlitBack(int dx);
// The game's cursor sprite at its position shifted by dx, or at a frame
// position given by the host (the mouse over the side areas).
void     PortPixmapWideCursor(CPixmap* pixmap, int dx, const struct tagPOINT* at = NULL);
// The target is shown at the next PortPixmapPresentIfDirty.
void     PortPixmapWidePresent();
bool     PortPixmapWidePresentIfDirty();   // true when the view replaces the screen
void     PortPixmapWideRelease();
// Test: the 640x480 region of the target at x equals the back buffer.
bool     PortPixmapWideRegionEqualsBack(int x);
// The target, for screenshots (false without a view).
bool     PortPixmapCopyWide(std::vector<uint32_t>& argb, int& width, int& height);

// Delivers the MCI end-of-song notification of the music player (sound.cpp).
void     PortSoundPoll();

// Hash of the simulation state of CDecor, CEvent and the C runtime generator
// (state_hash.cpp).
uint64_t PortStateHash();
// One line per hashed field: name, size, hash (diagnoses trace differences).
void     PortStateDump(FILE* out);

// Test-only state probes (probe.cpp): load a script, apply before a frame.
bool     PortProbeLoad(FILE* file);
void     PortProbeApply(long frame);
void     PortProbeApplyBeforeDraw(long frame);

// The state changes CDecor::Build made while drawing (end_of_tick.cpp).
class CDecor;
void     EndOfTick(CDecor& decor);

// "Display options" screen (options_screen.cpp), shown on the game's settings
// screens. apply is called after every change of the settings.
// windowChoice: offer window or full screen (desktop; Android is always full screen).
void     PortOptionsInit(CPixmap* pixmap, CSound* sound, PortSettings* settings, void (*apply)(),
                         bool windowChoice);
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

// Editor input over the side areas of a wide view (editor_wide.cpp); pos in
// frame coordinates, outside the frame.
void     PortEditorSideMouse(CEvent* event, CDecor* decor, unsigned int message, int fwKeys, struct tagPOINT pos);

// Languages (language.cpp, port/docs/LANGUAGES.md). Init after the game roots
// are set: recognises the edition and the installed language, loads the packs
// and selects wanted (a pack code; empty: the installed language).
void        PortLanguageInit(const std::string& wanted);
bool        PortLanguageSelect(int index);
int         PortLanguageCount();
int         PortLanguageCurrent();
std::string PortLanguageCode(int index);
std::string PortLanguageName(int index);   // in the game's code page, for its font
// The current pack's string (Windows-1252), the edition's English one, or NULL.
const char* PortLanguageString(unsigned id);
// A port text (ids from 9000, see LANGUAGES.md): the pack's, or english.
std::string PortLanguageText(unsigned id, const char* english);
const std::string& PortEdition();          // "blupi" or "eggbert"
const char* PortEditionCredit();           // gamer-screen credit; empty for Blupi
// --export-language: a built-in pack ("de", "eggbert-de") written to path.
bool        PortLanguageExport(const std::string& which, const std::string& path);
// The current pack's language variant of the game's code ("de": German layouts).
extern std::string g_portLanguageVariant;

#endif
