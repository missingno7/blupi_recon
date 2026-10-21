// Port replacement of src/sound.cpp (historical: Planet Blupi-derived
// DirectSound + MCI implementation of CSound).
//
// Sound effects: the historical bodies are kept (volume/pan arithmetic in
// DirectSound hundredths of dB, per-Blupi channel stealing, the .blp wave
// header reader); IDirectSoundBuffer calls go to the port mixer
// (port/platform/audio.cpp).
//
// MIDI music: the historical MCI-sequencer flow is kept (open + play with
// MCI_NOTIFY per song, close in SuspendMusic, midiOut volume table); the
// sequencer is the port's SoundFont synthesizer (port/platform/music.cpp).
// Like MCI, the end of a song posts MM_MCINOTIFY(MCI_NOTIFY_SUCCESSFUL) to the
// window, whose handler (game/main.cpp, copied from blupi.cpp) suspends and
// restarts the music; closing a song that is still playing posts
// MCI_NOTIFY_ABORTED. Without a SoundFont, PlayMusic fails like an MCI open
// failure and logs it once.

#include <stdio.h>
#include "def.h"
#include "sound.h"
#include "misc.h"
#include "resource.h"

#include "game/port_game.h"
#include "platform/audio.h"
#include "platform/music.h"
#include "platform/host.h"
#include "platform/vfs.h"

#include <vector>

struct IDirectSound
{
};

struct IDirectSoundBuffer
{
	int id;
};

#define DS_OK ((HRESULT)0)

/////////////////////////////////////////////////////////////////////////////

struct WaveHeader
{
    BYTE        RIFF[4];          // "RIFF"
    DWORD       dwSize;           // Size of data to follow
    BYTE        WAVE[4];          // "WAVE"
    BYTE        fmt_[4];          // "fmt "
    DWORD       dw16;             // 16
    WORD        wOne_0;           // 1
    WORD        wChnls;           // Number of Channels
    DWORD       dwSRate;          // Sample Rate
    DWORD       BytesPerSec;      // Sample Rate
    WORD        wBlkAlign;        // 1
    WORD        BitsPerSample;    // Sample size
    BYTE        DATA[4];          // "DATA"
    DWORD       dwDSize;          // Number of Samples
};

static_assert(sizeof(WaveHeader) == 44, "historical wave header layout");

// Port: the original created an empty DirectSound buffer, then ReadData
// filled it. The mixer needs the samples up front, so both steps happen in
// CreateBufferFromWaveFile; these keep their historical signatures.

BOOL CSound::CreateSoundBuffer(int dwBuf, DWORD dwBufSize, DWORD dwFreq, DWORD dwBitsPerSample, DWORD dwBlkAlign, BOOL bStereo)
{
	return FALSE;
}

BOOL CSound::ReadData(LPDIRECTSOUNDBUFFER lpDSB, FILE* pFile, DWORD dwSize, DWORD dwPos)
{
	return FALSE;
}

// Creates a DirectSound buffer from a wave file.

