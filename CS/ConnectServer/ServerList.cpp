// ServerList.cpp
#include "Header.h"
#include "ServerList.h"
#include "ServerConfigLoader.h"
#include "ServerDisplayer.h"
#include "Util.h"
#include "Log.h"

CServerList gServerList;

// Construction/Destruction

CServerList::CServerList()
{
	this->m_JoinServerState = 0;
	this->m_JoinServerStateTime = 0;
	this->m_JoinServerQueueSize = 0;
	this->m_ServerListInfo.clear();
}

CServerList::~CServerList()
{

}

// Carga lista de GameServers desde el archivo de configuracion
void CServerList::Init(const char* path)
{
	// Objeto en stack: sin heap, sin delete, sin nullptr check.
	// CServerConfigLoader es pequeño y de vida acotada a esta función.
	CServerConfigLoader serverConfigLoader;

	if (serverConfigLoader.SetBuffer(path) == 0)
	{
		gUtil.ErrorMessageBox(serverConfigLoader.GetLastError());
		return;
	}

	this->m_ServerListInfo.clear();

	try
	{
		while (true)
		{
			eTokenResult token = serverConfigLoader.GetToken();

			if (token == TOKEN_END)
			{
				break;
			}

			if (token != TOKEN_NUMBER)
			{
				continue;
			}

			SERVER_LIST_INFO info = {};

			info.ServerCode = (WORD)serverConfigLoader.GetNumber();

			strncpy_s(info.ServerName, serverConfigLoader.GetAsString(), sizeof(info.ServerName) - 1);
			strncpy_s(info.ServerAddress, serverConfigLoader.GetAsString(), sizeof(info.ServerAddress) - 1);

			info.ServerPort = static_cast<WORD>(serverConfigLoader.GetAsNumber());
			info.ServerShow = (strcmp(serverConfigLoader.GetAsString(), "SHOW") == 0);
			info.ServerState = 0;
			info.ServerStateTime = 0;
			info.UserTotal = 0;
			info.UserCount = 0;
			info.AccountCount = 0;
			info.PCPointCount = 0;
			info.MaxUserCount = 0;

			this->m_ServerListInfo.insert(std::make_pair(info.ServerCode, info));
		}
	}
	catch (...)
	{
		gUtil.ErrorMessageBox(serverConfigLoader.GetLastError());
	}

	Log.ToDisp(LOG_BLUE, "Lista de servidores cargada correctamente");

}

// Verifica timeouts de JoinServer y los GameServers
void CServerList::CheckServerTimeouts()
{
	if (this->m_JoinServerState != 0 && (GetTickCount64() - this->m_JoinServerStateTime) > MAX_JOINSERVER_OFFLINE_TIME)
	{
		this->m_JoinServerState = 0;
		this->m_JoinServerStateTime = 0;
		Log.ToDisp(LOG_RED, "JoinServer fuera de linea");
	}

	for (auto& it : this->m_ServerListInfo)
	{
		if (it.second.ServerState != 0 && (GetTickCount64() - it.second.ServerStateTime) > MAX_GAMESERVER_OFFLINE_TIME)
		{
			it.second.ServerState = 0;
			it.second.ServerStateTime = 0;

			Log.ToDisp(LOG_BLACK, "GameServer fuera de linea (%s) (%d)", it.second.ServerName, it.second.ServerCode);
		}
	}
}

// Verifica si JoinServer esta en linea o saturado
bool CServerList::IsJoinServerOnline() const
{
	if (this->m_JoinServerState == 0)
	{
		return false;
	}

	if (this->m_JoinServerQueueSize > MAX_JOIN_SERVER_QUEUE_SIZE)
	{
		return false;
	}

	return true;
}

// Genera la lista de servidores para mostrar a los cleintes, ocultando los que no estan disponibles o no se deben mostrar
long CServerList::GenerateServerList(BYTE* lpMsg, int* size)
{
	int count = 0;

	PMSG_SERVER_LIST info{};

	if (this->IsJoinServerOnline())
	{
		for (const auto& it : this->m_ServerListInfo)
		{
			if (it.second.ServerShow != 0 && it.second.ServerState != 0)
			{
				info.ServerCode = it.second.ServerCode;
				info.UserTotal = it.second.UserTotal;
				info.type = 0xCC;

				memcpy(&lpMsg[(*size)], &info, sizeof(info));
				(*size) += sizeof(info);

				count++;
			}
		}
	}

	return count;
}

// Busca servidor por codigo
SERVER_LIST_INFO* CServerList::GetGameServerInfo(int serverCode)
{
	auto it = this->m_ServerListInfo.find(serverCode);

	if (it == this->m_ServerListInfo.end())
	{
		return nullptr;
	}

	return &it->second;
}

// Determina si los paquetes pertenecen a JoinServer o GameServer y los manda procesar
void CServerList::ProcessServerStatusPacket(BYTE head, BYTE* lpMsg, int size)
{
	UNREFERENCED_PARAMETER(size);

	switch (head)
	{
	case 0x01:
		this->ProcessGameServerHeartbeat((SDHP_GAME_SERVER_LIVE_RECV*)lpMsg);
		break;
	case 0x02:
		this->ProcessJoinServerHeartbeat((SDHP_JOIN_SERVER_LIVE_RECV*)lpMsg);
		break;
	}
}

// Procesa heartbeat de GameServer, actualizando su estado y tamaño de cola
void CServerList::ProcessGameServerHeartbeat(SDHP_GAME_SERVER_LIVE_RECV* lpMsg)
{
	SERVER_LIST_INFO* lpServerListInfo = this->GetGameServerInfo(lpMsg->ServerCode);

	if (lpServerListInfo == nullptr)
	{
		return;
	}

	if (lpServerListInfo->ServerState == 0)
	{
		Log.ToDisp(LOG_BLACK, "GameServer en linea (%s) (%d)", lpServerListInfo->ServerName, lpServerListInfo->ServerCode);
		// Actualiza la lista de servidores en la pantalla
		gServerDisplayer.Refresh();
	}

	lpServerListInfo->ServerState = 1;

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
	if (this->m_JoinServerState == 0)
	{
		Log.ToDisp(LOG_GREEN, "JoinServer en linea");
	}

	this->m_JoinServerState = 1;
	this->m_JoinServerStateTime = GetTickCount64();
	this->m_JoinServerQueueSize = lpMsg->QueueSize;

}

int CServerList::GetOnlineGameServerCount() const
{
	int count = 0;

	for (const auto& it : this->m_ServerListInfo)
	{
		if (it.second.ServerState != 0)
		{
			count++;
		}
	}

	return count;
}

// CServerList implementacion de clase
const std::map<int, SERVER_LIST_INFO>& CServerList::GetGameServerList() const
{
	return this->m_ServerListInfo;
}
