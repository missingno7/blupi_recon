// SB1 wire vocabulary, decoded before any historical buffer is filled.
#ifndef BLUPI_SB1_NETWORK_PROTOCOL_H
#define BLUPI_SB1_NETWORK_PROTOCOL_H
#include <array>
#include <cstddef>
#include <cstdint>
#pragma push_macro("rand")
#undef rand
#include <functional>
#pragma pop_macro("rand")

namespace blupi::sb1::net {
constexpr uint32_t Protocol = 0x53423155, Version = 1;
enum class Class { Invalid, Lobby, Gameplay };
struct Player { bool present = false, ready = false; uint32_t id = 0; int team = 0; };
struct Context {
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
} // namespace blupi::sb1::net
#endif
