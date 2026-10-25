#ifndef BLUPI_SB1_NETWORK_PORT_H
#define BLUPI_SB1_NETWORK_PORT_H
#include <string>
void PortNetworkConfigure(const std::string& auditPath);
void PortNetworkTick(long frame);
#endif
