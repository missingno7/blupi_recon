// Shared launcher. No game headers or historical compile switches belong here.
// Only the message boundary uses the existing declaration-only compat vocabulary.
#ifndef BLUPI_APP_LAUNCHER_H
#define BLUPI_APP_LAUNCHER_H
#include <cstdio>
// Game adapters force-include the historical RNG redirect. The standard
// library's <functional>/<algorithm> must see its own std::rand declaration.
#pragma push_macro("rand")
#undef rand
#include <functional>
#pragma pop_macro("rand")
#include <string>
#include <utility>
#include <vector>
#include "windows.h"
#include "platform/host.h"
#include "ui/settings.h"
#include "net/session.h"

namespace blupi::app {
struct ScriptedInput { long tick = 0; std::string kind; int x = 0, y = 0; };

struct Options
{
    blupi::host::Options host;
    std::string dataRoot;
    std::string userRoot;
    std::string soundFont;
    bool        help = false;
    bool        version = false;      // --version: print it and exit
    int         colorDepth = 0;       // 0 = from config.def
    long        ticks = -1;           // stop after this many WM_UPDATE frames
    std::string trace;                // per-frame "frame phase statehash screenhash"
    long        traceEvery = 1;
    std::vector<std::pair<long, std::string>> stateDumps;
    std::string displayLog;
    std::string displayDumpDir;
    long        displayDumpEvery = 0;
    bool        checkCommands = false;
    bool        checkScene = false;
    PortSettings settings;                   // file values, then the options below
    bool        setWide = false, setInterpolate = false, setPresent = false;
    bool        setFullscreen = false, setScale = false, setIntegerScale = false;
    bool        setLanguage = false;
    bool        checkView = false;
    bool        checkWide = false;
    int         presentRepeat = 0;           // --present-repeat (test)
    std::string probe;
    std::vector<std::pair<long, std::string>> screenshots;
    double      screenshotAlpha = -1.0;      // --screenshot-alpha (test)
    std::vector<ScriptedInput> inputs;
    int wideWidth = 0, renderRepeat = 0;
    bool reference = false;
    bool        verbose = false;
    blupi::net::Options net;
};


struct Game
{
    const char *name, *title, *dataTitle, *dataEnvironment, *settingsFile, *userApp;
    const char *extraUsage = "";
    const unsigned char* icon = nullptr;
    unsigned iconSize = 0;
    PortSettings* settings;
    std::string* settingsPath;
    bool* quit;
    int* timer;
    bool* arrowHeld;
    std::function<bool(const std::string&)> recognizesData;
    // Return 1 if handled, 0 if unknown, -1 if invalid; advance i for arguments.
    std::function<int(const std::string&, int&, int, char**)> option;
    // Early commands (e.g. language export): -1 means continue startup.
    std::function<int()> earlyCommand;
    std::function<bool(const Options&)> validateOptions;
    std::function<void(const Options&)> configure;
    // Zero on success; otherwise the launcher's exit code (1 startup, 2 input/I/O).
    std::function<int(const Options&)> initialize;
    std::function<void()> finish, timerStep, soundPoll, saveSettings, maintenance;
    std::function<int()> timerInterval, viewWidth;
    std::function<void(long)> prepareTick, completedTick;
    std::function<UINT()> phase;
    std::function<uint64_t()> stateHash, screenHash;
    std::function<void(FILE*)> stateDump;
    std::function<void(double)> present;
    std::function<void()> presentIfDirty;
    std::function<bool(const char*)> screenshot;
    std::function<void(FILE*, const char*, long)> displayCapture;
    std::function<int(const Options&)> checkResult;
    std::function<LRESULT(UINT, WPARAM, LPARAM)> windowProc;
    std::function<UINT(UINT)> postedMessage;
    std::function<bool(bool)> editorScrollBlocked;
    std::function<bool()> wideEditor;
    std::function<void(UINT, int, int, int)> editorMouse;
    std::function<bool(int, int, int)> mouseInSideArea;
};

int Run(int argc, char** argv, Game& game);
// Common installed/CD layout plus the marker supplied by the game's recognizer.
bool RecognizesData(const std::string& root, const char* marker);
} // namespace blupi::app
#endif
