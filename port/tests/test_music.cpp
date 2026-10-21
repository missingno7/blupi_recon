// Music player test (no audio device): renders a game song offline through
// the SoundFont synthesizer and checks the MCI-like end-of-song signal.
// Usage: test_music <game root> <soundfont.sf2>

#include "platform/music.h"

#include <cmath>
#include <cstdio>
#include <vector>

static int failures = 0;

static void check(bool ok, const char* what)
{
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) failures++;
}

static std::vector<uint8_t> read_file(const std::string& path)
{
    std::vector<uint8_t> data;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return data;
    uint8_t buf[16384];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) data.insert(data.end(), buf, buf + n);
    std::fclose(f);
    return data;
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::printf("usage: test_music <game root> <soundfont.sf2>\n");
        return 2;
    }
    const int rate = 44100, chunk = 1024;
    std::vector<uint8_t> song = read_file(std::string(argv[1]) + "/SOUND/MUSIC006.BLP");
    check(!song.empty(), "MUSIC006.BLP read");
    check(blupi::music::play(song.data(), song.size()) == 0, "play fails without a SoundFont");
    check(blupi::music::load_soundfont(argv[2]), "SoundFont loads");

    unsigned id = blupi::music::play(song.data(), song.size());
    check(id != 0, "song starts");
    std::vector<float> buf((size_t)chunk * 2);
    double seconds = 0.0, peak = 0.0;
    unsigned finished = 0;
    while (seconds < 600.0 && finished == 0)
    {
        std::fill(buf.begin(), buf.end(), 0.0f);
        blupi::music::render_mix(buf.data(), chunk, rate);
        for (float s : buf) peak = std::fmax(peak, std::fabs(s));
        seconds += (double)chunk / rate;
        finished = blupi::music::take_finished();
    }
    std::printf("     song length %.1f s, peak %.3f\n", seconds, peak);
    check(finished == id, "end of song is reported for this song");
    check(seconds > 10.0 && seconds < 600.0, "song plays for a plausible time");
    check(peak > 0.01, "output is audible");
    check(blupi::music::take_finished() == 0, "end is reported once");
    check(!blupi::music::stop(id), "closing a finished song is not an abort");

    unsigned id2 = blupi::music::play(song.data(), song.size());
    check(id2 != 0 && id2 != id, "a new play gets a new device id");
    std::fill(buf.begin(), buf.end(), 0.0f);
    blupi::music::render_mix(buf.data(), chunk, rate);
    check(blupi::music::stop(id2), "closing a playing song reports it as aborted");
    check(blupi::music::take_finished() == 0, "an aborted song does not report its end");

    blupi::music::set_level(0.0f);
    unsigned id3 = blupi::music::play(song.data(), song.size());
    double silentPeak = 0.0;
    for (int i = 0; i < 100; i++)
    {
        std::fill(buf.begin(), buf.end(), 0.0f);
        blupi::music::render_mix(buf.data(), chunk, rate);
        for (float s : buf) silentPeak = std::fmax(silentPeak, std::fabs(s));
    }
    check(silentPeak == 0.0, "midi volume 0 is silent");
    blupi::music::stop(id3);
    blupi::music::shutdown();
    return failures == 0 ? 0 : 1;
}
