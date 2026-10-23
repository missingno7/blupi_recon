// SB2 test probes, extending SB1 game/probe.cpp. State injection is active
// only with --probe, before the tick or at @draw (after simulation).
// Additional commands and the explicit coverage fixture: tests/sb2_scene.probe.
// Test-only state probes (port/docs/ROADMAP.md stage 4, coverage).
//
// Some drawing paths are not reached by the recorded demos or the scripted
// menus: Blupi's shield, cloud and hide effects (the hide effect composes in
// CHTEMP), notification texts, the mission-10 tutorial clamp, network Blupis.
// A probe script sets that state between two ticks, so the equivalence gates
// (scene, commands, native versus reference, golden traces) cover those paths.
// Probes are not gameplay: they never press keys or plan moves, and they are
// only active with --probe.
//
// Script lines: "FRAME what [values]" or "FIRST-LAST what [values]", applied
// before that frame's tick; "FRAME @draw what [values]" applies after the
// tick's simulation, right before drawing (for state the simulation would
// otherwise overwrite during the tick, e.g. network Blupis).
//   effect none|shield|power|cloud|hide [timeShield]   Blupi's effect flags
//   front 0|1                                           m_blupiFront
//   notify TEXT...                                      CDecor::NotifPush
//   mission N                                           m_mission
//   private 0|1                                         m_bPrivate
//   netblupi P icon dx dy sec transport                 network Blupi P drawn at Blupi + (dx, dy)
//   multi 0|1                                           CDecor::m_bMulti (drawing of network Blupis)

#include "def.h"
#include "decor.h"
#include "network.h"
#include "event.h"
#include "jauge.h"

#include "game/port_game.h"
#include "game/member_access.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern CDecor*   g_pDecor;
extern CEvent* g_pEvent;
extern CNetwork* g_pNetwork;

namespace
{
#define PROBE_DECOR_MEMBERS(X) \
	X(m_blupiShield) X(m_blupiPower) X(m_blupiCloud) X(m_blupiHide) \
	X(m_blupiTimeShield) X(m_blupiFront) X(m_mission) X(m_bPrivate) \
	X(m_bMulti) X(m_blupiPos) X(m_netBlupiPos) X(m_netBlupiIcon) \
	X(m_netBlupiSec) X(m_netBlupiTransport) \
    X(m_decor) X(m_bigDecor) X(m_moveObject) X(m_posDecor) X(m_dimDecor) \
    X(m_time) X(m_bPause) X(m_decorAction) X(m_decorPhase) X(m_blupiCle) \
    X(m_blupiDynamite) X(m_blupiFocus) X(m_voyageIcon) X(m_voyageChannel) \
    X(m_voyagePhase) X(m_voyageTotal) X(m_voyageStart) X(m_voyageEnd) \
    X(m_jauges) X(m_nbTresor) X(m_phase) X(m_bBuildOfficialMissions)
BLUPI_MEMBER_ACCESS(DecorWrite, CDecor, PROBE_DECOR_MEMBERS)

#define PROBE_JAUGE_MEMBERS(X) X(m_bMinimizeRedraw) X(m_bRedraw)
BLUPI_MEMBER_ACCESS(JaugeWrite, CJauge, PROBE_JAUGE_MEMBERS)

struct Probe
{
	long        first, last;
	bool        beforeDraw;
	std::string what;
	std::string args;
};

std::vector<Probe> g_probes;
}  // namespace

bool PortProbeLoad(FILE* file)
{
	char line[300];
	while (fgets(line, sizeof(line), file))
	{
		char range[32] = {}, what[32] = {};
		int used = 0;
		if (line[0] == '#' || sscanf(line, "%31s %31s %n", range, what, &used) < 2) continue;
		long first = 0, last = 0;
		if (sscanf(range, "%ld-%ld", &first, &last) < 2) last = first;
		bool beforeDraw = strcmp(what, "@draw") == 0;
		if (beforeDraw)
		{
			int more = 0;
			if (sscanf(line + used, "%31s %n", what, &more) < 1) continue;
			used += more;
		}
		std::string args = line + used;
		size_t hash = args.find('#');
		if (hash != std::string::npos) args.resize(hash);
		while (!args.empty() && (args.back() == '\n' || args.back() == '\r' || args.back() == ' ')) args.pop_back();
		g_probes.push_back(Probe{first, last, beforeDraw, what, args});
	}
	return true;
}

