// SB2 adapter, derived from port/games/sb1/game/movie.cpp and
// games/sb2/games/sb2/src/movie.cpp (historical origin).
// Port replacement of games/sb2/src/movie.cpp (historical: Planet Blupi-derived MCI AVI
// playback of CMovie).
//
// AVI playback is NOT IMPLEMENTED. CMovie reports itself disabled, exactly as
// the original did when the MCI AVI driver was unavailable; CEvent::StartMovie
// then skips the movie and continues with the following phase. The movie
// files are not part of the installed data either (they stayed on the CD).

#include "def.h"
#include "movie.h"
#include "platform/host.h"

CMovie::CMovie()
{
	m_bEnable      = FALSE;
	m_wMCIDeviceID = 0;
	m_hwndMovie    = NULL;
	m_fPlaying     = FALSE;
	m_fMovieOpen   = FALSE;
}

CMovie::~CMovie()
{
}

BOOL CMovie::Create()
{
	static bool reported = false;
	if (!reported) { blupi::host::log_info("SB2: AVI movies unavailable; continuing without movie playback."); reported = true; }
	m_bEnable = FALSE;
	return FALSE;
}

BOOL CMovie::GetEnable()
{
	return m_bEnable;
}

BOOL CMovie::IsExist(char *pFilename)
{
	return FALSE;
}

BOOL CMovie::Play(HWND hWnd, RECT rect, char *pFilename)
{
	return FALSE;
}

void CMovie::Stop(HWND hWnd)
{
}
