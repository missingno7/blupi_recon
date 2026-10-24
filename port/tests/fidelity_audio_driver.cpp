// Compile the real implementation in this TU so its private callback can be
// driven without a public production test API. Do not replace conversion/mixing.
#include "../common/platform/audio.cpp"
#include "fidelity_audio_driver.h"
#include <stdexcept>

std::vector<float> audio_test::render(int frames)
{
    SDL_AudioSpec spec = {SDL_AUDIO_F32, 2, 44100};
    SDL_AudioStream* stream = SDL_CreateAudioStream(&spec, &spec);
    if (!stream) throw std::runtime_error(SDL_GetError());
    const int bytes = frames * 2 * sizeof(float);
    blupi::audio::feed(nullptr, stream, bytes, bytes);
    std::vector<float> samples(frames * 2);
    int got = SDL_GetAudioStreamData(stream, samples.data(), bytes);
    SDL_DestroyAudioStream(stream);
    if (got != bytes) throw std::runtime_error("offline callback returned a short block");
    return samples;
}
