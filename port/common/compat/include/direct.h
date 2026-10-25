// Port compatibility header: <direct.h> routed through the port file system
// (port/common/platform/vfs.cpp) so user writes never touch the game data.
#ifndef BLUPI_COMPAT_DIRECT_H
#define BLUPI_COMPAT_DIRECT_H

int _mkdir(const char* path);

#endif