static void apply(long frame, bool beforeDraw)
{
	if (g_pDecor == NULL) return;
	CDecor& d = *g_pDecor;
	for (const Probe& p : g_probes)
	{
		if (frame < p.first || frame > p.last || p.beforeDraw != beforeDraw) continue;
		if (p.what == "phase") g_pEvent->ChangePhase((unsigned)atoi(p.args.c_str()));
        else if (p.what == "clear")
        {
            for (int x = 0; x < MAXCELX; ++x) for (int y = 0; y < MAXCELY; ++y)
            { (d.*DecorWrite::m_decor)[x][y].icon = -1; (d.*DecorWrite::m_bigDecor)[x][y].icon = -1; }
            for (auto& object : d.*DecorWrite::m_moveObject) object = {};
        }
        else if (p.what == "tile" || p.what == "big")
        {
            int x, y, icon;
            if (sscanf(p.args.c_str(), "%d %d %d", &x, &y, &icon) != 3 || x < 0 || x >= MAXCELX || y < 0 || y >= MAXCELY) continue;
            auto& cells = p.what == "tile" ? d.*DecorWrite::m_decor : d.*DecorWrite::m_bigDecor;
            cells[x][y].icon = icon;
        }
        else if (p.what == "object")
        {
            int slot, type, ch, icon, x, y, dx = 64, dy = 0;
            if (sscanf(p.args.c_str(), "%d %d %d %d %d %d %d %d", &slot, &type, &ch, &icon, &x, &y, &dx, &dy) < 6 || slot < 0 || slot >= MAXMOVEOBJECT) continue;
            MoveObject& o = (d.*DecorWrite::m_moveObject)[slot];
            o = {}; o.type = type; o.channel = ch; o.icon = icon;
            o.posCurrent = o.posStart = POINT{x, y}; o.posEnd = POINT{x + dx, y + dy};
        }
        else if (p.what == "camera" || p.what == "position")
        {
            int x, y;
            if (sscanf(p.args.c_str(), "%d %d", &x, &y) != 2) continue;
            if (p.what == "camera") { d.*DecorWrite::m_posDecor = POINT{x, y}; d.*DecorWrite::m_dimDecor = POINT{1, 1}; }
            else d.*DecorWrite::m_blupiPos = POINT{x, y};
        }
        else if (p.what == "clock") d.*DecorWrite::m_time = atoi(p.args.c_str());
        else if (p.what == "pause") d.*DecorWrite::m_bPause = atoi(p.args.c_str());
        else if (p.what == "treasure") d.*DecorWrite::m_nbTresor = atoi(p.args.c_str());
        else if (p.what == "drawphase") d.*DecorWrite::m_phase = atoi(p.args.c_str());
        else if (p.what == "official") d.*DecorWrite::m_bBuildOfficialMissions = atoi(p.args.c_str());
        else if (p.what == "shake")
        {
            int action, phase;
            if (sscanf(p.args.c_str(), "%d %d", &action, &phase) == 2)
            { d.*DecorWrite::m_decorAction = action; d.*DecorWrite::m_decorPhase = phase; }
        }
        else if (p.what == "hud")
        {
            int cle, dynamite, focus;
            if (sscanf(p.args.c_str(), "%d %d %d", &cle, &dynamite, &focus) == 3)
            { d.*DecorWrite::m_blupiCle = cle; d.*DecorWrite::m_blupiDynamite = dynamite; d.*DecorWrite::m_blupiFocus = focus; }
        }
        else if (p.what == "voyage")
        {
            int icon, ch, phase, total, sx, sy, ex, ey;
            if (sscanf(p.args.c_str(), "%d %d %d %d %d %d %d %d", &icon, &ch, &phase, &total, &sx, &sy, &ex, &ey) == 8 && total > 0)
            {
                d.*DecorWrite::m_voyageIcon = icon; d.*DecorWrite::m_voyageChannel = ch;
                d.*DecorWrite::m_voyagePhase = phase; d.*DecorWrite::m_voyageTotal = total;
                d.*DecorWrite::m_voyageStart = POINT{sx, sy}; d.*DecorWrite::m_voyageEnd = POINT{ex, ey};
            }
        }
        else if (p.what == "gauge")
        {
            int i, level, hide, minimize, redraw;
            if (sscanf(p.args.c_str(), "%d %d %d %d %d", &i, &level, &hide, &minimize, &redraw) == 5 && i >= 0 && i < 2)
            {
                CJauge& gauge = (d.*DecorWrite::m_jauges)[i];
                gauge.SetLevel(level); gauge.SetHide(hide);
                gauge.*JaugeWrite::m_bMinimizeRedraw = minimize;
                gauge.*JaugeWrite::m_bRedraw = redraw;
            }
        }
        else if (p.what == "effect")
		{
			char kind[16] = {};
			int timeShield = 100;
			sscanf(p.args.c_str(), "%15s %d", kind, &timeShield);
			d.*DecorWrite::m_blupiShield = strcmp(kind, "shield") == 0;
			d.*DecorWrite::m_blupiPower = strcmp(kind, "power") == 0;
			d.*DecorWrite::m_blupiCloud = strcmp(kind, "cloud") == 0;
			d.*DecorWrite::m_blupiHide = strcmp(kind, "hide") == 0;
			d.*DecorWrite::m_blupiTimeShield = timeShield;
		}
		else if (p.what == "front") d.*DecorWrite::m_blupiFront = atoi(p.args.c_str()) != 0;
		else if (p.what == "notify")
		{
			char text[100];
			snprintf(text, sizeof(text), "%s", p.args.c_str());
			d.NotifPush(text);
		}
		else if (p.what == "mission") d.*DecorWrite::m_mission = atoi(p.args.c_str());
		else if (p.what == "private") d.*DecorWrite::m_bPrivate = atoi(p.args.c_str()) != 0;
		else if (p.what == "multi") d.*DecorWrite::m_bMulti = atoi(p.args.c_str()) != 0;
		else if (p.what == "netblupi")
		{
			int player = 0, icon = -1, dx = 0, dy = 0, sec = 0, transport = 2;
			sscanf(p.args.c_str(), "%d %d %d %d %d %d", &player, &icon, &dx, &dy, &sec, &transport);
			if (player < 0 || player >= MAXNETPLAYER) continue;
			POINT pos = d.*DecorWrite::m_blupiPos;
			(d.*DecorWrite::m_netBlupiPos)[player] = POINT{pos.x + dx, pos.y + dy};
			(d.*DecorWrite::m_netBlupiIcon)[player] = icon;
			(d.*DecorWrite::m_netBlupiSec)[player] = sec;
			(d.*DecorWrite::m_netBlupiTransport)[player] = transport;
			if (g_pNetwork != NULL) g_pNetwork->m_players[player].team = player;
		}
		else
		{
			fprintf(stderr, "unknown probe '%s' at frame %ld\n", p.what.c_str(), frame);
		}
	}
}

void PortProbeApply(long frame)
{
	apply(frame, false);
}

void PortProbeApplyBeforeDraw(long frame)
{
	apply(frame, true);
}
