// Network.cpp
//

#include <stdio.h>
#include <windows.h>
#include "dplay.h"
#include "misc.h"
#include "network.h"
#include "def.h"

// a0f94abe-11c3-d111-be62-0040f6944838
static const GUID APP_GUID = { 0xbe4af9a0, 0xc311, 0x11d1, { 0xbe, 0x62, 0x00, 0x40, 0xf6, 0x94, 0x48, 0x38 } };

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
	if (m_pDP) m_pDP->Release();
}

static BOOL CALLBACK EnumProvidersCallback(LPGUID lpguidSP, LPSTR lpSPName,
	DWORD dwMajorVersion, DWORD dwMinorVersion, NamedGUIDList* lpContext)
{
	NamedGUID* lpGuid;
	if (lpContext->nb < MAXSESSION)
	{
		lpGuid = &(*lpContext->pList)[lpContext->nb];
		lpGuid->guid = *lpguidSP;
		strcpy(lpGuid->name, lpSPName);
		lpContext->nb++;
	}
	return TRUE;
}

BOOL CNetwork::EnumProviders()
{
	FreeProviderList();
	m_providers.nb = 0;
	m_providers.pList = (NamedGUID(*)[MAXSESSION]) malloc(MAXSESSION * sizeof(NamedGUID));

	if (m_providers.pList == NULL) return FALSE;
#ifdef _UNICODE
	if (DirectPlayEnumerate((LPDPENUMDPCALLBACK)EnumProvidersCallback, &m_providers) != DP_OK)
#else
	if (DirectPlayEnumerate((LPDPENUMDPCALLBACKA)EnumProvidersCallback, &m_providers) != DP_OK)
#endif
	{
		FreeProviderList();
		return FALSE;
	}
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
	LPDIRECTPLAY lpDP = NULL;
	BOOL bOK = FALSE;

	if (index >= m_providers.nb) return FALSE;

	if (DirectPlayCreate(&(*m_providers.pList)[index].guid, &lpDP, 0) == DP_OK)
	{
		if (lpDP->QueryInterface(IID_IDirectPlay2A, (LPVOID*)&m_pDP) == DP_OK)
		{
			bOK = TRUE;
		}
	}

	if (lpDP) lpDP->Release();
	return bOK;
}

void CNetwork::FreeProviderList()
{
	if (m_providers.pList) free(m_providers.pList);

	m_providers.nb = 0;
	m_providers.pList = NULL;
}


static BOOL CALLBACK EnumSessionsCallback(LPDPSESSIONDESC2 lpThisSD,
	LPDWORD lpdwTimeOut, DWORD dwFlags, NamedGUIDList* lpContext)
{
	NamedGUID* lpGuid;
	if (dwFlags & DPESC_TIMEDOUT) return FALSE;

	if (lpContext->nb < MAXSESSION)
	{
		lpGuid = &(*lpContext->pList)[lpContext->nb];
		lpGuid->guid = lpThisSD->guidInstance;
		strcpy(lpGuid->name, lpThisSD->lpszSessionNameA);
		lpContext->nb++;
	}
	return TRUE;
}

BOOL CNetwork::EnumSessions()
{
	DPSESSIONDESC2 desc;

	FreeSessionList();
	m_sessions.nb = 0;
	m_sessions.pList = (NamedGUID(*)[MAXSESSION]) malloc(MAXSESSION * sizeof(NamedGUID));

	if (!m_sessions.pList) return FALSE;

	ZeroMemory(&desc, sizeof(desc));

	desc.dwSize = sizeof(desc);
	desc.guidApplication = APP_GUID;

	if (m_pDP->EnumSessions(&desc, 0, (LPDPENUMSESSIONSCALLBACK2)EnumSessionsCallback, &m_sessions, DPENUMSESSIONS_AVAILABLE) != DP_OK)
	{
		FreeSessionList();
		return FALSE;
	}

	return TRUE;
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
	DPNAME name;
	DPSESSIONDESC2 desc;
	HRESULT hr;

	if (index >= m_sessions.nb) return FALSE;

	ZeroMemory(&desc, sizeof(desc));

	desc.dwSize = sizeof(desc);
	desc.guidInstance = (*m_sessions.pList)[index].guid;

	hr = m_pDP->Open(&desc, DPOPEN_OPENSESSION);
	if (hr != DP_OK)
	{
		TraceErrorDP(hr);
		return FALSE;
	}

	ZeroMemory(&name, sizeof(name));
	name.dwSize = sizeof(name);
	name.lpszShortNameA = pPlayerName;
	name.lpszLongNameA = NULL;
	hr = m_pDP->CreatePlayer(&m_dpid, &name, NULL, NULL, 0, 0);
	if (hr != DP_OK)
	{
		TraceErrorDP(hr);
		m_pDP->Close();
		return FALSE;
	}
	else
	{
		m_bHost = FALSE;
		return TRUE;
	}
}

