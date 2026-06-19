// ServerManager.h
#pragma once
#include "SocketManager.h"

#define MAX_SERVER 400
#define SERVER_RANGE(x) (((x)<0)?0:((x)>=MAX_SERVER)?0:1)

// Tiempo minimo (ms) que un slot debe llevar offline para ser
// considerado "reutilizable" por SearchFreeServerIndex.
#define MIN_OFFLINE_TIME_FOR_REUSE 10000

enum eServerState
{
	SERVER_OFFLINE = 0,
	SERVER_ONLINE = 1,
};

class CServerManager
{
public:
	CServerManager();
	virtual ~CServerManager();
	bool CheckState();
	bool CheckAlloc();
	void AddServer(int index,char* ip,SOCKET socket);
	void DelServer();
	void SetServerInfo(char* name,WORD port,WORD code);
	// Busca entre MinIndex y MaxIndex el slot que lleve más tiempo offline
	// (al menos MinTime ms). Devuelve el índice encontrado o -1 si no hay ninguno.
	// Es el equivalente de SearchFreeClientIndex en ClientManager de ConnectServer.
	int SearchFreeServerIndex(int MinIndex, int MaxIndex, DWORD MinTime);

	// Retorna el índice de un slot libre en gServerManager[]:
	// primero intenta reutilizar un slot offline con IO contexts ya asignados
	// (evita new/delete); si no encuentra ninguno, hace búsqueda circular
	// desde gServerCount.
	int GetFreeServerIndex();

	// Recorre gServerManager[] y retorna el primero cuyo ServerCode coincida.
	// Retorna nullptr si ningún servidor activo tiene ese código.
	CServerManager* FindServerByCode(WORD ServerCode);
public:
	int m_index;
	eServerState m_state;
	char m_IpAddr[16];
	SOCKET m_socket;
	IO_RECV_CONTEXT* m_IoRecvContext;
	IO_SEND_CONTEXT* m_IoSendContext;
	char m_ServerName[32];
	WORD m_ServerPort;
	WORD m_ServerCode;
	ULONGLONG m_OnlineTime;
	ULONGLONG m_PacketTime;
	int m_CurUserCount;
	int m_MaxUserCount;
};

extern CServerManager gServerManager[MAX_SERVER];
extern int            gServerCount;   // cursor circular de búsqueda de slots libres
