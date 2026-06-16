// ServerProtocol.cpp
#include "ServerProtocol.h"
#include "ClientManager.h"
#include "ServerList.h"
#include "SocketManager.h"
#include "Log.h"

void ConnectServerProtocolCore(int index, BYTE head, BYTE* lpMsg, int size)
{

	UNREFERENCED_PARAMETER(size);

	gClientManager[index].m_PacketTime = GetTickCount64(); // Modificado para GetTickCount64()

	switch (head)
	{
	case MSG_HEADER_TYPE_F4: // Usamos la nueva constante
		switch (lpMsg[3])
		{
		case MSG_HEADER_SERVER_INFO_RECV:
			CCServerInfoRecv((PMSG_SERVER_INFO_RECV*)lpMsg, index);
			break;
		case MSG_HEADER_SERVER_LIST_RECV:
			CCServerListRecv((PMSG_SERVER_LIST_RECV*)lpMsg, index);
			break;
		}
		break;
	}

}

void CCServerInfoRecv(PMSG_SERVER_INFO_RECV* lpMsg, int index)
{
	if (gServerList.IsJoinServerOnline() == 0)
	{
		return;
	}

	SERVER_LIST_INFO* lpServerListInfo = gServerList.GetGameServerInfo(lpMsg->ServerCode);

	if (lpServerListInfo == nullptr)
	{
		return;
	}

	if (lpServerListInfo->ServerShow == 0 || lpServerListInfo->ServerState == 0)
	{
		return;
	}

	PMSG_SERVER_INFO_SEND pMsg = {};

	pMsg.header.set(MSG_HEADER_TYPE_F4, MSG_HEADER_SERVER_INFO_RECV, static_cast<BYTE>(sizeof(pMsg)));

	memcpy(pMsg.ServerAddress, lpServerListInfo->ServerAddress, sizeof(pMsg.ServerAddress));
	pMsg.ServerPort = lpServerListInfo->ServerPort;

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);
}

void CCServerListRecv(PMSG_SERVER_LIST_RECV* lpMsg, int index)
{
	UNREFERENCED_PARAMETER(lpMsg);

	BYTE send[MAX_MAIN_PACKET_SIZE] = {};

	PMSG_SERVER_LIST_SEND pMsg = {};

	pMsg.header.set(MSG_HEADER_TYPE_F4, MSG_HEADER_SERVER_LIST_RECV, 0);

	int size = static_cast<int>(sizeof(pMsg));
	int count = gServerList.GenerateServerList(send, &size);

	pMsg.count[0] = static_cast<BYTE>(SET_NUMBERHB(static_cast<DWORD>(count)));
	pMsg.count[1] = static_cast<BYTE>(SET_NUMBERLB(static_cast<DWORD>(count)));
	pMsg.header.size[0] = static_cast<BYTE>(SET_NUMBERHB(static_cast<DWORD>(size)));
	pMsg.header.size[1] = static_cast<BYTE>(SET_NUMBERLB(static_cast<DWORD>(size)));

	memcpy(send, &pMsg, sizeof(pMsg));

	gSocketManager.DataSend(index, send, size);
}

void CCServerInitSend(int index, int result) {

	PMSG_SERVER_INIT_SEND pMsg = {}; // Inicializar todos los miembros de pMsg a cero

	pMsg.header.set(MSG_HEADER_SERVER_INIT_SEND, static_cast<BYTE>(sizeof(pMsg))); // Usa la constante MSG_HEADER_SERVER_INIT_SEND

	pMsg.result = static_cast<BYTE>(result);

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);
}
