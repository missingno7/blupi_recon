#include "platform/language.h"

#include <cctype>
#include <cstdlib>

namespace blupi::language
{
namespace
{
// Windows-1252 0x80-0x9F (0: undefined).
const uint16_t k_cp1252_80[32] = {
    0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
    0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E, 0x0178};

// Unaccented letter for U+00C0-U+024F ('.': none), from Unicode's canonical
// decompositions.
const char k_bases[] =
    "AAAAAA.CEEEEIIII.NOOOOO..UUUUY..aaaaaa.ceeeeiiii.nooooo..uuuuy.yAaAaAaCcCcCcCcDd..EeEeEeEeEeGgGgGgGg"
    "Hh..IiIiIiIiI...JjKk.LlLlLl....NnNnNn...OoOoOo..RrRrRrSsSsSsSsTtTt..UuUuUuUuUuUuWwYyYZzZzZz........."
    "........................Oo.............Uu............................AaIiOoUuUuUuUuUu.AaAa....GgKkOo"
    "Oo..j...Gg..NnAa....AaAaEeEeIiIiOoOoRrRrUuUuSsTt..Hh......AaEeOoOoOoOoYy............................";

std::string trim(const std::string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t')) a++;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r')) b--;
    return s.substr(a, b - a);
}

// "A", "#1d" -> letter or glyph rank.
bool letter_or_glyph(const std::string& s, uint32_t& letter, int& glyph)
{
    letter = 0;
    glyph = -1;
    if (s.size() >= 2 && s[0] == '#')
    {
        char* end = nullptr;
        long v = std::strtol(s.c_str() + 1, &end, 16);
        if (*end != 0 || v < 0 || v > 255) return false;
        glyph = (int)v;
        return true;
    }
    size_t i = 0;
    if (!next_codepoint(s, i, letter) || i != s.size()) return false;
    return true;
}

bool offset(const std::string& s, int& x, int& y)
{
    size_t comma = s.find(',');
    if (comma == std::string::npos) return false;
    char* e1 = nullptr;
    char* e2 = nullptr;
    std::string a = trim(s.substr(0, comma)), b = trim(s.substr(comma + 1));
    x = (int)std::strtol(a.c_str(), &e1, 10);
    y = (int)std::strtol(b.c_str(), &e2, 10);
    return !a.empty() && !b.empty() && *e1 == 0 && *e2 == 0 && x >= -128 && x <= 127 && y >= -128 && y <= 127;
}

bool parse_char(const std::string& lhs, const std::string& rhs, CharSpec& spec)
{
    size_t i = 0;
    if (!next_codepoint(lhs, i, spec.letter) || i != lhs.size()) return false;
    size_t plus = rhs.find('+');
    if (plus == std::string::npos)
    {
        // alias or glyph
        return letter_or_glyph(rhs, spec.alias, spec.glyph) && (spec.alias != 0 || spec.glyph >= 0);
    }
    if (!letter_or_glyph(trim(rhs.substr(0, plus)), spec.base, spec.baseGlyph)) return false;
    std::string rest = trim(rhs.substr(plus + 1));
    size_t at = rest.find('@');
    if (at == std::string::npos) return false;
    std::string accent = trim(rest.substr(0, at));
    spec.accent = accent_glyph(accent);
    if (spec.accent < 0)
    {
        uint32_t unused = 0;
        if (!letter_or_glyph(accent, unused, spec.accent) || spec.accent < 0) return false;
    }
    rest = trim(rest.substr(at + 1));
    size_t baseAt = rest.find("base");
    std::string accentOffset = trim(rest.substr(0, baseAt));
    if (!offset(accentOffset, spec.ax, spec.ay)) return false;
    if (baseAt != std::string::npos)
    {
        std::string b = trim(rest.substr(baseAt + 4));
        if (b.empty() || b[0] != '@' || !offset(trim(b.substr(1)), spec.bx, spec.by)) return false;
    }
    return true;
}

// A string value: optional surrounding quotes (for edge spaces), \n and \\ escapes.
std::string unescape(const std::string& raw)
{
    std::string s = raw;
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') s = s.substr(1, s.size() - 2);
    std::string out;
    for (size_t i = 0; i < s.size(); i++)
    {
        if (s[i] == '\\' && i + 1 < s.size())
        {
            char e = s[++i];
            if (e == 'n') out += '\n';
            else if (e == '\\') out += '\\';
            else { out += '\\'; out += e; }
            continue;
        }
        out += s[i];
    }
    return out;
}
}  // namespace

int accent_glyph(const std::string& name)
{
    if (name == "acute") return 0x02;
    if (name == "grave") return 0x03;
    if (name == "tilde") return 0x7E;
    return -1;
}

