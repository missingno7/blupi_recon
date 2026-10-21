// Simulation state hash for traces and equivalence gates (port/docs/ROADMAP.md,
// stage 0.1). Reads CDecor and CEvent through pointer-to-member accessors
// formed in derived structs, which standard C++ allows for protected members,
// so the historical headers stay untouched.
//
// Hashed: every value member of CDecor and CEvent that holds game or UI state.
// Not hashed: pointers and handles, CDecor::m_unknownTail (size-only
// placeholder, never accessed), buffers of transient file-dialog data
// (CEvent::m_filenameBuffer, m_names), the demo buffer contents (owned by the
// demo file) and the CJauge/CButton/CMenu objects' pointers (their visible
// state is hashed through public getters). Character arrays are hashed up to
// their terminating NUL, because bytes after it are never read by the game.
// CEvent::m_joyID and m_nbJoy are excluded: the historical constructor leaves
// them uninitialised (heap contents), and the game only reads them after the
// setup screen assigns them (event.cpp:2754, 3371).

#include "def.h"
#include "decor.h"
#include "event.h"

#include "game/port_game.h"

#include <cstring>

extern CDecor* g_pDecor;
extern CEvent* g_pEvent;
extern "C" unsigned int blupi_rand_state(void);

namespace
{
struct Hash
{
	uint64_t value = 1469598103934665603ULL;

	void bytes(const void* p, size_t n)
	{
		const unsigned char* b = static_cast<const unsigned char*>(p);
		for (size_t i = 0; i < n; i++)
		{
			value ^= b[i];
			value *= 1099511628211ULL;
		}
	}

	template <typename T> void pod(const T& v) { bytes(&v, sizeof(v)); }

