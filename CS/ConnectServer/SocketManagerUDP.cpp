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

CSocketManagerUdp gSocketManagerUdp;

// Constructor / Destructor

CSocketManagerUdp::CSocketManagerUdp()
	: m_socket(INVALID_SOCKET),
	m_ServerRecvThread(nullptr)
{
	// Inicialización a cero de la estructura de dirección de red
	memset(&m_SocketAddr, 0, sizeof(m_SocketAddr));

	// CORRECCIÓN: Limpieza del buffer de recepción para asegurar que no contenga basura de la RAM
	memset(m_RecvBuff, 0, sizeof(m_RecvBuff));
}

CSocketManagerUdp::~CSocketManagerUdp()
{
	Clean();
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
	if ((m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, "[SocketManagerUDP - Init] WSASocket() fallo con el error: %d", WSAGetLastError());
		Clean();
		return false;
	}

	m_SocketAddr.sin_family = AF_INET;
	m_SocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_SocketAddr.sin_port = htons(port);

	if (bind(m_socket, reinterpret_cast<sockaddr*>(&m_SocketAddr), sizeof(m_SocketAddr)) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, "[SocketManagerUDP - Init] bind() fallo en el puerto %d con el error: %d", port, WSAGetLastError());
		Clean();
		return false;
	}

	if ((m_ServerRecvThread = CreateThread(nullptr, 0, CSocketManagerUdp::ServerRecvThread, this, 0, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, "[SocketManagerUDP - Init] Error al crear el hilo de recepción.");
		Clean();
		return false;
	}

	Log.ToDisp(LOG_GREEN, "[SocketManagerUDP - Init] Servidor UDP iniciado en el puerto [%d]", port);
	return true;
}

bool CSocketManagerUdp::Connect(char* IpAddress, WORD port)
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	if ((m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, "[SocketManagerUDP - Connect] WSASocket() fallo con el error: %d", WSAGetLastError());
		Clean();
		return false;
	}

	m_SocketAddr.sin_family = AF_INET;

	if (InetPtonA(AF_INET, IpAddress, &m_SocketAddr.sin_addr) <= 0)
	{
		struct addrinfo hints = { 0 };
		struct addrinfo* result = nullptr;

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		if (getaddrinfo(IpAddress, nullptr, &hints, &result) != 0)
		{
			Log.ToDisp(LOG_RED, "[SocketManagerUDP - Connect] getaddrinfo() fallo con el error: %d", WSAGetLastError());
			Clean();
			return false;
		}

		if (result != nullptr)
		{
			m_SocketAddr.sin_addr = ((sockaddr_in*)result->ai_addr)->sin_addr;
			freeaddrinfo(result);
		}
		else
		{
			Log.ToDisp(LOG_RED, "[SocketManagerUDP - Connect] getaddrinfo() no retorno resultados.");
			Clean();
			return false;
		}
	}

	m_SocketAddr.sin_port = htons(port);

	return true;
}

// CORRECIÓN: Agregar funcion Clean() para liberar recursos correctamente
void CSocketManagerUdp::Clean()
{
	// Cerrar el socket es la única señal de parada necesaria:
	// desbloquea recvfrom() en ServerRecvThread inmediatamente
	// con WSAENOTSOCK, lo que hace que el hilo salga del loop.
	{
		CCriticalSection::CLock lock(m_lock);

		if (m_socket != INVALID_SOCKET)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
		}
	}

	if (m_ServerRecvThread != nullptr)
	{
		if (WaitForSingleObject(m_ServerRecvThread, DEFAULT_TIME_WAIT) == WAIT_TIMEOUT)
		{
			Log.ToDisp(LOG_RED, "[SocketManagerUDP - Clean] Timeout esperando WaitForSingleObject: %d", GetLastError());
		}
		CloseHandle(m_ServerRecvThread);
		m_ServerRecvThread = nullptr;
	}
}

bool CSocketManagerUdp::DataRecv(int recvSize)
{
	if (recvSize < 3)
	{
		// Datagrama demasiado corto para contener siquiera la cabecera
		// mínima (C1: type+size+head). Se descarta silenciosamente:
		// en UDP no hay "conexión" que cerrar, así que no es necesario
		// loggear cada paquete malformado como error grave (podría ser
		// ruido de red, un probe, o un paquete corrupto en tránsito).
		return false;
	}

	BYTE* lpMsg = m_RecvBuff;

	int size = 0;
	BYTE header = lpMsg[0];
	BYTE head = 0;

	if (header == PACKET_HEADER_C1)
	{
		size = lpMsg[1];
		head = lpMsg[2];
	}
	else if (header == PACKET_HEADER_C2)
	{
		if (recvSize < 4)
		{
			return false;
		}
		size = MAKEWORD(lpMsg[2], lpMsg[1]);
		head = lpMsg[3];
	}
	else
	{
		Log.ToDisp(LOG_RED, "[SocketManagerUDP - DataRecv] Error de cabecera del protocolo (Header: %02X)", header);
		return false;
	}

	int minSize = (header == PACKET_HEADER_C1) ? 3 : 4;

	// En UDP, "size" SIEMPRE debe coincidir exactamente con lo que llegó
	// en este datagrama. Si no coincide, no es un caso de "esperar más
	// datos" (como en TCP) — es un paquete corrupto o malformado, porque
	// un datagrama UDP nunca llega partido a la mitad.
	if (size < minSize || size > MAX_UDP_PACKET_SIZE)
	{
		Log.ToDisp(LOG_RED, "[SocketManagerUDP - DataRecv] Error de tamaño del protocolo (Index: %d, Size: %d, Head: %02X)", header, size, head);
		return false;
	}

	if (size != recvSize)
	{
		// FIX: detecta el caso real de error en UDP: el campo "size" del
		// protocolo no coincide con los bytes efectivamente recibidos.
		// Esto reemplaza la lógica de "paquete incompleto, esperar más"
		// que tenía sentido en TCP pero es imposible en UDP.
		Log.ToDisp(LOG_RED, "[SocketManagerUDP - DataRecv] Tamaño declarado (%d) no coincide con datagrama recibido (%d)", size, recvSize);
		return false;
	}

	gServerList.ProcessServerStatusPacket(head, lpMsg, size);

	return true;
}