bool parse(const std::string& text, const std::string& source, Pack& pack)
{
    pack = Pack();
    pack.source = source;
    enum { Header, Chars, Strings } section = Header;
    size_t pos = 0;
    int lineNo = 0;
    if (text.compare(0, 3, "\xEF\xBB\xBF") == 0) pos = 3;   // UTF-8 BOM
    while (pos < text.size())
    {
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) end = text.size();
        std::string line = text.substr(pos, end - pos);
        pos = end + 1;
        lineNo++;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string t = trim(line);
        if (t.empty() || t[0] == '#') continue;   // a string line starts with its id
        if (t[0] == '[')
        {
            if (t == "[chars]") section = Chars;
            else if (t == "[strings]") section = Strings;
            else pack.problems.push_back("line " + std::to_string(lineNo) + ": unknown section " + t);
            continue;
        }
        size_t eq = line.find('=');
        if (eq == std::string::npos)
        {
            pack.problems.push_back("line " + std::to_string(lineNo) + ": no '='");
            continue;
        }
        std::string key = trim(line.substr(0, eq));
        if (section == Header)
        {
            std::string value = trim(line.substr(eq + 1));
            if (key == "name") pack.name = value;
            else if (key == "code") pack.code = value;
            else if (key == "game") pack.game = value;
            else if (key == "edition") pack.edition = value;
            else if (key == "variant") pack.variant = value;
            else pack.problems.push_back("line " + std::to_string(lineNo) + ": unknown key " + key);
        }
        else if (section == Chars)
        {
            CharSpec spec;
            if (parse_char(key, trim(line.substr(eq + 1)), spec)) pack.chars.push_back(spec);
            else pack.problems.push_back("line " + std::to_string(lineNo) + ": bad character entry");
        }
        else
        {
            char* idEnd = nullptr;
            unsigned long id = std::strtoul(key.c_str(), &idEnd, 10);
            if (key.empty() || *idEnd != 0)
            {
                pack.problems.push_back("line " + std::to_string(lineNo) + ": bad string id " + key);
                continue;
            }
            // the text is everything after "= " (or "="), up to the end of the line
            size_t start = eq + 1;
            if (start < line.size() && line[start] == ' ') start++;
            pack.strings[(unsigned)id] = unescape(line.substr(start));
        }
    }
    if (pack.code.empty() || pack.game.empty())
    {
        pack.problems.push_back("missing code or game");
        return false;
    }
    if (pack.name.empty()) pack.name = pack.code;
    if (pack.variant.empty()) pack.variant = pack.code;
    return true;
}

bool next_codepoint(const std::string& text, size_t& i, uint32_t& cp)
{
    if (i >= text.size()) return false;
    unsigned char c = (unsigned char)text[i++];
    if (c < 0x80) { cp = c; return true; }
    int more = c >= 0xF0 ? 3 : c >= 0xE0 ? 2 : c >= 0xC0 ? 1 : -1;
    if (more < 0) { cp = 0xFFFD; return true; }
    cp = c & (0x3F >> more);
    for (int k = 0; k < more; k++)
    {
        if (i >= text.size() || ((unsigned char)text[i] & 0xC0) != 0x80) { cp = 0xFFFD; return true; }
        cp = (cp << 6) | ((unsigned char)text[i++] & 0x3F);
    }
    return true;
}

int cp1252(uint32_t cp)
{
    if (cp < 0x80 || (cp >= 0xA0 && cp <= 0xFF)) return (int)cp;
    for (int k = 0; k < 32; k++)
    {
        if (k_cp1252_80[k] != 0 && k_cp1252_80[k] == cp) return 0x80 + k;
    }
    return -1;
}

uint32_t from_cp1252(unsigned char byte)
{
    if (byte >= 0x80 && byte < 0xA0) return k_cp1252_80[byte - 0x80] ? k_cp1252_80[byte - 0x80] : 0xFFFD;
    return byte;
}

uint32_t base_letter(uint32_t cp)
{
    if (cp < 0xC0 || cp >= 0xC0 + sizeof(k_bases) - 1) return 0;
    char b = k_bases[cp - 0xC0];
    return b == '.' ? 0 : (uint32_t)b;
}

std::string utf8(uint32_t cp)
{
    std::string out;
    if (cp < 0x80) out += (char)cp;
    else if (cp < 0x800) { out += (char)(0xC0 | cp >> 6); out += (char)(0x80 | (cp & 0x3F)); }
    else if (cp < 0x10000)
    {
        out += (char)(0xE0 | cp >> 12);
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    }
    else
    {
        out += (char)(0xF0 | cp >> 18);
        out += (char)(0x80 | ((cp >> 12) & 0x3F));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    }
    return out;
}
}  // namespace blupi::language
