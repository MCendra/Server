// ClientManager.cpp
#include "ClientManager.h"
#include "IpManager.h"
#include "Log.h"

CClientManager gClientManager[MAX_CLIENT];

int gClientSearchStart = 0; // cursor circular para busqueda, no contador

// Seccion critica que protege operaciones sobre gClientCount / busqueda de indices
CCriticalSection gClientArrayLock;

// Construction/Destruction


CClientManager::CClientManager()
	: m_index(-1), m_state(CLIENT_OFFLINE), m_socket(INVALID_SOCKET),
	m_IoRecvContext(nullptr), m_IoSendContext(nullptr),
	m_OnlineTime(0), m_PacketTime(0)
{
	memset(this->m_IpAddr, 0, sizeof(this->m_IpAddr));
}

CClientManager::~CClientManager()
{

}

bool CClientManager::CheckState() 
{
	return CLIENT_RANGE(this->m_index)
		&& this->m_state != CLIENT_OFFLINE
		&& this->m_socket != INVALID_SOCKET;
}

bool CClientManager::CheckAlloc()
{
	return this->m_IoRecvContext && this->m_IoSendContext;
}

bool CClientManager::CheckOnlineTime() const
{
	return (GetTickCount64() - this->m_OnlineTime) <= MAX_ONLINE_TIME; // Cambio a GetTickCount64
}

void CClientManager::AddClient(int index, char* ip, SOCKET socket)
{
	{
		// Proteger indice de clientes
		CCriticalSection::CLock lock(this->m_lock);

		// CORRECCIÓN: strcpy_s: incluir tamaño del destino
		strcpy_s(this->m_IpAddr, sizeof(this->m_IpAddr), ip);
		bool FirstAllocation = false;

		// Asignacion de contextos IO (reserva si no existe)
		if (this->m_IoRecvContext == nullptr)
		{
			this->m_IoRecvContext = new IO_RECV_CONTEXT;
			FirstAllocation = true;
		}

		if (this->m_IoSendContext == nullptr)
		{
			this->m_IoSendContext = new IO_SEND_CONTEXT;
			FirstAllocation = true;
		}
		// Actualizamos la estructura del cliente. La modificacion de gClientCount se protege.
		this->m_index = index;
		this->m_state = CLIENT_ONLINE;
		this->m_socket = socket;

		{
			// FIX C4456: renombrado de "lock" a "arrayLock" para evitar
			// shadowing del lock externo (this->m_lock) en este scope anidado.
			CCriticalSection::CLock arrayLock(gClientArrayLock);

			// Avanzar el cursor circular para la proxima busqueda de slot.
			// gClientSearchStart no es un contador de clientes.
			gClientSearchStart = (this->m_index + 1) % MAX_CLIENT;
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

		this->m_OnlineTime = GetTickCount64();
		this->m_PacketTime = 0;

	}

	// Insertar IP en el gestor (se asume que gIpManager maneja su propia sincronizacion)
	gIpManager.InsertIpAddress(this->m_IpAddr);

	// Enviar init al cliente (no se mantiene la seccion critica durante esta llamada)
	CCServerInitSend(this->m_index, 1);
}

void CClientManager::DelClient()
{

	// NOTA: esta función se llama con m_lock YA adquirido desde
	// OnRecv/OnSend. NO intentar tomarlo aquí → deadlock.
	// El contrato es: el caller siempre tiene m_lock antes de llamar.

	// Idempotente: si ya está OFFLINE, no hace nada.
	// Esto protege el caso donde tanto OnRecv como OnSend reciben
	// IoSize=0 para el mismo cliente (puede pasar si había un recv
	// y un send pendientes simultáneamente al cerrar el socket).
	if (this->m_state == CLIENT_OFFLINE)
		return;

	// Marcar OFFLINE primero para que cualquier otra llamada concurrente
	// a DelClient() salga inmediatamente por el check de arriba.
	this->m_state = CLIENT_OFFLINE;

	// Eliminar IP del manager
	gIpManager.RemoveIpAddress(this->m_IpAddr);

	this->m_index = -1;
	this->m_state = CLIENT_OFFLINE;
	memset(this->m_IpAddr, 0, sizeof(this->m_IpAddr));
	this->m_socket = INVALID_SOCKET; // ya fue cerrado en Disconnect()
	this->m_OnlineTime = GetTickCount64();
	this->m_PacketTime = 0;

	// Liberar memoria de los contextos de IO (seguro)
	if (this->m_IoRecvContext != nullptr)
	{
		delete this->m_IoRecvContext;
		this->m_IoRecvContext = nullptr;
	}
	if (this->m_IoSendContext != nullptr)
	{
		delete this->m_IoSendContext;
		this->m_IoSendContext = nullptr;
	}
}

int CClientManager::GetFreeClientIndex()
{
	// Proteger el recorrido para evitar lecturas inconsistentes del array de clientes
	CCriticalSection::CLock lock(gClientArrayLock);

	// Primero busca slot reutilizable (con IO contexts ya asignados)
	int index = SearchFreeClientIndex(0, MAX_CLIENT, 10000);
	if (index != -1)
	{
		return index;
	}

	// Búsqueda circular iniciando desde gClientSearchStart
	int start = gClientSearchStart;
	for (int n = 0; n < MAX_CLIENT; n++)
	{
		int i = (start + n) % MAX_CLIENT;
		if (gClientManager[i].m_state == CLIENT_OFFLINE)
		{
			return i;
		}
	}

	return -1;
}

// Busca slot libre previamente asignado que lleva mas tiempo sin reutilizarse.
// IMPORTANTE:
// Esta funcion debe ejecutarse con gClientArrayLock ya adquirido.
int CClientManager::SearchFreeClientIndex(int MinIndex, int MaxIndex, ULONGLONG MinTime)
{
	ULONGLONG MaxIdleTime = 0;
	int index = -1;

	// Proteger el recorrido para evitar lecturas inconsistente del array de clientes
	// CCriticalSection::CLock lock(gClientArrayLock);

	for (int n = MinIndex; n < MaxIndex; n++)
	{
		if (gClientManager[n].m_state == CLIENT_OFFLINE && gClientManager[n].m_IoRecvContext != nullptr && gClientManager[n].m_IoSendContext != nullptr)
		{
			// CORRECCIÓN: Uso de CurOnlineTime - renombrado a CurIdleTime para reflejar mejor su proposito
			ULONGLONG CurIdleTime = GetTickCount64() - gClientManager[n].m_OnlineTime;
			if (CurIdleTime > MinTime && CurIdleTime > MaxIdleTime)
			{
				index = n;
				MaxIdleTime = CurIdleTime;
			}
		}
	}

	return index;
}
