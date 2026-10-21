// Port settings file (see settings.h): a flat JSON object of booleans and
// numbers, read leniently (unknown keys are ignored) and written whole.

#include "game/settings.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>

// Host paths: the forced include maps fopen to the game file system.
#pragma push_macro("fopen")
#undef fopen
static FILE* host_fopen(const char* path, const char* mode)
{
	return fopen(path, mode);
}
#pragma pop_macro("fopen")

namespace
{
// key -> raw value text (true, false, a number or a quoted string)
bool ParseFlatObject(const std::string& text, std::map<std::string, std::string>& out)
{
	size_t i = text.find('{');
	if (i == std::string::npos) return false;
	i++;
	auto skip = [&]() { while (i < text.size() && isspace((unsigned char)text[i])) i++; };
	for (;;)
	{
		skip();
		if (i >= text.size()) return false;
		if (text[i] == '}') return true;
		if (text[i] == ',') { i++; continue; }
		if (text[i] != '"') return false;
		size_t end = text.find('"', i + 1);
		if (end == std::string::npos) return false;
		std::string key = text.substr(i + 1, end - i - 1);
		i = end + 1;
		skip();
		if (i >= text.size() || text[i] != ':') return false;
		i++;
		skip();
		size_t start = i;
		if (i < text.size() && text[i] == '"')
		{
			end = text.find('"', i + 1);
			if (end == std::string::npos) return false;
			i = end + 1;
		}
		else
		{
			while (i < text.size() && text[i] != ',' && text[i] != '}' && !isspace((unsigned char)text[i])) i++;
		}
		out[key] = text.substr(start, i - start);
	}
}

void ReadBool(const std::map<std::string, std::string>& values, const char* key, bool& value)
{
	auto it = values.find(key);
	if (it == values.end()) return;
	if (it->second == "true") value = true;
	else if (it->second == "false") value = false;
}

void ReadInt(const std::map<std::string, std::string>& values, const char* key, int& value, int lo, int hi)
{
	auto it = values.find(key);
	if (it == values.end()) return;
	char* end = NULL;
	long v = strtol(it->second.c_str(), &end, 10);
	if (end != it->second.c_str() && v >= lo && v <= hi) value = (int)v;
}
}  // namespace

bool PortSettingsLoad(const char* path, PortSettings& settings)
{
	FILE* file = host_fopen(path, "rb");
	if (file == NULL) return false;
	std::string text;
	char buffer[512];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0) text.append(buffer, n);
	fclose(file);

	std::map<std::string, std::string> values;
	if (!ParseFlatObject(text, values)) return false;
	ReadBool(values, "widescreen", settings.widescreen);
	ReadBool(values, "interpolate", settings.interpolate);
	ReadBool(values, "displayRate", settings.displayRate);
	ReadBool(values, "fullscreen", settings.fullscreen);
	ReadBool(values, "integerScale", settings.integerScale);
	ReadInt(values, "scale", settings.scale, 1, 8);
	return true;
}

bool PortSettingsSave(const char* path, const PortSettings& settings)
{
	FILE* file = host_fopen(path, "wb");
	if (file == NULL) return false;
	auto b = [](bool v) { return v ? "true" : "false"; };
	fprintf(file,
			"{\n"
			"  \"widescreen\": %s,\n"
			"  \"interpolate\": %s,\n"
			"  \"displayRate\": %s,\n"
			"  \"fullscreen\": %s,\n"
			"  \"integerScale\": %s,\n"
			"  \"scale\": %d\n"
			"}\n",
			b(settings.widescreen), b(settings.interpolate), b(settings.displayRate),
			b(settings.fullscreen), b(settings.integerScale), settings.scale);
	bool ok = ferror(file) == 0;
	fclose(file);
	return ok;
}
