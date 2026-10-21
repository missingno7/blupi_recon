// The options screen's music volume, through the historical CSound calls:
// the volume buttons do SetMidiVolume(v) + SuspendMusic() (event.cpp), and
// returning to play does AdaptVolumeMusic() (InitMidiVolume + RestartMusic).
// Checks the resulting synthesizer level against the historical
// midiOutSetVolume table and the historical quirks of volume 0.
// Usage: test_music_volume <game root> <soundfont.sf2> <user dir>

#include "def.h"
#include "sound.h"
#include "port_messages.h"

#include "platform/music.h"
#include "platform/vfs.h"

#include <cmath>
#include <vector>

static int failures = 0;

static void check(bool ok, const char* what)
{
	printf("%s %s\n", ok ? "ok  " : "FAIL", what);
	if (!ok) failures++;
}

// Peak of the first two seconds after a (re)start.
static double render_peak()
{
	std::vector<float> buf(1024 * 2);
	double peak = 0.0;
	for (int i = 0; i < 86; i++)
	{
		std::fill(buf.begin(), buf.end(), 0.0f);
		blupi::music::render_mix(buf.data(), 1024, 44100);
		for (float s : buf) peak = std::fmax(peak, std::fabs(s));
	}
	return peak;
}

static int pop_notify()
{
	PortMessage msg;
	int result = 0;
	while (PortPopMessage(msg))
	{
		if (msg.message == MM_MCINOTIFY) result = (int)msg.wParam;
	}
	return result;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		printf("usage: test_music_volume <game root> <soundfont.sf2> <user dir>\n");
		return 2;
	}
	blupi::vfs::set_roots(argv[1], argv[3]);
	check(blupi::music::load_soundfont(argv[2]), "SoundFont loads");

	CSound sound;
	sound.Create(NULL);
	sound.SetMidiVolume(20);
	check(sound.PlayMusic(NULL, "sound\\music003.blp") == TRUE, "PlayMusic at volume 20");
	double p20 = render_peak();
	check(p20 > 0.01, "volume 20 is audible");

	// Options: volume down to 10. The button suspends (closes) the music.
	sound.SetMidiVolume(10);
	sound.SuspendMusic();
	check(pop_notify() == MCI_NOTIFY_ABORTED, "closing the playing song posts MCI_NOTIFY_ABORTED");
	{
		// A closed song ends its notes with a quick release (a few ms), then silence.
		std::vector<float> tail(4096 * 2, 0.0f);
		blupi::music::render_mix(tail.data(), 4096, 44100);
	}
	check(render_peak() == 0.0, "music is silent while the options are open");
	check(sound.IsPlayingMusic() == TRUE, "the song is remembered for the return to play");

	// Back to play: AdaptVolumeMusic applies the table and restarts the song.
	sound.AdaptVolumeMusic();
	double p10 = render_peak();
	double expected = (double)0xAAAA / 0xFFFF;
	printf("     peak 20: %.4f  peak 10: %.4f  ratio %.4f (table %.4f)\n", p20, p10, p10 / p20, expected);
	check(std::fabs(p10 / p20 - expected) < 1e-3, "volume 10 is the table level 0xAAAA of volume 20");

	sound.SetMidiVolume(15);
	sound.SuspendMusic();
	pop_notify();
	sound.AdaptVolumeMusic();
	double p15 = render_peak();
	check(std::fabs(p15 / p20 - (double)0xF222 / 0xFFFF) < 1e-3, "volume 15 (default) is the table level 0xF222");

	// Volume 0, historical quirk: SuspendMusic skips the close when the volume
	// is already 0 but still forgets the device id, so the song keeps running;
	// AdaptVolumeMusic silences it and RestartMusic opens nothing.
	sound.SetMidiVolume(0);
	sound.SuspendMusic();
	check(pop_notify() == 0, "volume 0: no close, no notification");
	sound.AdaptVolumeMusic();
	check(render_peak() == 0.0, "volume 0 is silent");

	// Raising it again: the forgotten song is not closed (no notification);
	// AdaptVolumeMusic restarts the remembered file at the new level. MCI could
	// have kept the forgotten song running too; the port plays one song (see
	// port/docs/ARCHITECTURE.md section 8).
	sound.SetMidiVolume(20);
	sound.SuspendMusic();
	check(pop_notify() == 0, "leaving volume 0: the forgotten song is not closed");
	sound.AdaptVolumeMusic();
	check(std::fabs(render_peak() - p20) < 1e-6, "volume 20 again restarts the song at full level");

	// A new game start with volume 0 does not open the music at all.
	sound.StopMusic();
	pop_notify();
	sound.SetMidiVolume(0);
	check(sound.PlayMusic(NULL, "sound\\music003.blp") == TRUE && sound.IsPlayingMusic() == FALSE,
		  "PlayMusic at volume 0 succeeds without playing");

	blupi::music::shutdown();
	return failures == 0 ? 0 : 1;
}
