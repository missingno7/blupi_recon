// Real game CSound + real offline SDL mixer. Only MIDI is a silent test double.
// References are src/sound.cpp and games/sb2/src/sound.cpp, not service goldens.
#include "def.h"
#include "sound.h"
#include "platform/audio.h"
#include "platform/music.h"
#include "platform/vfs.h"
#include "game/port_game.h"
#include "../games/sb1/game/member_access.h"
#include "fidelity_audio_driver.h"
#undef rand
#undef srand
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <vector>

#define SOUND_FIELDS(X) X(m_bEnable) X(m_bState) X(m_lpDS) X(m_lpDSB) X(m_channelBlupi) \
    X(m_MidiDeviceID) X(m_audioVolume) X(m_midiVolume) X(m_lastMidiVolume) X(m_nbSuspendSkip)
BLUPI_MEMBER_ACCESS(SoundAccess, CSound, SOUND_FIELDS)
#if FIDELITY_AUDIO_SB2
#define SB2_FIELDS(X) X(m_bCDAudio) X(m_music)
BLUPI_MEMBER_ACCESS(Sound2Access, CSound, SB2_FIELDS)
#endif
static std::vector<float> levels;
namespace blupi::music {
bool load_soundfont(const std::string&) {return false;} bool has_soundfont() {return false;}
unsigned play(const uint8_t*,size_t) {return 0;} bool stop(unsigned) {return false;}
unsigned take_finished() {return 0;} void set_level(float x) {levels.push_back(x);}
void render_mix(float*,int,int) {} void shutdown() {}
}
namespace {
int failures,checks;
constexpr int defaultMidi=FIDELITY_AUDIO_SB2?10:15;
void check(bool ok,const char* why) {
    ++checks; if(!ok) {++failures;std::printf("FAIL %s\n",why);}
}
void samples(const std::vector<float>& out,const std::vector<double>& want,const char* why) {
    bool ok=out.size()==want.size(); double worst=0;
    for(size_t i=0;i<std::min(out.size(),want.size());++i) {
        ok=ok&&std::isfinite(out[i]); worst=std::max(worst,std::fabs(out[i]-want[i]));
    }
    if(!ok||worst>2e-7) std::printf("  sample error %.9g\n",worst);
    check(ok&&worst<=2e-7,why);
}
void put(std::vector<uint8_t>& b,int at,unsigned x,int n) {
    for(int i=0;i<n;++i) b[at+i]=(x>>(8*i))&255;
}
std::vector<uint8_t> wave() {
    std::vector<uint8_t> b(44+16);
    std::copy_n("RIFF",4,b.begin());put(b,4,b.size()-8,4);
    std::copy_n("WAVEfmt ",8,b.begin()+8);put(b,16,16,4);put(b,20,1,2);
    put(b,22,1,2);put(b,24,44100,4);put(b,28,88200,4);put(b,32,2,2);put(b,34,16,2);
    std::copy_n("data",4,b.begin()+36);put(b,40,16,4);
    for(int i=0;i<8;++i) put(b,44+i*2,(i+1)*2048,2);
    return b;
}
std::string root;
void write(const std::vector<uint8_t>& b) {
    std::ofstream f(root+"/fixture.wav",std::ios::binary|std::ios::trunc);
    f.write((const char*)b.data(),b.size()); check(bool(f),"fixture written to test user root");
}
bool cache(CSound& s,const std::vector<uint8_t>& b) {
    write(b); return s.Cache(0,(char*)"fixture.wav");
}
void constructor() {
    levels.clear();
    {
        CSound s;
        check(!(s.*SoundAccess::m_bEnable)&&!(s.*SoundAccess::m_bState)&&!(s.*SoundAccess::m_lpDS),"constructor disabled with null device");
        check((s.*SoundAccess::m_MidiDeviceID)==0&&(s.*SoundAccess::m_audioVolume)==20&&
              (s.*SoundAccess::m_midiVolume)==defaultMidi&&(s.*SoundAccess::m_lastMidiVolume)==0&&
              (s.*SoundAccess::m_nbSuspendSkip)==0,"historical constructor volumes/MIDI counters");
        for(auto p:s.*SoundAccess::m_lpDSB) check(p==nullptr,"all constructed buffers null");
        for(auto c:s.*SoundAccess::m_channelBlupi) check(c==-1,"all constructed actor channels unassigned");
#if FIDELITY_AUDIO_SB2
        check((s.*Sound2Access::m_bCDAudio)&&(s.*Sound2Access::m_music)==0,"SB2 constructor CD default and no remembered song");
#endif
        check(!s.Cache(0,(char*)"fixture.wav"),"disabled Cache fails");
        check(s.PlaySoundDS(-1,0,0,0),"disabled playback is successful no-op");
        check(s.GetAudioVolume()==0&&s.GetMidiVolume()==0,"disabled volume getters");
    }
    check(levels.empty(),"disabled destructor does not reset MIDI level");
}
void loading_and_lifecycle() {
    levels.clear();
    {
        CSound s; check(s.Create(nullptr)&&s.GetEnable(),"offline CSound Create enables logical service");
        check((s.*SoundAccess::m_lpDS)!=nullptr,"Create owns logical device");
        check(s.GetAudioVolume()==20&&s.GetMidiVolume()==defaultMidi,"enabled getters expose defaults");
        check(!s.Cache(-1,(char*)"fixture.wav")&&!s.Cache(MAXSOUND,(char*)"fixture.wav"),"invalid Cache channel failure");
        check(!s.Cache(0,(char*)"missing.wav"),"file-open failure propagates");
        auto b=wave();check(cache(s,b),"fixed historical 44-byte header loads");
        s.SetState(TRUE);check(s.PlaySoundDS(0,0,0,0),"valid buffer starts");
        samples(audio_test::render(2),{.0625,.0625,.125,.125},"WAV payload samples reach real mixer");
        s.StopSound(0);samples(audio_test::render(2),{0,0,0,0},"CSound StopSound silences");
        s.PlaySoundDS(0,0,0,0);samples(audio_test::render(1),{.0625,.0625},"CSound StopSound rewinds");
        s.StopSound(0);s.SetAudioVolume(10);s.PlayImage(0,{640,528},-1,0);
        samples(audio_test::render(1),{.000625,.0625*std::pow(10.0,-1500/2000.0)},
                "real mixer combines historical offscreen/master dB and right pan");
        s.Play(0,320,240);
        double adjusted=.125*std::pow(10.0,-1250/2000.0);
        samples(audio_test::render(1),{adjusted,adjusted},"adjust-only game call updates gains without restarting cursor");
        s.StopSound(0);s.SetAudioVolume(20);
        write(b);check(s.Cache(1,(char*)"fixture.wav"),"second game channel loads");
        s.PlayImage(0,{320,240},7,1);audio_test::render(2);
        s.PlayImage(1,{320,240},7,0);
        samples(audio_test::render(1),{.0625,.0625},"actor steal stops old loop and starts new voice");
        s.PlayImage(1,{320,240},7,0);
        samples(audio_test::render(1),{.0625,.0625},"same actor and effect restart through stop/rewind");
        s.PlayImage(0,{320,240},8,0);
        samples(audio_test::render(1),{.1875,.1875},"stolen channel restarts from rewind and adds to other actor");
        s.Flush(0);s.Flush(1);samples(audio_test::render(1),{0,0},"Flush destroys active voices");
        check(!s.PlaySoundDS(0,0,0,0),"flushed slot fails Play");
        for(size_t n=0;n<44;++n) {
            check(!cache(s,std::vector<uint8_t>(b.begin(),b.begin()+n)),"short 44-byte fread propagates failure");
            check(!(s.*SoundAccess::m_lpDSB)[0],"header failure leaves no port voice");
        }
        for(size_t n=44;n<b.size();++n)
            check(!cache(s,std::vector<uint8_t>(b.begin(),b.begin()+n)),"short ReadData payload propagates failure");
        auto bad=b;put(bad,40,0,4);check(!cache(s,bad),"zero secondary buffer size fails");
        for(unsigned bytes:{1u,2u,3u,0x10000000u,0xffffffffu}) {
            bad=b;put(bad,40,bytes,4);check(!cache(s,bad),"DirectSound secondary buffer size bounds reject before allocation");
        }
        for(int bits:{0,12,24,32}) {
            bad=b;put(bad,34,bits,2);check(!cache(s,bad),"invalid PCM bits failure reaches Cache caller");
        }
        bad=b;put(bad,24,0,4);check(!cache(s,bad),"invalid PCM frequency failure reaches Cache caller");
        for(int align:{0,1,3,4}) {
            bad=b;put(bad,32,align,2);check(!cache(s,bad),"inconsistent PCM block alignment fails");
        }
        bad=b;put(bad,40,15,4);check(!cache(s,bad),"partial PCM data frame fails");
        // The original does not parse RIFF chunks or validate these fields:
        // CreateSoundBuffer forces PCM and recalculates average byte rate.
        for(int offset:{0,4,8,12,16,20,28,36}) {
            bad=b;put(bad,offset,0xffffffff,offset==20?2:4);
            check(cache(s,bad),"unused WAV tags/sizes/encoding/byte-rate remain ignored like historical loader");
            s.PlaySoundDS(0,0,0,0);samples(audio_test::render(1),{.0625,.0625},"ignored metadata does not move PCM offset");
        }
        bad=b;put(bad,22,0,2);check(cache(s,bad),"historical channel count zero maps to mono");
        bad=b;put(bad,22,3,2);put(bad,32,4,2);
        check(cache(s,bad),"historical channel counts above one map to stereo");
        s.PlaySoundDS(0,0,0,0);samples(audio_test::render(1),{.0625,.125},"mapped stereo preserves left/right PCM order");
        bad=b;bad.insert(bad.end(),10,255);check(cache(s,bad),"extra file bytes ignored after dwDSize");
        s.PlaySoundDS(0,0,0,0);audio_test::render(9);
        samples(audio_test::render(1),{0,0},"trailing bytes never play");
        check(cache(s,b),"restore before failed replacement");s.PlaySoundDS(0,0,0,1);
        bad=b;bad.resize(45);check(!cache(s,bad),"Cache replacement flushes old buffer then reports short-read failure");
        samples(audio_test::render(1),{0,0},"failed replacement cannot keep old looping sound");
        check(cache(s,b),"restore active buffer before destruction");s.PlaySoundDS(0,0,0,1);
    }
    samples(audio_test::render(2),{0,0,0,0},"enabled destructor releases active mixer buffers");
    check(levels.size()==1&&std::fabs(levels[0]-0xf222/65535.0f)<1e-7,"enabled destructor restores historical MIDI level 15 once");
}
void originals(const std::string& data) {
    CSound s;s.Create(nullptr);
    int count=0;
    for(auto& p:std::filesystem::directory_iterator(data+"/SOUND")) {
        std::ifstream f(p.path(),std::ios::binary);char head[12]={};f.read(head,sizeof(head));
        if(std::string(head,4)!="RIFF"||std::string(head+8,4)!="WAVE") continue;
        std::string name="sound/"+p.path().filename().string();
        check(s.Cache(0,const_cast<char*>(name.c_str())),"original SOUND WAV loads through real CSound and SDL");++count;
    }
    check(count>0,"original SOUND WAV coverage nonempty");
    std::printf("original effects loaded: %d\n",count);
}
}
int main(int argc,char** argv) {
    if(argc!=3) return 2;
    root=argv[2];std::filesystem::create_directories(root);
    blupi::vfs::set_roots(argv[1],root);PortLanguageInit("");blupi::audio::init(false);
    constructor();loading_and_lifecycle();originals(argv[1]);blupi::audio::shutdown();
    std::printf("real CSound/offline mixer: %d checks, %d failures\n",checks,failures);
    return failures?1:0;
}
