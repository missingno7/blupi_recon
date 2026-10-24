// Independent pixel placement from historical MouseRectSprite/pixtables and
// hotspot data. No current-frame golden; original element.blp is read-only.
#include <filesystem>
#include "def.h"
#include "pixmap.h"
#include "game/port_game.h"
#include "game/member_access.h"
#include "platform/host.h"
#include "platform/vfs.h"
#include "render/bmp.h"
#include <cstdio>
#include <vector>
#include <string>
#include <cstdlib>
extern short table_icon_element[];
#define PARTS(X) X(m_mouseHotSpot) X(m_mouseBackPos) X(m_bMouseBack)
BLUPI_MEMBER_ACCESS(Mouse, CPixmap, PARTS)
static int failures;
static void check(bool ok,const char* message) { if(!ok) { printf("FAIL %s\n",message); ++failures; } }
static uint32_t color(blupi::render::Rgb c,int depth)
{
    if(depth==16) { unsigned r=c.r>>3,g=c.g>>2,b=c.b>>3;
        c={(uint8_t)((r<<3)|(r>>2)),(uint8_t)((g<<2)|(g>>4)),(uint8_t)((b<<3)|(b>>2))}; }
    return 0xff000000u | (c.r<<16) | (c.g<<8) | c.b;
}
static std::vector<uint32_t> capture()
{ std::vector<uint32_t> result; int w=0,h=0; check(PortPixmapCopyScreen(result,w,h)&&w==640&&h==480,"capture 640x480"); return result; }
int main(int argc,char** argv)
{
    if(argc!=3) return 2;
    std::error_code made; std::filesystem::create_directories(argv[2],made);  // review captures go to its root
    blupi::vfs::set_roots(argv[1],argv[2]);
    blupi::host::Options options; options.headless=true;
    if(!blupi::host::init("cursor contract",640,480,options)) return 2;
    FILE* file=blupi::vfs::open("image08/element.blp","rb"); if(!file) return 2;
    std::vector<uint8_t> bytes; uint8_t block[8192]; size_t n;
    while((n=fread(block,1,sizeof(block),file))) bytes.insert(bytes.end(),block,block+n);
    fclose(file); blupi::render::Bitmap atlas;
    if(!blupi::render::decode_bmp(bytes.data(),bytes.size(),atlas)) return 2;
    // First four historical entries; SB2's remaining values are explicitly
    // modeled original .data neighbors, not invented usable hotspots.
    const int hotspots[24]={FIDELITY_SB2?25:95,0,30,30,20,15,30,30,
        1954047348,1886151214,0,1735745898,1818373733,112,1953789282,808480367,
        1886151214,0,1819310181,1818373743,112,1835363429,779382373,7367778};
    int cases=0;
    for(int depth: {8,16}) {
        CPixmap p;
#if FIDELITY_SB2
        check(p.Create(nullptr,{640,480},TRUE,depth==16,depth==16,MOUSETYPEGRA),"Create");
#else
        check(p.Create(nullptr,{640,480},TRUE,depth==16,MOUSETYPEGRA),"Create");
#endif
        PortPixmapCaptureBegin(true);
        check(p.Cache(CHPIXMAP_CURSOR,(char*)"image08/element.blp",{atlas.width,atlas.height},{60,60},TRUE),"cache original cursor atlas");
        std::vector<blupi::render::Command> loaded;
        PortPixmapCaptureEnd(loaded);
        // Palette realization is a separate renderer contract. Use the
        // realized palette for this placement/backup test, including the
        // historical reserved black/white entries at either end.
        blupi::render::Palette palette={};
        for(auto& command:loaded) if(command.kind==blupi::render::Command::Palette)
            for(int i=0;i<256;++i) palette[i]=command.palette[i];
        p.SetTransparent(CHPIXMAP_CURSOR,RGB(0,0,255));
        blupi::render::Surface background(640,480,depth);
        std::vector<uint32_t> baseline(640*480);
        for(int y=0;y<480;++y) for(int x=0;x<640;++x) {
            int index=(x/7+y/11)%256; auto c=depth==8?palette[index]:atlas.palette[index];
            background.set_pixel(x,y,depth==8?index:blupi::render::rgb_to_565(c));
            baseline[y*640+x]=color(c,depth);
        }
        PortPixmapSubmit({blupi::render::make_upload(blupi::render::kBackBuffer,background)});
        p.Display(); check(capture()==baseline,"background fixture");
        for(int sprite=SPRITE_ARROW;sprite<=SPRITE_WAIT;++sprite) {
            int hx=hotspots[sprite*2],hy=hotspots[sprite*2+1];
            if(!FIDELITY_SB2 && sprite>=4) {hx=1701867625;hy=1769414770;}
            for(POINT pos: {POINT{320,240},POINT{330,247},POINT{40,80},POINT{0,0},POINT{639,479}}) {
                p.MouseBackClear(); p.MouseInvalidate();
                // Demo=true delays redraw, just as historical playback does.
                p.SetMousePosSprite(pos,sprite,TRUE);
                auto hot=p.*Mouse::m_mouseHotSpot;
                check(hot.x==hx && hot.y==hy,"historical hotspot");
                p.MouseBackDraw(); p.Display();
                auto expected=baseline;
                int rank=sprite==SPRITE_POINTER?38:sprite==SPRITE_WAIT?39:37;
                const short* t=table_icon_element+rank*6+1;
                long long dx=(long long)pos.x-hx+t[2],dy=(long long)pos.y-hy+t[3];
                for(int y=0;y<t[5];++y) for(int x=0;x<t[4];++x) {
                    long long xx=dx+x,yy=dy+y;
                    if(xx<0||xx>=640||yy<0||yy>=480) continue;
                    auto c=atlas.color(t[0]+x,t[1]+y);
                    if(c.r==0&&c.g==0&&c.b==255) continue;
                    if(depth==8) c=palette[atlas.indices[(t[1]+y)*atlas.width+t[0]+x]];
                    expected[(size_t)yy*640+(size_t)xx]=color(c,depth);
                }
                auto actual=capture();
                if(actual!=expected) printf("sprite=%d depth=%d at %d,%d\n",sprite,depth,pos.x,pos.y);
                check(actual==expected,"captured cursor placement/clipping");
                if(sprite==SPRITE_ARROW && pos.x==320 && pos.y==240) {
                    std::string name="cursor-"+std::to_string(depth)+".bmp";
                    auto path=blupi::vfs::resolve_write(name.c_str());
                    check(PortPixmapWriteScreenBmp(path.c_str()),"write review capture");
                }
                p.MouseBackClear(); p.Display(); check(capture()==baseline,"restore exact background");
                ++cases;
            }
        }
        // Live MouseUpdate must erase an overlapping and a disjoint old cursor.
        p.SetMousePosSprite({320,240},SPRITE_ARROW,TRUE); p.MouseBackDraw(); p.Display();
        for(POINT pos: {POINT{325,245},POINT{500,300}}) {
            p.SetMousePos(pos,FALSE); auto live=capture();
            p.MouseBackClear(); p.MouseInvalidate(); p.MouseBackDraw(); p.Display();
            check(capture()==live,"live position update equals fresh draw");
        }
        p.MouseBackClear(); p.MouseShow(FALSE); p.MouseBackDraw(); p.Display();
        check(capture()==baseline,"hidden cursor does not draw");
        p.MouseShow(TRUE); p.SetMouseSprite(SPRITE_EMPTY,TRUE); p.MouseBackDraw(); p.Display();
        check(capture()==baseline,"empty cursor does not draw");
        p.SetMouseSprite(SPRITE_DISABLE,TRUE); p.MouseBackDraw(); p.Display();
        check(capture()==baseline,"disabled cursor does not draw");
        p.MouseInvalidate(); check(!(p.*Mouse::m_bMouseBack),"invalidate backup");
    }
    blupi::host::shutdown();
    printf("%d historical-table cursor frame cases, failures=%d\n",cases,failures);
    return failures?1:0;
}
