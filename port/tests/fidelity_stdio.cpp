// VC5 _IOERR distinguishes an actual I/O error from a short read at EOF.
// Exercise the shim with real streams and deterministic access-mode errors.
#define BLUPI_HIST_STDIO_IMPLEMENTATION
#include "port_hist_stdio.h"
#include "def.h"
#include "decor.h"
#include "event.h"
#include "platform/vfs.h"
#include <cstdio>
#include <string>
static int failures;
static int inject,closes;
extern "C" {
size_t __real_blupi_hist_fread(void*,size_t,size_t,BlupiFile*);
size_t __wrap_blupi_hist_fread(void* b,size_t s,size_t n,BlupiFile* f)
{if(inject==1) {inject=0;f->_flag|=_IOERR;return 0;}return __real_blupi_hist_fread(b,s,n,f);}
size_t __real_blupi_hist_fwrite(const void*,size_t,size_t,BlupiFile*);
size_t __wrap_blupi_hist_fwrite(const void* b,size_t s,size_t n,BlupiFile* f)
{if(inject==2) {inject=0;f->_flag|=_IOERR;return 0;}return __real_blupi_hist_fwrite(b,s,n,f);}
int __real_blupi_hist_fclose(BlupiFile*);
int __wrap_blupi_hist_fclose(BlupiFile* f) {if(f) ++closes;return __real_blupi_hist_fclose(f);}
}
static void check(bool ok,const char* why) {if(!ok) {printf("FAIL %s\n",why);++failures;}}
int main(int argc,char** argv)
{
    if(argc!=2) return 2;
    blupi::vfs::set_roots(argv[1],argv[1]);
    check(blupi_hist_fopen("absent.blp","rb")==nullptr,"missing file fails open");
    auto f=blupi_hist_fopen("short.blp","wb"); if(!f) return 2;
    check(blupi_hist_fwrite("abc",1,3,f)==3&&f->_flag==0,"complete write no error");
    check(blupi_hist_fclose(f)==0,"successful write close");
    f=blupi_hist_fopen("short.blp","rb"); if(!f) return 2;
    char b[8]={}; check(blupi_hist_fread(b,2,3,f)==1,"short read returns complete element count");
    check(!(f->_flag&_IOERR),"EOF is not VC5 _IOERR");
    check(blupi_hist_fread(b,1,1,f)==0&&!(f->_flag&_IOERR),"repeated EOF preserves clean error flag");
    check(blupi_hist_fwrite("x",1,1,f)==0&&(f->_flag&_IOERR),"write to read-only stream sets _IOERR");
    check(blupi_hist_fread(b,0,1,f)==0&&(f->_flag&_IOERR),"error bit remains sticky");
    blupi_hist_fclose(f);
    f=blupi_hist_fopen("write-only.blp","wb"); if(!f) return 2;
    check(blupi_hist_fread(b,1,1,f)==0&&(f->_flag&_IOERR),"read from write-only stream sets _IOERR");
    blupi_hist_fclose(f);
    check(blupi_hist_fclose(nullptr)==EOF,"null close fails");
    // CopyMission returns TRUE on failure (the historical bError contract).
    closes=0;
    check(CEvent::CopyMission((char*)"missing.blp",(char*)"no-copy.blp")==TRUE&&closes==0,"CopyMission missing source");
    closes=0;
    check(CEvent::CopyMission((char*)"short.blp",(char*)"short.blp/blocked.blp")==TRUE&&closes==1,"CopyMission destination open failure closes source");
    for(int error: {1,2}) {
        inject=error;closes=0;
        check(CEvent::CopyMission((char*)"short.blp",(char*)"failed-copy.blp")==TRUE&&closes==2&&inject==0,
            "CopyMission injected read/write error closes both streams");
    }
    closes=0;
    check(CEvent::CopyMission((char*)"short.blp",(char*)"copy.blp")==FALSE&&closes==2,"CopyMission short final block succeeds and closes both");
    f=blupi_hist_fopen("copy.blp","rb"); if(!f) return 2;
    char copied[4]={};check(blupi_hist_fread(copied,1,4,f)==3&&std::string(copied,3)=="abc","CopyMission exact bytes");blupi_hist_fclose(f);
    printf("stdio error/EOF contracts failures=%d\n",failures);
    return failures?1:0;
}
