// Shared SB1/SB2 launcher, extracted from SB1's SDL adapter.
// Compat message declarations are confined to app/ and compat/; platform/ stays clean.
#include "app/launcher.h"
#include "port_messages.h"
#include "ui/options_events.h"
#include "platform/vfs.h"
#include "platform/audio.h"
#include "platform/music.h"
#include "presentation/timing.h"
#include <cstdlib>
#include <cctype>

namespace blupi::app {
bool RecognizesData(const std::string& root, const char* marker)
{
    if (root.empty()) return false;
    blupi::vfs::set_roots(root, "");
    return !blupi::vfs::resolve_read("data\\config.def").empty() &&
           !blupi::vfs::resolve_read("image08\\init.blp").empty() &&
           !blupi::vfs::resolve_read(marker).empty();
}

namespace {
void Usage(const Game& game)
{
    printf(
        "%s " BLUPI_PORT_VERSION " [options]\n"
        "  --data DIR          game root holding DATA/, IMAGE08/, IMAGE16/, SOUND/ (read-only);\n"
        "                      default: %s, the folder chosen on the first run (kept\n"
        "                      in the settings), the program's folder\n"
        "  --user DIR          writable directory for saves and user missions\n"
        "  --help, -h          show these options and exit\n"
        "  --windowed          start in a window\n"
        "  --fullscreen        start in desktop fullscreen (Alt+Enter toggles)\n"
        "  --scale N           initial window scale (default 2)\n"
        "  --present display|tick  present at the display rate (default; the game still\n"
        "                      runs at its 50 ms tick) or only when the game draws (vanilla)\n"
        "  --interpolate       smooth movement between ticks when presenting at the display\n"
        "                      rate (visual only; shows the world up to one tick later)\n"
        "  --wide auto|off|W|A:B  widescreen: auto follows the window shape (4:3 to 21:9);\n"
        "                      W or A:B is a fixed view W pixels wide (or of aspect A:B at\n"
        "                      height 480). The world shows more around the 640x480 frame;\n"
        "                      HUD and menus stay in the centred frame\n"
        "settings: the window and display options are kept in %s in the user\n"
        "directory and set on the game's settings screen (Display options); the options\n"
        "above override them for one run.\n"
        "  --integer-scale     scale only by whole factors\n"
        "  --color 8|16        override TrueColor from config.def\n"
        "  --soundfont FILE    SoundFont (.sf2) for the MIDI music; default: BLUPI_SOUNDFONT,\n"
        "                      then soundfont.sf2 in the user directory or next to the program\n"
        "  --verbose           log the game's debug output\n"
        "  --version           print the version and exit\n"
        "  --language CODE     built-in pack or a pack in the user folder languages/\n"
        "test options:\n"
        "  --headless          no window, no audio device, frames run back to back\n"
        "  --frames N          exit after N game frames (WM_UPDATE)\n"
        "  --trace FILE        write 'frame phase state-hash screen-hash' per frame\n"
        "  --trace-every N     write only every Nth frame (checkpoints)\n"
        "  --state-dump N:FILE write every hashed state field after frame N\n"
        "  --display-log FILE  raw back-buffer hash at every Display (cross-build oracle)\n"
        "  --display-dump DIR:N  raw frame file at every Nth Display\n"
        "  --check-commands    replay every frame's rendering commands on a second device\n"
        "                      and require identical pixels (exit code 3 otherwise)\n"
        "  --render-repeat K   render each tick's frame K extra times (must change nothing)\n"
        "  --present-repeat K  present each tick's frame K extra times with the cursor (test)\n"
        "  --reference         historical SetDecor (CDecor::Build and DrawButtons drawing\n"
        "                      directly) instead of scene + EndOfTick + captured UI\n"
        "  --check-scene       build the frame scene before CDecor::Build and require the\n"
        "                      same rendering commands (exit code 4 otherwise)\n"
        "  --check-view        require the view renderer at alpha 1 to issue the faithful\n"
        "                      frame's rendering commands (exit code 5 otherwise)\n"
        "  --check-wide        with --wide: require the centre of the wide view to equal the\n"
        "                      faithful frame where it is centred (exit code 6 otherwise)\n"
        "  --screenshot N:FILE write the screen after frame N as a 24-bit BMP\n"
        "  --screenshot-alpha A  present the view at alpha A (0..1 between the previous\n"
        "                      tick and frame N) before writing screenshots\n"
        "  --probe FILE        test-only state injection before a frame\n"
        "  --input FILE        scripted input: lines 'FRAME click|move X Y' or\n"
        "                      'FRAME key|keydown|keyup VK' (may be given more than once)\n"
        "                      viewclick|viewmove|viewrightclick X Y use view coordinates\n"
        "%s", game.name, game.dataEnvironment, game.settingsFile, game.extraUsage);
}
bool LoadInputScript(const std::string& path, std::vector<ScriptedInput>& inputs)
{
    FILE* f = fopen(path.c_str(), "r");
    if (!f) return false;
    char line[256], kind[16];
    while (fgets(line, sizeof(line), f)) {
        ScriptedInput in;
        if (line[0] == '#' || sscanf(line, "%ld %15s %d %d", &in.tick, kind, &in.x, &in.y) < 3) continue;
        in.kind = kind;
        if (in.kind != "key" && in.kind != "keydown" && in.kind != "keyup" && in.kind != "click" && in.kind != "move" &&
            in.kind != "viewclick" && in.kind != "viewmove" && in.kind != "viewrightclick") { fclose(f); return false; }
        inputs.push_back(in);
    }
    fclose(f);
    return true;
}
bool ParseOptions(int argc, char** argv, Options& o, Game& game)
{
    bool missingArgument = false;
    for (int i = 1; i < argc; i++)
    {
        std::string a = argv[i];
        auto next = [&]() -> const char* {
            if (i + 1 < argc) return argv[++i];
            missingArgument = true;
            return "";
        };
        if (a == "--data") o.dataRoot = next();
        else if (a == "--user") o.userRoot = next();
        else if (a == "--fullscreen") { o.settings.fullscreen = true; o.setFullscreen = true; }
        else if (a == "--windowed") { o.settings.fullscreen = false; o.setFullscreen = true; }
        else if (a == "--scale") { o.settings.scale = atoi(next()); o.setScale = true; }
        else if (a == "--integer-scale") { o.settings.integerScale = true; o.setIntegerScale = true; }
        else if (a == "--color") o.colorDepth = atoi(next());
        else if (a == "--soundfont") o.soundFont = next();
        else if (a == "--verbose") o.verbose = true;
        else if (a == "--headless") o.host.headless = true;
        else if (a == "--frames") o.ticks = atol(next());
        else if (a == "--trace") o.trace = next();
        else if (a == "--trace-every") { o.traceEvery = atol(next()); if (o.traceEvery < 1) o.traceEvery = 1; }
        else if (a == "--display-log") o.displayLog = next();
        else if (a == "--check-commands") o.checkCommands = true;
        else if (a == "--check-scene") o.checkScene = true;
        else if (a == "--check-view") o.checkView = true;
        else if (a == "--screenshot-alpha") o.screenshotAlpha = atof(next());
        else if (a == "--interpolate") { o.settings.interpolate = true; o.setInterpolate = true; }
        else if (a == "--check-wide") o.checkWide = true;
        else if (a == "--wide")
        {
            std::string v = next();
            o.setWide = true;
            if (v == "auto" || v == "off")
            {
                o.settings.widescreen = v == "auto";
                o.wideWidth = 0;
                continue;
            }
            o.settings.widescreen = false;
            size_t colon = v.find(':');
            if (colon != std::string::npos && atoi(v.c_str() + colon + 1) <= 0) return false;
            int width = colon == std::string::npos ? atoi(v.c_str())
                        : (480 * atoi(v.c_str()) + atoi(v.c_str() + colon + 1) - 1) / atoi(v.c_str() + colon + 1);
            width = (width + 1) & ~1;   // even: the frame is centred on whole pixels
            if (width < 640 || width > 4 * 640) return false;
            o.wideWidth = width == 640 ? 0 : width;
        }
        else if (a == "--reference") o.reference = true;
        else if (a == "--render-repeat") o.renderRepeat = atoi(next());
        else if (a == "--present")
        {
            std::string mode = next();
            if (mode == "tick") o.settings.displayRate = false;
            else if (mode == "display") o.settings.displayRate = true;
            else return false;
            o.setPresent = true;
        }
        else if (a == "--present-repeat") o.presentRepeat = atoi(next());
        else if (a == "--probe") o.probe = next();
        else if (a == "--display-dump")
        {
            std::string v = next();
            size_t colon = v.rfind(':');
            if (colon == std::string::npos || colon < 2) return false;
            o.displayDumpDir = v.substr(0, colon);
            o.displayDumpEvery = atol(v.substr(colon + 1).c_str());
        }
        else if (a == "--state-dump")
        {
            std::string v = next();
            size_t colon = v.find(':');
            if (colon == std::string::npos) return false;
            o.stateDumps.push_back({atol(v.substr(0, colon).c_str()), v.substr(colon + 1)});
        }
        else if (a == "--screenshot")
        {
            std::string v = next();
            size_t colon = v.find(':');
            if (colon == std::string::npos) return false;
            o.screenshots.push_back({atol(v.substr(0, colon).c_str()), v.substr(colon + 1)});
        }
        else if (a == "--input")
        {
            if (!LoadInputScript(next(), o.inputs)) return false;
        }
        else if (a == "--help" || a == "-h") o.help = true;
        else if (a == "--version") o.version = true;
        else if (a == "--language") { o.settings.language = next(); o.setLanguage = true; }
        else {
            int result = game.option ? game.option(a, i, argc, argv) : 0;
            if (result <= 0) {
                printf(result == 0 ? "unknown option: %s\n" : "invalid option: %s\n", a.c_str());
                return false;
            }
        }
    }
    return !missingArgument && o.presentRepeat >= 0 && o.renderRepeat >= 0;
}
LPARAM MousePos(int x, int y)
{
    return MAKELPARAM(x, y);
}

WPARAM MouseKeys(const blupi::host::Event& e)
{
    WPARAM w = 0;
    if (e.leftDown) w |= MK_LBUTTON;
    if (e.rightDown) w |= MK_RBUTTON;
    if (e.shift) w |= MK_SHIFT;
    if (e.control) w |= MK_CONTROL;
    return w;
}

// Host event -> the window messages Windows would have delivered.
void TranslateHostEvent(const blupi::host::Event& e, Game& game)
{
    using blupi::host::EventType;
    switch (e.type)
    {
    case EventType::Quit:
        game.windowProc(WM_CLOSE, 0, 0);
        break;
    case EventType::FocusGained:
        game.windowProc(WM_ACTIVATEAPP, 1, 0);
        break;
    case EventType::FocusLost:
        game.windowProc(WM_ACTIVATEAPP, 0, 0);
        break;
    case EventType::KeyDown:
        if (PortOptionsKey(WM_KEYDOWN, (WPARAM)e.key)) break;
        if (e.key == VK_LEFT || e.key == VK_RIGHT)
        {
            const bool right = e.key == VK_RIGHT;
            if (game.editorScrollBlocked(right)) break;
            game.arrowHeld[right] = true;
        }
        game.windowProc(WM_KEYDOWN, (WPARAM)e.key, (LPARAM)(1 | (e.repeat ? (1 << 30) : 0)));
        // TranslateMessage: control characters that become WM_CHAR.
        if (e.key == VK_BACK || e.key == VK_RETURN || e.key == VK_ESCAPE || e.key == VK_TAB)
            game.windowProc(WM_CHAR, (WPARAM)e.key, 1);
        break;
    case EventType::KeyUp:
        if (PortOptionsKey(WM_KEYUP, (WPARAM)e.key)) break;
        if (e.key == VK_LEFT || e.key == VK_RIGHT)
        {
            const bool right = e.key == VK_RIGHT;
            if (!game.arrowHeld[right]) break;   // already released at the editor's edge
            game.arrowHeld[right] = false;
        }
        game.windowProc(WM_KEYUP, (WPARAM)e.key, (LPARAM)(1 | (1 << 30) | (1u << 31)));
        break;
    case EventType::Text:
        if (PortOptionsKey(WM_CHAR, (WPARAM)e.ch)) break;
        game.windowProc(WM_CHAR, (WPARAM)e.ch, 1);
        break;
    case EventType::MouseMove:
    case EventType::MouseDown:
    case EventType::MouseUp:
    {
        UINT message = WM_MOUSEMOVE;
        if (e.type == EventType::MouseDown) message = e.button == 1 ? WM_LBUTTONDOWN : WM_RBUTTONDOWN;
        if (e.type == EventType::MouseUp) message = e.button == 1 ? WM_LBUTTONUP : WM_RBUTTONUP;
        if (game.wideEditor() && !PortOptionsOpen() && e.x >= 0 && e.x < game.viewWidth() && e.y >= 0 && e.y < 480)
        {
            game.editorMouse(message, (int)MouseKeys(e), e.x, e.y);
            break;
        }
        if (!e.inside)
        {
            // Outside the game's window (the side areas of a wide view in play,
            // the letterbox): as before.
            if (!game.mouseInSideArea(game.viewWidth(), e.x, e.y) && message == WM_MOUSEMOVE)
            {
                game.windowProc(WM_NCMOUSEMOVE, 0, 0);
            }
            break;
        }
        if (PortOptionsMouse(message, MouseKeys(e), MousePos(e.x, e.y))) break;
        game.windowProc(message, MouseKeys(e), MousePos(e.x, e.y));
        if (message == WM_MOUSEMOVE) PortOptionsHover(MousePos(e.x, e.y));
        break;
    }
    }
}

// TranslateMessage for posted key presses (scripted input): the historical
// message loop turned them into a following WM_CHAR. Unshifted keyboard state.
// Live keyboard input gets its WM_CHAR from TranslateHostEvent instead.
int TranslatedChar(WPARAM vk)
{
    if (vk == VK_BACK || vk == VK_TAB || vk == VK_RETURN || vk == VK_ESCAPE || vk == VK_SPACE) return (int)vk;
    if (vk >= '0' && vk <= '9') return (int)vk;
    if (vk >= 'A' && vk <= 'Z') return (int)(vk - 'A' + 'a');
    return 0;
}

void DispatchPosted(Game& game)
{
    PortMessage msg;
    while (!*game.quit && PortPopMessage(msg))
    {
        if (PortOptionsCommand(msg.message)) continue;   // the port's own buttons
        // Scripted mouse input meets the port's screen like live input.
        if ((msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN || msg.message == WM_LBUTTONUP ||
             msg.message == WM_RBUTTONDOWN || msg.message == WM_RBUTTONUP) &&
            PortOptionsMouse(msg.message, msg.wParam, msg.lParam)) continue;
        if ((msg.message == WM_KEYDOWN || msg.message == WM_KEYUP || msg.message == WM_CHAR) &&
            PortOptionsKey(msg.message, msg.wParam)) continue;
        if (game.postedMessage) msg.message = game.postedMessage(msg.message);
        game.windowProc(msg.message, msg.wParam, msg.lParam);
        int ch = msg.message == WM_KEYDOWN ? TranslatedChar(msg.wParam) : 0;
        if (ch != 0 && !*game.quit) game.windowProc(WM_CHAR, (WPARAM)ch, msg.lParam);
    }
}

void PostScriptedInput(const Options& o, long frame, Game& game) {
    for (const ScriptedInput& in : o.inputs) {
        if (in.tick != frame) continue;
        // Presentation-coordinate editor smoke checks use the same route as
        // live host input; ordinary scripts keep their historical coordinates.
        if (in.kind == "viewclick" || in.kind == "viewmove" || in.kind == "viewrightclick") {
            blupi::host::Event e{};
            e.x = in.x; e.y = in.y;
            e.inside = in.x >= 0 && in.x < 640 && in.y >= 0 && in.y < 480;
            e.type = blupi::host::EventType::MouseMove;
            TranslateHostEvent(e, game);
            if (in.kind != "viewmove") {
                e.button = in.kind == "viewrightclick" ? 3 : 1;
                e.type = blupi::host::EventType::MouseDown;
                e.leftDown = e.button == 1; e.rightDown = e.button != 1;
                TranslateHostEvent(e, game);
                e.type = blupi::host::EventType::MouseUp;
                e.leftDown = e.rightDown = false;
                TranslateHostEvent(e, game);
            }
            continue;
        }
        if (in.kind == "move" || in.kind == "click") {
            // Post historical messages without warping the host pointer. This
            // also avoids synthetic host mouse events on desktop script runs.
            PostMessage(nullptr, WM_MOUSEMOVE, 0, MAKELPARAM(in.x, in.y));
            if (in.kind == "click") {
                PostMessage(nullptr, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(in.x, in.y));
                PostMessage(nullptr, WM_LBUTTONUP, 0, MAKELPARAM(in.x, in.y));
            }
        } else {
            if (in.kind != "keyup") PostMessage(nullptr, WM_KEYDOWN, in.x, 1);
            if (in.kind != "keydown") PostMessage(nullptr, WM_KEYUP, in.x, 1 | (1u<<30) | (1u<<31));
        }
    }
}

} // namespace
int Run(int argc, char** argv, Game& game)
{
    Options o;
    if (!ParseOptions(argc, argv, o, game))
    {
        Usage(game);
        return 2;
    }
    if (o.help) { Usage(game); return 0; }
    if (o.version)
    {
        printf("%s %s\n", game.name, BLUPI_PORT_VERSION);
        return 0;
    }
    if (game.earlyCommand) { int rc = game.earlyCommand(); if (rc >= 0) return rc; }
    if (game.validateOptions && !game.validateOptions(o)) { Usage(game); return 2; }
    // Port settings (game/settings.h): the file, then this run's options.
    // Headless runs use defaults and options only, and save nothing.
    const bool headless = o.host.headless;
    {
        PortSettings fromFile;
        if (!headless)
        {
            std::string user = o.userRoot.empty() ? host::pref_path("Epsitec", game.userApp) : o.userRoot + "/";
            (*game.settingsPath) = user + game.settingsFile;
            PortSettingsLoad((*game.settingsPath).c_str(), fromFile);
        }
        PortSettings& s = o.settings;
        if (!o.setWide) s.widescreen = fromFile.widescreen;
        if (!o.setInterpolate) s.interpolate = fromFile.interpolate;
        if (!o.setPresent) s.displayRate = fromFile.displayRate;
        if (!o.setFullscreen) s.fullscreen = fromFile.fullscreen;
        if (!o.setScale) s.scale = fromFile.scale;
        if (!o.setIntegerScale) s.integerScale = fromFile.integerScale;
        s.dataFolder = fromFile.dataFolder;
        if (!o.setLanguage) s.language = fromFile.language;
        (*game.settings) = s;
    }
    o.host.fullscreen = (*game.settings).fullscreen;
    o.host.scale = (*game.settings).scale;
    o.host.integerScale = (*game.settings).integerScale;
    o.host.autoWide = (*game.settings).widescreen && o.wideWidth == 0;
    o.host.viewWidth = o.wideWidth;
    game.configure(o);
    o.host.iconPng = game.icon;
    o.host.iconPngSize = game.iconSize;
    if (!host::init(game.title, 640, 480, o.host)) return 1;
    host::set_verbose(o.verbose);
    host::log_info((std::string(game.name) + " " BLUPI_PORT_VERSION).c_str());
    {
        char text[80];
        snprintf(text, sizeof(text), "view %dx%d", blupi::host::view_width(), 480);
        blupi::host::log_debug(text);
    }

    // Locate the read-only game data and the writable user directory.
    std::vector<std::string> candidates;
    if (!o.dataRoot.empty()) candidates.push_back(o.dataRoot);
    if (const char* env = getenv(game.dataEnvironment)) candidates.push_back(env);
    if (!(*game.settings).dataFolder.empty()) candidates.push_back((*game.settings).dataFolder);
    candidates.push_back(blupi::host::base_path());
    for (const std::string& dir : blupi::host::data_dirs()) candidates.push_back(dir);
    candidates.push_back(".");
    std::string root;
    for (const std::string& c : candidates)
    {
        if (game.recognizesData(c)) { root = c; break; }
        if (!o.dataRoot.empty()) break;  // an explicit --data must be right
    }
    // Desktop first run: the player shows where their copy of the game is (its
    // installed folder, or a folder holding it, like the CD's), and the choice
    // is kept in the settings file.
    const std::string choose = std::string("Choose the folder where ") + game.dataTitle +
        " is installed: the one that contains DATA, IMAGE08, IMAGE16 and SOUND.";
    std::string ask = std::string(game.dataTitle) + " needs the files of the original game.\n\n" + choose;
    while (root.empty() && o.dataRoot.empty() && !headless)
    {
        const std::string chosen = blupi::host::choose_folder(game.title, ask.c_str());
        if (chosen.empty()) break;
        std::vector<std::string> tries = {chosen};
        for (const std::string& sub : blupi::host::subdirectories(chosen)) tries.push_back(sub);
        for (const std::string& c : tries)
        {
            if (game.recognizesData(c)) { root = c; break; }
        }
        if (root.empty())
        {
            ask = std::string("That folder does not contain ") + game.dataTitle + " game data.\n\n" + choose;
            continue;
        }
        PortSettings file;
        PortSettingsLoad((*game.settingsPath).c_str(), file);
        file.dataFolder = root;
        (*game.settings).dataFolder = root;
        if (!PortSettingsSave((*game.settingsPath).c_str(), file))
        {
            blupi::host::log_info(("cannot write " + (*game.settingsPath)).c_str());
        }
    }
    if (root.empty())
    {
        std::string message = std::string(game.dataTitle) + " data not found.\n";
        if (!o.dataRoot.empty()) message += "The selected folder is not a " + std::string(game.dataTitle) +
            " game-data folder:\n" + o.dataRoot + "\n";
        message += "Pass --data with a folder from " + std::string(game.dataTitle) + ".";
        blupi::host::message_box(game.title, message.c_str());
        blupi::host::shutdown();
        return 1;
    }
    std::string user = o.userRoot.empty() ? host::pref_path("Epsitec", game.userApp) : o.userRoot;
    blupi::vfs::set_roots(root, user);
    // A CD (Speedy Blupi 1 and 2): the data at its root, SOUND in each language
    // folder (identical in all of them). The English one is preferred.
    if (blupi::vfs::resolve_read("sound").empty())
    {
        std::string chosen;
        for (const std::string& sub : blupi::host::subdirectories(root))
        {
            blupi::vfs::set_language_root(sub);
            bool has = !blupi::vfs::resolve_read("sound").empty();
            blupi::vfs::set_language_root("");
            if (!has) continue;
            std::string name = sub.substr(sub.find_last_of("/\\") + 1);
            for (char& c : name) c = (char)tolower((unsigned char)c);
            if (chosen.empty() || name == "us" || name == "english") chosen = sub;
        }
        blupi::vfs::set_language_root(chosen);
        if (!chosen.empty()) blupi::host::log_info(("CD layout: language folder " + chosen).c_str());
    }
    blupi::audio::init(!o.host.headless);

    // SoundFont for the MIDI music. Headless runs load one only on request, so
    // automated runs stay independent of the audio clock.
    {
        std::vector<std::string> fonts;
        if (!o.soundFont.empty()) fonts.push_back(o.soundFont);
        else if (!o.host.headless)
        {
            if (const char* env = getenv("BLUPI_SOUNDFONT")) fonts.push_back(env);
            fonts.push_back(user + "/soundfont.sf2");
            fonts.push_back(blupi::host::base_path() + "soundfont.sf2");
            fonts.push_back("soundfont.sf2");   // Android: the APK asset
        }
        for (const std::string& f : fonts)
        {
            if (blupi::music::load_soundfont(f))
            {
                blupi::host::log_info(("MIDI music: SoundFont " + f).c_str());
                break;
            }
        }
    }

    FILE* trace = NULL;
    if (!o.trace.empty()) trace = fopen(o.trace.c_str(), "w");
    FILE* displayLog = o.displayLog.empty() ? NULL : fopen(o.displayLog.c_str(), "w");
    if ((!o.trace.empty() && !trace) || (!o.displayLog.empty() && !displayLog)) {
        if (trace) fclose(trace);
        if (displayLog) fclose(displayLog);
        audio::shutdown(); music::shutdown(); host::shutdown();
        return 2;
    }
    game.displayCapture(displayLog, o.displayDumpDir.c_str(), o.displayDumpEvery);
    int result = 0;
    if (int rc = game.initialize(o)) {
        game.finish();
        game.displayCapture(nullptr, "", 0);
        if (trace) fclose(trace);
        if (displayLog) fclose(displayLog);
        audio::shutdown(); music::shutdown(); host::shutdown();
        return rc;
    }
    const uint64_t interval = (uint64_t)game.timerInterval() * 1000000ull;
    uint64_t nextTick = blupi::host::now_ns() + interval;
    long frame = 0;
    int lastTimer = 0;
    long presents = 0;
    const uint64_t loopStart = blupi::host::now_ns();
    uint64_t lastTick = loopStart;   // when the latest tick ran (interpolation)
    PortPerf perf;

    while (!*game.quit && (o.ticks < 0 || frame < o.ticks))
    {
        blupi::host::Event e;
        while (!*game.quit && blupi::host::poll_event(e)) TranslateHostEvent(e, game);
        if (*game.quit) break;

        // The wide editor at a world edge: stop scrolling that way.
        for (int right = 0; right < 2; right++)
        {
            if (game.arrowHeld[right] && game.editorScrollBlocked(right != 0))
            {
                game.arrowHeld[right] = false;
                game.windowProc(WM_KEYUP, right ? VK_RIGHT : VK_LEFT, (LPARAM)(1 | (1 << 30) | (1u << 31)));
            }
        }

        uint64_t now = blupi::host::now_ns();
        // Messages already queued (and those they post, e.g. a button's command
        // after WM_LBUTTONUP) are handled before the timer's WM_UPDATE, as in
        // the original message loop, where the timer thread posted WM_UPDATE
        // behind them. [EVIDENCE] cross-build oracle, scripted tour 2026-10-21.
        game.soundPoll();
        DispatchPosted(game);
        if (*game.quit) break;
        if (o.host.headless || now >= nextTick)
        {
            PostScriptedInput(o, frame, game);
            game.prepareTick(frame);
            DispatchPosted(game);
            if (*game.quit) break;
            const uint64_t tickStart = blupi::host::now_ns();
            game.timerStep();
            perf.Tick(blupi::host::now_ns() - tickStart);
            lastTick = now;
            nextTick = PortNextTick(nextTick, now, interval);
        }
        lastTimer = *game.timer;
        DispatchPosted(game);
        if (*game.quit) break;

        if (lastTimer == 1 && *game.timer == 0)  // a WM_UPDATE frame completed
        {
            frame++;
            game.completedTick(frame);
            if (o.host.headless)
            {
                // No audio device: advance the synthesizer by one timer interval per
                // frame so music, and its end-of-song notification, follow game time.
                static std::vector<float> discard;
                int frames = 44100 * game.timerInterval() / 1000;
                discard.assign((size_t)frames * 2, 0.0f);
                blupi::music::render_mix(discard.data(), frames, 44100);
            }
            if (trace && frame % o.traceEvery == 0)
            {
                fprintf(trace, "%ld %u %016llx %016llx\n", frame, game.phase(),
                        (unsigned long long)game.stateHash(),
                        (unsigned long long)game.screenHash());
            }
            for (const auto& dump : o.stateDumps)
            {
                if (dump.first != frame) continue;
                if (FILE* f = fopen(dump.second.c_str(), "w")) { game.stateDump(f); fclose(f); }
                else result = 2;
            }
            for (const auto& shot : o.screenshots)
            {
                if (shot.first != frame) continue;
                // What is presented: overlays and the wide view included, at
                // --screenshot-alpha between the ticks (test).
                game.present(o.screenshotAlpha >= 0.0 ? o.screenshotAlpha : 1.0);
                if (!game.screenshot(shot.second.c_str())) result = 2;
            }
            if (o.ticks >= 0 && frame >= o.ticks)
            {
                game.windowProc(WM_CLOSE, 0, 0);
                break;
            }
            // test: with --interpolate at alpha 1/K .. K/K, ending on the faithful frame
            for (int i = 0; i < o.presentRepeat; i++) game.present(((*game.settings).interpolate && (*game.settings).displayRate) ? (i + 1.0) / o.presentRepeat : 1.0);
            // Tick-rate presentation ("Frame rate: Original"): once per tick, with
            // the port's overlays and the wide view.
            if (!(*game.settings).displayRate && !o.host.headless) game.present(1.0);
        }

        // Settings changed outside the options screen (Alt+Enter).
        if (!o.host.headless && blupi::host::fullscreen() != (*game.settings).fullscreen)
        {
            (*game.settings).fullscreen = blupi::host::fullscreen();
            PortOptionsSync();
            game.saveSettings();
        }
        game.maintenance();

        if ((*game.settings).displayRate && !o.host.headless)
        {
            // Display rate: the latest frame and the current cursor every loop
            // iteration; SDL_RenderPresent waits for the display (vsync).
            const uint64_t t0 = blupi::host::now_ns();
            game.present(PortFrameAlpha(t0, lastTick, interval));
            const uint64_t t1 = blupi::host::now_ns();
            game.presentIfDirty();
            const uint64_t t2 = blupi::host::now_ns();
            presents++;
            perf.Present(t1 - t0, t2 - t1, t2, frame, game.viewWidth());
            if (!blupi::host::presents_with_vsync())
            {
                now = blupi::host::now_ns();
                uint64_t pause = 4000000;   // no vsync: about 250 presents per second at most
                if (now < nextTick && nextTick - now < pause) pause = nextTick - now;
                if (now < nextTick && blupi::host::wait_event(e, pause)) TranslateHostEvent(e, game);
            }
            continue;
        }

        game.presentIfDirty();

        if (!o.host.headless)
        {
            now = blupi::host::now_ns();
            if (now < nextTick && blupi::host::wait_event(e, nextTick - now)) TranslateHostEvent(e, game);
        }
    }

    if (presents > 0)
    {
        char text[120];
        double seconds = (blupi::host::now_ns() - loopStart) / 1e9;
        snprintf(text, sizeof(text), "presented %ld frames in %.1f s (%.0f per second), %ld game frames",
                 presents, seconds, presents / seconds, frame);
        blupi::host::log_info(text);
    }
    if (trace) fclose(trace);
    game.displayCapture(nullptr, "", 0);
    if (displayLog) fclose(displayLog);
    int checked = game.checkResult(o);
    if (checked) result = checked;
    game.finish();
    audio::shutdown(); music::shutdown(); host::shutdown();
    return result;
}

} // namespace blupi::app
