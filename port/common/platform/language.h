// Language packs (port/docs/LANGUAGES.md): one UTF-8 text file per language,
// with a header, [chars] (how letters are drawn with the game's font) and
// [strings] (the game's string table). This file parses packs and converts
// text; game/language.cpp applies a pack to the game. No game types here, so
// every game uses it.
#ifndef BLUPI_PLATFORM_LANGUAGE_H
#define BLUPI_PLATFORM_LANGUAGE_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace blupi::language
{
// One [chars] line: how one letter is drawn.
//   "Ç = ç"                     like another letter (alias)
//   "x = #1d"                   a glyph of the font, by its rank
//   "Á = A + acute @ 2,-2"      a base letter's glyph plus an accent glyph at an
//                               offset ("base @ x,y" may follow for the base)
struct CharSpec
{
    uint32_t letter = 0;
    uint32_t alias = 0;      // the letter it is drawn like (alias form)
    int      glyph = -1;     // glyph rank (glyph form)
    uint32_t base = 0;       // composed form: the base letter...
    int      baseGlyph = -1; // ...or the base glyph rank ("#xx")
    int      accent = -1;    // accent glyph rank
    int      ax = 0, ay = 0; // accent offset
    int      bx = 0, by = 0; // base offset
};

struct Pack
{
    std::string name;      // shown in the Language list
    std::string code;      // "de", "us", ...; identifies the pack
    std::string game;      // "sb1"
    std::string edition;   // "blupi", "eggbert"; empty: any edition
    std::string variant;   // language variant of the game's code ("de": German layouts)
    std::string source;    // where it came from ("built-in", a file path)
    std::vector<CharSpec> chars;
    std::map<unsigned, std::string> strings;   // UTF-8
    std::vector<std::string> problems;         // for the log
};

// Accent glyph ranks of the SB1 font, by the names packs use.
int accent_glyph(const std::string& name);   // -1 if unknown

bool parse(const std::string& text, const std::string& source, Pack& pack);

// UTF-8 decoding: the next code point at i (advanced); false at the end. A
// malformed sequence yields U+FFFD.
bool next_codepoint(const std::string& text, size_t& i, uint32_t& cp);
// Windows-1252 byte for a code point, or -1.
int cp1252(uint32_t cp);
// Code point of a Windows-1252 byte.
uint32_t from_cp1252(unsigned char byte);
// The unaccented Latin letter for an accented one (U+00C0-U+024F), or 0.
uint32_t base_letter(uint32_t cp);
std::string utf8(uint32_t cp);
}  // namespace blupi::language

#endif
