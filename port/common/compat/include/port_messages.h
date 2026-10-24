// Port-only: the posted-message queue behind the compat PostMessage().
// common/app/launcher.cpp drains it and dispatches to the game's window procedure.
#ifndef BLUPI_PORT_MESSAGES_H
#define BLUPI_PORT_MESSAGES_H

#include "windows.h"

struct PortMessage
{
    UINT   message;
    WPARAM wParam;
    LPARAM lParam;
};

bool PortPopMessage(PortMessage& msg);
// For PostMessage(WM_UPDATE) gating and tests: number of queued messages.
int  PortPendingMessages();

#endif
