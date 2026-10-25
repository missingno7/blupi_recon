#ifndef BLUPI_SB2_NETWORK_PORT_H
#define BLUPI_SB2_NETWORK_PORT_H
#include <string>
#include "net/game_protocol.h"
// The rules the shared CNetwork adapter (port/common/net/cnetwork.cpp) applies to this game.
inline const blupi::net::game::Rules& PortNetworkRules() { return blupi::net::game::SB2; }
void PortNetworkConfigure(const std::string& auditPath);
void PortNetworkTick(long frame);
#endif
