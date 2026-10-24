// SB1 save contract from src/decor.cpp CurrentWrite/CurrentRead and decor.h.
// Observe real F3/F4 calls at their boundaries: no gameplay injection, no golden.
// Only persisted values are hashed; RNG, pause, UI and notifications are not saved.
#include "def.h"
#include "decor.h"
#include "game/member_access.h"
#include "platform/vfs.h"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <vector>
// Inspect the actual resolved disk path with the host CRT, outside game I/O.
#undef fopen

#define SAVED_MEMBERS(X) \
 X(m_decor) X(m_bigDecor) X(m_moveObject) X(m_posDecor) X(m_dimDecor) \
 X(m_phase) X(m_term) X(m_music) X(m_region) X(m_time) X(m_missionTitle) \
 X(m_nbRankCaisse) X(m_rankCaisse) X(m_nbLinkCaisse) X(m_linkCaisse) \
 X(m_blupiPos) X(m_blupiValidPos) X(m_blupiAction) X(m_blupiDir) \
 X(m_blupiPhase) X(m_blupiVitesse) X(m_blupiIcon) X(m_blupiSec) \
 X(m_blupiChannel) X(m_blupiVector) X(m_blupiTransport) X(m_blupiFocus) \
 X(m_blupiAir) X(m_blupiHelico) X(m_blupiJeep) X(m_blupiTank) \
 X(m_blupiSkate) X(m_blupiNage) X(m_blupiSurf) X(m_blupiVent) \
 X(m_blupiSuspend) X(m_blupiJumpAie) X(m_blupiShield) X(m_blupiPower) \
 X(m_blupiCloud) X(m_blupiHide) X(m_blupiPosHelico) X(m_blupiPosMagic) \
 X(m_blupiRestart) X(m_blupiFront) X(m_blupiBullet) X(m_blupiPerso) \
 X(m_blupiNoBarre) X(m_blupiTimeShield) X(m_blupiTimeFire) \
 X(m_blupiTimeOuf) X(m_blupiActionOuf) X(m_blupiFifoNb) X(m_blupiFifoPos) \
 X(m_blupiStartPos) X(m_blupiStartDir) X(m_blupiLevel) X(m_blupiEnergyUnused) \
 X(m_bHelicoMarch) X(m_bHelicoStop) X(m_bJeepMarch) X(m_bJeepStop) \
 X(m_bFoundCle) X(m_bPrivate) X(m_bCheatDoors) X(m_bSuperBlupi) \
 X(m_bDrawSecret) X(m_bJoystick) X(m_mission) X(m_doors) X(m_nbVies) \
 X(m_nbTresor) X(m_totalTresor) X(m_goalPhase) X(m_scrollFollow) \
 X(m_scrollPoint) X(m_scrollAdd) X(m_voyageIcon) X(m_voyageChannel) \
 X(m_voyagePhase) X(m_voyageTotal) X(m_voyageStart) X(m_voyageEnd) \
 X(m_decorAction) X(m_decorPhase)
#define ACCESS_MEMBERS(X) SAVED_MEMBERS(X) X(m_jauges)
BLUPI_MEMBER_ACCESS(SaveAccess, CDecor, ACCESS_MEMBERS)

