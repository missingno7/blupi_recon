#include "platform/music.h"

#define TSF_IMPLEMENTATION
#include "third_party/tinysoundfont/tsf.h"
#define TML_IMPLEMENTATION
#include "third_party/tinysoundfont/tml.h"

#include <mutex>
#include <vector>

namespace blupi::music
{
namespace
{
// Fixed output calibration of the synthesizer "device" (the 1998 game played
// through whatever MIDI synth Windows had, at that device's level). Not a user
// setting: the game's own music volume option drives set_level() through the
// historical midiOutSetVolume table. -6 dB keeps GeneralUser GS at full option
// volume below clipping (peak 0.87 on MUSIC003).
constexpr float k_gainDb = -6.0f;

std::mutex   g_lock;
tsf*         g_font = nullptr;
int          g_rate = 0;
tml_message* g_song = nullptr;
tml_message* g_cursor = nullptr;
double       g_msec = 0.0;
unsigned     g_length = 0;     // time of the last event, ms
unsigned     g_id = 0;         // current song, 0 = none
bool         g_playing = false;
unsigned     g_nextId = 0;
unsigned     g_finished = 0;
float        g_level = 1.0f;
std::vector<float> g_block;

void reset_channels()
{
    tsf_reset(g_font);
    for (int ch = 0; ch < 16; ch++)
    {
        tsf_channel_set_presetnumber(g_font, ch, 0, ch == 9);
    }
}

void release_song()
{
    if (g_song) tml_free(g_song);
    g_song = nullptr;
    g_cursor = nullptr;
    g_playing = false;
}

void dispatch(const tml_message* m)
{
    switch (m->type)
    {
    case TML_PROGRAM_CHANGE: tsf_channel_set_presetnumber(g_font, m->channel, m->program, m->channel == 9); break;
    case TML_NOTE_ON:        tsf_channel_note_on(g_font, m->channel, m->key, m->velocity / 127.0f); break;
    case TML_NOTE_OFF:       tsf_channel_note_off(g_font, m->channel, m->key); break;
    case TML_PITCH_BEND:     tsf_channel_set_pitchwheel(g_font, m->channel, m->pitch_bend); break;
    case TML_CONTROL_CHANGE: tsf_channel_midi_control(g_font, m->channel, m->control, m->control_value); break;
    default: break;
    }
}
}  // namespace

bool load_soundfont(const std::string& path)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (g_font) return true;
    g_font = tsf_load_filename(path.c_str());
    return g_font != nullptr;
}

bool has_soundfont()
{
    std::lock_guard<std::mutex> guard(g_lock);
    return g_font != nullptr;
}

unsigned play(const uint8_t* data, size_t size)
{
    tml_message* song = tml_load_memory(data, (int)size);
    std::lock_guard<std::mutex> guard(g_lock);
    if (g_font == nullptr || song == nullptr)
    {
        if (song) tml_free(song);
        return 0;
    }
    release_song();
    reset_channels();
    g_song = song;
    g_cursor = song;
    g_msec = 0.0;
    g_length = 0;
    tml_get_info(song, nullptr, nullptr, nullptr, nullptr, &g_length);
    g_playing = true;
    g_id = ++g_nextId;
    if (g_id == 0) g_id = ++g_nextId;
    return g_id;
}

bool stop(unsigned id)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (id == 0 || id != g_id) return false;
    bool wasPlaying = g_playing;
    release_song();
    if (g_font) reset_channels();
    g_id = 0;
    if (g_finished == id) g_finished = 0;
    return wasPlaying;
}

unsigned take_finished()
{
    std::lock_guard<std::mutex> guard(g_lock);
    unsigned id = g_finished;
    g_finished = 0;
    return id;
}

void set_level(float level)
{
    std::lock_guard<std::mutex> guard(g_lock);
    g_level = level < 0.0f ? 0.0f : (level > 1.0f ? 1.0f : level);
}

void render_mix(float* out, int frames, int rate)
{
    std::lock_guard<std::mutex> guard(g_lock);
    if (g_font == nullptr || (!g_playing && tsf_active_voice_count(g_font) == 0)) return;
    if (rate != g_rate)
    {
        tsf_set_output(g_font, TSF_STEREO_INTERLEAVED, rate, k_gainDb);
        g_rate = rate;
    }
    g_block.resize((size_t)TSF_RENDER_EFFECTSAMPLEBLOCK * 2);
    for (int done = 0; done < frames; done += TSF_RENDER_EFFECTSAMPLEBLOCK)
    {
        int n = frames - done < TSF_RENDER_EFFECTSAMPLEBLOCK ? frames - done : TSF_RENDER_EFFECTSAMPLEBLOCK;
        if (g_playing)
        {
            g_msec += n * 1000.0 / rate;
            while (g_cursor && g_msec >= g_cursor->time)
            {
                dispatch(g_cursor);
                g_cursor = g_cursor->next;
            }
            // End of the sequence: the MCI sequencer completed the play command here.
            if (g_cursor == nullptr && g_msec >= g_length)
            {
                g_playing = false;
                g_finished = g_id;
            }
        }
        tsf_render_float(g_font, g_block.data(), n, 0);
        for (int i = 0; i < n * 2; i++) out[(size_t)done * 2 + i] += g_block[i] * g_level;
    }
}

void shutdown()
{
    std::lock_guard<std::mutex> guard(g_lock);
    release_song();
    if (g_font) tsf_close(g_font);
    g_font = nullptr;
    g_id = 0;
    g_rate = 0;
}
}  // namespace blupi::music
