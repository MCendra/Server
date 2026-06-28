// ServerList.cpp
#include "Header.h"
#include "ServerList.h"
#include "ScriptParser.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Log.h"

CServerList gServerList;

// Construction/Destruction

CServerList::CServerList()
	: m_JoinServerState(false),
	m_JoinServerStateTime(0),
	m_JoinServerQueueSize(0)
{
}

CServerList::~CServerList() = default;

// Carga lista de GameServers desde el archivo de configuracion
void CServerList::Init(const char* path)
{
	// Objeto en stack: sin heap, sin delete, sin nullptr check.
	// CScriptParser es pequeño y de vida acotada a esta función.
	CScriptParser gScriptParser;

	if (!gScriptParser.SetBuffer(path))
	{
		gUtil.ErrorMessageBox(gScriptParser.GetLastError());
		return;
	}

	m_ServerListInfo.clear();

	try
	{
		while (true)
		{
			eTokenResult token = gScriptParser.GetToken();

			if (token == TOKEN_END)
			{
				break;
			}

			if (token != TOKEN_NUMBER)
			{
				continue;
			}

			SERVER_LIST_INFO info{};

			info.ServerCode = static_cast<WORD>(gScriptParser.GetNumber());

			strncpy_s(info.ServerName, gScriptParser.GetAsString(), sizeof(info.ServerName) - 1);
			strncpy_s(info.ServerAddress, gScriptParser.GetAsString(), sizeof(info.ServerAddress) - 1);

			info.ServerPort = static_cast<WORD>(gScriptParser.GetAsNumber());
			info.ServerShow = (strcmp(gScriptParser.GetAsString(), "SHOW") == 0);
			info.ServerState = false;
			info.ServerStateTime = 0;
			info.UserTotal = 0;
			info.UserCount = 0;
			info.AccountCount = 0;
			info.PCPointCount = 0;
			info.MaxUserCount = 0;

			m_ServerListInfo.emplace(info.ServerCode, info);
		}
	}
	catch (...)
	{
		gUtil.ErrorMessageBox(gScriptParser.GetLastError());
	}

	Log.ToDisp(LOG_BLUE, "Lista de servidores cargada correctamente");

}

// Verifica timeouts de JoinServer y los GameServers
void CServerList::CheckServerTimeouts()
{
	ULONGLONG tick = GetTickCount64();

	if (m_JoinServerState && (tick - m_JoinServerStateTime) > MAX_JOINSERVER_OFFLINE_TIME)
	{
		m_JoinServerState = false;
		m_JoinServerStateTime = 0;
		Log.ToDisp(LOG_RED, "JoinServer fuera de linea");
	}

	for (auto& it : m_ServerListInfo)
	{
		if (it.second.ServerState && (tick - it.second.ServerStateTime) > MAX_GAMESERVER_OFFLINE_TIME)
		{
			it.second.ServerState = false;
			it.second.ServerStateTime = 0;

			Log.ToDisp(LOG_BLACK, "GameServer fuera de linea (%s) (%d)", it.second.ServerName, it.second.ServerCode);
		}
	}
}

// Verifica si JoinServer esta en linea o saturado
bool CServerList::IsJoinServerOnline() const
{
	if (!m_JoinServerState)
	{
		return false;
	}

	if (m_JoinServerQueueSize > MAX_JOIN_SERVER_QUEUE_SIZE)
	{
		return false;
	}

	return true;
}

// Genera la lista de servidores para mostrar a los cleintes, ocultando los que no estan disponibles o no se deben mostrar
long CServerList::GenerateServerList(BYTE* lpMsg, int* size)
{
	if (lpMsg == nullptr || size == nullptr)
	{
		return 0;
	}

	long count = 0;

	if (!IsJoinServerOnline())
	{
		return 0;
	}

	for (const auto& it : m_ServerListInfo)
	{
		if (it.second.ServerShow && it.second.ServerState)
		{
			PMSG_SERVER_LIST info{};

			info.ServerCode = it.second.ServerCode;
			info.UserTotal = it.second.UserTotal;
			info.type = 0xCC;

			memcpy(&lpMsg[(*size)], &info, sizeof(info));
			(*size) += sizeof(info);

			count++;
		}
	}

	return count;
}

// Busca servidor por codigo
SERVER_LIST_INFO* CServerList::GetGameServerInfo(int serverCode)
{
	auto it = m_ServerListInfo.find(serverCode);

	if (it == m_ServerListInfo.end())
	{
		return nullptr;
	}

	return &it->second;
}

// Determina si los paquetes pertenecen a JoinServer o GameServer y los manda procesar
void CServerList::ProcessServerStatusPacket(BYTE head, BYTE* lpMsg, int size)
{
	UNREFERENCED_PARAMETER(size);

	if (lpMsg == nullptr)
	{
		return;
	}

	switch (head)
	{
	case 0x01:
		ProcessGameServerHeartbeat((SDHP_GAME_SERVER_LIVE_RECV*)lpMsg);
		break;
	case 0x02:
		ProcessJoinServerHeartbeat((SDHP_JOIN_SERVER_LIVE_RECV*)lpMsg);
		break;
	}
}

// Procesa heartbeat de GameServer, actualizando su estado y tamaño de cola
void CServerList::ProcessGameServerHeartbeat(SDHP_GAME_SERVER_LIVE_RECV* lpMsg)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	SERVER_LIST_INFO* lpServerListInfo = GetGameServerInfo(lpMsg->ServerCode);

	if (lpServerListInfo == nullptr)
	{
		return;
	}

	if (!lpServerListInfo->ServerState)
	{
		Log.ToDisp(LOG_BLACK, "GameServer en linea (%s) (%d)", lpServerListInfo->ServerName, lpServerListInfo->ServerCode);
		// Actualiza la lista de servidores en la pantalla
		gServerDisplayer.Refresh();
	}

	lpServerListInfo->ServerState = true;

	lpServerListInfo->ServerStateTime = GetTickCount64();

	lpServerListInfo->UserTotal = lpMsg->UserTotal;

	lpServerListInfo->UserCount = lpMsg->UserCount;

	lpServerListInfo->AccountCount = lpMsg->AccountCount;

	lpServerListInfo->PCPointCount = lpMsg->PCPointCount;

	lpServerListInfo->MaxUserCount = lpMsg->MaxUserCount;
}

// Procesa heartbeat de JoinServer, actualizando su estado y tamaño de cola
void CServerList::ProcessJoinServerHeartbeat(SDHP_JOIN_SERVER_LIVE_RECV* lpMsg)
{
	if (lpMsg == nullptr)
	{
		return;
	}

	if (!m_JoinServerState)
	{
		Log.ToDisp(LOG_GREEN, "JoinServer en linea");
	}

	m_JoinServerState = true;
	m_JoinServerStateTime = GetTickCount64();
	m_JoinServerQueueSize = lpMsg->QueueSize;

}

int CServerList::GetOnlineGameServerCount() const
{
	int count = 0;

	for (const auto& it : m_ServerListInfo)
	{
		if (it.second.ServerState)
		{
			count++;
		}
	}

	return count;
}

// CServerList implementacion de clase
const std::map<int, SERVER_LIST_INFO>& CServerList::GetGameServerList() const
{
	return m_ServerListInfo;
}