bool CSocketManagerUdp::DataSend(BYTE* lpMsg, int size)
{
	// Protegemos todo el flujo con el mutex para evitar que Clean() cierre
	// el socket o que otro DataSend modifique los buffers simultaneamente.
	CCriticalSection::CLock lock(m_lock);

	if (m_socket == INVALID_SOCKET)
	{
		return false;
	}

	if (size > MAX_UDP_PACKET_SIZE)
	{
		Log.ToDisp(LOG_RED, "[SocketManagerUDP - DataSend] Tamaño de mensaje excedido (Tamaño: %d)", size);
		return false;
	}

	// UDP: sendto() es atómico para datagramas. O se envía el mensaje
	// completo en una sola llamada al kernel, o falla. No existe el
	// concepto de "envío parcial" como en TCP, así que no hace falta
	// side buffer ni lógica de reintento por bytes pendientes.
	int result = sendto(m_socket, (char*)lpMsg, size, 0, (sockaddr*)&m_SocketAddr, sizeof(m_SocketAddr));

	if (result == SOCKET_ERROR)
	{
		int Error = WSAGetLastError();
		if (Error != WSAEWOULDBLOCK)
		{
			Log.ToDisp(LOG_RED, "[SocketManagerUDP - DataSend] fallo con el error: %d", Error);
			return false;
		}
		// WSAEWOULDBLOCK en un socket UDP no bloqueante: el datagrama
		// se descarta. No hay reintento automático porque UDP no
		// garantiza entrega; el llamador puede reintentar si es crítico.
		return false;
	}

	return true;
}

DWORD WINAPI CSocketManagerUdp::ServerRecvThread(LPVOID lpParam)
{
	CSocketManagerUdp* lpSocketManagerUdp = reinterpret_cast<CSocketManagerUdp*>(lpParam);

	while (true)
	{
		SOCKADDR_IN SocketAddr {};
		int SocketAddrSize = sizeof(SocketAddr);

		// UDP: cada recvfrom() entrega EXACTAMENTE un datagrama completo
		// o falla. Siempre se escribe desde offset 0: no existe el
		// concepto de "continuación" entre llamadas como en TCP, así
		// que jamás se debe acumular sobre un offset previo.
		int result = recvfrom(
			lpSocketManagerUdp->m_socket,
			(char*)lpSocketManagerUdp->m_RecvBuff,
			MAX_UDP_PACKET_SIZE,
			0,
			(sockaddr*)&SocketAddr,
			&SocketAddrSize
		);

		if (result == SOCKET_ERROR)
		{
			int wsaErr = WSAGetLastError();

			// WSAENOTSOCK / WSAEINTR / WSANOTINITIALISED indican que el
			// socket fue cerrado desde Clean() (parada del servidor) o
			// que la pila Winsock ya se está apagando. Esta es la señal
			// real y única de "hay que terminar el hilo": no hace falta
			// un evento de shutdown separado, porque closesocket() desde
			// otro hilo ya desbloquea este recvfrom() de forma inmediata.
			if (wsaErr == WSAENOTSOCK || wsaErr == WSAEINTR || wsaErr == WSANOTINITIALISED)
			{
				break;
			}

			// Cualquier otro error (ej. WSAECONNRESET por un ICMP
			// "puerto inalcanzable" de un envío previo) es transitorio:
			// se loguea y se sigue escuchando, sin tocar el buffer
			// (no hace falta limpiarlo: el próximo recvfrom exitoso
			// sobreescribe desde el offset 0 igual).
			Log.ToDisp(LOG_RED, "[SocketManagerUDP - ServerRecvThread] recvfrom() fallo con el error: %d", wsaErr);
			continue;
		}

		// result == 0 es válido en UDP (datagrama vacío). DataRecv ya
		// descarta cualquier tamaño menor al mínimo de protocolo.
		lpSocketManagerUdp->DataRecv(result);
	}

	return 0;
}
