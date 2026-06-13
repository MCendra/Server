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

void CServerList::Init(const char* path)
{
	CServerConfigLoader* lpServerConfigLoader = new CServerConfigLoader;

	if (lpServerConfigLoader == nullptr)
	{
		gUtil.ErrorMessageBox(SERVER_CONFIG_ALLOC_ERROR, path);
		return;
	}

	if (lpServerConfigLoader->SetBuffer(path) == 0)
	{
		gUtil.ErrorMessageBox(lpServerConfigLoader->GetLastError());
		delete lpServerConfigLoader;
		return;
	}

	this->m_ServerListInfo.clear();

	try
	{
		while (true)
		{
			if (lpServerConfigLoader->GetToken() == TOKEN_END)
			{
				break;
			}

			if (strcmp("end", lpServerConfigLoader->GetString()) == 0)
			{
				break;
			}

			SERVER_LIST_INFO info = {};

			info.ServerCode = static_cast<WORD>(lpServerConfigLoader->GetNumber());

			strncpy_s(info.ServerName, lpServerConfigLoader->GetAsString(), sizeof(info.ServerName) - 1);

			strncpy_s(info.ServerAddress, lpServerConfigLoader->GetAsString(), sizeof(info.ServerAddress) - 1);

			info.ServerPort = static_cast<WORD>(lpServerConfigLoader->GetAsNumber());

			info.ServerShow = (strcmp(lpServerConfigLoader->GetAsString(), "SHOW") == 0) ? true : false;

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
		gUtil.ErrorMessageBox(lpServerConfigLoader->GetLastError());
	}

	Log.ToDisp(LOG_BLUE, "[HAI DAI SU] Lista de servidores cargada correctamente");

	delete lpServerConfigLoader;
}

void CServerList::MainProc()
{
	if (this->m_JoinServerState != 0 && (GetTickCount64() - this->m_JoinServerStateTime) > 10000)
	{
		this->m_JoinServerState = 0;
		this->m_JoinServerStateTime = 0;
		Log.ToDisp(LOG_RED, "[HAI DAI SU] JoinServer fuera de linea");
	}

	for (std::map<int, SERVER_LIST_INFO>::iterator it = this->m_ServerListInfo.begin();it != this->m_ServerListInfo.end();it++)
	{
		if (it->second.ServerState != 0 && (GetTickCount64() - it->second.ServerStateTime) > 10000)
		{
			it->second.ServerState = 0;
			it->second.ServerStateTime = 0;
			Log.ToDisp(LOG_BLACK, "[HAI DAI SU] GameServer fuera de linea (%s) (%d)", it->second.ServerName, it->second.ServerCode);
		}
	}
}

bool CServerList::CheckJoinServerState() const
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

long CServerList::GenerateServerList(BYTE* lpMsg, int* size)
{
	int count = 0;

	PMSG_SERVER_LIST info{};

	if (this->CheckJoinServerState() != 0)
	{
		for (std::map<int, SERVER_LIST_INFO>::iterator it = this->m_ServerListInfo.begin();it != this->m_ServerListInfo.end();it++)
		{
			if (it->second.ServerShow != 0 && it->second.ServerState != 0)
			{
				info.ServerCode = it->second.ServerCode;

				info.UserTotal = it->second.UserTotal;

				info.type = 0xCC;

				memcpy(&lpMsg[(*size)], &info, sizeof(info));
				(*size) += sizeof(info);

				count++;
			}
		}
	}

	return count;
}

SERVER_LIST_INFO* CServerList::GetServerListInfo(int ServerCode)
{
	std::map<int, SERVER_LIST_INFO>::iterator it = this->m_ServerListInfo.find(ServerCode);

	if (it == this->m_ServerListInfo.end())
	{
		return 0;
	}
	else
	{
		return &it->second;
	}
}

void CServerList::ServerProtocolCore(BYTE head, BYTE* lpMsg, int size)
{
	UNREFERENCED_PARAMETER(size);

	switch (head)
	{
	case 0x01:
		this->GCGameServerLiveRecv((SDHP_GAME_SERVER_LIVE_RECV*)lpMsg);
		break;
	case 0x02:
		this->JCJoinServerLiveRecv((SDHP_JOIN_SERVER_LIVE_RECV*)lpMsg);
		break;
	}
}

void CServerList::GCGameServerLiveRecv(SDHP_GAME_SERVER_LIVE_RECV* lpMsg)
{
	SERVER_LIST_INFO* lpServerListInfo = this->GetServerListInfo(lpMsg->ServerCode);

	if (lpServerListInfo == 0)
	{
		return;
	}

	if (lpServerListInfo->ServerState == 0)
	{
		Log.ToDisp(LOG_BLACK, "[HAI DAI SU] GameServer en linea (%s) (%d)", lpServerListInfo->ServerName, lpServerListInfo->ServerCode);
	}

	lpServerListInfo->ServerState = 1;

	lpServerListInfo->ServerStateTime = GetTickCount64();

	lpServerListInfo->UserTotal = lpMsg->UserTotal;

	lpServerListInfo->UserCount = lpMsg->UserCount;

	lpServerListInfo->AccountCount = lpMsg->AccountCount;

	lpServerListInfo->PCPointCount = lpMsg->PCPointCount;

	lpServerListInfo->MaxUserCount = lpMsg->MaxUserCount;
}

void CServerList::JCJoinServerLiveRecv(SDHP_JOIN_SERVER_LIVE_RECV* lpMsg)
{
	if (this->m_JoinServerState == 0)
	{
		Log.ToDisp(LOG_GREEN, "[HAI DAI SU] JoinServer en linea");
	}

	this->m_JoinServerState = 1;

	this->m_JoinServerStateTime = GetTickCount64();

	this->m_JoinServerQueueSize = lpMsg->QueueSize;
}


void CServerList::NotifyServerStateChange(int serverCode)
{
    // Aqui deberiamos comunicar el cambio de estado a `ServerDisplayer`.
    // Esto puede ser mediante una llamada a una funcion publica en `ServerDisplayer` o mediante una señal.
    gServerDisplayer.UpdateServerState(serverCode);
}