struct Field { const char* name; std::vector<unsigned char> bytes; };
using Snapshot = std::vector<Field>;
template<class T> void append(Snapshot& s, const char* name, const T& value)
{
    const auto* p = reinterpret_cast<const unsigned char*>(&value);
    s.push_back({name, {p, p + sizeof(value)}});
}
static Snapshot snapshot(CDecor& d)
{
    Snapshot s;
#define ADD(m) append(s, #m, d.*SaveAccess::m);
    SAVED_MEMBERS(ADD)
#undef ADD
    for (int i = 0; i < 2; ++i) {
        auto& gauge = (d.*SaveAccess::m_jauges)[i];
        append(s, "jaugeHide", gauge.GetHide());
        append(s, "jaugeType", gauge.GetType());
        append(s, "jaugeLevel", gauge.GetLevel());
    }
    return s;
}
static uint64_t hash(const Snapshot& s)
{
    uint64_t h = 1469598103934665603ULL;
    for (const auto& f : s) for (auto b : f.bytes) { h ^= b; h *= 1099511628211ULL; }
    return h;
}
static Snapshot saved;
static POINT savedPos;
static int writes, reads, failures;
static bool previewLoad;
extern "C" BOOL __real__ZN6CDecor12CurrentWriteEii(CDecor*, int, int);
extern "C" BOOL __wrap__ZN6CDecor12CurrentWriteEii(CDecor* d, int gamer, int slot)
{
    // ReadAll temporarily writes slot 999 for the dialog's map preview.
    if (slot == 999) {
        previewLoad = true;
        return __real__ZN6CDecor12CurrentWriteEii(d, gamer, slot);
    }
    saved = snapshot(*d); savedPos = d->*SaveAccess::m_blupiPos;
    BOOL ok = __real__ZN6CDecor12CurrentWriteEii(d, gamer, slot);
    if (!ok) ++failures;
    char filename[80];
    std::snprintf(filename, sizeof(filename), "data/s%03d-%03d.blp", gamer, slot);
    std::string path = blupi::vfs::resolve_read(filename);
    FILE* file = std::fopen(path.c_str(), "rb");
    int header[3] = {};
    long length = -1;
    if (file) {
        if (std::fread(header, sizeof(header), 1, file) != 1) ++failures;
        std::fseek(file, 0, SEEK_END); length = std::ftell(file); std::fclose(file);
    }
    if (length != 54416 || header[0] != 54416 || header[1] != 1 || header[2] != 2) {
        std::puts("FAIL: real user save size/revision"); ++failures;
    }
    if (path.find(blupi::vfs::user_root()) != 0) {
        std::puts("FAIL: save is outside the isolated user root"); ++failures;
    }
    ++writes;
    std::printf("saved-state before write: %016llx fields=%zu slot=%d ok=%d\n",
                (unsigned long long)hash(saved), saved.size(), slot, ok);
    return ok;
}
extern "C" BOOL __real__ZN6CDecor11CurrentReadEiiPiS0_(CDecor*, int, int, int*, BOOL*);
extern "C" BOOL __wrap__ZN6CDecor11CurrentReadEiiPiS0_(CDecor* d, int gamer, int slot, int* mission, BOOL* priv)
{
    if (previewLoad) {
        BOOL ok = __real__ZN6CDecor11CurrentReadEiiPiS0_(d, gamer, slot, mission, priv);
        if (slot == 999) previewLoad = false;
        return ok;
    }
    POINT walked = d->*SaveAccess::m_blupiPos;
    if (saved.empty() || std::memcmp(&walked, &savedPos, sizeof(walked)) == 0) {
        std::puts("FAIL: no saved state or no actual displacement before load"); ++failures;
    }
    BOOL ok = __real__ZN6CDecor11CurrentReadEiiPiS0_(d, gamer, slot, mission, priv);
    if (!ok) ++failures;
    Snapshot loaded = snapshot(*d);
    for (size_t i = 0; i < saved.size(); ++i) if (saved[i].bytes != loaded[i].bytes) {
        std::printf("FAIL restored field %s\n", saved[i].name); ++failures;
    }
    if (ok && (*mission != d->*SaveAccess::m_mission || *priv != d->*SaveAccess::m_bPrivate)) ++failures;
    ++reads;
    std::printf("saved-state after read:   %016llx fields=%zu slot=%d ok=%d\n",
                (unsigned long long)hash(loaded), loaded.size(), slot, ok);
    return ok;
}
int BlupiMain(int, char**);
int main(int argc, char** argv)
{
    static_assert(sizeof(DescGameFile) == 54416, "SB1 disk ABI");
    int rc = BlupiMain(argc, argv);
    std::printf("save restoration: writes=%d reads=%d failures=%d\n", writes, reads, failures);
    return rc ? rc : (writes != 1 || reads != 1 || failures ? 1 : 0);
}
