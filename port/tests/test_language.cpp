// Language packs (platform/language.cpp, port/docs/LANGUAGES.md):
//   - every built-in pack parses without problems;
//   - the US pack's strings equal the reconstructed src/res/blupi.rc;
//   - the format's escapes, quotes and [chars] forms;
//   - text conversion (Windows-1252, base letters).
// Usage: test_language <port/languages folder>

#include "platform/language.h"
#include "rc_strings.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace lang = blupi::language;

static int failures = 0;

static void check(bool ok, const std::string& what)
{
	printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
	if (!ok) failures++;
}

// UTF-8 to Windows-1252, the way built-in packs are encoded for the game.
static std::string to_cp1252(const std::string& text)
{
	std::string out;
	size_t i = 0;
	uint32_t cp;
	while (lang::next_codepoint(text, i, cp)) out += (char)lang::cp1252(cp);
	return out;
}

int main(int argc, char** argv)
{
	if (argc < 2) return 2;
	namespace fs = std::filesystem;
	int packs = 0;
	for (const auto& entry : fs::directory_iterator(argv[1]))
	{
		if (entry.path().extension() != ".txt") continue;
		std::ifstream in(entry.path(), std::ios::binary);
		std::stringstream text;
		text << in.rdbuf();
		lang::Pack pack;
		bool ok = lang::parse(text.str(), entry.path().filename().string(), pack);
		std::string name = entry.path().filename().string();
		check(ok && pack.problems.empty() && pack.strings.size() == 329,
			  name + ": parses, 329 strings, no problems");
		for (const std::string& p : pack.problems) printf("     %s\n", p.c_str());
		for (const auto& s : pack.strings)
		{
			size_t i = 0;
			uint32_t cp;
			bool encodable = true;
			while (lang::next_codepoint(s.second, i, cp)) encodable = encodable && lang::cp1252(cp) >= 0;
			if (!encodable) check(false, name + ": string " + std::to_string(s.first) + " is Windows-1252");
		}
		packs++;
		if (name == "sb1-blupi-us.txt")
		{
			int same = 0;
			for (const auto& s : pack.strings)
			{
				const char* rc = blupi::rc_strings::string(s.first);
				if (rc != nullptr && to_cp1252(s.second) == rc) same++;
				else printf("     differs from blupi.rc: %u\n", s.first);
			}
			check(same == (int)pack.strings.size() && same == blupi::rc_strings::count(),
				  "US pack equals src/res/blupi.rc (" + std::to_string(same) + " strings)");
		}
	}
	check(packs == 9, "nine built-in packs (" + std::to_string(packs) + ")");

	lang::Pack p;
	const std::string sample =
		"\xEF\xBB\xBF# comment\nname = Test\ncode = xx\ngame = sb1\n\n[chars]\n"
		"\xC3\x87 = \xC3\xA7\n"                       // Ç = ç
		"\xC3\x81 = A + acute @ 2,-2\n"               // Á = A + acute
		"x = #1d\n"
		"\xC4\x8D = c + #05 @ 0,-3 base @ 1,0\n"      // č with a custom accent glyph
		"[strings]\n"
		"# comment between strings\n"
		"1 = plain text\n"
		"2 = \"  edge spaces  \"\n"
		"3 = path\\\\User and\\nnew line\n"
		"4 = #8, starts with a hash\n"
		"5 =\n";
	check(lang::parse(sample, "sample", p) && p.problems.empty(), "sample parses without problems");
	check(p.name == "Test" && p.code == "xx" && p.game == "sb1" && p.variant == "xx", "header");
	check(p.chars.size() == 4, "four [chars] entries");
	if (p.chars.size() == 4)
	{
		check(p.chars[0].letter == 0xC7 && p.chars[0].alias == 0xE7, "alias entry");
		check(p.chars[1].base == 'A' && p.chars[1].accent == 0x02 && p.chars[1].ax == 2 && p.chars[1].ay == -2,
			  "composed entry");
		check(p.chars[2].glyph == 0x1d, "glyph entry");
		check(p.chars[3].letter == 0x10D && p.chars[3].accent == 5 && p.chars[3].ay == -3 && p.chars[3].bx == 1,
			  "composed entry with a glyph accent and a base offset");
	}
	check(p.strings[1] == "plain text", "plain string");
	check(p.strings[2] == "  edge spaces  ", "quoted edge spaces");
	check(p.strings[3] == "path\\User and\nnew line", "escapes");
	check(p.strings[4] == "#8, starts with a hash", "hash at the start of a text");
	check(p.strings.count(5) && p.strings[5].empty(), "empty text");

	lang::Pack bad;
	lang::parse("name = x\n[strings]\nabc = 1\nno equals\n", "bad", bad);
	check(bad.problems.size() == 3, "problems reported: bad id, no '=', missing code/game");

	check(lang::cp1252(0x20AC) == 0x80 && lang::cp1252(0xE9) == 0xE9 && lang::cp1252(0x10D) == -1, "Windows-1252");
	check(lang::base_letter(0x10D) == 'c' && lang::base_letter(0x159) == 'r' && lang::base_letter(0xFD) == 'y' &&
			  lang::base_letter(0x3A9) == 0,
		  "base letters");
	return failures == 0 ? 0 : 1;
}
