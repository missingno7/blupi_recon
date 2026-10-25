// The games' wire vocabulary, decoded before any historical buffer is filled. Speedy Blupi 2
// inherited Speedy Blupi's messages and layouts unchanged (games/sb2/src/decor.h, network.h, def.h
// PK_* / MESS_*); what differs per game is a table of rules: the object types, sounds, icons and maps
// each game's code can emit, and a protocol id so that SB1 and SB2 sessions never meet.
#ifndef BLUPI_NET_GAME_PROTOCOL_H
#define BLUPI_NET_GAME_PROTOCOL_H
#include <array>
#include <cstddef>
#include <cstdint>
#pragma push_macro("rand")
#undef rand
#include <functional>
#pragma pop_macro("rand")

namespace blupi::net::game {
constexpr uint32_t Version = 1;
struct Rules {
    const char* game;          // content fingerprint prefix: "sb1", "sb2"
    uint32_t protocol;         // session protocol id
    int worldBase, maps;       // the lobby's map choice m (1..maps) loads world worldBase+m
    int tiles;                 // decor icons: table_icon_object[0]
    int elementIcons;          // pose icons on channel 10: table_icon_element[0]
    int blupiIcons;            // pose icons on channels 2 and 11..13: table_icon_blupi[0]
    int lastSound;             // highest sound id
    bool (*transient)(int type);            // object types the original sends unreliably
    bool (*startType)(int type,int speed);  // ObjectStart(..., TRUE) emitters
    bool (*deleteType)(int type);           // ObjectDelete(..., TRUE) emitters
};
extern const Rules SB1, SB2;
enum class Class { Invalid, Lobby, Gameplay };
struct Player { bool present = false, ready = false; uint32_t id = 0; int team = 0; };
struct Context {
    const Rules* rules = nullptr;
    uint32_t origin = 0, local = 0, host = 1;
    bool receiverHost = false, started = false, reliable = false, trusted = false;
    std::array<Player,4> players{};
    std::function<bool(uint32_t)> known;
    std::function<bool(int)> liveLift;
};
Class Classify(const uint8_t*, size_t);
// Returns a stable rejection kind, or nullptr on success. No state writes.
const char* Validate(const uint8_t*, size_t, const Context&);
uint32_t U32(const uint8_t*);
int S16(const uint8_t*);
// Canonicalise only historically unused bytes, never meaningful fields.
void Canonicalise(uint8_t*, size_t);
} // namespace blupi::net::game
#endif
