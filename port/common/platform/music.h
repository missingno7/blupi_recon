// MIDI music: a SoundFont synthesizer (TinySoundFont) mixed into the port's
// SDL3 audio stream. Plays one song at a time, like the MCI "sequencer"
// device the original opened per song, and reports the end of the song so
// CSound can deliver the MM_MCINOTIFY the game expects.
//
// Rendering happens on the audio thread; every call here is thread-safe.
#ifndef BLUPI_PLATFORM_MUSIC_H
#define BLUPI_PLATFORM_MUSIC_H

#include <cstddef>
#include <cstdint>
#include <string>

namespace blupi::music
{
// Loads the SoundFont once. Without one, play() fails (no music).
bool load_soundfont(const std::string& path);
bool has_soundfont();

// Starts a Standard MIDI File from the beginning. Returns a non-zero song id,
// or 0 when the data cannot be parsed or no SoundFont is loaded.
unsigned play(const uint8_t* data, size_t size);
// Stops and discards the song if it is still the current one. Returns true if
// it was still playing (the end had not been reached).
bool stop(unsigned id);
// Returns the id of a song that reached its end since the last call, else 0.
unsigned take_finished();

// Output level 0..1, applied linearly (see CSound/InitMidiVolume).
void set_level(float level);

// Mixes `frames` stereo float frames at `rate` into `out` (audio thread).
void render_mix(float* out, int frames, int rate);

void shutdown();
}  // namespace blupi::music

#endif