void CNetwork::FreeSessionList()
{
	if (m_sessions.pList) free(m_sessions.pList);

	m_sessions.nb = 0;
	m_sessions.pList = NULL;
}

BOOL CNetwork::CreateSession(char* pSessionName, char* pPlayerName)
{
	DPNAME name;
	DPSESSIONDESC2 desc;
	HRESULT hr;

	ZeroMemory(&desc, sizeof(desc));

	desc.dwSize = sizeof(desc);
	desc.dwFlags = DPSESSION_KEEPALIVE | DPSESSION_MIGRATEHOST;
	desc.guidApplication = APP_GUID;
	desc.dwMaxPlayers = MAXNETPLAYER;
	desc.lpszSessionNameA = pSessionName;

	hr = m_pDP->Open(&desc, DPOPEN_CREATE);
	if (hr != DP_OK)
	{
		TraceErrorDP(hr);
		return FALSE;
	}
	else
	{
		ZeroMemory(&name, sizeof(name));
		name.dwSize = sizeof(name);
		name.lpszShortNameA = pPlayerName;
		name.lpszLongNameA = NULL;
		hr = m_pDP->CreatePlayer(&m_dpid, &name, NULL, NULL, 0, 0);
		if (hr != DP_OK)
		{
			TraceErrorDP(hr);
			m_pDP->Close();
			return FALSE;
		}
		else
		{
			m_bHost = TRUE;
			return TRUE;
		}
	}
}

BOOL CNetwork::Send(LPVOID lpData, DWORD dwDataSize, DWORD dwFlags)
{
	HRESULT hr;

	if (!m_pDP) return FALSE;

	if (hr = m_pDP->Send(m_dpid, 0, dwFlags != 0, lpData, dwDataSize), hr != DP_OK)
	{
		TraceErrorDP(hr);
		return FALSE;
	}
	return TRUE;
}

BOOL CNetwork::Receive(LPVOID pDest, DWORD dwDataSize, LPDWORD lpdwPlayer)
{
	DPID from = 0, to = 0;
	DWORD dataSize = 500;
	char dataBuffer[500];
	HRESULT hr;

	hr = m_pDP->Receive(&from, &to, DPRECEIVE_ALL, dataBuffer, &dataSize);
	if (hr != DP_OK)
	{
		if (hr != DPERR_NOMESSAGES) TraceErrorDP(hr);
		return FALSE;
	}

	memcpy(pDest, dataBuffer, dwDataSize);

	*lpdwPlayer = -1;
	for (int i = 0; i < MAXNETPLAYER; i++)
	{
		if (m_players[i].bIsPresent && from == m_players[i].dpid)
		{
			*lpdwPlayer = i;
			break;
		}
	}

	return TRUE;
}

