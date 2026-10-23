// SB2 adapter, derived from port/games/sb1/game/network.cpp and
// games/sb2/games/sb2/src/network.cpp (historical origin).
// Port replacement of games/sb2/src/network.cpp (historical: SB2-derived DirectPlay
// implementation of CNetwork).
//
// Multiplayer is NOT IMPLEMENTED. CNetwork enumerates zero service providers,
// which is what the original showed on a machine without DirectPlay service
// providers, so the multiplayer screens open but offer nothing to join or
// create. Every operation that would need a session fails. The list handling
// below is the historical code so the menus see the same states.

#include <stdio.h>
#include <stdlib.h>
#include "def.h"
#include "network.h"
#include "misc.h"
#include "platform/host.h"

CNetwork::CNetwork()
{
	m_pDP = NULL;
	m_dpid = 0;
	m_bHost = FALSE;
	m_providers.nb = 0;
	m_providers.pList = NULL;
	m_sessions.nb = 0;
	m_sessions.pList = NULL;
	m_unknown.nb = 0;
	m_unknown.pList = NULL;
}

CNetwork::~CNetwork()
{
	FreeProviderList();
	FreeSessionList();
	FreeUnknownList();
}

BOOL CNetwork::EnumProviders()
{
	static bool reported = false;
	if (!reported) { blupi::host::log_info("SB2: DirectPlay multiplayer unavailable (no service providers)."); reported = true; }
	FreeProviderList();
	m_providers.nb = 0;
	m_providers.pList = (NamedGUID(*)[MAXSESSION]) malloc(MAXSESSION * sizeof(NamedGUID));
	if (m_providers.pList == NULL) return FALSE;
	return TRUE;
}

int CNetwork::GetNbProviders()
{
	return m_providers.nb;
}

char* CNetwork::GetProviderName(int index)
{
	if (index >= m_providers.nb) return NULL;
	return (*m_providers.pList)[index].name;
}

BOOL CNetwork::CreateProvider(int index)
{
	return FALSE;
}

void CNetwork::FreeProviderList()
{
	if (m_providers.pList) free(m_providers.pList);
	m_providers.nb = 0;
	m_providers.pList = NULL;
}

BOOL CNetwork::EnumSessions()
{
	FreeSessionList();
	m_sessions.nb = 0;
	m_sessions.pList = (NamedGUID(*)[MAXSESSION]) malloc(MAXSESSION * sizeof(NamedGUID));
	if (m_sessions.pList == NULL) return FALSE;
	return FALSE;
}

int CNetwork::GetNbSessions()
{
	return m_sessions.nb;
}

char* CNetwork::GetSessionName(int index)
{
	if (index >= m_sessions.nb) return NULL;
	return (*m_sessions.pList)[index].name;
}

BOOL CNetwork::JoinSession(int index, char* pPlayerName)
{
	return FALSE;
}

void CNetwork::FreeSessionList()
{
	if (m_sessions.pList) free(m_sessions.pList);
	m_sessions.nb = 0;
	m_sessions.pList = NULL;
}

BOOL CNetwork::CreateSession(char* pSessionName, char* pPlayerName)
{
	return FALSE;
}

BOOL CNetwork::Send(LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	return FALSE;
}

BOOL CNetwork::Receive(LPVOID pDest, DWORD dwDataSize, LPDWORD lpdwPlayer)
{
	return FALSE;
}

BOOL CNetwork::Close()
{
	m_bHost = FALSE;
	return TRUE;
}

void CNetwork::FreeUnknownList()
{
	if (m_unknown.pList) free(m_unknown.pList);
	m_unknown.nb = 0;
	m_unknown.pList = NULL;
}

BOOL CNetwork::IsHost()
{
	return m_bHost;
}

void TraceErrorDP(HRESULT hErr)
{
}
