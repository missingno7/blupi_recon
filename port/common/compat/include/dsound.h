// Port compatibility header: DirectSound *names only*.
// The historical CSound declaration (src/sound.h) keeps these member types;
// the port's CSound (port/game/sound.cpp) uses them as opaque handles.
#ifndef BLUPI_COMPAT_DSOUND_H
#define BLUPI_COMPAT_DSOUND_H

#include "windows.h"

struct IDirectSound;
struct IDirectSoundBuffer;
typedef struct IDirectSound*       LPDIRECTSOUND;
typedef struct IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;

#endif
