// Port file system: maps the historical relative paths ("data\\world001.blp",
// "\\User\\x.xch") onto two roots.
//
//   game root  read-only original data (DATA/, IMAGE08/, IMAGE16/, SOUND/), looked
//              up case-insensitively so the original upper-case CD names work.
//   user root  every write lands here (saves, user missions, recorded demos);
//              reads look here first, so a user file shadows the game file.
//
// This keeps assets/ immutable and gives Android/Linux a single place for
// writable data later (SDL_GetPrefPath).
#ifndef BLUPI_PLATFORM_VFS_H
#define BLUPI_PLATFORM_VFS_H

#include <cstdio>
#include <string>
#include <vector>

namespace blupi::vfs
{
    void set_roots(const std::string& gameRoot, const std::string& userRoot);
    const std::string& game_root();
    const std::string& user_root();

    // Existing file for reading: user root first, then game root. Empty if absent.
    std::string resolve_read(const char* gamePath);
    // Destination in the user root; parent directories are created.
    std::string resolve_write(const char* gamePath);

    FILE* open(const char* gamePath, const char* mode);
    bool  remove(const char* gamePath);
    bool  make_dir(const char* gamePath);

    // Names (not paths) of files matching a "dir\\*.ext" pattern in either root,
    // de-duplicated, in directory order of the user root then the game root.
    std::vector<std::string> list(const char* gamePattern);
}

#endif
