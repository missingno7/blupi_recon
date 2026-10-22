#include "rc_strings.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>

namespace blupi::rc_strings
{
namespace
{
const unsigned char k_rc[] = {
#include "blupi_rc.inc"
    0x00  // terminator: the parser uses strstr
};

// Decodes one RC string literal starting at the opening quote.
std::string parse_literal(const char*& p, const char* end)
{
    std::string out;
    ++p;  // opening quote
    while (p < end)
    {
        char c = *p++;
        if (c == '"')
        {
            if (p < end && *p == '"') { out += '"'; ++p; continue; }  // "" -> "
            break;
        }
        if (c == '\\' && p < end)
        {
            char e = *p++;
            switch (e)
            {
            case 'n': out += '\n'; break;
            case 'r': out += '\r'; break;
            case 't': out += '\t'; break;
            case 'a': out += '\a'; break;
            case '0': out += '\0'; break;
            case 'x':
            {
                int v = 0, n = 0;
                while (p < end && n < 2 && std::isxdigit((unsigned char)*p))
                {
                    char h = *p++;
                    v = v * 16 + (h <= '9' ? h - '0' : (h | 0x20) - 'a' + 10);
                    n++;
                }
                out += (char)v;
                break;
            }
            default: out += e; break;  // \\ and \" and anything else
            }
            continue;
        }
        out += c;
    }
    return out;
}

const std::map<unsigned, std::string>& table()
{
    static std::map<unsigned, std::string> strings;
    static bool parsed = false;
    if (parsed) return strings;
    parsed = true;

    const char* text = reinterpret_cast<const char*>(k_rc);
    const char* end = text + sizeof(k_rc) - 1;
    const char* p = text;
    while (p < end)
    {
        const char* table = std::strstr(p, "STRINGTABLE");
        if (table == nullptr || table >= end) break;
        const char* begin = std::strstr(table, "BEGIN");
        if (begin == nullptr) break;
        p = begin + 5;
        // Lines: <id> "<text>" until END.
        while (p < end)
        {
            while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
            if (end - p >= 3 && std::strncmp(p, "END", 3) == 0) { p += 3; break; }
            if (!std::isdigit((unsigned char)*p))
            {
                while (p < end && *p != '\n') p++;  // comment or unexpected line
                continue;
            }
            unsigned id = (unsigned)std::strtoul(p, const_cast<char**>(&p), 10);
            while (p < end && (*p == ' ' || *p == '\t' || *p == ',')) p++;
            if (p < end && *p == '"') strings[id] = parse_literal(p, end);
            while (p < end && *p != '\n') p++;
        }
    }
    return strings;
}
}  // namespace

const char* string(unsigned id)
{
    const auto& t = table();
    auto it = t.find(id);
    return it == t.end() ? nullptr : it->second.c_str();
}

int count()
{
    return (int)table().size();
}
}  // namespace blupi::rc_strings
