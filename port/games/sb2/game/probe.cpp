// Test-only phase injection, following SB1's game/probe.cpp. Not gameplay.
// Exercises the publisher phases that historical _INTRO=0 skips at startup.
#include "def.h"
#include "decor.h"
#include "event.h"
#include "game/port_game.h"

#include <cstring>
#include <vector>

extern CEvent* g_pEvent;
namespace {
struct PhaseProbe { long frame; unsigned phase; };
std::vector<PhaseProbe> probes;
}

bool PortProbeLoad(FILE* file)
{
    char line[256], kind[32];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        PhaseProbe p;
        if (sscanf(line, "%ld %31s %u", &p.frame, kind, &p.phase) != 3 ||
            strcmp(kind, "phase") != 0 || p.frame < 0) return false;
        probes.push_back(p);
    }
    return true;
}

void PortProbeApply(long frame)
{
    for (const auto& p : probes)
        if (p.frame == frame) g_pEvent->ChangePhase(p.phase);
}
