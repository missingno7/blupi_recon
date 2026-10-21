// Port compatibility header: the winmm names used by the shared game TUs.
// Joystick queries report no device (joystick support: see port/README.md).
#ifndef BLUPI_COMPAT_MMSYSTEM_H
#define BLUPI_COMPAT_MMSYSTEM_H

#include "windows.h"

typedef UINT MMRESULT;
typedef UINT MCIDEVICEID;

// MCI completion notification (posted by the port's CSound music player).
#define MM_MCINOTIFY            0x03B9
#define MCI_NOTIFY_SUCCESSFUL   0x0001
#define MCI_NOTIFY_SUPERSEDED   0x0002
#define MCI_NOTIFY_ABORTED      0x0004
#define MCI_NOTIFY_FAILURE      0x0008

#define JOYERR_NOERROR    0
#define JOYERR_PARMS      165
#define JOYERR_UNPLUGGED  167
#define JOY_RETURNALL     0x000000FF
#define JOY_BUTTON1       0x0001
#define JOY_BUTTON2       0x0002
#define JOY_BUTTON3       0x0004
#define JOY_BUTTON4       0x0008
#define MAXPNAMELEN       32

typedef struct joyinfoex_tag
{
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwXpos;
	DWORD dwYpos;
	DWORD dwZpos;
	DWORD dwRpos;
	DWORD dwUpos;
	DWORD dwVpos;
	DWORD dwButtons;
	DWORD dwButtonNumber;
	DWORD dwPOV;
	DWORD dwReserved1;
	DWORD dwReserved2;
} JOYINFOEX, *LPJOYINFOEX;

typedef struct tagJOYCAPSA
{
	WORD wMid;
	WORD wPid;
	CHAR szPname[MAXPNAMELEN];
	UINT wXmin, wXmax, wYmin, wYmax, wZmin, wZmax;
	UINT wNumButtons;
	UINT wPeriodMin, wPeriodMax;
	UINT wRmin, wRmax, wUmin, wUmax, wVmin, wVmax;
	UINT wCaps;
	UINT wMaxAxes, wNumAxes, wMaxButtons;
	CHAR szRegKey[32];
	CHAR szOEMVxD[260];
} JOYCAPS, *LPJOYCAPS;

UINT     joyGetNumDevs(void);
MMRESULT joyGetDevCaps(UINT id, LPJOYCAPS caps, UINT size);
MMRESULT joyGetPosEx(UINT id, LPJOYINFOEX info);
DWORD    timeGetTime(void);

#endif
