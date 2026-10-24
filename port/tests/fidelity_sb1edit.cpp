// Differential handler test: the unchanged src/event.cpp handlers are the
// reference, not a recording of the wide editor. Injection/copies stay here.
#include "def.h"
#include "decor.h"
#include "event.h"
#include "game/port_game.h"
#include "game/member_access.h"
#include <cstdio>
#include <memory>

extern CEvent* g_pEvent;
extern "C" unsigned int blupi_rand_state(void);
#define EVENT_PARTS(X) X(BuildDown) X(BuildMove) X(GetButtonIndex) X(m_buttons)
BLUPI_MEMBER_ACCESS(Editor, CEvent, EVENT_PARTS)
#define BUTTON_PARTS(X) X(m_nbMenu)
BLUPI_MEMBER_ACCESS(Tool, CButton, BUTTON_PARTS)
#define DECOR_PARTS(X) X(m_posDecor) X(m_posCelHili) X(m_dimCelHili)
BLUPI_MEMBER_ACCESS(World, CDecor, DECOR_PARTS)

static int failures, comparisons, selections, invalidChecks, mutations;
static bool tested;
static void check(bool ok, int tool, int menu, int keys, UINT message,
                  POINT cell, int side, int step)
{
    if (!ok) {
        if (failures < 16)
            std::printf("FAIL tool=%d menu=%d keys=%x message=%u cell=%d,%d side=%d step=%d\n",
                        tool, menu, keys, message, int(cell.x), int(cell.y), side, step);
        ++failures;
    }
}

