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

#include "game/port_game.h"
#include "game/member_access.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

extern CDecor*   g_pDecor;
extern CNetwork* g_pNetwork;

namespace
{
#define PROBE_DECOR_MEMBERS(X) \
	X(m_blupiShield) X(m_blupiPower) X(m_blupiCloud) X(m_blupiHide) \
	X(m_blupiTimeShield) X(m_blupiFront) X(m_mission) X(m_bPrivate) \
	X(m_bMulti) X(m_blupiPos) X(m_netBlupiPos) X(m_netBlupiIcon) \
	X(m_netBlupiSec) X(m_netBlupiTransport)
BLUPI_MEMBER_ACCESS(DecorWrite, CDecor, PROBE_DECOR_MEMBERS)

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
		if (p.what == "effect")
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
