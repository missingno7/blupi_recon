// Port language support for Speedy Blupi 1 and Speedy Eggbert 1
// (port/docs/LANGUAGES.md). Port-owned; no historical origin.
//
// - The edition (Blupi or Eggbert) is recognised from the data (the title
//   image), the installed language from the installed executable (its size
//   and FNV-1a hash only; it is never run).
// - The packs are the built-in ones of that edition (port/languages, embedded)
//   and the player's (languages/*.txt in the user folder; one with a built-in
//   code replaces that pack).
// - Applying a pack: its strings answer LoadString, encoded in the game's
//   single-byte code page (Windows-1252), and its [chars] entries go into the
//   font table table_char. That table is font data, not simulation state; the
//   original's is restored before each pack is applied.

#include <stdio.h>
#include <string.h>
#include "def.h"
#include "game/port_game.h"
#include "platform/host.h"
#include "platform/language.h"
#include "platform/vfs.h"

#include <map>
#include <set>
#include <string>
#include <vector>

extern char table_char[];

std::string g_portLanguageVariant = "us";

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
namespace lang = blupi::language;

struct BuiltinPack
{
	const char* file;
	const unsigned char* data;
	size_t size;
};
#include "language_packs.inc"   // k_builtinPacks[] (cmake/embed_packs.cmake)

struct KnownBuild
{
	unsigned long long hash;   // FNV-1a 64 of the whole file
	long size;
	const char* edition;
	const char* code;
};

// The executables of the editions and languages we know (2026-10-22).
const KnownBuild k_builds[] = {
	{0xd640855283598e8aULL, 535552, "blupi", "us"},
	{0x610b8c906f647304ULL, 538112, "blupi", "de"},
	{0xbef9df9c1ae3cf57ULL, 538112, "blupi", "fr"},
	{0xf8f0339277cb092dULL, 534528, "eggbert", "en"},
	{0xce7ac17f5f5dcd4cULL, 537088, "eggbert", "de"},
	{0xf7a88c99973366f9ULL, 537088, "eggbert", "es"},
	{0xd0c7f8ae9ceb08c7ULL, 537088, "eggbert", "fr"},
	{0xce181f1e1c988a20ULL, 536576, "eggbert", "it"},
	{0x459defd64dd9b032ULL, 536576, "eggbert", "pt"},
};

// IMAGE08\INIT.BLP, the title image, tells the editions apart.
const struct { unsigned long long hash; const char* edition; } k_titles[] = {
	{0xe9654ea706e78a9bULL, "blupi"},
	{0x847cfb0febbd955aULL, "eggbert"},
};

const int  kCharBytes = 6;
const int  kTableSize = 256 * kCharBytes;
const char kBlank[kCharBytes] = {1, 0, 0, (char)0xff, 0, 0};   // a code the font does not draw

struct Encoded
{
	std::vector<std::string> strings;   // indexed like the pack's string ids
	std::set<unsigned> ids;
};

std::string g_edition = "blupi";
std::string g_installed;              // code of the recognised installed executable
std::vector<lang::Pack> g_packs;
int g_current = -1;
int g_fallback = -1;                  // the edition's English pack
std::map<unsigned, std::string> g_text;       // current strings, Windows-1252
std::map<unsigned, std::string> g_fallbackText;
std::map<uint32_t, unsigned char> g_encode;   // letters given a code by the pack
char g_original[kTableSize];
bool g_saved = false;

unsigned long long Fnv(const std::string& bytes)
{
	unsigned long long h = 0xcbf29ce484222325ULL;
	for (unsigned char c : bytes)
	{
		h ^= c;
		h *= 1099511628211ULL;
	}
	return h;
}

bool ReadFile(const std::string& path, std::string& out, long limit)
{
	FILE* file = host_fopen(path.c_str(), "rb");
	if (file == NULL) return false;
	out.clear();
	char buffer[16384];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0)
	{
		out.append(buffer, n);
		if ((long)out.size() > limit) break;
	}
	fclose(file);
	return (long)out.size() <= limit;
}

const char* EditionFromData()
{
	std::string path = blupi::vfs::resolve_read("image08\\init.blp");
	std::string bytes;
	if (path.empty() || !ReadFile(path, bytes, 4 << 20)) return NULL;
	unsigned long long h = Fnv(bytes);
	for (const auto& t : k_titles)
	{
		if (t.hash == h) return t.edition;
	}
	return NULL;
}

// The installed executable in the game root, else the language folder of a
// CD. Only its size and hash are read.
const KnownBuild* InstalledBuild()
{
	for (const std::string& name : blupi::vfs::list("*.exe"))
	{
		std::string path = blupi::vfs::resolve_read(name.c_str());
		std::string bytes;
		if (path.empty() || !ReadFile(path, bytes, 2 << 20)) continue;
		unsigned long long h = Fnv(bytes);
		for (const KnownBuild& b : k_builds)
		{
			if (b.hash == h && b.size == (long)bytes.size()) return &b;
		}
	}
	return NULL;
}

