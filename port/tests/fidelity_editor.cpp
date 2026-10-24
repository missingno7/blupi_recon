// The real editor is initialized by its existing menu script. At its first
// draw, compare each tool/modifier dispatch against historical BuildDown/Move.
// State copies belong only to this test executable; no gameplay hook is added.
#include "def.h"
#include "decor.h"
#include "event.h"
#include "game/port_game.h"
#include "game/member_access.h"
#include <cstdio>
#include <memory>
extern CEvent* g_pEvent;
#define EVENT_PARTS(X) X(BuildDown) X(BuildMove) X(GetButtonIndex) X(m_buttons)
BLUPI_MEMBER_ACCESS(Editor, CEvent, EVENT_PARTS)
#define BUTTON_PARTS(X) X(m_nbMenu)
BLUPI_MEMBER_ACCESS(Tool, CButton, BUTTON_PARTS)
#define DECOR_PARTS(X) X(m_posDecor) X(m_posCelHili) X(m_dimCelHili)
BLUPI_MEMBER_ACCESS(World, CDecor, DECOR_PARTS)
static int failures,cases;
static bool tested;
static void matrix(CDecor& decor)
{
    auto saved=std::make_unique<CDecor>(decor);
    auto event=std::make_unique<CEvent>(*g_pEvent);
    for(int tool=WM_DECOR1;tool<=WM_DECOR11;++tool) {
        for(int other=WM_DECOR1;other<=WM_DECOR11;++other) g_pEvent->SetState(other,other==tool?1:0);
        int index=(g_pEvent->*Editor::GetButtonIndex)(tool);
        if(index<0) {++failures;continue;}
        int menus=(g_pEvent->*Editor::m_buttons)[index].*Tool::m_nbMenu;
        if(menus<1) menus=1;
        for(int menu=0;menu<menus;++menu) {
            g_pEvent->SetMenu(tool,menu);
            for(int buttons: {0,MK_LBUTTON,MK_RBUTTON,MK_LBUTTON|MK_RBUTTON})
            for(int modifier: {0,MK_SHIFT,MK_CONTROL,MK_SHIFT|MK_CONTROL})
            for(bool drag: {false,true})
            for(int outside: {0,-600,400}) {
                decor=*saved;
                decor.*World::m_posDecor={1200,640};
                auto start=std::make_unique<CDecor>(decor);
                int keys=buttons|modifier;
                blupi_srand(123);
                if(drag) (g_pEvent->*Editor::BuildMove)({400,200},keys);
                else (g_pEvent->*Editor::BuildDown)({400,200},keys,TRUE);
                auto expected=PortStateHash();
                decor=*start;
                // A different camera and side coordinate address the same cell.
                (decor.*World::m_posDecor).x-=outside;
                blupi_srand(123);
                PortEditorSideMouse(g_pEvent,&decor,drag?WM_MOUSEMOVE:WM_LBUTTONDOWN,keys,{400+outside,200});
                decor.*World::m_posDecor=(*start).*World::m_posDecor;
                if(PortStateHash()!=expected) {
                    if(failures<12) printf("FAIL tool=%d menu=%d buttons=%d modifier=%d drag=%d side=%d\n",tool,menu,buttons,modifier,drag,outside);
                    ++failures;
                }
                ++cases;
            }
        }
    }
    // Invalid cells cannot mutate the world, and a drag clears the highlight.
    for(POINT pos: {POINT{-5000,200},POINT{10000,200},POINT{200,-5000},POINT{200,10000}}) {
        decor=*saved; decor.*World::m_posDecor={0,0};
        decor.*World::m_posCelHili={-1,-1};
        auto expected=PortStateHash();
        PortEditorSideMouse(g_pEvent,&decor,WM_LBUTTONDOWN,MK_LBUTTON,pos);
        if(PortStateHash()!=expected) ++failures;
        PortEditorSideMouse(g_pEvent,&decor,WM_MOUSEMOVE,MK_LBUTTON,pos);
        if(PortStateHash()!=expected) ++failures;
        cases+=2;
    }
    decor=*saved; *g_pEvent=*event;
}
extern "C" void __real__Z9EndOfTickR6CDecor(CDecor&);
extern "C" void __wrap__Z9EndOfTickR6CDecor(CDecor& decor)
{
    if(!tested&&g_pEvent&&g_pEvent->GetPhase()==WM_PHASE_BUILD) {tested=true;matrix(decor);}
    __real__Z9EndOfTickR6CDecor(decor);
}
int BlupiMain(int,char**);
int main(int argc,char** argv)
{
    int rc=BlupiMain(argc,argv);
    printf("wide editor: %d historical-handler comparisons, failures=%d\n",cases,failures);
    return rc?rc:(!tested||failures?1:0);
}
