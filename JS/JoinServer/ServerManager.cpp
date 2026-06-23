// ServerManager.cpp:
#include "Header.h"
#include "ServerManager.h"
#include "Log.h"
#include "AccountManager.h"
#include "Util.h"

// Mensajes de log
constexpr char ADDSERVER_SUCCESS_MSG[] = "[ServerManager - AddServer] Servidor agregado (Index: %d, IP: %s)";
constexpr char DELSERVER_SUCCESS_MSG[] = "[ServerManager - DelServer] Servidor eliminado (Index: %d, IP: %s)";

CServerManager gServerManager[MAX_SERVER];

// Cursor circular para busqueda
int gServerSearchStart = 0;

// Seccion critica que protege operaciones sobre gServerCount / busqueda de indices
CCriticalSection gServerArrayLock;

// Construction/Destruction

CServerManager::CServerManager()
	: m_index(-1), m_state(SERVER_OFFLINE), m_socket(INVALID_SOCKET),
	m_IoRecvContext(nullptr), m_IoSendContext(nullptr),
	m_ServerPort(0xFFFF), m_ServerCode(0xFFFF),
	m_LastStateChangeTime(0), m_LastPacketTime(0), m_CurUserCount(0), m_MaxUserCount(0)
{
	memset(this->m_IpAddr, 0, sizeof(this->m_IpAddr));
	memset(this->m_ServerName, 0, sizeof(this->m_ServerName));
}

CServerManager::~CServerManager()
{

}

bool CServerManager::IsOnline()
{
	return SERVER_RANGE(this->m_index) && this->m_state != SERVER_OFFLINE && this->m_socket != INVALID_SOCKET;
}

// Forzar al compilador a inyectar el código directamente en el bucle para no perder rendimiento
__forceinline bool CServerManager::CheckAlloc()
{
	return this->m_IoRecvContext && this->m_IoSendContext;
}

// Agrega un nuevo servidor a la lista de servidores activos
void CServerManager::AddServer(int index,char* ip,SOCKET socket)
{
	{
		CCriticalSection::CLock lock(this->m_lock);

		this->m_index = index;
		this->m_state = SERVER_ONLINE;
		this->m_socket = socket;

		strcpy_s(this->m_IpAddr, sizeof(this->m_IpAddr), ip);

		// Avanzar el cursor circular solo cuando el slot no tenía IO contexts
		// previamente asignados (primera conexión en este slot, no reutilización).
		// Si reutilizamos un slot ya allocado, el cursor no cambia: el slot
		// reutilizado puede estar en cualquier posición del array, no
		// necesariamente adyacente al cursor actual.
		const bool firstAlloc = !this->CheckAlloc();

		if (this->m_IoRecvContext == nullptr)
			this->m_IoRecvContext = new IO_RECV_CONTEXT;

		if (this->m_IoSendContext == nullptr)
			this->m_IoSendContext = new IO_SEND_CONTEXT;

		if (firstAlloc)
		{
			CCriticalSection::CLock arrayLock(gServerArrayLock);
			gServerSearchStart = (this->m_index + 1) % MAX_SERVER;
		}

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
		this->m_LastStateChangeTime = GetTickCount64();
		this->m_LastPacketTime = 0;
		this->m_CurUserCount = 0;
		this->m_MaxUserCount = 0;
	}

	Log.ToDisp(LOG_BLACK, ADDSERVER_SUCCESS_MSG, this->m_index, this->m_IpAddr);
}

// Elimina un servidor de la lista de servidores activos
void CServerManager::DelServer()
{
	// NOTA: esta función se llama con m_lock YA adquirido desde
	// OnRecv/OnSend. NO intentar tomarlo aquí → deadlock.
	// El contrato es: el caller siempre tiene m_lock antes de llamar.
	if (this->m_state == SERVER_OFFLINE)
		return;

	// Marcar OFFLINE primero para que cualquier otra llamada concurrente
	// a DelClient() salga inmediatamente por el check de arriba.
	this->m_state = SERVER_OFFLINE;
	this->m_index = -1;

	Log.ToDisp(LOG_BLACK, DELSERVER_SUCCESS_MSG, this->m_index, this->m_IpAddr);
	// Eliminar info de las cuentas servidor del manager
	gAccountManager.ClearServerAccountInfo(this->m_ServerCode);
	memset(this->m_IpAddr, 0, sizeof(this->m_IpAddr));
	memset(this->m_ServerName, 0, sizeof(this->m_ServerName));
	this->m_socket = INVALID_SOCKET;
	this->m_ServerPort = 0xFFFF;
	this->m_ServerCode = 0xFFFF;
	this->m_LastStateChangeTime = GetTickCount64();
	this->m_LastPacketTime = 0;
	this->m_CurUserCount = 0;
	this->m_MaxUserCount = 0;
}

void CServerManager::SetServerInfo(char* name, WORD port, WORD code)
{
	strcpy_s(this->m_ServerName, name);
	this->m_ServerPort = port;
	this->m_ServerCode = code;

	Log.ToDisp(LOG_BLACK, "[ServerManager][%d] ServerInfo (%s) (%d) (%d)", this->m_index, this->m_ServerName, this->m_ServerPort, this->m_ServerCode);
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


// Recorre el array buscando el servidor activo con el código indicado.
CServerManager* FindServerByCode(int ServerCode)
{
	for (int n = 0; n < MAX_SERVER; n++)
	{
		if (gServerManager[n].IsOnline() == true && gServerManager[n].m_ServerCode == ServerCode)
		{
			return &gServerManager[n];
		}
	}
	return nullptr;
}
