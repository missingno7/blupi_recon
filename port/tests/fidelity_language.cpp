// Reads independent original RT_STRING expectations supplied by the Python
// driver, then checks canonical, embedded/exported and applied runtime packs.
#include "def.h"
#include "game/port_game.h"
#include "platform/language.h"
#include "platform/vfs.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <map>
#include <cstring>
namespace lang=blupi::language;
static std::string read(const char* path) {std::ifstream f(path,std::ios::binary);std::ostringstream s;s<<f.rdbuf();return s.str();}
static std::string encode(const std::string& text)
{std::string s;size_t i=0;uint32_t cp;while(lang::next_codepoint(text,i,cp)) s+=(char)lang::cp1252(cp);return s;}
int main(int argc,char** argv)
{
    if(argc!=7) return 2;
    blupi::vfs::set_roots(argv[1],argv[2]);
    blupi::vfs::set_language_root(argv[6]);
    std::string which=argv[3],code=which.substr(which.find('-')+1);
    PortLanguageInit(code);
    lang::Pack canonical,embedded;
    std::string exported=std::string(argv[2])+"/exported.txt";
    // set_roots creates write parents only on a write request.
    blupi::vfs::resolve_write("touch.txt");
    if(!PortLanguageExport(which,exported)||!lang::parse(read(argv[4]),argv[4],canonical)||
       !lang::parse(read(exported.c_str()),exported,embedded)||!canonical.problems.empty()||!embedded.problems.empty()) return 1;
    if(canonical.strings!=embedded.strings||!canonical.chars.empty()||!embedded.chars.empty()) return 1;
    std::ifstream f(argv[5],std::ios::binary);
    uint32_t count=0;f.read((char*)&count,4);
    if(count!=canonical.strings.size()||count==0) return 1;
    for(uint32_t i=0;i<count;++i) {
        uint32_t id,n;f.read((char*)&id,4);f.read((char*)&n,4);if(n>65536) return 1;
        std::string expected(n,'\0');f.read(expected.data(),n);
        auto p=canonical.strings.find(id);const char* runtime=PortLanguageString(id);
        if(!f||p==canonical.strings.end()||encode(p->second)!=expected||!runtime||expected!=runtime) {
            printf("FAIL original resource %s id %u\n",argv[3],id);return 1;
        }
    }
    printf("%s: %u original resources equal canonical, exported and runtime strings\n",argv[3],count);
    return 0;
}
