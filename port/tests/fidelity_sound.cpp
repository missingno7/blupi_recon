// SB2 CSound -> service contracts. Expected arithmetic, channel stealing and
// MIDI index are from games/sb2/src/sound.cpp, not a recorded port golden.
// Services are test doubles: this proves requests, not actual mixer output.
#include "def.h"
#include "sound.h"
#include "port_messages.h"
#include "game/port_game.h"
#include "platform/vfs.h"
#include "platform/music.h"
#include "platform/audio.h"
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
static std::vector<std::string> calls;
static std::vector<uint8_t> midi;
static float level;
static unsigned device,finished;
static bool playing;
void InitMidiVolume(int);
static int failures;
static void check(bool b,const char* why) { if(!b) { printf("FAIL %s\n",why); ++failures; } }
static void expect(std::vector<std::string> wanted,const char* why)
{ if(calls!=wanted) { for(auto& c:calls) printf(" actual %s\n",c.c_str()); } check(calls==wanted,why); calls.clear(); }
namespace blupi::audio {
bool init(bool) {return true;} void shutdown() {}
static int next;
int create_buffer(const uint8_t*,size_t bytes,int hz,int bits,int channels)
{ check(bytes>0&&hz>0&&(bits==8||bits==16)&&(channels==1||channels==2),"PCM header reaches service"); return next++; }
void destroy_buffer(int id) {calls.push_back("destroy "+std::to_string(id));}
void play(int id,bool loop) {calls.push_back("play "+std::to_string(id)+" "+std::to_string(loop));}
void stop(int id) {calls.push_back("stop "+std::to_string(id));}
void rewind(int id) {calls.push_back("rewind "+std::to_string(id));}
void set_volume(int id,int v) {calls.push_back("volume "+std::to_string(id)+" "+std::to_string(v));}
void set_pan(int id,int v) {calls.push_back("pan "+std::to_string(id)+" "+std::to_string(v));}
bool is_playing(int) {return false;}
}
namespace blupi::music {
bool load_soundfont(const std::string&) {return true;} bool has_soundfont() {return true;}
unsigned play(const uint8_t* data,size_t size) {midi.assign(data,data+size);playing=true;return ++device;}
bool stop(unsigned id) {bool was=playing&&id==device;playing=false;return was;}
unsigned take_finished() {unsigned id=finished;finished=0;return id;}
void set_level(float v) {level=v;}
void render_mix(float*,int,int) {} void shutdown() {}
}
static int notify()
{ PortMessage msg; int result=0; while(PortPopMessage(msg)) if(msg.message==MM_MCINOTIFY) result=(int)msg.wParam; return result; }
static std::vector<uint8_t> read(const char* path)
{ std::vector<uint8_t> out; FILE* f=blupi::vfs::open(path,"rb"); if(!f) return out;
  uint8_t b[4096];size_t n;while((n=fread(b,1,sizeof(b),f))) out.insert(out.end(),b,b+n);fclose(f);return out; }
int main(int argc,char** argv)
{
    if(argc!=3) return 2;
    blupi::vfs::set_roots(argv[1],argv[2]); PortLanguageInit("");
    CSound s; check(s.Create(nullptr),"Create"); s.SetState(TRUE); s.SetAudioVolume(20);
    const unsigned table[]={0,0x1111,0x2222,0x3333,0x4444,0x5555,0x6666,0x7777,0x8888,0x9999,
        0xaaaa,0xbbbb,0xcccc,0xdddd,0xeeee,0xf222,0xf555,0xf777,0xfaaa,0xfddd,0xffff};
    for(int v=-1;v<=21;++v) {
        InitMidiVolume(v); int clamped=v<0?0:v>20?20:v;
        check(std::fabs(level-table[clamped]/65535.0f)<1e-7,"all historical MIDI table entries and clamps");
    }
    check(s.Cache(0,(char*)"sound/sound000.blp")&&s.Cache(1,(char*)"sound/sound001.blp"),"cache two original PCM effects");
    check(s.PlayImage(0,{320,240},0,0),"center request");
    expect({"volume 0 0","pan 0 0","play 0 0"},"center full volume");
    s.PlayImage(1,{-64,480},0,1);
    expect({"stop 0","rewind 0","volume 1 -250","pan 1 -3000","play 1 1"},"actor channel stealing precedes offscreen volume/pan/loop");
    s.SetAudioVolume(10); s.PlayImage(0,{640,528},-1,0);
    expect({"volume 0 -1500","pan 0 2500","play 0 0"},"Y attenuation -250 plus master -1250");
    s.Play(0,0,0); expect({"volume 0 -1250","pan 0 -2500"},"adjust-only overload never starts playback");
    s.StopSound(1); expect({"stop 1","rewind 1"},"stop then rewind");
    s.SetAudioVolume(20); s.PlayImage(0,{-640,0},-1,0); expect({},"-2500 cutoff makes no service calls");
    s.SetState(FALSE); s.PlaySoundDS(0,0,0,0); expect({},"disabled effects make no service calls"); s.SetState(TRUE);
    check(!s.PlaySoundDS(-1,0,0,0)&&!s.PlaySoundDS(MAXSOUND,0,0,0)&&!s.PlaySoundDS(2,0,0,0),"invalid/uncached channels fail");
    s.Flush(0); expect({"destroy 0"},"flush releases buffer");
    check(!s.PlaySoundDS(0,0,0,0),"flushed channel fails");
    s.SetCDAudio(FALSE); s.SetMidiVolume(20);
    check(s.PlayMusic(nullptr,4),"integer MIDI id 4 opens");
    auto original=read("sound/music003.blp");
    check(!original.empty()&&midi==original,"ID 4 delivers original music003 bytes");
    check(level==1.0f&&s.IsPlayingMusic(),"full level and remembered song");
    s.SetSuspendSkip(1); s.SuspendMusic(); check(playing&&notify()==0,"suspend skip leaves device playing");
    s.SuspendMusic(); check(!playing&&notify()==MCI_NOTIFY_ABORTED&&s.IsPlayingMusic(),"close aborts but remembers id");
    check(s.RestartMusic()&&midi==original,"restart reopens remembered integer id");
    playing=false;finished=device; PortSoundPoll(); check(notify()==MCI_NOTIFY_SUCCESSFUL,"finished device notifies success");
    s.SuspendMusic(); check(notify()==0,"completed device does not notify aborted");
    s.SetMidiVolume(10); s.AdaptVolumeMusic(); check(std::fabs(level-0xaaaa/65535.0f)<1e-7,"historical MIDI volume table");
    s.SetMidiVolume(0); s.SuspendMusic(); check(playing&&notify()==0,"historical zero-volume suspend skips close");
    s.AdaptVolumeMusic(); check(level==0,"zero volume silences service");
    s.StopMusic(); check(!s.IsPlayingMusic(),"stop forgets integer id");
    s.SetMidiVolume(20); check(!s.RestartMusic(),"no remembered song fails restart");
    s.SetCDAudio(TRUE); check(!s.PlayMusic(nullptr,4),"CD audio explicitly unavailable");
    s.SetCDAudio(FALSE); check(!s.PlayMusic(nullptr,999),"missing MIDI fails");
    printf("SB2 %s sound/MIDI request contracts, failures=%d\n",PortEdition().c_str(),failures);
    return failures?1:0;
}
