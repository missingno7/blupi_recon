// Port replacement of src/misc.cpp (historical: Planet Blupi-derived utility
// TU). InitRandom/Random/ConvLongToPos/AddCDPath keep their historical bodies
// (rand/srand resolve to the Microsoft LIBC generator via port_forced.h).
// The Windows-resource and path helpers are re-implemented:
//   LoadString   -> the current language pack, else the edition's English
//                   one (language.cpp; every built-in pack has every id)
//   ChangeSprite -> SDL system cursors (only for MouseType 2/3; the default
//                   MouseType 1 draws the cursor sprite in CPixmap)
//   AddUserPath  -> path left relative; the port file system writes it to
//                   the user root instead of "c:\Speedy Blupi\".

#include <stdio.h>
#include <stdlib.h>
#include "def.h"
#include "misc.h"

#include "game/port_game.h"
#include "platform/host.h"
#include "platform/vfs.h"


// Variables globales

HINSTANCE		g_hInstance;
int				g_lastSprite = 0;
extern BOOL		g_bFullScreen;	// FALSE si mode de test
extern int		g_mouseType;
extern char		g_CDPath[MAX_PATH];



// Initialise HInstance.

void InitHInstance(HINSTANCE hInstance)
{
	g_hInstance = hInstance;
}


// Affiche un message de debug.

void OutputDebug(char *pMessage)
{
	blupi::host::log_debug(pMessage);
}


// Charge un texte dans une ressource.

void LoadString(UINT nID, char *pBuffer, int lgBuffer)
{
	const char*	pText;

	if ( lgBuffer <= 0 )  return;
	pText = PortLanguageString(nID);
	if ( pText == NULL )
	{
		pBuffer[0] = 0;
		return;
	}
	strncpy(pBuffer, pText, lgBuffer-1);
	pBuffer[lgBuffer-1] = 0;
}

// Change le lutin de la souris.

void ChangeSprite(int sprite)
{
	using blupi::host::Cursor;
	Cursor cursor = Cursor::Arrow;

	if ( g_mouseType != MOUSETYPEGRA && g_lastSprite != sprite )
	{
		if ( sprite == SPRITE_ARROW )   cursor = Cursor::Arrow;
		if ( sprite == SPRITE_POINTER ) cursor = Cursor::Pointer;
		if ( sprite == SPRITE_ARROWU )  cursor = Cursor::SizeNS;
		if ( sprite == SPRITE_ARROWD )  cursor = Cursor::SizeNS;
		if ( sprite == SPRITE_ARROWL )  cursor = Cursor::SizeWE;
		if ( sprite == SPRITE_ARROWR )  cursor = Cursor::SizeWE;
		if ( sprite == SPRITE_ARROWUL ) cursor = Cursor::SizeNWSE;
		if ( sprite == SPRITE_ARROWUR ) cursor = Cursor::SizeNESW;
		if ( sprite == SPRITE_ARROWDL ) cursor = Cursor::SizeNESW;
		if ( sprite == SPRITE_ARROWDR ) cursor = Cursor::SizeNWSE;
		if ( sprite == SPRITE_WAIT )    cursor = Cursor::Wait;
		if ( sprite == SPRITE_EMPTY )   cursor = Cursor::Hidden;
		blupi::host::set_system_cursor(cursor);
		g_lastSprite = sprite;
	}
}


// Conversion de la position de la souris.

POINT ConvLongToPos(LPARAM lParam)
{
	POINT	pos;

	pos.x = LOWORD(lParam);  // horizontal position of cursor
	pos.y = HIWORD(lParam);  // vertical position of cursor

	return pos;
}


// Réinitialise le générateur aléatoire.

void InitRandom()
{
	srand(1);
}

// Retourne un nombre aléatoire compris entre
// deux bornes (inclues).

int Random(int min, int max)
{
	long	n;

	n = rand();
	n = min+(n%(max-min+1));

	return (int)n;
}


// Retourne le nom de dossier en cours.
// Port: game paths are relative to the game root; nothing to prepend.

void GetCurrentDir(char *pName, int lg)
{
	if ( lg > 0 )  pName[0] = 0;
}


// Ajoute le chemin permettant de lire un fichier
// sur le CD-Rom.

void AddCDPath(char *pFilename)
{
	char	temp[MAX_PATH];
	BOOL	bDaniel = FALSE;

	if ( g_CDPath[0] == 0 )  return;

	if ( !bDaniel &&
		 (strstr(pFilename, "data\\")    == pFilename ||
		  strstr(pFilename, "image\\")   == pFilename ||
		  strstr(pFilename, "image08\\") == pFilename ||
		  strstr(pFilename, "image16\\") == pFilename ||
		  strstr(pFilename, "movie\\")   == pFilename) )
	{
		strcpy(temp, g_CDPath);
		strcat(temp, "..\\");
		strcat(temp, pFilename);
	}
	else
	{
		strcpy(temp, g_CDPath);
		strcat(temp, pFilename);
	}

	strcpy(pFilename, temp);
}

// Ajoute le chemin permettant de lire un fichier
// utilisateur.
// Port: writes already land in the user root (platform/vfs), reads fall back
// to the game data, so the historical relative name is kept as is.

void AddUserPath(char *pFilename)
{
}


void TraceErrorDD(HRESULT hErr, char *sFile, int nLine)
{
	char	err[300];

	sprintf(err, "DirectDraw-equivalent error %d in file %s at line %d\n", (int)hErr, sFile, nLine);
	OutputDebug(err);
}

void TraceErrorDS(HRESULT hErr, char *sFile, int nLine)
{
	char	err[300];

	sprintf(err, "DirectSound-equivalent error %d in file %s at line %d\n", (int)hErr, sFile, nLine);
	OutputDebug(err);
}