static void matrix(CDecor& decor)
{
    auto saved = std::make_unique<CDecor>(decor);
    auto event = std::make_unique<CEvent>(*g_pEvent);
    unsigned int savedRng = blupi_rand_state();
    // First/last cells in both axes, plus first/last pixels in an interior cell.
    struct Sample { POINT cell; int remainder; };
    const Sample samples[] = {{{0,0},0}, {{0,99},63}, {{99,0},63},
                              {{99,99},63}, {{25,13},0}, {{25,13},63}};
    for (int tool = WM_DECOR1; tool <= WM_DECOR8; ++tool) {
        int index = (g_pEvent->*Editor::GetButtonIndex)(tool);
        if (index < 0) { ++failures; continue; }
        int menus = (g_pEvent->*Editor::m_buttons)[index].*Tool::m_nbMenu;
        if (menus < 1) menus = 1; // the eraser has no submenu
        std::printf("tool %d: %d selections\n", tool - WM_DECOR1 + 1, menus);
        for (int menu = 0; menu < menus; ++menu) {
            decor = *saved; *g_pEvent = *event;
            g_pEvent->SetMenu(tool, menu);
            // Use the historical selection dispatch too: this sets highlight
            // dimensions, lift icon and lift-end state, unlike SetMenu alone.
            g_pEvent->TreatEventBase(tool, 0, 0);
            for (int other = WM_DECOR1; other <= WM_DECOR8; ++other)
                check(g_pEvent->GetState(other) == (other == tool ? 1 : 0),
                      tool, menu, 0, 0, {0,0}, 0, -2);
            check(g_pEvent->GetMenu(tool) == menu, tool, menu, 0, 0, {0,0}, 0, -2);
            ++selections;
            auto selected = std::make_unique<CDecor>(decor);
            auto selectedEvent = std::make_unique<CEvent>(*g_pEvent);
            // 0x10 is Win32 MK_MBUTTON. The game routes only left/right down;
            // include its bit in the handler masks to verify it is ignored.
            for (int mask = 0; mask < 8; ++mask)
            for (int modifier : {0, MK_SHIFT, MK_CONTROL, MK_SHIFT | MK_CONTROL})
            for (UINT message : {UINT(WM_LBUTTONDOWN), UINT(WM_RBUTTONDOWN), UINT(WM_MOUSEMOVE)})
            for (const auto& sample : samples)
            for (int side : {0, -600, 400}) {
                int keys = modifier | ((mask & 1) ? MK_LBUTTON : 0) |
                    ((mask & 2) ? MK_RBUTTON : 0) | ((mask & 4) ? 0x10 : 0);
                decor = *selected; *g_pEvent = *selectedEvent;
                POINT dim = decor.*World::m_dimCelHili;
                POINT camera = {sample.cell.x * DIMOBJX + sample.remainder - 400 + dim.x * 32 - 32,
                                sample.cell.y * DIMOBJY + sample.remainder - 200 + dim.y * 32 - 32};
                decor.*World::m_posDecor = camera;
                auto start = std::make_unique<CDecor>(decor);
                // Two repeated events exercise cycling/replacement and lift
                // endpoint state, as well as an isolated click or drag sample.
                uint64_t expected[2];
                unsigned int seed = 123u + unsigned(menu * 19 + tool);
                blupi_srand(seed);
                uint64_t before = PortStateHash();
                for (int step = 0; step < 2; ++step) {
                    if (message == WM_MOUSEMOVE)
                        (g_pEvent->*Editor::BuildMove)({400,200}, keys);
                    else (g_pEvent->*Editor::BuildDown)({400,200}, keys, TRUE);
                    expected[step] = PortStateHash();
                }
                if (expected[0] != before) ++mutations;
                decor = *start; *g_pEvent = *selectedEvent;
                (decor.*World::m_posDecor).x -= side;
                blupi_srand(seed);
                for (int step = 0; step < 2; ++step) {
                    PortEditorSideMouse(g_pEvent, &decor, message, keys, {400 + side,200});
                    // ONLY m_posDecor differs by construction. Restore it for
                    // the complete PortStateHash (including RNG and UI state).
                    decor.*World::m_posDecor = camera;
                    check(PortStateHash() == expected[step], tool, menu, keys,
                          message, sample.cell, side, step);
                    ++comparisons;
                    (decor.*World::m_posDecor).x -= side;
                }
            }
            // Independent documented guard contract, NOT historical-handler
            // equality: the derivative rejects out-of-world cells and floors
            // negative numerators. Test -1 pixel, exact world end, and far out.
            for (POINT pixel : {POINT{-1,640}, POINT{640,-1}, POINT{6400,640},
                                POINT{640,6400}, POINT{-5000,640}, POINT{10000,640}})
            for (int modifier : {0, MK_SHIFT, MK_CONTROL, MK_SHIFT | MK_CONTROL})
            for (int mask = 0; mask < 8; ++mask)
            for (UINT message : {UINT(WM_LBUTTONDOWN), UINT(WM_RBUTTONDOWN), UINT(WM_MOUSEMOVE)}) {
                decor = *selected; *g_pEvent = *selectedEvent;
                POINT dim = decor.*World::m_dimCelHili;
                // Camera makes the world pixel equal to the input coordinate.
                decor.*World::m_posDecor = {dim.x * 32 - 32, dim.y * 32 - 32};
                decor.*World::m_posCelHili = {7,9};
                blupi_srand(987);
                if (message == WM_MOUSEMOVE) decor.*World::m_posCelHili = {-1,-1};
                auto expected = PortStateHash();
                decor.*World::m_posCelHili = {7,9};
                int keys = modifier | ((mask & 1) ? MK_LBUTTON : 0) |
                    ((mask & 2) ? MK_RBUTTON : 0) | ((mask & 4) ? 0x10 : 0);
                PortEditorSideMouse(g_pEvent, &decor, message, keys, pixel);
                check(PortStateHash() == expected, tool, menu, keys, message, pixel, 0, -1);
                ++invalidChecks;
            }
        }
    }
    decor = *saved; *g_pEvent = *event; blupi_srand(savedRng);
}

extern "C" void __real__Z9EndOfTickR6CDecor(CDecor&);
extern "C" void __wrap__Z9EndOfTickR6CDecor(CDecor& decor)
{
    if (!tested && g_pEvent && g_pEvent->GetPhase() == WM_PHASE_BUILD) {
        tested = true; matrix(decor);
    }
    __real__Z9EndOfTickR6CDecor(decor);
}
int BlupiMain(int, char**);
int main(int argc, char** argv)
{
    int rc = BlupiMain(argc, argv);
    std::printf("SB1 wide editor: selections=%d comparisons=%d invalid-guard=%d mutations=%d failures=%d\n",
                selections, comparisons, invalidChecks, mutations, failures);
    return rc ? rc : (!tested || !selections || !mutations || failures ? 1 : 0);
}
