// ServerManager.cpp:
#include "Header.h"
#include "ServerManager.h"
#include "Log.h"
#include "CharacterManager.h"
#include "GuildMatching.h"
#include "PartyMatching.h"

CServerManager gServerManager[MAX_SERVER];
// Seccion critica que protege operaciones sobre gServerCount / busqueda de indices
CCriticalSection gServerArrayLock;

// Construction/Destruction

CServerManager::CServerManager()
	: m_index(-1), m_state(SERVER_OFFLINE), m_socket(INVALID_SOCKET),
	m_IoRecvContext(nullptr), m_IoSendContext(nullptr),
	m_ServerPort(0xFFFF), m_ServerCode(0xFFFF),
	m_LastStateChangeTime(0), m_LastPacketTime(0)
{
	memset(m_IpAddr, 0, sizeof(m_IpAddr));
	memset(m_ServerName, 0, sizeof(m_ServerName));
}

CServerManager::~CServerManager()
{

}

bool CServerManager::IsOnline()
{
	return SERVER_RANGE(m_index) && m_state != SERVER_OFFLINE && m_socket != INVALID_SOCKET;
}

// Forzar al compilador a inyectar el código directamente en el bucle para no perder rendimiento
__forceinline bool CServerManager::CheckAlloc()
{
	return m_IoRecvContext && m_IoSendContext;
}

// Agrega un nuevo servidor a la lista de servidores activos
void CServerManager::AddServer(int index, char* ip, SOCKET socket)
{
	{
		CCriticalSection::CLock lock(m_lock);

		m_index = index;
		m_state = SERVER_ONLINE;
		m_socket = socket;

		strcpy_s(m_IpAddr, sizeof(m_IpAddr), ip);

		// Avanzar el cursor circular solo cuando el slot no tenía IO contexts
		// previamente asignados (primera conexión en este slot, no reutilización).
		// Si reutilizamos un slot ya allocado, el cursor no cambia: el slot
		// reutilizado puede estar en cualquier posición del array, no
		// necesariamente adyacente al cursor actual.
		const bool firstAlloc = !CheckAlloc();

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

		memset(m_ServerName, 0, sizeof(m_ServerName));

		m_ServerPort = 0xFFFF;
		m_ServerCode = 0xFFFF;
		m_LastStateChangeTime = GetTickCount64();
		m_LastPacketTime = 0;
	}

	Log.ToDisp(LOG_BLACK, "[ServerManager - AddServer] Servidor agregado (Index: %d, IP: %s)", m_index, m_IpAddr);
}

// Elimina un servidor de la lista de servidores activos
void CServerManager::DelServer()
{
	// NOTA: esta función se llama con m_lock YA adquirido desde
	// OnRecv/OnSend. NO intentar tomarlo aquí → deadlock.
	// El contrato es: el caller siempre tiene m_lock antes de llamar.
	if (m_state == SERVER_OFFLINE)
		return;

	// Marcar OFFLINE primero para que cualquier otra llamada concurrente
	// a DelClient() salga inmediatamente por el check de arriba.
	m_state = SERVER_OFFLINE;
	m_index = -1;

	Log.ToDisp(LOG_BLACK, "[ServerManager - DelServer] Servidor eliminado (Index: %d, IP: %s)", m_index, m_IpAddr);
	// Eliminar info de las cuentas servidor del manager
	gCharacterManager.ClearServerCharacterInfo(m_ServerCode);
	gGuildMatching.ClearGuildMatchingInfo(m_ServerCode);
	gGuildMatching.ClearGuildMatchingJoinInfo(m_ServerCode);
	gPartyMatching.ClearPartyMatchingInfo(m_ServerCode);
	gPartyMatching.ClearPartyMatchingJoinInfo(m_ServerCode);

	memset(m_IpAddr, 0, sizeof(m_IpAddr));
	memset(m_ServerName, 0, sizeof(m_ServerName));
	m_socket = INVALID_SOCKET;
	m_ServerPort = 0xFFFF;
	m_ServerCode = 0xFFFF;
	m_LastStateChangeTime = GetTickCount64();
	m_LastPacketTime = 0;
}

void CServerManager::SetServerInfo(char* name, WORD port, WORD code)
{
	strcpy_s(m_ServerName, name);
	m_ServerPort = port;
	m_ServerCode = code;

	Log.ToDisp(LOG_BLACK, "[ServerManager][%d] ServerInfo (%s) (%d) (%d)", m_index, m_ServerName, m_ServerPort, m_ServerCode);
}

// Retorna un índice libre en gServerManager[].
// Primero intenta reutilizar un slot con IO contexts ya asignados
// (evita heap alloc en el camino crítico de aceptación).
// Si no hay ninguno elegible, hace búsqueda circular desde gServerSearchStart.
int CServerManager::GetFreeServerIndex()
{
	// Proteger el recorrido para evitar lecturas inconsistentes del array de clientes
	CCriticalSection::CLock lock(gServerArrayLock);

	// Intento 1: slot offline con IO contexts y suficiente tiempo offline
	int index = SearchFreeServerIndex(0, MAX_SERVER, MAX_SERVER_OFFLINE_TIME_FOR_REUSE);
	if (index != -1) return index;

	// Intento 2: búsqueda circular desde el cursor
	int start = gServerSearchStart;
	for (int n = 0; n < MAX_SERVER; n++)
	{
		int i = (start + n) % MAX_SERVER;
		if (gServerManager[i].m_state == SERVER_OFFLINE)
		{
			return i;
		}
	}

	return -1;
}

// Busca slot libre previamente asignado que lleva mas tiempo sin reutilizarse.
// IMPORTANTE:
// Esta funcion debe ejecutarse con gServerArrayLock ya adquirido.
int CServerManager::SearchFreeServerIndex(int MinIndex, int MaxIndex, DWORD MaxTime)
{
	int index = -1;
	ULONGLONG maxOfflineTime = 0;

	for (int n = MinIndex; n < MaxIndex; n++)
	{
		if (gServerManager[n].m_state == SERVER_OFFLINE && gServerManager[n].CheckAlloc())
		{
			// CORRECCIÓN: Uso de CurOnlineTime - renombrado a curOfflineTime para reflejar mejor su proposito
			ULONGLONG CurOfflineTime = GetTickCount64() - gServerManager[n].m_LastStateChangeTime;
			if (CurOfflineTime < MaxTime && CurOfflineTime > maxOfflineTime)
			{
				index = n;
				maxOfflineTime = CurOfflineTime;
			}
		}
	}

	return index;
}