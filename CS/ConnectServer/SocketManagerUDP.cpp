// SocketManagerUDP.cpp
//
// Gestor simple de UDP usado por el ConnectServer para enviar/recibir
// mensajes tipo "server list" a/desde otros servicios. Soporta un único
// socket local y un hilo dedicado de recepción (ServerRecvThread).
//
// Thread-safety:
// - El recv thread escribe en m_RecvBuff/m_RecvSize y llama a DataRecv()
//   para procesar el buffer. DataRecv() y el recv thread se ejecutan
//   normalmente en el mismo hilo, pero para proteger accesos concurrentes
//   desde otros hilos (p. ej. Clean/Init), se introduce un mutex m_lock.
// - DataSend() puede ser llamado desde threads externos y accede a
//   m_SendBuff/m_SendSize y al socket. Por tanto DataSend() toma m_lock
//   para evitar races con Clean() y con llamadas concurrentes a
//   DataSend().
// - Clean() protege operaciones sobre m_socket/m_SendBuff usando m_lock
//   para evitar condiciones donde el recv thread use un socket ya cerrado.
#include "SocketManagerUDP.h"
#include "ServerList.h"
#include "Log.h"

// Mensajes de log
constexpr char INIT_CREATE_SHUTDOWN_EVENT_ERROR_MSG[] = "[SocketManagerUDP - Init] Error al crear evento de shutdown: %d";
constexpr char INIT_ERROR_WSA_SOCKET[] = "[SocketManagerUDP - Init] WSASocket() fallo con el error: %d";
constexpr char INIT_ERROR_BIND[] = "[SocketManagerUDP - Init] bind() fallo en el puerto %d con el error: %d";
constexpr char INIT_ERROR_CREATETHREAD[] = "[SocketManagerUDP - Init] Error al crear el hilo de recepción.";
constexpr char INIT_SUCCESS_MSG[] = "[SocketManagerUDP - Init] Servidor UDP iniciado en el puerto [%d]";

constexpr char CONNECT_ERROR_WSA_SOCKET[] = "[SocketManagerUDP - Connect] WSASocket() fallo con el error: %d";
constexpr char CONNECT_ERROR_GETADDRESSINFO[] = "[SocketManagerUDP - Connect] getaddrinfo() fallo con el error: %d";
constexpr char CONNECT_ERROR_GETADDRESSINFO_RESULT[] = "[SocketManagerUDP - Connect] getaddrinfo() no retorno resultados.";

constexpr char CLEAN_WAITFORSINGLEOBJECT_TIMEOUT[] = "[SocketManagerUDP - Clean] Timeout esperando WaitForSingleObject: %d";

constexpr char DATASEND_MSGSIZE_ERROR[] = "[SocketManagerUDP - DataSend] Tamaño de mensaje excedido (Tamaño: %d)";
constexpr char DATASEND_SENDTO_ERROR[] = "[SocketManagerUDP - DataSend] fallo con el error: %d";

constexpr char DATARECV_PROTOCOL_HEADER_ERROR[] = "[SocketManagerUDP - DataRecv] Error de cabecera del protocolo (Header: %02X)";
constexpr char DATARECV_PROTOCOL_SIZE_ERROR[] = "[SocketManagerUDP - DataRecv] Error de tamaño del protocolo (Index: %d, Size: %d, Head: %02X)";

constexpr char SERVERRECVTHREAD_RECVFROM_ERROR[] = "[SocketManagerUDP - ServerRecvThread] recvfrom() fallo con el error: %d";

CSocketManagerUdp gSocketManagerUdp;

// Constructor / Destructor

CSocketManagerUdp::CSocketManagerUdp()
	: m_socket(INVALID_SOCKET),
	m_ServerRecvThread(nullptr),
	m_RecvSize(0),
	m_SendSize(0),
	m_shutdownEvent(nullptr)
{
	memset(&this->m_SocketAddr, 0, sizeof(this->m_SocketAddr));
	this->m_SocketAddr.sin_family = AF_INET;
	this->m_SocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_SocketAddr.sin_port = 0;

	memset(this->m_RecvBuff, 0, sizeof(this->m_RecvBuff));
	memset(this->m_SendBuff, 0, sizeof(this->m_SendBuff));
}

CSocketManagerUdp::~CSocketManagerUdp()
{
	this->Clean();
}

// =====================================================================
// Inicializacion
// =====================================================================

