// File I/O of the shared historical TUs (compat/include/port_hist_stdio.h):
// BlupiFile streams over the port's file system, with the VC5 error bit.

#define BLUPI_HIST_STDIO_IMPLEMENTATION
#include "port_hist_stdio.h"

#include <new>

extern "C" BlupiFile* blupi_hist_fopen(const char* path, const char* mode)
{
	FILE* file = fopen(path, mode);   // blupi_fopen (port_forced.h)
	if (file == NULL) return NULL;
	BlupiFile* stream = new (std::nothrow) BlupiFile{file, 0};
	if (stream == NULL) fclose(file);
	return stream;
}

extern "C" int blupi_hist_fclose(BlupiFile* stream)
{
	if (stream == NULL) return EOF;
	int result = fclose(stream->file);
	delete stream;
	return result;
}

extern "C" size_t blupi_hist_fread(void* buffer, size_t size, size_t count, BlupiFile* stream)
{
	size_t n = fread(buffer, size, count, stream->file);
	if (ferror(stream->file)) stream->_flag |= _IOERR;
	return n;
}

extern "C" size_t blupi_hist_fwrite(const void* buffer, size_t size, size_t count, BlupiFile* stream)
{
	size_t n = fwrite(buffer, size, count, stream->file);
	if (ferror(stream->file)) stream->_flag |= _IOERR;
	return n;
}
