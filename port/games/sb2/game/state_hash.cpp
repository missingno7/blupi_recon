// SB2 state trace, derived from port/games/sb1/game/state_hash.cpp;
// fields follow games/sb2/src/{decor,event}.h. No raw pointer/padding hashes.
// Simulation state hash for traces and equivalence gates (port/docs/ROADMAP.md,
// stage 0.1). Reads CDecor and CEvent through pointer-to-member accessors
// formed in derived structs, which standard C++ allows for protected members,
// so the historical headers stay untouched.
//
// Hashed: every value member of CDecor and CEvent that holds game or UI state.
// Not hashed: pointers and handles, CDecor::m_unknownTail (size-only
// placeholder, never accessed), buffers of transient file-dialog data
// (CEvent::m_filenameBuffer, m_names), the demo buffer contents (owned by the
// demo file), the CJauge/CButton objects' pointers and handles (their other
// members are hashed), and CEvent::m_menu (transient editor menu, outside
// this attract-mode gate). Character arrays are hashed up to
// their terminating NUL, because bytes after it are never read by the game.
// CEvent::m_joyID and m_nbJoy are excluded: the historical constructor leaves
// them uninitialised (heap contents), and the game only reads them after the
// setup screen assigns them (event.cpp:2754, 3371).

#include "def.h"
#include "decor.h"
#include "event.h"

#include "game/port_game.h"
#include "game/member_access.h"

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
	X(m_decor) X(m_bigDecor) X(m_balleTraj) X(m_moveTraj) X(m_moveObject) X(m_keyPress) X(m_lastKeyPress) \
	X(m_posDecor) X(m_dimDecor) X(m_posCelHili) X(m_dimCelHili) X(m_liftEnd) X(m_phase) \
	X(m_term) X(m_music) X(m_region) X(m_lastRegion) X(m_iconLift) X(m_time) X(m_bPause) \
	X(m_nbRankCaisse) X(m_rankCaisse) X(m_nbLinkCaisse) X(m_linkCaisse) X(m_blupiPos) \
	X(m_blupiValidPos) X(m_blupiAction) X(m_blupiDir) X(m_blupiPhase) X(m_blupiVitesse) \
	X(m_blupiIcon) X(m_blupiSec) X(m_blupiChannel) X(m_blupiVector) X(m_blupiTransport) \
	X(m_blupiFocus) X(m_blupiAir) X(m_blupiHelico) X(m_blupiJeep) X(m_blupiTank) \
	X(m_blupiSkate) X(m_blupiNage) X(m_blupiSurf) X(m_blupiVent) X(m_blupiSuspend) \
	X(m_blupiJumpAie) X(m_blupiShield) X(m_blupiPower) X(m_blupiCloud) X(m_blupiHide) X(m_blupiOver) X(m_blupiInvert) X(m_blupiBalloon) X(m_blupiEcrase) \
	X(m_blupiPosHelico) X(m_blupiPosMagic) X(m_blupiRestart) X(m_blupiFront) \
	X(m_blupiBullet) X(m_blupiCle) X(m_blupiDynamite) X(m_blupiTimeNoAsc) X(m_blupiPerso) X(m_blupiNoBarre) X(m_blupiTimeShield) \
	X(m_blupiTimeFire) X(m_blupiTimeMockery) X(m_blupiTimeOuf) X(m_blupiActionOuf) \
	X(m_blupiFifoNb) X(m_blupiFifoPos) X(m_blupiStartPos) X(m_blupiStartDir) X(m_bMulti) \
	X(m_team) X(m_netPacketsSent) X(m_netPacketsSent2) X(m_netPacketsReceived) \
	X(m_netPacketsReceived2) X(m_netBlupiPos) X(m_netBlupiIcon) X(m_netBlupiSec) \
	X(m_netBlupiTransport) X(m_netTransports) X(m_netTimes) X(m_netPrevTimes) \
	X(m_netTimeSincePacket) X(m_netVitesses) X(m_netBlupiPrevPos) X(m_netPacketPos) \
	X(m_netPacketIcon) X(m_netMessages) X(m_netMessageIndex1) X(m_netMessageIndex2) \
	X(m_netMessageIndex3) X(m_notifTime) X(m_blupiLevel) X(m_blupiEnergyUnused) \
	X(m_bHelicoMarch) X(m_bHelicoStop) X(m_bJeepMarch) X(m_bJeepStop) X(m_bFoundCle) \
	X(m_bPrivate) X(m_bCheatDoors) X(m_bSuperBlupi) X(m_bDrawSecret) X(m_bBuildOfficialMissions) X(m_detectIcon) X(m_bNetPacked) \
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
	X(m_bInfoHelp) X(m_9984) X(m_menuDecor)