BOOL CNetwork::Close()
{
	HRESULT hr;
	hr = m_pDP->Close();
	return hr == DP_OK;
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
	char dperr[256];
	char err[1024];

	switch (hErr)
	{
	case DPERR_OUTOFMEMORY: sprintf(dperr, "DPERR_OUTOFMEMORY"); break;
	case DPERR_UNSUPPORTED: sprintf(dperr, "DPERR_UNSUPPORTED"); break;
	case DPERR_NOINTERFACE: sprintf(dperr, "DPERR_NOINTERFACE"); break;
	case DPERR_GENERIC: sprintf(dperr, "DPERR_GENERIC"); break;
	case DPERR_INVALIDPARAMS: sprintf(dperr, "DPERR_INVALIDPARAMS"); break;
	case DPERR_ACTIVEPLAYERS: sprintf(dperr, "DPERR_ACTIVEPLAYERS"); break;
	case DPERR_ACCESSDENIED: sprintf(dperr, "DPERR_ACCESSDENIED"); break;
	case DPERR_CANTADDPLAYER: sprintf(dperr, "DPERR_CANTADDPLAYER"); break;
	case DPERR_CANTCREATEPLAYER: sprintf(dperr, "DPERR_CANTCREATEPLAYER"); break;
	case DPERR_CANTCREATEGROUP: sprintf(dperr, "DPERR_CANTCREATEGROUP"); break;
	case DPERR_CANTCREATESESSION: sprintf(dperr, "DPERR_CANTCREATESESSION"); break;
	case DPERR_BUFFERTOOSMALL: sprintf(dperr, "DPERR_BUFFERTOOSMALL"); break;
	case DPERR_CAPSNOTAVAILABLEYET: sprintf(dperr, "DPERR_CAPSNOTAVAILABLEYET"); break;
	case DPERR_ALREADYINITIALIZED: sprintf(dperr, "DPERR_ALREADYINITIALIZED"); break;
	case DPERR_INVALIDFLAGS: sprintf(dperr, "DPERR_INVALIDFLAGS"); break;
	case DPERR_EXCEPTION: sprintf(dperr, "DPERR_EXCEPTION"); break;
	case DPERR_INVALIDPLAYER: sprintf(dperr, "DPERR_INVALIDPLAYER"); break;
	case DPERR_INVALIDOBJECT: sprintf(dperr, "DPERR_INVALIDOBJECT"); break;
	case DPERR_NOCONNECTION: sprintf(dperr, "DPERR_NOCONNECTION"); break;
	case DPERR_NOCAPS: sprintf(dperr, "DPERR_NOCAPS"); break;
	case DPERR_NONAMESERVERFOUND: sprintf(dperr, "DPERR_NONAMESERVERFOUND"); break;
	case DPERR_NOMESSAGES: sprintf(dperr, "DPERR_NOMESSAGES"); break;
	case DPERR_NOSESSIONS: sprintf(dperr, "DPERR_NOSESSIONS"); break;
	case DPERR_NOPLAYERS: sprintf(dperr, "DPERR_NOPLAYERS"); break;
	case DPERR_TIMEOUT: sprintf(dperr, "DPERR_TIMEOUT"); break;
	case DPERR_SENDTOOBIG: sprintf(dperr, "DPERR_SENDTOOBIG"); break;
	case DPERR_BUSY: sprintf(dperr, "DPERR_BUSY"); break;
	case DPERR_UNAVAILABLE: sprintf(dperr, "DPERR_UNAVAILABLE"); break;
	case DPERR_PLAYERLOST: sprintf(dperr, "DPERR_PLAYERLOST"); break;
	case DPERR_USERCANCEL: sprintf(dperr, "DPERR_USERCANCEL"); break;
	case DPERR_BUFFERTOOLARGE: sprintf(dperr, "DPERR_BUFFERTOOLARGE"); break;
	case DPERR_SESSIONLOST: sprintf(dperr, "DPERR_SESSIONLOST"); break;
	case DPERR_APPNOTSTARTED: sprintf(dperr, "DPERR_APPNOTSTARTED"); break;
	case DPERR_CANTCREATEPROCESS: sprintf(dperr, "DPERR_CANTCREATEPROCESS"); break;
	case DPERR_UNKNOWNAPPLICATION: sprintf(dperr, "DPERR_UNKNOWNAPPLICATION"); break;
	case DPERR_INVALIDINTERFACE: sprintf(dperr, "DPERR_INVALIDINTERFACE"); break;
	case DPERR_NOTLOBBIED: sprintf(dperr, "DPERR_NOTLOBBIED"); break;
	case DP_OK: sprintf(dperr, "DP_OK"); break;

	default: sprintf(dperr, "Unknown Error"); break;
	}
	sprintf(err, "DirectPlay Error %s\n", dperr);
	OutputDebug(err);
}
