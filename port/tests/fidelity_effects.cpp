// Link-time observation only: the production game library is unchanged.
// Build is the independent historical reference for EndOfTick. GNU ld --wrap
// observes calls across TUs; calls resolved within their defining TU are not
// intercepted (the mixer boundary below still observes CSound's delivered calls).
#include "def.h"
#include "decor.h"
#include "sound.h"
#include "game/member_access.h"
#include <cstdio>
#include <cstdlib>
#include <vector>

#undef fopen
static FILE* logFile;
static long draw;
#define PARTS(X) X(m_time) X(m_decorPhase) X(m_decorAction) X(m_blupiSec) X(m_blupiPos)
BLUPI_MEMBER_ACCESS(Observe, CDecor, PARTS)
static CDecor* active;
static void snapshot(const char* kind)
{
    if (!logFile) return;
    fprintf(logFile, "%ld %s", draw, kind);
    if (active) fprintf(logFile, " time=%d shake=%d/%d sec=%d actor=%d,%d",
        active->*Observe::m_time, active->*Observe::m_decorAction,
        active->*Observe::m_decorPhase, active->*Observe::m_blupiSec,
        (active->*Observe::m_blupiPos).x, (active->*Observe::m_blupiPos).y);
}
static void begin(CDecor* d) { active=d; snapshot("begin"); fprintf(logFile,"\n"); }
static void end() { snapshot("end"); fprintf(logFile,"\n"); active=nullptr; ++draw; }
extern "C" {
void __real__ZN6CDecor5BuildE7tagRECT8tagPOINT(CDecor*, RECT, POINT);
void __wrap__ZN6CDecor5BuildE7tagRECT8tagPOINT(CDecor* d, RECT r, POINT p)
{ begin(d); __real__ZN6CDecor5BuildE7tagRECT8tagPOINT(d,r,p); end(); }
void __real__Z9EndOfTickR6CDecor(CDecor&);
void __wrap__Z9EndOfTickR6CDecor(CDecor& d)
{ begin(&d); __real__Z9EndOfTickR6CDecor(d); end(); }
BOOL __real__ZN6CSound9PlayImageEi8tagPOINTii(CSound*,int,POINT,int,int);
BOOL __wrap__ZN6CSound9PlayImageEi8tagPOINTii(CSound* s,int ch,POINT p,int rank,int flag)
{ snapshot("PlayImage"); fprintf(logFile," %d %d %d %d %d\n",ch,p.x,p.y,rank,flag); return __real__ZN6CSound9PlayImageEi8tagPOINTii(s,ch,p,rank,flag); }
BOOL __real__ZN6CSound9StopSoundEi(CSound*,int);
BOOL __wrap__ZN6CSound9StopSoundEi(CSound* s,int ch)
{ snapshot("StopSound"); fprintf(logFile," %d\n",ch); return __real__ZN6CSound9StopSoundEi(s,ch); }
BOOL __real__ZN6CDecor11ObjectStartE8tagPOINTiii(CDecor*,POINT,int,int,int);
BOOL __wrap__ZN6CDecor11ObjectStartE8tagPOINTiii(CDecor* d,POINT p,int type,int speed,int flag)
{ snapshot("ObjectStart"); fprintf(logFile," %d %d %d %d %d\n",p.x,p.y,type,speed,flag); return __real__ZN6CDecor11ObjectStartE8tagPOINTiii(d,p,type,speed,flag); }
void __real__ZN6CDecor12NetPlaySoundEs8tagPOINT(CDecor*,short,POINT);
void __wrap__ZN6CDecor12NetPlaySoundEs8tagPOINT(CDecor* d,short ch,POINT p)
{ snapshot("NetPlaySound"); fprintf(logFile," %d %d %d\n",ch,p.x,p.y); __real__ZN6CDecor12NetPlaySoundEs8tagPOINT(d,ch,p); }
#if FIDELITY_SB2
BOOL __real__ZN6CSound9PlayMusicEP11BlupiHWND__i(CSound*,HWND,int);
BOOL __wrap__ZN6CSound9PlayMusicEP11BlupiHWND__i(CSound* s,HWND w,int music)
{ snapshot("PlayMusic"); fprintf(logFile," %d\n",music); return __real__ZN6CSound9PlayMusicEP11BlupiHWND__i(s,w,music); }
#endif
void __real__ZN6CSound12SuspendMusicEv(CSound*);
void __wrap__ZN6CSound12SuspendMusicEv(CSound* s)
{ snapshot("SuspendMusic"); fprintf(logFile,"\n"); __real__ZN6CSound12SuspendMusicEv(s); }
BOOL __real__ZN6CSound12RestartMusicEv(CSound*);
BOOL __wrap__ZN6CSound12RestartMusicEv(CSound* s)
{ snapshot("RestartMusic"); fprintf(logFile,"\n"); return __real__ZN6CSound12RestartMusicEv(s); }
void __real__ZN6CSound9StopMusicEv(CSound*);
void __wrap__ZN6CSound9StopMusicEv(CSound* s)
{ snapshot("StopMusic"); fprintf(logFile,"\n"); __real__ZN6CSound9StopMusicEv(s); }
}

// The offline backend records the exact delivered channel, dB, pan and loop
// sequence. It intentionally does not claim waveform or device timing parity.
namespace blupi::audio {
bool init(bool) { return true; } void shutdown() {}
static int next;
int create_buffer(const uint8_t*, size_t bytes,int hz,int bits,int channels)
{ fprintf(logFile,"buffer %d %zu %d %d %d\n",next,bytes,hz,bits,channels); return next++; }
void destroy_buffer(int id) { fprintf(logFile,"destroy %d\n",id); }
void play(int id,bool loop) { snapshot("play"); fprintf(logFile," %d %d\n",id,loop); }
void stop(int id) { snapshot("stop"); fprintf(logFile," %d\n",id); }
void rewind(int id) { snapshot("rewind"); fprintf(logFile," %d\n",id); }
void set_volume(int id,int db) { snapshot("volume"); fprintf(logFile," %d %d\n",id,db); }
void set_pan(int id,int db) { snapshot("pan"); fprintf(logFile," %d %d\n",id,db); }
bool is_playing(int) { return false; }
}
int BlupiMain(int,char**);
int main(int argc,char** argv)
{
    if(argc<2) return 2;
    logFile=fopen(argv[1],"w"); if(!logFile) return 2;
    std::vector<char*> args{argv[0]};
    for(int i=2;i<argc;++i) args.push_back(argv[i]);
    int rc=BlupiMain((int)args.size(),args.data());
    fclose(logFile); return rc;
}