	void text(const char* s, size_t max)
	{
		size_t n = strnlen(s, max);
		bytes(s, n);
		pod(n);
	}
};

#define DECOR_VALUES(X) \
	X(m_decor) X(m_bigDecor) X(m_moveObject) X(m_keyPress) X(m_lastKeyPress) \
	X(m_posDecor) X(m_dimDecor) X(m_posCelHili) X(m_dimCelHili) X(m_liftEnd) X(m_phase) \
	X(m_term) X(m_music) X(m_region) X(m_lastRegion) X(m_iconLift) X(m_time) X(m_bPause) \
	X(m_nbRankCaisse) X(m_rankCaisse) X(m_nbLinkCaisse) X(m_linkCaisse) X(m_blupiPos) \
	X(m_blupiValidPos) X(m_blupiAction) X(m_blupiDir) X(m_blupiPhase) X(m_blupiVitesse) \
	X(m_blupiIcon) X(m_blupiSec) X(m_blupiChannel) X(m_blupiVector) X(m_blupiTransport) \
	X(m_blupiFocus) X(m_blupiAir) X(m_blupiHelico) X(m_blupiJeep) X(m_blupiTank) \
	X(m_blupiSkate) X(m_blupiNage) X(m_blupiSurf) X(m_blupiVent) X(m_blupiSuspend) \
	X(m_blupiJumpAie) X(m_blupiShield) X(m_blupiPower) X(m_blupiCloud) X(m_blupiHide) \
	X(m_blupiPosHelico) X(m_blupiPosMagic) X(m_blupiRestart) X(m_blupiFront) \
	X(m_blupiBullet) X(m_blupiPerso) X(m_blupiNoBarre) X(m_blupiTimeShield) \
	X(m_blupiTimeFire) X(m_blupiTimeMockery) X(m_blupiTimeOuf) X(m_blupiActionOuf) \
	X(m_blupiFifoNb) X(m_blupiFifoPos) X(m_blupiStartPos) X(m_blupiStartDir) X(m_bMulti) \
	X(m_team) X(m_netPacketsSent) X(m_netPacketsSent2) X(m_netPacketsReceived) \
	X(m_netPacketsReceived2) X(m_netBlupiPos) X(m_netBlupiIcon) X(m_netBlupiSec) \
	X(m_netBlupiTransport) X(m_netTransports) X(m_netTimes) X(m_netPrevTimes) \
	X(m_netTimeSincePacket) X(m_netVitesses) X(m_netBlupiPrevPos) X(m_netPacketPos) \
	X(m_netPacketIcon) X(m_netMessages) X(m_netMessageIndex1) X(m_netMessageIndex2) \
	X(m_netMessageIndex3) X(m_notifTime) X(m_blupiLevel) X(m_blupiEnergyUnused) \
	X(m_bHelicoMarch) X(m_bHelicoStop) X(m_bJeepMarch) X(m_bJeepStop) X(m_bFoundCle) \
	X(m_bPrivate) X(m_bCheatDoors) X(m_bSuperBlupi) X(m_bDrawSecret) X(m_bNetPacked) \
	X(m_bNetMovePredict) X(m_bNetDebug) X(m_bJoystick) X(m_bDemoPlay) X(m_mission) \
	X(m_doors) X(m_nbVies) X(m_nbTresor) X(m_totalTresor) X(m_goalPhase) X(m_scrollFollow) \
	X(m_scrollPoint) X(m_scrollAdd) X(m_voyageIcon) X(m_voyageChannel) X(m_voyagePhase) \
	X(m_voyageTotal) X(m_voyageStart) X(m_voyageEnd) X(m_decorAction) X(m_decorPhase) \
	X(m_lastDecorIndexes)

#define DECOR_TEXTS(X) X(m_missionTitle) X(m_notifText)

#define EVENT_VALUES(X) \
	X(m_joyIndex) X(m_gamer) X(m_textHiliStart) X(m_textHiliEnd) \
	X(m_textCursorIndex) X(m_bNamesExist) X(m_nbFiles) X(m_filePageOffset) X(m_fileIndex) \
	X(m_saveIndex) X(m_nbVies) X(m_speed) X(m_mission) X(m_private) X(m_multi) X(m_phase) \
	X(m_index) X(m_6d30) X(m_bDrawMap) X(m_bPrivate) X(m_bMulti) X(m_bBuildOfficialMissions) \
	X(m_bFullScreen) X(m_mouseType) X(m_phaseAfterMovie) X(m_bRunMovie) X(m_keyPress) \
	X(m_96b4) X(m_oldMousePos) X(m_bMousePress) X(m_bHili) X(m_fileWorld) X(m_fileTime) \
	X(m_posToolTips) X(m_mouseSprite) X(m_bWaitMouse) X(m_bHideMouse) X(m_bDisableMouse) \
	X(m_bShowMouse) X(m_bMouseRelease) X(m_rankCheat) X(m_posCheat) X(m_bMovie) X(m_9950) \
	X(m_bAllMissions) X(m_bHiliInfoButton) X(m_tryPhase) X(m_tryInsertCount) X(m_bDemoRec) \
	X(m_bDemoPlay) X(m_demoTime) X(m_demoIndex) X(m_demoEnd) X(m_demoNumber) X(m_introTime) \
	X(m_bInfoHelp)

#define EVENT_TEXTS(X) \
	X(m_textInput) X(m_name) X(m_movieToStart) X(m_textToolTips) X(m_textToolTips2) X(m_chatZone)

template <typename F, size_t N>
void visit_texts(F& f, const char* name, const char (&s)[N])
{
	f(name, s, strnlen(s, N));
}

template <typename F, size_t N, size_t M>
void visit_texts(F& f, const char* name, const char (&s)[N][M])
{
	for (size_t i = 0; i < N; i++) f(name, s[i], strnlen(s[i], M));
}

struct DecorAccess : CDecor
{
#define DECL(m) static constexpr auto m = &DecorAccess::m;
	DECOR_VALUES(DECL)
	DECOR_TEXTS(DECL)
#undef DECL
	static constexpr auto jauges = &DecorAccess::m_jauges;
};

struct EventAccess : CEvent
{
#define DECL(m) static constexpr auto m = &EventAccess::m;
	EVENT_VALUES(DECL)
	EVENT_TEXTS(DECL)
#undef DECL
	static constexpr auto buttons = &EventAccess::m_buttons;
};

// Calls f(name, bytes, size) for every hashed piece of state, in a fixed order.
template <typename F>
void visit_state(F&& f)
{
	unsigned int seed = blupi_rand_state();
	f("rand", &seed, sizeof(seed));
	if (g_pDecor != NULL)
	{
		const CDecor& d = *g_pDecor;
#define VISIT(m) f("CDecor::" #m, &(d.*DecorAccess::m), sizeof(d.*DecorAccess::m));
		DECOR_VALUES(VISIT)
#undef VISIT
#define VISIT(m) visit_texts(f, "CDecor::" #m, d.*DecorAccess::m);
		DECOR_TEXTS(VISIT)
#undef VISIT
		int i = 0;
		for (CJauge& j : const_cast<CDecor&>(d).*DecorAccess::jauges)
		{
			int v[3] = {j.GetHide(), j.GetLevel(), j.GetType()};
			f(i++ == 0 ? "CDecor::m_jauges[0]" : "CDecor::m_jauges[1]", v, sizeof(v));
		}
	}
	if (g_pEvent != NULL)
	{
		const CEvent& e = *g_pEvent;
#define VISIT(m) f("CEvent::" #m, &(e.*EventAccess::m), sizeof(e.*EventAccess::m));
		EVENT_VALUES(VISIT)
#undef VISIT
#define VISIT(m) visit_texts(f, "CEvent::" #m, e.*EventAccess::m);
		EVENT_TEXTS(VISIT)
#undef VISIT
		for (CButton& b : const_cast<CEvent&>(e).*EventAccess::buttons)
		{
			int v[4] = {b.GetState(), b.GetMenu(), b.GetEnable(), b.GetHide()};
			f("CEvent::m_buttons[]", v, sizeof(v));
		}
	}
}
}  // namespace

uint64_t PortStateHash()
{
	Hash h;
	visit_state([&](const char*, const void* p, size_t n) { h.bytes(p, n); h.pod(n); });
	return h.value;
}

void PortStateDump(FILE* out)
{
	visit_state([&](const char* name, const void* p, size_t n) {
		Hash h;
		h.bytes(p, n);
		fprintf(out, "%-36s %8zu %016llx\n", name, n, (unsigned long long)h.value);
	});
}
