// Software mixer over one SDL3 audio stream. Each buffer behaves like the
// DirectSound secondary buffer the original created per sound channel: a
// single voice with a play position, optional looping, volume and pan in
// hundredths of a decibel (DirectSound units, so CSound keeps its arithmetic).
#ifndef BLUPI_PLATFORM_AUDIO_H
#define BLUPI_PLATFORM_AUDIO_H

#include <cstddef>
#include <cstdint>

namespace blupi::audio
{
bool init(bool enableDevice);
void shutdown();

// PCM as stored in the .blp/.wav data chunk. Returns a handle or -1.
int  create_buffer(const uint8_t* pcm, size_t bytes, int frequency, int bitsPerSample, int channels);
void destroy_buffer(int id);

void play(int id, bool loop);     // keeps the current position, like IDirectSoundBuffer::Play
void stop(int id);                // keeps the current position
void rewind(int id);              // SetCurrentPosition(0)
void set_volume(int id, int hundredthsDb);   // 0 .. -10000
void set_pan(int id, int hundredthsDb);      // -10000 (left) .. 10000 (right)
bool is_playing(int id);
}

#endif
