// Independent references: PCM integer normalization, decibel amplitude equation,
// and DirectSound secondary-buffer cursor/loop contracts. See w-audio/REPORT.md.
// No expected samples come from SDL conversion or from a previous mixer run.
#include "platform/audio.h"
#include "fidelity_audio_driver.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <climits>
#include <vector>

namespace blupi::music { void render_mix(float*, int, int) {} }
namespace {
int failures, checks;
void check(bool ok, const char* why) {
    ++checks; if (!ok) { ++failures; std::printf("FAIL %s\n", why); }
}
// Four float operations plus powf rounding: 2e-7 absolute is < 0.007 of
// a signed-16 PCM LSB. Silence and raw power-of-two PCM are checked exactly.
void samples(const std::vector<float>& actual, const std::vector<double>& want,
             const char* why, double tolerance = 2e-7) {
    double worst = 0;
    bool ok = actual.size() == want.size();
    for (size_t i=0; i<std::min(actual.size(),want.size()); ++i) {
        if (!std::isfinite(actual[i])) ok=false;
        worst=std::max(worst,std::fabs(actual[i]-want[i]));
    }
    if (!ok || worst>tolerance) std::printf("  max error %.9g tolerance %.9g\n",worst,tolerance);
    check(ok && worst<=tolerance,why);
}
void gain_samples(const std::vector<float>& actual,const std::vector<double>& want) {
    bool ok=actual.size()==want.size();
    for(size_t i=0;i<std::min(actual.size(),want.size());++i)
        ok=ok&&std::isfinite(actual[i])&&
            std::fabs(actual[i]-want[i])<=std::fabs(want[i])*1e-6+1e-18;
    // Relative tolerance keeps compounded -100 dB volume/-100 dB pan audible
    // to the assertion. 1e-6 permits float pow/multiply rounding, not a PCM LSB.
    check(ok,"PCM/dB/pan against integer and analytic reference (relative 1e-6)");
}
std::vector<uint8_t> encode(const std::vector<int>& pcm, int bits) {
    std::vector<uint8_t> b;
    for (int x:pcm) {
        b.push_back(x & 255);
        if (bits==16) b.push_back((x >> 8) & 255);
    }
    return b;
}
int buffer(const std::vector<int>& pcm, int bits=16, int channels=1, int rate=44100) {
    auto b=encode(pcm,bits);
    int id=blupi::audio::create_buffer(b.data(),b.size(),rate,bits,channels);
    check(id>=0,"valid PCM creates real SDL buffer"); return id;
}
std::vector<double> expected(const std::vector<int>& pcm, int bits, int channels,
                             int volume=0,int pan=0) {
    std::vector<double> out;
    for(size_t i=0;i<pcm.size();i+=channels) for(int ch=0;ch<2;++ch) {
        double x=pcm[i+(channels==1?0:ch)];
        x=bits==8?(x-128)/128.0:x/32768.0;
        // Compose attenuation in dB, independently of the mixer's gain products.
        int attenuation=volume-(ch==0?std::max(pan,0):std::max(-pan,0));
        out.push_back(x*std::exp(std::log(10.0)*attenuation/2000.0));
    }
    return out;
}
void gains() {
    for(int bits:{8,16}) for(int channels:{1,2}) {
        std::vector<int> p=bits==8?std::vector<int>{0,1,64,127,128,129,192,254,255,128}:
                                        std::vector<int>{-32768,-32767,-16384,-1,0,1,16384,32766,32767,0};
        int id=buffer(p,bits,channels);
        for(int volume:{0,-1,-125,-600,-1250,-2499,-5000,-10000})
            for(int pan:{-10000,-2500,-1,0,1,2500,10000}) {
                blupi::audio::rewind(id); blupi::audio::set_volume(id,volume);
                blupi::audio::set_pan(id,pan); blupi::audio::play(id,false);
                gain_samples(audio_test::render(p.size()/channels),expected(p,bits,channels,volume,pan));
            }
        blupi::audio::destroy_buffer(id);
    }
}
void lifecycle() {
    using namespace blupi::audio;
    int id=buffer({4096,8192,12288,16384});
    samples(audio_test::render(2),{0,0,0,0},"created buffer is stopped",0);
    play(id,false); samples(audio_test::render(1),{.125,.125},"starts at cursor zero",0);
    play(id,true); samples(audio_test::render(1),{.25,.25},"Play while playing preserves cursor",0);
    stop(id); check(!is_playing(id),"Stop clears playing state");
    samples(audio_test::render(3),std::vector<double>(6),"Stop produces silence without advancing",0);
    play(id,false); samples(audio_test::render(1),{.375,.375},"Stop/Play resumes next sample",0);
    rewind(id); samples(audio_test::render(1),{.125,.125},"rewind while playing takes effect",0);
    stop(id); rewind(id); play(id,true);
    samples(audio_test::render(10),{.125,.125,.25,.25,.375,.375,.5,.5,.125,.125,.25,.25,
                                  .375,.375,.5,.5,.125,.125,.25,.25},"multiple loop wraps",0);
    play(id,false); samples(audio_test::render(2),{.375,.375,.5,.5},"new Play flags supersede loop",0);
    check(!is_playing(id),"one-shot stops exactly at buffer end");
    play(id,false); samples(audio_test::render(1),{.125,.125},"completed buffer restarts",0);
    stop(id);rewind(id);play(id,true);
    samples(audio_test::render(4),{.125,.125,.25,.25,.375,.375,.5,.5},"loop completes exactly one callback block",0);
    play(id,false);
    samples(audio_test::render(1),{.125,.125},"loop cursor wraps before replacing Play flags at boundary",0);
    rewind(id);play(id,false);
    samples(audio_test::render(1),{.125,.125},"explicit rewind starts the boundary fixture again",0);
    set_volume(id,-2000);set_pan(id,2000);
    set_volume(id,1);set_volume(id,-10001);set_pan(id,-10001);set_pan(id,10001);
    samples(audio_test::render(1),{.0025,.025},"invalid dB/pan requests preserve last valid attenuation");
    destroy_buffer(id); check(!is_playing(id),"destroy clears playing voice");
    play(id,true); stop(-1); rewind(INT_MAX); set_volume(-1,0); set_pan(INT_MAX,0); destroy_buffer(-1);
    samples(audio_test::render(3),std::vector<double>(6),"destroyed/invalid handles stay silent",0);
    id=buffer({8192}); play(id,true);
    samples(audio_test::render(2),{.25,.25,.25,.25},"reused slot resets cursor gains and loop",0);
    shutdown(); check(!is_playing(id),"shutdown invalidates buffers");
    samples(audio_test::render(2),std::vector<double>(4),"shutdown removes all voices",0);
    init(false);
    int a=buffer({24576,-24576}), b=buffer({24576,-24576}), c=buffer({-8192,8192});
    play(a,false);play(b,false);play(c,false);
    samples(audio_test::render(3),{1,1,-1,-1,0,0},"voices add before output saturation",0);
    destroy_buffer(a);destroy_buffer(b);destroy_buffer(c);
}
void formats() {
    // Every rate/depth in the read-only SOUND inventory (927 WAVs); stereo
    // adds channel-isolation coverage although the inventory contains mono only.
    const double pi=std::acos(-1.0);
    for(int rate:{11025,22050}) for(int bits:{8,16}) for(int channels:{1,2}) {
        std::vector<int> p;
        for(int n=0;n<rate/4;++n) for(int ch=0;ch<channels;++ch) {
            double x=.4*std::sin(2*pi*(ch==0?100:200)*n/rate);
            p.push_back(bits==8?128+(int)std::lround(x*128):(int)std::lround(x*32768));
        }
        int id=buffer(p,bits,channels,rate); blupi::audio::play(id,false);
        int count=(int)(p.size()/channels)*44100/rate;
        auto out=audio_test::render(count-1);
        check(blupi::audio::is_playing(id),"converted duration has not ended one frame early");
        auto last=audio_test::render(1);
        out.insert(out.end(),last.begin(),last.end());
        check(!blupi::audio::is_playing(id),"converted duration stops on exact expected final frame");
        auto tail=audio_test::render(8);
        out.insert(out.end(),tail.begin(),tail.end());
        double worst=0;
        // Exclude 64 output frames at either end: DirectSound specifies rate,
        // not a resampling kernel or edge padding. Compare analytic low-band
        // sine in the interior; <=1 U8 or <=2 S16 quantization steps.
        for(int n=64;n<count-64;++n) for(int ch=0;ch<2;++ch) {
            double want=.4*std::sin(2*pi*((channels==1||ch==0)?100:200)*n/44100);
            worst=std::max(worst,std::fabs(out[n*2+ch]-want));
        }
        double tolerance=bits==8?1.0/128:2.0/32768;
        std::printf("resample %d Hz %d bit %d ch max_error=%.9g bound=%.9g\n",rate,bits,channels,worst,tolerance);
        check(worst<=tolerance,"rate conversion preserves independently generated sine");
        check(!blupi::audio::is_playing(id),"converted duration ends at expected frame count");
        samples(std::vector<float>(out.end()-16,out.end()),std::vector<double>(16),"no samples after converted duration",0);
        blupi::audio::destroy_buffer(id);
    }
}
void malformed() {
    using namespace blupi::audio;
    uint8_t p[16]={};
    for(int bits:{0,1,12,24,32}) {
        int id=create_buffer(p,sizeof(p),22050,bits,1);
        check(id<0,"unsupported PCM bit depth fails, never reinterpreted as S16"); destroy_buffer(id);
    }
    for(int ch:{-1,0,3,8}) {
        int id=create_buffer(p,sizeof(p),22050,8,ch);
        check(id<0,"non-mono/stereo PCM fails"); destroy_buffer(id);
    }
    for(int rate:{-1,0,1,99,100001}) {
        int id=create_buffer(p,sizeof(p),rate,8,1);
        check(id<0,"invalid sample rate fails"); destroy_buffer(id);
    }
    for(int bytes:{1,2,3,5}) {
        int id=create_buffer(p,bytes,22050,16,2);
        check(id<0,"partial stereo PCM frame fails"); destroy_buffer(id);
    }
    check(create_buffer(p,0,22050,8,1)<0,"empty PCM fails");
    // These must fail before SDL can read the deliberately unavailable data.
    check(create_buffer(nullptr,16,22050,8,1)<0,"null PCM fails");
    check(create_buffer(p,(size_t)INT_MAX+1,22050,8,1)<0,"SDL int-length overflow fails before conversion");
}
}
int main() {
    check(blupi::audio::init(false),"device-free init");
    gains(); lifecycle(); formats(); malformed(); blupi::audio::shutdown();
    std::printf("real offline mixer: %d checks, %d failures\n",checks,failures);
    return failures?1:0;
}