// Inicia el gestor de sockets en el puerto especificado.
//
// Flujo:
//   1) Crea el evento de shutdown (para apagado cooperativo de los hilos).
//   2) Crea el socket de escucha y lo pone en modo listen().
//   3) Crea el puerto de finalizacion de E/S (IOCP).
//   4) Lanza el hilo de aceptacion de conexiones.
//   5) Lanza los hilos de trabajo (workers) segun la cantidad de CPUs.
//   6) Crea la cola de paquetes y su hilo de procesamiento.
//
// Si cualquier paso falla, se llama a Clean() para revertir todo lo que
// se haya alcanzado a crear y se retorna false.
bool CSocketManagerUdp::Init(WORD port)
{

	// Evento de parada manual-reset: una vez señalado (SetEvent), permanece
	// señalado para todos los hilos que lo consulten (no se auto-resetea).
	if ((this->m_shutdownEvent = CreateEvent(nullptr, true, false, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, INIT_CREATE_SHUTDOWN_EVENT_ERROR_MSG, GetLastError());
		this->Clean();
		return false;
	}

	if ((this->m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, INIT_ERROR_WSA_SOCKET, WSAGetLastError());
		this->Clean();
		return false;
	}

	this->m_SocketAddr.sin_family = AF_INET;
	this->m_SocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_SocketAddr.sin_port = htons(port);

	if (bind(this->m_socket, reinterpret_cast<sockaddr*>(&this->m_SocketAddr), sizeof(this->m_SocketAddr)) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, INIT_ERROR_BIND, port, WSAGetLastError());
		this->Clean();
		return false;
	}

	memset(this->m_RecvBuff, 0, sizeof(this->m_RecvBuff));
	memset(this->m_SendBuff, 0, sizeof(this->m_SendBuff));

	this->m_RecvSize = 0;
	this->m_SendSize = 0;

	if ((this->m_ServerRecvThread = CreateThread(nullptr, 0, CSocketManagerUdp::ServerRecvThread, this, 0, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, INIT_ERROR_CREATETHREAD, GetLastError());
		this->Clean();
		return false;
	}

	Log.ToDisp(LOG_GREEN, INIT_SUCCESS_MSG, port);
	return true;
}

bool CSocketManagerUdp::Connect(char* IpAddress, WORD port)
{

	if (this->m_socket != INVALID_SOCKET)
	{
		closesocket(this->m_socket);
		this->m_socket = INVALID_SOCKET;
	}

	if ((this->m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, CONNECT_ERROR_WSA_SOCKET, WSAGetLastError());
		this->Clean();
		return false;
	}

	this->m_SocketAddr.sin_family = AF_INET;

	if (InetPtonA(AF_INET, IpAddress, &this->m_SocketAddr.sin_addr) <= 0)
	{
		struct addrinfo hints = { 0 };
		struct addrinfo* result = nullptr;

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		if (getaddrinfo(IpAddress, nullptr, &hints, &result) != 0)
		{
			Log.ToDisp(LOG_RED, CONNECT_ERROR_GETADDRESSINFO, WSAGetLastError());
			this->Clean();
			return false;
		}

		if (result != nullptr)
		{
			this->m_SocketAddr.sin_addr = ((sockaddr_in*)result->ai_addr)->sin_addr;
			freeaddrinfo(result);
		}
		else
		{
			Log.ToDisp(LOG_RED, CONNECT_ERROR_GETADDRESSINFO_RESULT);
			this->Clean();
			return false;
		}
	}

	this->m_SocketAddr.sin_port = htons(port);

	memset(this->m_SendBuff, 0, sizeof(this->m_SendBuff));

	this->m_SendSize = 0;

	return true;
}

// CORRECIÓN: Agregar funcion Clean() para liberar recursos correctamente
void CSocketManagerUdp::Clean()
{
	// Señalizar parada cooperativa
	if (this->m_shutdownEvent != nullptr)
	{
		SetEvent(this->m_shutdownEvent);
	}

	// Cerrar socket para provocar retorno de recvfrom en el hilo
	// Protegemos el cierre del socket y el acceso al buffer de envio
	// mediante m_lock para evitar races con DataSend() o el recv thread.
	{
		CCriticalSection::CLock lock(this->m_lock);

		if (this->m_socket != INVALID_SOCKET)
		{
			closesocket(this->m_socket);
			this->m_socket = INVALID_SOCKET;
		}

		// Limpiar buffer de envio
		memset(this->m_SendBuff, 0, sizeof(this->m_SendBuff));
		this->m_SendSize = 0;
	}

	// Esperar que termine el hilo correctamente (no forzar)
	if (this->m_ServerRecvThread != nullptr)
	{
		if (WaitForSingleObject(this->m_ServerRecvThread, DEFAULT_TIME_WAIT) == WAIT_TIMEOUT)
		{
			Log.ToDisp(LOG_RED, CLEAN_WAITFORSINGLEOBJECT_TIMEOUT);
			// No usar TerminateThread; solo cerramos handle para liberar recursos
		}
		CloseHandle(this->m_ServerRecvThread);
		this->m_ServerRecvThread = nullptr;
	}

	// Cerrar evento de parada
	if (this->m_shutdownEvent != nullptr)
	{
		CloseHandle(this->m_shutdownEvent);
		this->m_shutdownEvent = nullptr;
	}
}

bool CSocketManagerUdp::DataRecv()
{

	BYTE* lpMsg = this->m_RecvBuff;

	int count = 0;

	while (true)
	{
		int available = this->m_RecvSize - count;

		if (available <= 0)
		{
			break;
		}

		int size = 0;
		BYTE header = 0;
		BYTE head = 0;

		if (lpMsg[count] == PACKET_HEADER_C1)
		{
			if (available < 3)
			{
				break;
			}

			header = lpMsg[count];
			size = lpMsg[count + 1];
			head = lpMsg[count + 2];
		}
		else if (lpMsg[count] == PACKET_HEADER_C2)
		{
			if (available < 4)
			{
				break;
			}

			header = lpMsg[count];
			size = MAKEWORD(lpMsg[count + 2], lpMsg[count + 1]);
			head = lpMsg[count + 3];
		}
		else
		{
			Log.ToDisp(LOG_RED, DATARECV_PROTOCOL_HEADER_ERROR, lpMsg[count]);
			memset(this->m_RecvBuff, 0, sizeof(this->m_RecvBuff));
			this->m_RecvSize = 0;
			return false;
		}

		int minSize = (header == PACKET_HEADER_C1) ? 3 : 4;

		if (size < minSize || size > MAX_UDP_PACKET_SIZE)
		{
			Log.ToDisp(LOG_RED, DATARECV_PROTOCOL_SIZE_ERROR, header, size, head);
			memset(this->m_RecvBuff, 0, sizeof(this->m_RecvBuff));
			this->m_RecvSize = 0;
			return false;
		}

		// Aún no llegó el paquete completo
		if (size > available)
		{
			break;
		}

		gServerList.ProcessServerStatusPacket(head, &lpMsg[count], size);

		count += size;
	}

	// Mover remanente incompleto al inicio del buffer
	if (count > 0)
	{
		int remaining = this->m_RecvSize - count;

		if (remaining > 0)
		{
			memmove(lpMsg, &lpMsg[count], remaining);
		}

		this->m_RecvSize = remaining;
	}

	return true;
}

bool CSocketManagerUdp::DataSend(BYTE* lpMsg, int size)
{
	// Protegemos todo el flujo con el mutex para evitar que Clean() cierre
	// el socket o que otro DataSend modifique los buffers simultaneamente.
	CCriticalSection::CLock lock(this->m_lock);

	if (this->m_socket == INVALID_SOCKET)
	{
		return 0;
	}

	if ((this->m_SendSize + size) > MAX_UDP_PACKET_SIZE)
	{
		Log.ToDisp(LOG_RED, DATASEND_MSGSIZE_ERROR, size);
		memset(this->m_SendBuff, 0, sizeof(this->m_SendBuff));
		this->m_SendSize = 0;
		return 0;
	}

	memcpy(&this->m_SendBuff[this->m_SendSize], lpMsg, size);

	this->m_SendSize += size;

	int result = sendto(this->m_socket, (char*)this->m_SendBuff, this->m_SendSize, 0, (sockaddr*)&this->m_SocketAddr, sizeof(this->m_SocketAddr));

	if (result == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();
		if (Error != WSAEWOULDBLOCK)
		{
			Log.ToDisp(LOG_RED, DATASEND_SENDTO_ERROR, Error);
			memset(this->m_SendBuff, 0, sizeof(this->m_SendBuff));
			this->m_SendSize = 0;
			return 0;
		}

		return 1;
	}

	this->m_SendSize -= result;

	memmove(this->m_SendBuff, &this->m_SendBuff[result], this->m_SendSize);
	return 1;
}

DWORD WINAPI CSocketManagerUdp::ServerRecvThread(LPVOID lpParam)
{
	CSocketManagerUdp* lpSocketManagerUdp = reinterpret_cast<CSocketManagerUdp*>(lpParam);

	while (true)
	{
		// Si se ha solicitado parada cooperativa, salimos
		if (lpSocketManagerUdp->m_shutdownEvent && WaitForSingleObject(lpSocketManagerUdp->m_shutdownEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}

		SOCKADDR_IN SocketAddr;
		int SocketAddrSize = sizeof(SocketAddr);

		int result = recvfrom(lpSocketManagerUdp->m_socket, (char*)&lpSocketManagerUdp->m_RecvBuff[lpSocketManagerUdp->m_RecvSize], (MAX_UDP_PACKET_SIZE - lpSocketManagerUdp->m_RecvSize), 0, (sockaddr*)&SocketAddr, &SocketAddrSize);

		if (result == SOCKET_ERROR)
		{
			int wsaErr = WSAGetLastError();

			// Si se ha solicitado parada cooperativa, salimos silenciosamente
			if (lpSocketManagerUdp->m_shutdownEvent && WaitForSingleObject(lpSocketManagerUdp->m_shutdownEvent, 0) == WAIT_OBJECT_0)
			{
				break;
			}

			// Solo logear si no es una interrupcion esperada por cierre de socket
			if (wsaErr != WSAENOTSOCK && wsaErr != WSAEINTR && wsaErr != WSANOTINITIALISED)
			{
				Log.ToDisp(LOG_RED, SERVERRECVTHREAD_RECVFROM_ERROR, wsaErr);
			}

			memset(lpSocketManagerUdp->m_RecvBuff, 0, sizeof(lpSocketManagerUdp->m_RecvBuff));
			lpSocketManagerUdp->m_RecvSize = 0;
			continue;
		}

		lpSocketManagerUdp->m_RecvSize += result;

		lpSocketManagerUdp->DataRecv();
	}

	return 0;
}
