// ServerManager.cpp:
#include "Header.h"
#include "ServerManager.h"
#include "Log.h"
#include "AccountManager.h"
#include "Util.h"

CServerManager gServerManager[MAX_SERVER];
int            gServerCount = 0;

// Construction/Destruction

CServerManager::CServerManager()
{

	this->m_index = -1;
	this->m_state = SERVER_OFFLINE;
	this->m_socket = INVALID_SOCKET;
	this->m_IoRecvContext = 0;
	this->m_IoSendContext = 0;
	this->m_OnlineTime = 0;
	this->m_PacketTime = 0;

}

CServerManager::~CServerManager()
{

}

bool CServerManager::CheckState()
{

	return SERVER_RANGE(this->m_index) && this->m_state != SERVER_OFFLINE && this->m_socket != INVALID_SOCKET;

}

bool CServerManager::CheckAlloc()
{

	return this->m_IoRecvContext != nullptr && this->m_IoSendContext != nullptr;

}

void CServerManager::AddServer(int index,char* ip,SOCKET socket)
{

	this->m_index = index;
	this->m_state = SERVER_ONLINE;

	strcpy_s(this->m_IpAddr,ip);

	this->m_socket = socket;

	// Avanzar cursor solo si el slot NO tenía IO contexts asignados.
	// Si los tenía (reutilización), el cursor no cambia para no saltar
	// slots que podrían estar libres justo delante.
	if (this->CheckAlloc() == 0)
	{
		gServerCount = (++gServerCount >= MAX_SERVER) ? 0 : gServerCount;
	}

	if (this->m_IoRecvContext == nullptr)
		this->m_IoRecvContext = new IO_RECV_CONTEXT;

	if (this->m_IoSendContext == nullptr)
		this->m_IoSendContext = new IO_SEND_CONTEXT;

	memset(&this->m_IoRecvContext->overlapped, 0, sizeof(this->m_IoRecvContext->overlapped));
	this->m_IoRecvContext->wsabuf.buf = (char*)this->m_IoRecvContext->IoMainBuffer.buff;
	this->m_IoRecvContext->wsabuf.len = MAX_MAIN_PACKET_SIZE;
	this->m_IoRecvContext->IoType = IO_RECV;
	this->m_IoRecvContext->IoSize = 0;
	this->m_IoRecvContext->IoMainBuffer.size = 0;

	memset(&this->m_IoSendContext->overlapped, 0, sizeof(this->m_IoSendContext->overlapped));
	this->m_IoSendContext->wsabuf.buf = (char*)this->m_IoSendContext->IoMainBuffer.buff;
	this->m_IoSendContext->wsabuf.len = MAX_MAIN_PACKET_SIZE;
	this->m_IoSendContext->IoType = IO_SEND;
	this->m_IoSendContext->IoSize = 0;
	this->m_IoSendContext->IoMainBuffer.size = 0;
	this->m_IoSendContext->IoSideBuffer.size = 0;

	memset(this->m_ServerName, 0, sizeof(this->m_ServerName));

	this->m_ServerPort = 0xFFFF;
	this->m_ServerCode = 0xFFFF;
	this->m_OnlineTime = GetTickCount64();   // FIX: era GetTickCount()
	this->m_PacketTime = 0;
	this->m_CurUserCount = 0;
	this->m_MaxUserCount = 0;

	Log.ToDisp(LOG_BLACK, "[ServerManager][%d] AddServer (%s)", this->m_index, this->m_IpAddr);
}

void CServerManager::DelServer()
{
	Log.ToDisp(LOG_BLACK, "[ServerManager][%d] DelServer (%s)", this->m_index, this->m_IpAddr);

	WORD ServerCode = this->m_ServerCode;

	this->m_index = -1;
	this->m_state = SERVER_OFFLINE;

	memset(this->m_IpAddr, 0, sizeof(this->m_IpAddr));
	memset(this->m_ServerName, 0, sizeof(this->m_ServerName));

	this->m_socket = INVALID_SOCKET;
	this->m_ServerPort = 0xFFFF;
	this->m_ServerCode = 0xFFFF;
	this->m_OnlineTime = GetTickCount64();   // FIX: era GetTickCount()
	this->m_PacketTime = 0;
	this->m_CurUserCount = 0;
	this->m_MaxUserCount = 0;

	gAccountManager.ClearServerAccountInfo(ServerCode);
}

void CServerManager::SetServerInfo(char* name, WORD port, WORD code)
{
	strcpy_s(this->m_ServerName, name);
	this->m_ServerPort = port;
	this->m_ServerCode = code;

	Log.ToDisp(LOG_BLACK, "[ServerManager][%d] ServerInfo (%s) (%d) (%d)", this->m_index, this->m_ServerName, this->m_ServerPort, this->m_ServerCode);
}

// =====================================================================
// Funciones de búsqueda de slots
// =====================================================================

// Busca el slot offline con más tiempo (al menos MinTime ms).
// Prioritiza reutilizar slots que ya tienen IO contexts asignados
// (CheckAlloc) para evitar new/delete en cada conexión entrante.
int SearchFreeServerIndex(int MinIndex, int MaxIndex, DWORD MinTime)
{
	int    result = -1;
	ULONGLONG maxTime = 0;
	ULONGLONG curTime = GetTickCount64();

	for (int n = MinIndex; n < MaxIndex; n++)
	{
		if (gServerManager[n].CheckState() == 0 && gServerManager[n].CheckAlloc() != 0)
		{
			ULONGLONG offlineTime = curTime - gServerManager[n].m_OnlineTime;

			if (offlineTime > MinTime && offlineTime > maxTime)
			{
				result = n;
				maxTime = offlineTime;
			}
		}
	}

	return result;
}

// Retorna un índice libre en gServerManager[].
// Primero intenta reutilizar un slot con IO contexts ya asignados
// (evita heap alloc en el camino crítico de aceptación).
// Si no hay ninguno elegible, hace búsqueda circular desde gServerCount.
int GetFreeServerIndex()
{
	// Intento 1: slot offline con IO contexts y suficiente tiempo offline
	int index = SearchFreeServerIndex(0, MAX_SERVER, MIN_OFFLINE_TIME_FOR_REUSE);
	if (index != -1) return index;

	// Intento 2: búsqueda circular desde el cursor
	int start = gServerCount;
	for (int n = 0; n < MAX_SERVER; n++)
	{
		int i = (start + n) % MAX_SERVER;
		if (gServerManager[i].CheckState() == 0)
			return i;
	}

	return -1;
}

// Recorre el array buscando el servidor activo con el código indicado.
CServerManager* FindServerByCode(WORD ServerCode)
{
	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].CheckState() != 0 && gServerManager[n].m_ServerCode == ServerCode)
		{
			return &gServerManager[n];
		}
	}
	return nullptr;
}
