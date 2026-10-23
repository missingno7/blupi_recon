// Port settings: the presentation options the original does not have
// (port/docs/ROADMAP.md stage 9). Stored as a game-specific JSON file in the user
// directory, changed on the "Display options" screen (common/ui/options_screen.cpp)
// or overridden for one run by command-line options. None of them changes the
// simulation. The file also remembers where the game's files are.
#ifndef BLUPI_GAME_SETTINGS_H
#define BLUPI_GAME_SETTINGS_H

#include <string>

struct PortSettings
{
	bool widescreen = false;    // view width from the window shape, 4:3 to 21:9
	bool interpolate = false;   // smooth motion between ticks
	bool displayRate = true;    // present at the display rate (false: once per tick)
	bool fullscreen = false;
	bool integerScale = false;  // scale by whole factors only
	int  scale = 2;             // initial window size, times 640x480
	std::string dataFolder;     // the game's files, as chosen on the first run (desktop)
	std::string language;       // language pack code; empty: the installed language
};

// path is a host path (the user directory). Missing file or keys keep the
// values already in settings.
bool PortSettingsLoad(const char* path, PortSettings& settings);
bool PortSettingsSave(const char* path, const PortSettings& settings);

#endif
