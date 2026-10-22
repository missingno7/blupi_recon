#include "platform/vfs.h"
#include "platform/host.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace blupi::vfs
{
namespace
{
std::string g_gameRoot;
std::string g_userRoot;
std::string g_languageRoot;

// "data\\World001.blp", "\\User\\a.xch", "c:\\x\\y" -> {"data","World001.blp"} ...
std::vector<std::string> split(const char* gamePath)
{
    std::string p = gamePath ? gamePath : "";
    if (p.size() >= 2 && std::isalpha((unsigned char)p[0]) && p[1] == ':')
        p.erase(0, 2);  // drive letters have no meaning inside the roots
    std::vector<std::string> parts;
    std::string cur;
    auto flush = [&] {
        if (cur.empty() || cur == ".") { cur.clear(); return; }
        if (cur == "..") { if (!parts.empty()) parts.pop_back(); }
        else parts.push_back(cur);
        cur.clear();
    };
    for (char c : p)
    {
        if (c == '\\' || c == '/') flush();
        else cur += c;
    }
    flush();
    return parts;
}

bool iequal(const std::string& a, const std::string& b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
               return std::tolower((unsigned char)x) == std::tolower((unsigned char)y);
           });
}

// Case-insensitive lookup of one component; returns the on-disk spelling.
bool find_entry(const fs::path& dir, const std::string& name, fs::path& out)
{
    std::error_code ec;
    fs::path direct = dir / fs::u8path(name);
    if (fs::exists(direct, ec)) { out = direct; return true; }
    for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec))
    {
        if (iequal(it->path().filename().u8string(), name)) { out = it->path(); return true; }
    }
    return false;
}

bool lookup(const std::string& root, const std::vector<std::string>& parts, fs::path& out)
{
    if (root.empty()) return false;
    fs::path cur = fs::u8path(root);
    for (const std::string& part : parts)
    {
        if (!find_entry(cur, part, cur)) return false;
    }
    out = cur;
    return true;
}

// Write target: reuse existing spellings, keep the historical spelling otherwise.
fs::path user_target(const std::vector<std::string>& parts, bool createParents)
{
    fs::path cur = fs::u8path(g_userRoot);
    for (size_t i = 0; i < parts.size(); i++)
    {
        fs::path next;
        if (!find_entry(cur, parts[i], next)) next = cur / fs::u8path(parts[i]);
        if (createParents && i + 1 < parts.size())
        {
            std::error_code ec;
            fs::create_directories(next, ec);
        }
        cur = next;
    }
    return cur;
}

bool wildcard_match(const char* pat, const char* s)
{
    if (*pat == 0) return *s == 0;
    if (*pat == '*') return wildcard_match(pat + 1, s) || (*s && wildcard_match(pat, s + 1));
    if (*s == 0) return false;
    if (*pat == '?' || std::tolower((unsigned char)*pat) == std::tolower((unsigned char)*s))
        return wildcard_match(pat + 1, s + 1);
    return false;
}
}  // namespace

void set_roots(const std::string& gameRoot, const std::string& userRoot)
{
    g_gameRoot = gameRoot;
    g_userRoot = userRoot;
    std::error_code ec;
    fs::create_directories(fs::u8path(g_userRoot), ec);
}

void set_language_root(const std::string& languageRoot)
{
    g_languageRoot = languageRoot;
}

const std::string& game_root() { return g_gameRoot; }
const std::string& user_root() { return g_userRoot; }
const std::string& language_root() { return g_languageRoot; }

std::string resolve_read(const char* gamePath)
{
    std::vector<std::string> parts = split(gamePath);
    fs::path found;
    if (lookup(g_userRoot, parts, found) || lookup(g_languageRoot, parts, found) ||
        lookup(g_gameRoot, parts, found))
        return found.u8string();
    return std::string();
}

std::string resolve_write(const char* gamePath)
{
    return user_target(split(gamePath), true).u8string();
}

FILE* open(const char* gamePath, const char* mode)
{
    bool write = std::strpbrk(mode, "wa+") != nullptr;
    if (!write)
    {
        std::string path = resolve_read(gamePath);
        return path.empty() ? nullptr : std::fopen(path.c_str(), mode);
    }
    std::vector<std::string> parts = split(gamePath);
    fs::path target = user_target(parts, true);
    std::error_code ec;
    // Appending or updating a file that so far exists only as game data: copy it first.
    if (std::strpbrk(mode, "a+") && !fs::exists(target, ec))
    {
        fs::path original;
        if (lookup(g_languageRoot, parts, original) || lookup(g_gameRoot, parts, original))
            fs::copy_file(original, target, ec);
    }
    return std::fopen(target.u8string().c_str(), mode);
}

bool remove(const char* gamePath)
{
    std::vector<std::string> parts = split(gamePath);
    fs::path found;
    if (!lookup(g_userRoot, parts, found)) return false;  // game data is never deleted
    std::error_code ec;
    return fs::remove(found, ec);
}

bool make_dir(const char* gamePath)
{
    std::error_code ec;
    fs::path target = user_target(split(gamePath), true);
    return fs::create_directory(target, ec);
}

std::vector<std::string> list(const char* gamePattern)
{
    std::vector<std::string> parts = split(gamePattern);
    std::vector<std::string> names;
    if (parts.empty()) return names;
    std::string pattern = parts.back();
    parts.pop_back();
    for (const std::string& root : {g_userRoot, g_languageRoot, g_gameRoot})
    {
        fs::path dir;
        if (!lookup(root, parts, dir)) continue;
        std::error_code ec;
        for (fs::directory_iterator it(dir, ec), end; !ec && it != end; it.increment(ec))
        {
            std::string name = it->path().filename().u8string();
            if (!it->is_regular_file(ec) || !wildcard_match(pattern.c_str(), name.c_str())) continue;
            bool dup = false;
            for (const std::string& n : names) dup = dup || iequal(n, name);
            if (!dup) names.push_back(name);
        }
    }
    return names;
}
}  // namespace blupi::vfs

extern "C" FILE* blupi_fopen(const char* path, const char* mode)
{
    FILE* file = blupi::vfs::open(path, mode);
    if (file == nullptr)
    {
        std::string text = std::string("fopen failed: ") + path + " (" + mode + ")";
        blupi::host::log_debug(text.c_str());
    }
    return file;
}
