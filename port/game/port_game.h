// Port-only glue between the port's game-side replacements (pixmap, sound,
// misc, main). Not part of the historical interface.
#ifndef BLUPI_PORT_GAME_H
#define BLUPI_PORT_GAME_H

#include <cstdint>
#include <cstdio>
#include <vector>

#include "render/device.h"

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

// Delivers the MCI end-of-song notification of the music player (sound.cpp).
void     PortSoundPoll();

// Hash of the simulation state of CDecor, CEvent and the C runtime generator
// (state_hash.cpp).
uint64_t PortStateHash();
// One line per hashed field: name, size, hash (diagnoses trace differences).
void     PortStateDump(FILE* out);

#endif
