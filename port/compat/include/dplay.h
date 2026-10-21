// Port compatibility header: DirectPlay *names only*.
// Multiplayer is not implemented by the port yet (see port/README.md); the
// port's CNetwork (port/game/network.cpp) enumerates no providers.
#ifndef BLUPI_COMPAT_DPLAY_H
#define BLUPI_COMPAT_DPLAY_H

#include "windows.h"

typedef DWORD DPID, *LPDPID;

struct IDirectPlay2;
typedef struct IDirectPlay2* LPDIRECTPLAY2;

struct DPSESSIONDESC2;
typedef struct DPSESSIONDESC2* LPDPSESSIONDESC2;

#define DPID_ALLPLAYERS     0
#define DPSEND_GUARANTEED   0x00000001

#endif
