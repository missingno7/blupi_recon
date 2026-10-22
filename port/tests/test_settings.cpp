// The port settings file (game/settings.cpp): a save reads back the same, and
// a hand-written Windows path (single backslashes) reads as written.
// Usage: test_settings <scratch file>

#include "game/settings.h"

#include <cstdio>
#include <fstream>

static int failures = 0;

static void check(bool ok, const char* what)
{
	printf("%s %s\n", ok ? "ok  " : "FAIL", what);
	if (!ok) failures++;
}

int main(int argc, char** argv)
{
	if (argc < 2) return 2;
	const char* path = argv[1];

	PortSettings saved;
	saved.widescreen = true;
	saved.scale = 3;
	saved.dataFolder = "C:\\Games\\Speedy \"Blupi\"\\";
	check(PortSettingsSave(path, saved), "save");
	PortSettings loaded;
	check(PortSettingsLoad(path, loaded), "load");
	check(loaded.widescreen && loaded.scale == 3, "options read back");
	check(loaded.dataFolder == saved.dataFolder, "data folder read back");

	{
		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		file << "{ \"dataFolder\": \"D:\\Games\\Speedy Blupi\", \"interpolate\": true }";
	}
	PortSettings hand;
	check(PortSettingsLoad(path, hand), "load hand-written");
	check(hand.dataFolder == "D:\\Games\\Speedy Blupi", "single backslashes kept");
	check(hand.interpolate, "keys after the path");

	{
		std::ofstream file(path, std::ios::binary | std::ios::trunc);
		file << "{ \"scale\": 2 }";
	}
	PortSettings none;
	check(PortSettingsLoad(path, none) && none.dataFolder.empty(), "no data folder");
	return failures == 0 ? 0 : 1;
}
