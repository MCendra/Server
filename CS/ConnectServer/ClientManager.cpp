// ClientManager.cpp
#include "ClientManager.h"
#include "IpManager.h"
#include "Log.h"

CClientManager gClientManager[MAX_CLIENT];

// Cursor circular para busqueda
int gClientSearchStart = 0;

// Seccion critica que protege operaciones sobre gClientCount / busqueda de indices
CCriticalSection gClientArrayLock;

// Construction/Destruction

CClientManager::CClientManager()
	: m_index(-1), m_state(CLIENT_OFFLINE), m_socket(INVALID_SOCKET),
	m_IoRecvContext(nullptr), m_IoSendContext(nullptr),
	m_LastStateChangeTime(0), m_LastPacketTime(0)
{
	memset(m_IpAddr, 0, sizeof(m_IpAddr));
}

CClientManager::~CClientManager()
{

}

bool CClientManager::IsOnline() 
{
	return CLIENT_RANGE(m_index) && m_state != CLIENT_OFFLINE && m_socket != INVALID_SOCKET;
}

// Forzar al compilador a inyectar el código directamente en el bucle para no perder rendimiento
__forceinline bool CClientManager::CheckAlloc()
{
	return m_IoRecvContext && m_IoSendContext;
}

bool CClientManager::IsTimedOut() const
{
	return (GetTickCount64() - m_LastStateChangeTime) > MAX_CLIENT_OFFLINE_TIME_FOR_REUSE;
}

// Agrega un cliente a la lista de clientes activos
void CClientManager::AddClient(int index, char* ip, SOCKET socket)
{
	{
		CCriticalSection::CLock lock(m_lock);

		m_index = index;
		m_state = CLIENT_ONLINE;
		m_socket = socket;

		strcpy_s(m_IpAddr, sizeof(m_IpAddr), ip);

		if (m_IoRecvContext == nullptr)
			m_IoRecvContext = new IO_RECV_CONTEXT;

		if (m_IoSendContext == nullptr)
			m_IoSendContext = new IO_SEND_CONTEXT;

		memset(&m_IoRecvContext->overlapped, 0, sizeof(m_IoRecvContext->overlapped));

		m_IoRecvContext->wsabuf.buf = (char*)m_IoRecvContext->IoMainBuffer.buff;
		m_IoRecvContext->wsabuf.len = MAX_MAIN_PACKET_SIZE;
		m_IoRecvContext->IoType = IO_RECV;
		m_IoRecvContext->IoSize = 0;
		m_IoRecvContext->IoMainBuffer.size = 0;

		memset(&m_IoSendContext->overlapped, 0, sizeof(m_IoSendContext->overlapped));

		m_IoSendContext->wsabuf.buf = (char*)m_IoSendContext->IoMainBuffer.buff;
		m_IoSendContext->wsabuf.len = MAX_MAIN_PACKET_SIZE;
		m_IoSendContext->IoType = IO_SEND;
		m_IoSendContext->IoSize = 0;
		m_IoSendContext->IoMainBuffer.size = 0;
		m_IoSendContext->IoSideBuffer.size = 0;

		m_LastStateChangeTime = GetTickCount64();
		m_LastPacketTime = 0;

	}

	// Insertar IP en el gestor (se asume que gIpManager maneja su propia sincronizacion)
	gIpManager.InsertIpAddress(m_IpAddr);

	// Enviar init al cliente (no se mantiene la seccion critica durante esta llamada)
	CCServerInitSend(m_index, 1);
}

// Elimina un cliente de la lista de clientes activos
void CClientManager::DelClient()
{
	// NOTA: esta función se llama con m_lock YA adquirido desde
	// OnRecv/OnSend. NO intentar tomarlo aquí → deadlock.
	// El contrato es: el caller siempre tiene m_lock antes de llamar.
	if (m_state == CLIENT_OFFLINE)
		return;

	// Marcar OFFLINE primero para que cualquier otra llamada concurrente
	// a DelClient() salga inmediatamente por el check de arriba.
	m_state = CLIENT_OFFLINE;

	m_index = -1;
	m_socket = INVALID_SOCKET; // ya fue cerrado en Disconnect()
	m_LastStateChangeTime = GetTickCount64();
	m_LastPacketTime = 0;
	// Eliminar IP del manager
	gIpManager.RemoveIpAddress(m_IpAddr);
	memset(m_IpAddr, 0, sizeof(m_IpAddr));
}

// Retorna un índice libre en gClientManager[].
// Primero intenta reutilizar un slot con IO contexts ya asignados
// (evita heap alloc en el camino crítico de aceptación).
// Si no hay ninguno elegible, hace búsqueda circular desde gClientSearchStart.
int CClientManager::GetFreeClientIndex()
{
	CCriticalSection::CLock lock(gClientArrayLock);

	int index = SearchFreeClientIndex(0, MAX_CLIENT, MAX_CLIENT_OFFLINE_TIME_FOR_REUSE);

	if (index != -1)
	{
		gClientSearchStart = (index + 1) % MAX_CLIENT;
		return index;
	}

	int start = gClientSearchStart;

	for (int n = 0; n < MAX_CLIENT; n++)
	{
		int i = (start + n) % MAX_CLIENT;

		if (gClientManager[i].m_state == CLIENT_OFFLINE)
		{
			gClientSearchStart = (i + 1) % MAX_CLIENT;
			return i;
		}
	}

	return -1;
}

// Busca slot libre previamente asignado que lleva mas tiempo sin reutilizarse.
// IMPORTANTE:
// Esta funcion debe ejecutarse con gClientArrayLock ya adquirido.
int CClientManager::SearchFreeClientIndex(int MinIndex, int MaxIndex, ULONGLONG MaxTime)
{
	int index = -1;
	ULONGLONG maxOfflineTime = 0;

	for (int n = MinIndex; n < MaxIndex; n++)
	{
		if (gClientManager[n].m_state == CLIENT_OFFLINE && gClientManager[n].CheckAlloc())
		{
			// CORRECCIÓN: Uso de CurOnlineTime - renombrado a curOfflineTime para reflejar mejor su proposito
			ULONGLONG curOfflineTime = GetTickCount64() - gClientManager[n].m_LastStateChangeTime;
			if (curOfflineTime < MaxTime && curOfflineTime > maxOfflineTime)
			{
				index = n;
				maxOfflineTime = curOfflineTime;
			}
		}
	}

	return index;
}

// Clase para gestionar la informacion de cada cliente
void CClientManager::CheckClientTimeouts()
{
	for (int n = 0; n < MAX_CLIENT; n++)
	{
		if (gClientManager[n].IsOnline() && gClientManager[n].IsTimedOut())
		{
			gSocketManager.Disconnect(n);
		}
	}
}
