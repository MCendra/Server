// SocketManagerUDP.cpp
#include "SocketManagerUDP.h"
#include "ServerList.h"
#include "Log.h"

constexpr char SOCKET_INIT_ERROR[] = "[SocketManagerUdp] socket() fallo con error: %d";
constexpr char SOCKET_BIND_ERROR[] = "[SocketManagerUdp] bind() fallo con error: %d";
constexpr char THREAD_CREATION_ERROR[] = "[SocketManagerUdp] CreateThread() fallo con error: %d";
constexpr char CREATE_SHUTDOWN_EVENT_ERROR[] = "[SocketManagerUdp] CreateEvent() fallo con error: %d";
constexpr char SERVERRECVTHREAD_STOP_TIMEOUT[] = "[SocketManagerUdp] Timeout esperando ServerRecvThread al detenerse.";

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

bool CSocketManagerUdp::Init(WORD port)
{
	// Crear evento de parada (manual-reset)
	if ((this->m_shutdownEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, CREATE_SHUTDOWN_EVENT_ERROR, GetLastError());
		this->Clean();
		return false;
	}

	if ((this->m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, SOCKET_INIT_ERROR, WSAGetLastError());
		this->Clean();
		return false;
	}

	this->m_SocketAddr.sin_family = AF_INET;
	this->m_SocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	this->m_SocketAddr.sin_port = htons(port);

	if (bind(this->m_socket, reinterpret_cast<sockaddr*>(&this->m_SocketAddr), sizeof(this->m_SocketAddr)) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, SOCKET_BIND_ERROR, WSAGetLastError());
		this->Clean();
		return false;
	}

	if ((this->m_ServerRecvThread = CreateThread(nullptr, 0, CSocketManagerUdp::ServerRecvThread, this, 0, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, THREAD_CREATION_ERROR, GetLastError());
		this->Clean();
		return false;
	}

	return true;
}

bool CSocketManagerUdp::Connect(char* IpAddress, WORD port)
{
	if ((this->m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, "[SocketManagerUdp] socket() failed with error: %d", WSAGetLastError());
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
			Log.ToDisp(LOG_RED, "[SocketManagerUdp] getaddrinfo() failed with error: %d", WSAGetLastError());
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
			Log.ToDisp(LOG_RED, "[SocketManagerUdp] getaddrinfo() returned no results.");
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
			Log.ToDisp(LOG_RED, SERVERRECVTHREAD_STOP_TIMEOUT);
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

bool CSocketManagerUdp::DataRecv() // OK
{
	if (this->m_RecvSize < 3)
	{
		return 1;
	}

	BYTE* lpMsg = this->m_RecvBuff;

	int count = 0, size = 0;
	BYTE header, head;

	while (true)
	{
		if (lpMsg[count] == 0xC1)
		{
			header = lpMsg[count];
			size = lpMsg[count + 1];
			head = lpMsg[count + 2];
		}
		else if (lpMsg[count] == 0xC2)
		{
			header = lpMsg[count];
			size = MAKEWORD(lpMsg[count + 2], lpMsg[count + 1]);
			head = lpMsg[count + 3];
		}
		else
		{
			Log.ToDisp(LOG_RED, "[SocketManagerUdp] Protocol header error (Header: %x)", lpMsg[count]);
			memset(this->m_RecvBuff, 0, sizeof(this->m_RecvBuff));
			this->m_RecvSize = 0;
			return 0;
		}

		if (size < 3 || size > MAX_UDP_PACKET_SIZE)
		{
			Log.ToDisp(LOG_RED, "[SocketManagerUdp] Protocol size error (Header: %x, Size: %d, Head: %x)", header, size, head);
			memset(this->m_RecvBuff, 0, sizeof(this->m_RecvBuff));
			this->m_RecvSize = 0;
			return 0;
		}

		if (size <= this->m_RecvSize)
		{
			gServerList.ProcessServerStatusPacket(head, &lpMsg[count], size);

			count += size;

			this->m_RecvSize -= size;

			if (this->m_RecvSize <= 0)
			{
				break;
			}
		}
		else
		{
			if (count > 0 && this->m_RecvSize > 0 && this->m_RecvSize <= (MAX_UDP_PACKET_SIZE - count))
			{
				memmove(lpMsg, &lpMsg[count], this->m_RecvSize);
			}

			break;
		}
	}

	return 1;
}

bool CSocketManagerUdp::DataSend(BYTE* lpMsg, int size) // OK
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
		Log.ToDisp(LOG_RED, "[SocketManagerUdp] Max msg size (Size: %d)", size);
		memset(this->m_SendBuff, 0, sizeof(this->m_SendBuff));
		this->m_SendSize = 0;
		return 0;
	}

	memcpy(&this->m_SendBuff[this->m_SendSize], lpMsg, size);

	this->m_SendSize += size;

	int result = sendto(this->m_socket, (char*)this->m_SendBuff, this->m_SendSize, 0, (sockaddr*)&this->m_SocketAddr, sizeof(this->m_SocketAddr));

	if (result == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			Log.ToDisp(LOG_RED, "[SocketManagerUdp] sendto() failed with error: %d", WSAGetLastError());
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
			if (wsaErr != WSAENOTSOCK && wsaErr != WSAEINTR)
			{
				Log.ToDisp(LOG_RED, "[SocketManagerUdp] recvfrom() failed with error: %d", wsaErr);
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
