// Second forced include, for the shared historical TUs only (after
// port_forced.h): their file I/O on a port stream type instead of the C
// runtime's FILE.
//
// The historical code uses fopen, fread, fwrite and fclose, and in
// CEvent::CopyMission tests the stream's error bit directly
// (`file->_flag & _IOERR`): what the VC5 C runtime's ferror macro expanded
// to. That compiles only against a Microsoft C runtime, whose FILE has these
// members; glibc and bionic (where FILE is opaque) have none. BlupiFile has
// exactly what the historical code touches, with the VC5 meaning, and wraps
// the port's file system (blupi_fopen). The port's own TUs keep the real FILE.
#ifndef BLUPI_COMPAT_PORT_HIST_STDIO_H
#define BLUPI_COMPAT_PORT_HIST_STDIO_H

// First the general forced include (whatever order the build passes them in),
// so that its fopen macro is the one replaced below.
#include "port_forced.h"

#include <stdio.h>
#include <time.h>
#include <wchar.h>
#ifdef __cplusplus
#include <cstdio>
#include <cwchar>
#endif

struct BlupiFile
{
	FILE* file;
	int   _flag;     // _IOERR once a read or write failed (VC5 ferror)
};

#define _IOERR 0x0020    // VC5 stdio.h

#ifdef __cplusplus
extern "C" {
#endif
struct BlupiFile* blupi_hist_fopen(const char* path, const char* mode);
int    blupi_hist_fclose(struct BlupiFile* stream);
size_t blupi_hist_fread(void* buffer, size_t size, size_t count, struct BlupiFile* stream);
size_t blupi_hist_fwrite(const void* buffer, size_t size, size_t count, struct BlupiFile* stream);
#ifdef __cplusplus
}
#endif

#ifndef BLUPI_HIST_STDIO_IMPLEMENTATION
#undef fopen
#define FILE    BlupiFile
#define fopen   blupi_hist_fopen
#define fclose  blupi_hist_fclose
#define fread   blupi_hist_fread
#define fwrite  blupi_hist_fwrite
#endif

#endif
