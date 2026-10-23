// Extracted unchanged from SB1's pixmap capture for use by both games.
// No compat/VFS context: dump paths are host paths supplied on the command line.
#include "render/display_capture.h"
namespace blupi::render {
void DisplayCapture::write(const Surface& back, const Palette& pal)
{
	if ( m_log == NULL && m_dumpEvery <= 0 )  return;
	const size_t rowBytes = (size_t)back.width() * (back.bpp() / 8);
	uint64_t hash = 1469598103934665603ULL;
	for ( int y=0 ; y<back.height() ; y++ )
	{
		const uint8_t* row = back.row(y);
		for ( size_t i=0 ; i<rowBytes ; i++ )
		{
			hash ^= row[i];
			hash *= 1099511628211ULL;
		}
	}
	if ( m_log )
	{
		fprintf(m_log, "%ld %d %016llx\n", m_index, back.bpp(), (unsigned long long)hash);
	}
	if ( m_dumpEvery > 0 && m_index % m_dumpEvery == 0 )
	{
		char path[300];
		snprintf(path, sizeof(path), "%s/frame_%06ld.raw", m_dumpDir, m_index);
		FILE* f = fopen(path, "wb");
		if ( f != NULL )
		{
			int header[3] = { back.width(), back.height(), back.bpp() };
			fwrite("BLPF", 1, 4, f);
			fwrite(header, sizeof(int), 3, f);
			if ( back.bpp() == 8 )
			{
				for ( int i=0 ; i<256 ; i++ )
				{
					uint8_t e[4] = { pal[i].r, pal[i].g, pal[i].b, 0 };
					fwrite(e, 1, 4, f);
				}
			}
			for ( int y=0 ; y<back.height() ; y++ )  fwrite(back.row(y), 1, rowBytes, f);
			fclose(f);
		}
	}
	m_index++;
}

void DisplayCapture::configure(FILE* log, const char* dumpDir, long dumpEvery)
{
    m_log = log;
    m_dumpEvery = dumpEvery;
    if (dumpDir != nullptr) snprintf(m_dumpDir, sizeof(m_dumpDir), "%s", dumpDir);
}
} // namespace blupi::render
