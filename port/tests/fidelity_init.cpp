// Test-only pre-construction allocation poisoning. Historical source and the
// production libraries are unchanged. GNU ld wraps the six DoInit C1 calls.
// A poisoned byte surviving construction is NOT itself proof of a gameplay read.
#include "def.h"
#include "decor.h"
#include "event.h"
#include "pixmap.h"
#include "sound.h"
#include "movie.h"
#include "network.h"
#include "game/member_access.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <vector>
#undef fopen
static const char* selected="none";
static const char* member="all";
static int pattern=0xa5;
static FILE* receipt;
static bool listing;
static void field(const char* cls,const char* name,void* object,void* start,size_t size)
{
    size_t offset=static_cast<unsigned char*>(start)-static_cast<unsigned char*>(object);
    if(listing) fprintf(receipt,"%s %s %zu %zu\n",cls,name,offset,size);
    else if(!strcmp(cls,selected) && !strcmp(name,member)) memset(start,pattern,size);
}
#include "fidelity_init_fields.inc"
template<class T> static void require(T actual,T expected,const char* name)
{
    if(actual!=expected) { fprintf(receipt,"constructor contract failed: %s\n",name); fflush(receipt); exit(9); }
}
// Independent expectations: historical constructors, not a captured baseline.
static void after(CDecor* p) { require(p->*CDecorAccess::m_time,0,"decor time"); require(p->*CDecorAccess::m_bMulti,FALSE,"decor multi"); }
static void after(CEvent* p) { require(p->*CEventAccess::m_index,-1,"event phase index"); require(p->*CEventAccess::m_bDemoPlay,FALSE,"event demo"); }
static void after(CPixmap* p) { require(p->*CPixmapAccess::m_bMouseBack,FALSE,"pixmap mouse backup"); }
static void after(CSound* p) { require(p->*CSoundAccess::m_audioVolume,20,"sound volume"); }
static void after(CMovie* p) { require(p->*CMovieAccess::m_fMovieOpen,FALSE,"movie open"); }
static void after(CNetwork* p) { require(p->*CNetworkAccess::m_bHost,FALSE,"network host"); }
template<class T> static void before(const char* name,T* p)
{
    if(!strcmp(selected,"all") || !strcmp(selected,name)) {
        if(!strcmp(member,"all")) memset(p,pattern,sizeof(T));
        else fields(p);
    }
    fprintf(receipt,"construct %s %zu %02x %s %s\n",name,sizeof(T),pattern,selected,member);
    fflush(receipt);
}
#define WRAP(Class, Symbol) \
extern "C" void __real_##Symbol(Class*); \
extern "C" void __wrap_##Symbol(Class* p) { before(#Class,p); __real_##Symbol(p); after(p); }
WRAP(CDecor, _ZN6CDecorC1Ev)
WRAP(CEvent, _ZN6CEventC1Ev)
WRAP(CPixmap, _ZN7CPixmapC1Ev)
WRAP(CSound, _ZN6CSoundC1Ev)
WRAP(CMovie, _ZN6CMovieC1Ev)
WRAP(CNetwork, _ZN8CNetworkC1Ev)
int BlupiMain(int,char**);
int main(int argc,char** argv)
{
    // receipt class member pattern [ordinary BlupiMain arguments...]
    if(argc<5) return 2;
    receipt=fopen(argv[1],"w"); if(!receipt) return 2;
    selected=argv[2]; member=argv[3]; pattern=(int)strtol(argv[4],nullptr,0);
    if(!strcmp(selected,"savefields")) { savefields(); fclose(receipt); return 0; }
    if(!strcmp(selected,"list")) {
        listing=true;
        // Addresses only; no construction/destruction or member value reads.
        alignas(CDecor) unsigned char d[sizeof(CDecor)]; fields(reinterpret_cast<CDecor*>(d));
        alignas(CEvent) unsigned char e[sizeof(CEvent)]; fields(reinterpret_cast<CEvent*>(e));
        alignas(CPixmap) unsigned char p[sizeof(CPixmap)]; fields(reinterpret_cast<CPixmap*>(p));
        alignas(CSound) unsigned char s[sizeof(CSound)]; fields(reinterpret_cast<CSound*>(s));
        alignas(CMovie) unsigned char m[sizeof(CMovie)]; fields(reinterpret_cast<CMovie*>(m));
        alignas(CNetwork) unsigned char n[sizeof(CNetwork)]; fields(reinterpret_cast<CNetwork*>(n));
        fclose(receipt); return 0;
    }
    std::vector<char*> args{argv[0]};
    for(int i=5;i<argc;++i) args.push_back(argv[i]);
    int rc=BlupiMain((int)args.size(),args.data()); fclose(receipt); return rc;
}
