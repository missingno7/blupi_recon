// Network.h

#ifndef NETWORK_H
#define NETWORK_H

#include "dplay.h"
#include "def.h"

/////////////////////////////////////////////////////////////////////////////

typedef struct
{
	char  bIsPresent;
	char  ready;
	char  unk_2;
	char  unk_3;
	DPID  dpid;
	short team;
	char  name[22];
}
NetPlayer;

typedef struct
{
	GUID guid;
	char name[100];
}
NamedGUID;

typedef struct
{
	int nb;
	NamedGUID (*pList)[MAXSESSION];
}
NamedGUIDList;

class CNetwork
{
public:
	CNetwork();
	~CNetwork();

	BOOL EnumProviders();
	int GetNbProviders();
	char* GetProviderName(int index);
	BOOL CreateProvider(int index);
	void FreeProviderList();
	BOOL EnumSessions();
	int GetNbSessions();
	char* GetSessionName(int index);
	BOOL JoinSession(int index, char* pPlayerName);
	void FreeSessionList();
	BOOL CreateSession(char* pSessionName, char* pPlayerName);
	BOOL Send(LPVOID lpData, DWORD dwDataSize, DWORD dwFlags);
	BOOL Receive(LPVOID pDest, DWORD dwDataSize, LPDWORD lpdwPlayer);
	BOOL Close();
	void FreeUnknownList();
	BOOL IsHost();

	NetPlayer		m_players[MAXNETPLAYER];
	DPID			m_dpid;

protected:
	LPDIRECTPLAY2	m_pDP;
	NamedGUIDList	m_providers;
	NamedGUIDList	m_sessions;
	NamedGUIDList	m_unknown;
	BOOL			m_bHost;
};

static BOOL EnumProvidersCallback(LPGUID lpguidSP, LPSTR lpSTName,
	DWORD dwMajorVersion, DWORD dwMinorVersion, NamedGUIDList* lpContext);

static BOOL EnumSessionsCallback(LPDPSESSIONDESC2 lpThisSD,
	LPDWORD lpdwTimeOut, DWORD dwFlags, NamedGUIDList* lpContext);

void TraceErrorDP(HRESULT hErr);

#endif