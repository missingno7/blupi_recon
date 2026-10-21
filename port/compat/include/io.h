// Port compatibility header: <io.h> directory enumeration routed through the
// port file system (port/compat/src/vfs.cpp).
#ifndef BLUPI_COMPAT_IO_H
#define BLUPI_COMPAT_IO_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_SUBDIR 0x10

struct _finddata_t
{
	unsigned attrib;
	time_t   time_create;
	time_t   time_access;
	time_t   time_write;
	unsigned long size;
	char     name[260];
};

intptr_t _findfirst(const char* pattern, struct _finddata_t* data);
int      _findnext(intptr_t handle, struct _finddata_t* data);
int      _findclose(intptr_t handle);
int      _access(const char* path, int mode);

#endif