int Find(const std::string& code)
{
	for (size_t i = 0; i < g_packs.size(); i++)
	{
		if (g_packs[i].code == code) return (int)i;
	}
	return -1;
}

void Log(const lang::Pack& pack)
{
	for (const std::string& p : pack.problems)
	{
		blupi::host::log_info(("language " + pack.source + ": " + p).c_str());
	}
}

void LoadPacks()
{
	g_packs.clear();
	for (const BuiltinPack& b : k_builtinPacks)
	{
		lang::Pack pack;
		std::string text((const char*)b.data, b.size);
		if (lang::parse(text, std::string("built-in ") + b.file, pack) && pack.game == "sb1" &&
			pack.edition == g_edition)
		{
			g_packs.push_back(pack);
		}
	}
	// The player's packs (languages\*.txt in the user folder, or in the game
	// folder): same game, any or the same edition.
	for (const std::string& file : blupi::vfs::list("languages\\*.txt"))
	{
		std::string path = blupi::vfs::resolve_read(("languages\\" + file).c_str());
		std::string text;
		if (path.empty() || !ReadFile(path, text, 1 << 20)) continue;
		lang::Pack pack;
		bool ok = lang::parse(text, path, pack);
		Log(pack);
		if (!ok || pack.game != "sb1" || (!pack.edition.empty() && pack.edition != g_edition)) continue;
		int same = Find(pack.code);
		if (same >= 0) g_packs[same] = pack;
		else g_packs.push_back(pack);
	}
}

// Builds the [chars] entries into table_char and the letter -> code map.
void ApplyChars(lang::Pack& pack)
{
	memcpy(table_char, g_original, kTableSize);
	g_encode.clear();
	// Codes the pack's texts use as Windows-1252 letters stay theirs.
	std::set<int> used;
	for (const auto& s : pack.strings)
	{
		size_t i = 0;
		uint32_t cp;
		while (lang::next_codepoint(s.second, i, cp))
		{
			int c = lang::cp1252(cp);
			if (c >= 0) used.insert(c);
		}
	}
	auto code_of = [&](uint32_t letter) -> int {
		auto it = g_encode.find(letter);
		if (it != g_encode.end()) return it->second;
		return lang::cp1252(letter);
	};
	for (const lang::CharSpec& spec : pack.chars)
	{
		int code = lang::cp1252(spec.letter);
		if (code < 0)
		{
			for (int c = 0x80; c < 0x100 && code < 0; c++)
			{
				if (memcmp(g_original + c * kCharBytes, kBlank, kCharBytes) == 0 && used.count(c) == 0)
				{
					code = c;
					used.insert(c);
				}
			}
		}
		if (code < 0x20)
		{
			pack.problems.push_back("no free code for " + lang::utf8(spec.letter));
			continue;
		}
		char entry[kCharBytes];
		if (spec.alias != 0 || (spec.glyph >= 0 && spec.base == 0 && spec.baseGlyph < 0 && spec.accent < 0))
		{
			if (spec.glyph >= 0)
			{
				memcpy(entry, kBlank, kCharBytes);
				entry[0] = (char)spec.glyph;
			}
			else
			{
				int other = code_of(spec.alias);
				if (other < 0)
				{
					pack.problems.push_back("unknown letter in the entry for " + lang::utf8(spec.letter));
					continue;
				}
				memcpy(entry, table_char + other * kCharBytes, kCharBytes);
			}
		}
		else
		{
			int baseRank = spec.baseGlyph;
			if (baseRank < 0)
			{
				int b = code_of(spec.base);
				if (b < 0)
				{
					pack.problems.push_back("unknown base letter in the entry for " + lang::utf8(spec.letter));
					continue;
				}
				baseRank = (unsigned char)table_char[b * kCharBytes];
			}
			entry[0] = (char)baseRank;
			entry[1] = (char)spec.bx;
			entry[2] = (char)spec.by;
			entry[3] = (char)spec.accent;
			entry[4] = (char)spec.ax;
			entry[5] = (char)spec.ay;
		}
		memcpy(table_char + code * kCharBytes, entry, kCharBytes);
		g_encode[spec.letter] = (unsigned char)code;
	}
}

bool Drawable(int code)
{
	return code < 0x20 || memcmp(table_char + code * kCharBytes, kBlank, kCharBytes) != 0;
}