BOOL CSound::CreateBufferFromWaveFile(int dwBuf, char *pFileName)
{
	// Open the wave file
	FILE* pFile = fopen(pFileName, "rb");
	if ( pFile == NULL ) return FALSE;

	// Read in the wave header
	WaveHeader wavHdr;
	if ( fread(&wavHdr, sizeof(wavHdr), 1, pFile) != 1 )
	{
		fclose(pFile);
		return FALSE;
	}

	// Figure out the size of the data region
	DWORD dwSize = wavHdr.dwDSize;

	// Is this a stereo or mono file?
	BOOL bStereo = wavHdr.wChnls > 1 ? TRUE : FALSE;

	// Read the data for the wave file (the historical ReadData read exactly
	// dwSize bytes after the 44-byte header and failed on a short read).
	std::vector<uint8_t> data(dwSize);
	if ( dwSize == 0 || fread(data.data(), dwSize, 1, pFile) != 1 )
	{
		fclose(pFile);
		return FALSE;
	}
	fclose(pFile);

	int id = blupi::audio::create_buffer(data.data(), dwSize, (int)wavHdr.dwSRate,
										 wavHdr.BitsPerSample, bStereo ? 2 : 1);
	if ( id < 0 )  return FALSE;
	m_lpDSB[dwBuf] = new IDirectSoundBuffer{id};
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

// MCI state of the sequencer device: the song whose MCI_PLAY asked for
// MCI_NOTIFY and has not been answered yet, and the window to notify.
static unsigned	s_notifyDevice = 0;
static HWND		s_notifyWnd = NULL;

// Posts MM_MCINOTIFY(MCI_NOTIFY_SUCCESSFUL) when the song with a pending
// notification reached its end. Called by the host loop and before a close.

void PortSoundPoll()
{
	unsigned	finished = blupi::music::take_finished();

	if ( finished != 0 && finished == s_notifyDevice )
	{
		PostMessage(s_notifyWnd, MM_MCINOTIFY, MCI_NOTIFY_SUCCESSFUL, (LPARAM)finished);
		s_notifyDevice = 0;
	}
}

// Modifie le volume midi.
// Le volume est compris entre 0 et 20 !
// Port: the historical table of midiOutSetVolume levels (low word = left
// channel) drives the synthesizer output level linearly.

void InitMidiVolume(int volume)
{
	static unsigned int table[21] =
	{
		0x00000000,
		0x11111111,
		0x22222222,
		0x33333333,
		0x44444444,
		0x55555555,
		0x66666666,
		0x77777777,
		0x88888888,
		0x99999999,
		0xAAAAAAAA,
		0xBBBBBBBB,
		0xCCCCCCCC,
		0xDDDDDDDD,
		0xEEEEEEEE,
		0xF222F222,
		0xF555F555,
		0xF777F777,
		0xFAAAFAAA,
		0xFDDDFDDD,
		0xFFFFFFFF,
	};

	if ( volume < 0         )  volume = 0;
	if ( volume > MAXVOLUME )  volume = MAXVOLUME;

	blupi::music::set_level((float)(table[volume] & 0xFFFF) / 65535.0f);
}

/////////////////////////////////////////////////////////////////////////////

// Constructeur.

CSound::CSound()
{
	int		i;

	m_bEnable         = FALSE;
	m_bState          = FALSE;
	m_MidiDeviceID    = 0;
	m_MIDIFilename[0] = 0;
	m_audioVolume     = 20;
	m_midiVolume      = 15;
	m_lastMidiVolume  = 0;
	m_nbSuspendSkip   = 0;
	m_lpDS = NULL;

	for ( i=0 ; i<MAXSOUND ; i++ )
	{
		m_lpDSB[i] = NULL;
	}

	for ( i=0 ; i<MAXBLUPI ; i++ )
	{
		m_channelBlupi[i] = -1;
	}
}

// Destructeur.

CSound::~CSound()
{
	int		i;

	if ( m_bEnable )
	{
		InitMidiVolume(15);  // remet un volume moyen !
	}

	for ( i=0 ; i<MAXSOUND ; i++ )
	{
		Flush(i);
	}
	delete m_lpDS;
	m_lpDS = NULL;
}

// Initialisation de DirectSound.

BOOL CSound::Create(HWND hWnd)
{
	m_lpDS    = new IDirectSound;
	m_bEnable = TRUE;
	m_hWnd    = hWnd;
	return TRUE;
}

// Retourne l'état de DirectSound.

BOOL CSound::GetEnable()
{
	return m_bEnable;
}

// Enclenche ou déclenche le son.

void CSound::SetState(BOOL bState)
{
	m_bState = bState;
}

// Gestion des volumes audio (.wav) et midi (.mid).

void CSound::SetAudioVolume(int volume)
{
	m_audioVolume = volume;
}

int CSound::GetAudioVolume()
{
	if ( !m_bEnable )  return 0;
	return m_audioVolume;
}

void CSound::SetMidiVolume(int volume)
{
	m_midiVolume = volume;
}

int CSound::GetMidiVolume()
{
	if ( !m_bEnable )  return 0;
	return m_midiVolume;
}

// Cache tous les ficheirs son (.wav).

void CSound::CacheAll()
{
	int			i;
	char		name[50];

	if ( !m_bEnable )  return;

	for ( i=0 ; i<MAXSOUND ; i++ )
	{
		sprintf(name, "sound\\sound%.3d.blp", i);
		if ( !Cache(i, name) )  break;
	}
}

// Charge un fichier son (.wav).

BOOL CSound::Cache(int channel, char *pFilename)
{
	if ( !m_bEnable )  return FALSE;
	if ( channel < 0 || channel >= MAXSOUND )  return FALSE;

	if ( m_lpDSB[channel] != NULL )
	{
		Flush(channel);
	}

	return CreateBufferFromWaveFile(channel, pFilename);
}

// Décharge un son.

void CSound::Flush(int channel)
{
	if ( !m_bEnable )  return;
	if ( channel < 0 || channel >= MAXSOUND )  return;

	if ( m_lpDSB[channel] != NULL )
	{
		blupi::audio::destroy_buffer(m_lpDSB[channel]->id);
		delete m_lpDSB[channel];
		m_lpDSB[channel]= NULL;
	}
}

BOOL CSound::PlaySoundDS(int channel, int volume, int pan, int playFlag)
{
	if ( !m_bEnable || !m_bState || m_audioVolume == 0 )  return TRUE;

	volume -= (MAXVOLUME-m_audioVolume)*((10000/4)/MAXVOLUME);
	if ( volume <= -10000/4 )  return TRUE;

	if ( channel < 0 || channel >= MAXSOUND )  return FALSE;
	if ( m_lpDSB[channel] == NULL )  return FALSE;

	blupi::audio::set_volume(m_lpDSB[channel]->id, volume);
	blupi::audio::set_pan(m_lpDSB[channel]->id, pan);
	blupi::audio::play(m_lpDSB[channel]->id, playFlag != 0);  // DSBPLAY_LOOPING == 1

	return TRUE;
}

BOOL CSound::PlaySoundDS(int channel, int volume, int pan)
{
	if ( !m_bEnable || !m_bState || m_audioVolume == 0 )  return TRUE;

	volume -= (MAXVOLUME-m_audioVolume)*((10000/4)/MAXVOLUME);
	if ( volume <= -10000/4 )  return TRUE;

	if ( channel < 0 || channel >= MAXSOUND )  return FALSE;
	if ( m_lpDSB[channel] == NULL )  return FALSE;

	blupi::audio::set_volume(m_lpDSB[channel]->id, volume);
	blupi::audio::set_pan(m_lpDSB[channel]->id, pan);

	return TRUE;
}

BOOL CSound::StopSound(int channel)
{
	if ( !m_bEnable || !m_bState || m_audioVolume == 0 )  return TRUE;
	if ( channel < 0 || channel >= MAXSOUND )  return FALSE;
	if ( m_lpDSB[channel] == NULL )  return FALSE;

	blupi::audio::stop(m_lpDSB[channel]->id);
	blupi::audio::rewind(m_lpDSB[channel]->id);
	return TRUE;
}

BOOL CSound::PlayImage(int channel, POINT pos, int rank, int playFlag)
{
	int		stopCh, volumex, volumey, volume, pan;

	if ( rank >= 0 && rank < MAXBLUPI )
	{
		stopCh = m_channelBlupi[rank];
		if ( stopCh >= 0 && m_lpDSB[stopCh] != NULL )
		{
			blupi::audio::stop(m_lpDSB[stopCh]->id);
			blupi::audio::rewind(m_lpDSB[stopCh]->id);
		}
		m_channelBlupi[rank] = channel;
	}

	pan = (int)(((long)pos.x*5000L)/LXIMAGE)-2500L;

	volumex = 0;
	if ( pos.x < 0 )  volumex = (pos.x*2500)/LXIMAGE;
	if ( pos.x > LXIMAGE )  volumex = ((LXIMAGE-pos.x)*2500)/LXIMAGE;
	if ( volumex < -10000 )  volumex = -10000;

	volumey = 0;
	if ( pos.y < 0 )  volumey = (pos.y*2500)/LYIMAGE;
	if ( pos.y > LYIMAGE )  volumey = ((LYIMAGE-pos.y)*2500)/LYIMAGE;
	if ( volumey < -10000 )  volumey = -10000;

	if ( volumex < volumey )  volume = volumex;
	else                      volume = volumey;

	return PlaySoundDS(channel, volume, pan, playFlag);
}

BOOL CSound::Play(int channel, int x, int y)
{
	int	volumex, volumey, volume, pan;

	pan = (int)(((long)x*5000L)/LXIMAGE)-2500L;

	volumex = 0;
	if ( x < 0 )  volumex = (x*2500)/LXIMAGE;
	if ( x > LXIMAGE )  volumex = ((LXIMAGE-x)*2500)/LXIMAGE;
	if ( volumex < -10000 )  volumex = -10000;

	volumey = 0;
	if ( y < 0 )  volumey = (y*2500)/LYIMAGE;
	if ( y > LYIMAGE )  volumey = ((LYIMAGE-y)*2500)/LYIMAGE;
	if ( volumey < -10000 )  volumey = -10000;

	if ( volumex < volumey )  volume = volumex;
	else                      volume = volumey;

	return PlaySoundDS(channel, volume, pan);
}

// Uses MCI to play a MIDI file. The window procedure
// is notified when playback is complete.
// Port: the sequencer is the SoundFont synthesizer; the file is read through
// the port file system (the original prefixed GetCurrentDir()).

BOOL CSound::PlayMusic(HWND hWnd, LPSTR lpszMIDIFilename)
{
	static BOOL		bReported = FALSE;
	FILE*			file;
	std::vector<uint8_t> data;
	uint8_t			buffer[16384];
	size_t			n;
	unsigned		device;

	if ( !m_bEnable )  return TRUE;
	if ( m_midiVolume == 0 )  return TRUE;

	InitMidiVolume(m_midiVolume);
	m_lastMidiVolume = m_midiVolume;

	// Open the device by specifying the device and filename.
	if ( !blupi::music::has_soundfont() )
	{
		if ( !bReported )
		{
			blupi::host::log_info("No SoundFont loaded (--soundfont, BLUPI_SOUNDFONT or soundfont.sf2): playing without music.");
			bReported = TRUE;
		}
		return FALSE;
	}
	file = blupi::vfs::open(lpszMIDIFilename, "rb");
	if ( file == NULL )  return FALSE;
	while ( (n = fread(buffer, 1, sizeof(buffer), file)) > 0 )  data.insert(data.end(), buffer, buffer + n);
	fclose(file);

	device = blupi::music::play(data.data(), data.size());
	if ( device == 0 )
	{
		// Failed to open device. Don't close it; just return error.
		return FALSE;
	}

	// The device opened successfully; get the device ID.
	m_MidiDeviceID = device;

	// Begin playback (MCI_PLAY with MCI_NOTIFY to hWnd).
	// Port: one song plays at a time. Should the game open a second song
	// without closing the first (MCI would play both), the first is replaced.
	s_notifyDevice = device;
	s_notifyWnd    = hWnd;
	{
		char	text[100];
		sprintf(text, "MIDI open+play %s (device %u)\n", lpszMIDIFilename, device);
		OutputDebug(text);
	}

	strcpy(m_MIDIFilename, lpszMIDIFilename);
	return TRUE;
}

// Restart the MIDI player.

BOOL CSound::RestartMusic()
{
	if ( !m_bEnable )  return TRUE;
	if ( m_midiVolume == 0 )  return TRUE;
	if ( m_MIDIFilename[0] == 0 )  return FALSE;

	return PlayMusic(m_hWnd, m_MIDIFilename);
}

// Shuts down the MIDI player.

void CSound::SuspendMusic()
{
	if ( !m_bEnable )  return;

	if ( m_nbSuspendSkip != 0 )
	{
		m_nbSuspendSkip --;
		return;
	}

	if ( m_MidiDeviceID && m_midiVolume != 0 )
	{
		// MCI_CLOSE. A completed play has already posted its notification;
		// a play still running is answered with MCI_NOTIFY_ABORTED.
		char	text[60];
		sprintf(text, "MIDI close (device %u)\n", (unsigned)m_MidiDeviceID);
		OutputDebug(text);
		PortSoundPoll();
		if ( blupi::music::stop(m_MidiDeviceID) && s_notifyDevice == m_MidiDeviceID )
		{
			PostMessage(s_notifyWnd, MM_MCINOTIFY, MCI_NOTIFY_ABORTED, (LPARAM)m_MidiDeviceID);
		}
		if ( s_notifyDevice == m_MidiDeviceID )  s_notifyDevice = 0;
	}
	m_MidiDeviceID = 0;
}

// Shuts down the MIDI player.

void CSound::StopMusic()
{
	SuspendMusic();
	m_MIDIFilename[0] = 0;
}

// Retourne TRUE si une musique est en cours.

BOOL CSound::IsPlayingMusic()
{
	return (m_MIDIFilename[0] != 0);
}

// Adapte le volume de la musique en cours, si nécessaire.

void CSound::AdaptVolumeMusic()
{
	if ( m_midiVolume != m_lastMidiVolume )
	{
		InitMidiVolume(m_midiVolume);
		m_lastMidiVolume = m_midiVolume;
		RestartMusic();
	}
}

// Indique le nombre de suspend à sauter.

void CSound::SetSuspendSkip(int nb)
{
	m_nbSuspendSkip = nb;
}