#define EVENT_TEXTS(X) \
	X(m_textInput) X(m_name) X(m_movieToStart) X(m_textToolTips) X(m_textToolTips2) X(m_chatZone)

// Value members of the UI objects inside CDecor and CEvent (redraw flags
// included: DrawButtons and EndOfTick manage them).
#define HASH_JAUGE_MEMBERS(X) \
	X(m_bHide) X(m_pos) X(m_dim) X(m_type) X(m_level) X(m_bMinimizeRedraw) X(m_bRedraw)
BLUPI_MEMBER_ACCESS(JaugeAccess, CJauge, HASH_JAUGE_MEMBERS)

#define HASH_BUTTON_MEMBERS(X) \
	X(m_type) X(m_bEnable) X(m_bDemo) X(m_bHide) X(m_message) X(m_pos) X(m_dim) \
	X(m_state) X(m_mouseState) X(m_iconMenu) X(m_toolTips) X(m_nbMenu) X(m_nbToolTips) \
	X(m_selMenu) X(m_bMouseDown) X(m_bMinimizeRedraw) X(m_bRedraw)
BLUPI_MEMBER_ACCESS(ButtonAccess, CButton, HASH_BUTTON_MEMBERS)

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

#define HASH_DECOR_MEMBERS(X) DECOR_VALUES(X) DECOR_TEXTS(X) X(m_jauges)
BLUPI_MEMBER_ACCESS(DecorAccess, CDecor, HASH_DECOR_MEMBERS)

#define HASH_EVENT_MEMBERS(X) EVENT_VALUES(X) EVENT_TEXTS(X) X(m_buttons)
BLUPI_MEMBER_ACCESS(EventAccess, CEvent, HASH_EVENT_MEMBERS)

template <typename F, typename T>
void visit_value(F& f, const char* name, const T& v)
{
	f(name, &v, sizeof(v));
}

// The host's lift synchro messages (CDecor::MoveObjectStepLine, games/sb2/src/decmove.cpp:
// PK_ASCENSEURSTART/END) leave NetMessage::data1 uninitialised; only PK_OBJECTSTART carries a value
// there (the speed). The queue is hashed with that byte read as the network adapter sends it
// (port/common/net/game_protocol.cpp Canonicalise), so the hash does not depend on stack contents.
// Single-player runs never push these.
template <typename F, size_t N>
void visit_value(F& f, const char* name, const NetMessage (&v)[N])
{
	NetMessage copy[N];
	memcpy(copy, v, sizeof(copy));
	for (NetMessage& m : copy)
		if (m.type != PK_OBJECTSTART) m.data1 = 0;
	f(name, copy, sizeof(copy));
}

// Calls f(name, bytes, size) for every hashed piece of state, in a fixed order.
template <typename F>
void visit_state(F&& f)
{
	unsigned int seed = blupi_rand_state();
	f("rand", &seed, sizeof(seed));
	if (g_pDecor != NULL)
	{
		const CDecor& d = *g_pDecor;
#define VISIT(m) visit_value(f, "CDecor::" #m, d.*DecorAccess::m);
		DECOR_VALUES(VISIT)
#undef VISIT
#define VISIT(m) visit_texts(f, "CDecor::" #m, d.*DecorAccess::m);
		DECOR_TEXTS(VISIT)
#undef VISIT
		for (const CJauge& j : d.*DecorAccess::m_jauges)
		{
#define VISIT(m) f("CDecor::m_jauges[]." #m, &(j.*JaugeAccess::m), sizeof(j.*JaugeAccess::m));
			VISIT(m_bHide) VISIT(m_pos) VISIT(m_dim) VISIT(m_type) VISIT(m_level)
			VISIT(m_bMinimizeRedraw) VISIT(m_bRedraw)
#undef VISIT
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
		for (const CButton& b : e.*EventAccess::m_buttons)
		{
#define VISIT(m) f("CEvent::m_buttons[]." #m, &(b.*ButtonAccess::m), sizeof(b.*ButtonAccess::m));
			VISIT(m_type) VISIT(m_bEnable) VISIT(m_bDemo) VISIT(m_bHide) VISIT(m_message) VISIT(m_pos)
			VISIT(m_dim) VISIT(m_state) VISIT(m_mouseState) VISIT(m_iconMenu) VISIT(m_toolTips)
			VISIT(m_nbMenu) VISIT(m_nbToolTips) VISIT(m_selMenu) VISIT(m_bMouseDown)
			VISIT(m_bMinimizeRedraw) VISIT(m_bRedraw)
#undef VISIT
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