// The game's bytes for a text. A letter the font cannot draw becomes its base
// letter (č -> c) in the player's packs; the built-in packs keep the original
// bytes, as the original drew them (a few letters as the font's blank glyph).
std::string Encode(const std::string& text, lang::Pack* pack, bool builtIn, std::set<uint32_t>* reported)
{
	std::string out;
	size_t i = 0;
	uint32_t cp;
	while (lang::next_codepoint(text, i, cp))
	{
		auto it = g_encode.find(cp);
		if (it != g_encode.end()) { out += (char)it->second; continue; }
		int c = lang::cp1252(cp);
		if (c >= 0 && (builtIn || Drawable(c))) { out += (char)c; continue; }
		uint32_t b = lang::base_letter(cp);
		bool ok = b != 0 || c >= 0;
		out += b != 0 ? (char)b : c >= 0 ? (char)c : '?';
		if (pack != NULL && reported != NULL && reported->insert(cp).second)
		{
			pack->problems.push_back(std::string(ok ? "drawn as its base letter: " : "no way to draw: ") +
									 lang::utf8(cp));
		}
	}
	return out;
}

bool BuiltIn(const lang::Pack& pack)
{
	return pack.source.compare(0, 9, "built-in ") == 0;
}

std::map<unsigned, std::string> EncodeAll(lang::Pack& pack, bool report)
{
	std::map<unsigned, std::string> out;
	std::set<uint32_t> reported;
	for (const auto& s : pack.strings)
	{
		out[s.first] = Encode(s.second, report ? &pack : NULL, BuiltIn(pack), &reported);
	}
	return out;
}
}  // namespace

void PortLanguageInit(const std::string& wanted)
{
	if (!g_saved)
	{
		memcpy(g_original, table_char, kTableSize);
		g_saved = true;
	}
	const KnownBuild* installed = InstalledBuild();
	const char* edition = EditionFromData();
	g_edition = edition != NULL ? edition : installed != NULL ? installed->edition : "blupi";
	g_installed = installed != NULL && g_edition == installed->edition ? installed->code : "";
	LoadPacks();
	g_fallback = Find(g_edition == "eggbert" ? "en" : "us");

	int pick = wanted.empty() ? -1 : Find(wanted);
	if (pick < 0 && !g_installed.empty()) pick = Find(g_installed);
	if (pick < 0) pick = g_fallback;
	if (pick < 0 && !g_packs.empty()) pick = 0;
	char text[160];
	snprintf(text, sizeof(text), "language: edition %s, installed %s, %d packs", g_edition.c_str(),
			 g_installed.empty() ? "unknown" : g_installed.c_str(), (int)g_packs.size());
	blupi::host::log_debug(text);
	PortLanguageSelect(pick);
}

bool PortLanguageSelect(int index)
{
	if (index < 0 || index >= (int)g_packs.size()) return false;
	g_current = index;
	lang::Pack& pack = g_packs[index];
	ApplyChars(pack);
	g_text = EncodeAll(pack, true);
	g_fallbackText.clear();
	if (g_fallback >= 0 && g_fallback != index)
	{
		g_fallbackText = EncodeAll(g_packs[g_fallback], false);
	}
	g_portLanguageVariant = pack.variant;
	Log(pack);
	pack.problems.clear();
	blupi::host::log_info(("language: " + pack.name + " (" + pack.code + ", " + pack.source + ")").c_str());
	return true;
}

const char* PortLanguageString(unsigned id)
{
	auto it = g_text.find(id);
	if (it != g_text.end()) return it->second.c_str();
	it = g_fallbackText.find(id);
	if (it != g_fallbackText.end()) return it->second.c_str();
	return NULL;
}

std::string PortLanguageText(unsigned id, const char* english)
{
	const char* text = PortLanguageString(id);
	return text != NULL ? text : english;
}

int PortLanguageCount()
{
	return (int)g_packs.size();
}

int PortLanguageCurrent()
{
	return g_current;
}

std::string PortLanguageCode(int index)
{
	return index >= 0 && index < (int)g_packs.size() ? g_packs[index].code : std::string();
}

std::string PortLanguageName(int index)
{
	if (index < 0 || index >= (int)g_packs.size()) return std::string();
	return Encode(g_packs[index].name, NULL, false, NULL);   // in the game's code page, for its font
}

const std::string& PortEdition()
{
	return g_edition;
}

bool PortLanguageExport(const std::string& which, const std::string& path)
{
	// "eggbert-de", or "de" (the Blupi pack when both editions have one)
	const BuiltinPack* found = NULL;
	for (const BuiltinPack& b : k_builtinPacks)
	{
		lang::Pack pack;
		if (!lang::parse(std::string((const char*)b.data, b.size), b.file, pack)) continue;
		if (which == pack.edition + "-" + pack.code ||
			(which == pack.code && (found == NULL || pack.edition == "blupi")))
		{
			found = &b;
		}
	}
	if (found == NULL) return false;
	FILE* file = host_fopen(path.c_str(), "wb");
	if (file == NULL) return false;
	bool ok = fwrite(found->data, 1, found->size, file) == found->size;
	fclose(file);
	return ok;
}
