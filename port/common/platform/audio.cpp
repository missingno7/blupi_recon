#include "platform/audio.h"
#include "platform/music.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <mutex>
#include <vector>

namespace blupi::audio
{
namespace
{
constexpr int k_rate = 44100;

struct Voice
{
    bool used = false;
    std::vector<float> frames;  // interleaved stereo at k_rate
    size_t pos = 0;             // frame index
    bool playing = false;
    bool loop = false;
    float gainL = 1.0f, gainR = 1.0f;
    int volume = 0, pan = 0;
};

std::mutex g_lock;
std::vector<Voice> g_voices;
SDL_AudioStream* g_stream = nullptr;
std::vector<float> g_mix;

void update_gains(Voice& v)
{
    float g = std::pow(10.0f, (float)v.volume / 2000.0f);
    float left = g, right = g;
    if (v.pan < 0) right *= std::pow(10.0f, (float)v.pan / 2000.0f);
    if (v.pan > 0) left *= std::pow(10.0f, (float)-v.pan / 2000.0f);
    v.gainL = left;
    v.gainR = right;
}

void SDLCALL feed(void*, SDL_AudioStream* stream, int additional, int)
{
    int frames = additional / (int)(2 * sizeof(float));
    if (frames <= 0) return;
    g_mix.assign((size_t)frames * 2, 0.0f);
    {
        std::lock_guard<std::mutex> guard(g_lock);
        for (Voice& v : g_voices)
        {
            if (!v.used || !v.playing) continue;
            size_t total = v.frames.size() / 2;
            for (int i = 0; i < frames; i++)
            {
                if (v.pos >= total)
                {
                    if (!v.loop || total == 0) { v.playing = false; break; }
                    v.pos = 0;
                }
                g_mix[i * 2 + 0] += v.frames[v.pos * 2 + 0] * v.gainL;
                g_mix[i * 2 + 1] += v.frames[v.pos * 2 + 1] * v.gainR;
                v.pos++;
            }
        }
    }
    blupi::music::render_mix(g_mix.data(), frames, k_rate);
    for (float& s : g_mix) s = s > 1.0f ? 1.0f : (s < -1.0f ? -1.0f : s);
    SDL_PutAudioStreamData(stream, g_mix.data(), frames * 2 * (int)sizeof(float));
}

Voice* voice(int id)
{
    if (id < 0 || id >= (int)g_voices.size() || !g_voices[id].used) return nullptr;
    return &g_voices[id];
}
}  // namespace

bool init(bool enableDevice)
{
    if (!enableDevice) return true;
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        SDL_Log("Audio disabled: %s", SDL_GetError());
        return false;
    }
    SDL_AudioSpec spec = {SDL_AUDIO_F32, 2, k_rate};
    g_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, feed, nullptr);
    if (g_stream == nullptr)
    {
        SDL_Log("Audio disabled: %s", SDL_GetError());
        return false;
    }
    SDL_ResumeAudioStreamDevice(g_stream);
    return true;
}

void shutdown()
{
    if (g_stream) SDL_DestroyAudioStream(g_stream);
    g_stream = nullptr;
    std::lock_guard<std::mutex> guard(g_lock);
    g_voices.clear();
}

int create_buffer(const uint8_t* pcm, size_t bytes, int frequency, int bitsPerSample, int channels)
{
    SDL_AudioSpec src = {bitsPerSample == 8 ? SDL_AUDIO_U8 : SDL_AUDIO_S16LE, channels, frequency};
    SDL_AudioSpec dst = {SDL_AUDIO_F32, 2, k_rate};
    Uint8* out = nullptr;
    int outLen = 0;
    if (bytes == 0 || !SDL_ConvertAudioSamples(&src, pcm, (int)bytes, &dst, &out, &outLen)) return -1;

    std::lock_guard<std::mutex> guard(g_lock);
    int id = 0;
    while (id < (int)g_voices.size() && g_voices[id].used) id++;
    if (id == (int)g_voices.size()) g_voices.emplace_back();
    Voice& v = g_voices[id];
    v = Voice{};
    v.used = true;
    v.frames.assign(reinterpret_cast<float*>(out), reinterpret_cast<float*>(out) + outLen / sizeof(float));
    SDL_free(out);
    return id;
}

void destroy_buffer(int id)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (Voice* v = voice(id)) *v = Voice{};
}

void play(int id, bool loop)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (Voice* v = voice(id))
    {
        if (!v->playing && v->pos >= v->frames.size() / 2) v->pos = 0;  // a finished buffer replays from the start
        v->playing = true;
        v->loop = loop;
    }
}

void stop(int id)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (Voice* v = voice(id)) v->playing = false;
}

void rewind(int id)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (Voice* v = voice(id)) v->pos = 0;
}

void set_volume(int id, int hundredthsDb)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (Voice* v = voice(id)) { v->volume = hundredthsDb; update_gains(*v); }
}

void set_pan(int id, int hundredthsDb)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (Voice* v = voice(id)) { v->pan = hundredthsDb; update_gains(*v); }
}

bool is_playing(int id)
{
    std::lock_guard<std::mutex> guard(g_lock);
    Voice* v = voice(id);
    return v != nullptr && v->playing;
}
}  // namespace blupi::audio
