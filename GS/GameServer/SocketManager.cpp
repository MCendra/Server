// SocketManager.cpp
//
// Gestor de sockets basado en I/O Completion Ports (IOCP) para Mu Online
// Season 6 (GameServer).
//
// Responsabilidades principales:
//   - Crear y configurar el socket de escucha (listen socket).
//   - Crear el puerto de finalizacion de E/S (IOCP) y asociarle los sockets
//     de los clientes a medida que se conectan.
//   - Lanzar y administrar los hilos de:
//       * Aceptacion de conexiones (ServerAcceptThread)
//       * Procesamiento de E/S completada (ServerWorkerThread, uno por CPU)
//       * Procesamiento de paquetes en cola (ServerQueueThread)
//   - Parsear los paquetes recibidos segun el protocolo de MU (cabeceras
//     C1/C2/C3/C4) y encolarlos para su procesamiento por ConnectServerProtocolCore.
//   - Enviar datos a los clientes usando WSASend con buffer secundario
//     (side buffer) para no perder datos si hay un envio en curso.
//   - Apagado cooperativo y ordenado de todos los hilos (Clean()).
#include "SocketManager.h"
#include "PacketManager.h"
#include "IpManager.h"
#include "SerialCheck.h"
//#include "HackCheck.h"
#include "User.h"
#include "Log.h"

CSocketManager gSocketManager;

// Construccion / Destruccion

CSocketManager::CSocketManager()
	: m_Listen(INVALID_SOCKET),         // Socket de escucha invalido hasta CreateListenSocket().
	m_CompletionPort(nullptr),          // Puerto de finalizacion aun no creado.
	m_Port(0),                          // Puerto TCP, se asigna en Init().
	m_ServerAcceptThread(nullptr),      // Hilo de aceptacion, se crea en CreateAcceptThread().
	m_ServerQueueSemaphore(nullptr),    // Semaforo de la cola, se crea en CreateServerQueue().
	m_ServerQueueThread(nullptr),       // Hilo de la cola, se crea en CreateServerQueue().
	m_ServerWorkerThreadCount(0),       // Cantidad real de hilos de trabajo (segun CPUs).
	m_ShutdownEvent(nullptr)            // Evento de apagado cooperativo (manual-reset).
{
	// Inicializa cada hilo del servidor en el array m_ServerWorkerThread a nullptr.
	// Esto asegura que Clean() pueda iterar el array de forma segura sin
	// importar cuantos hilos llegaron a crearse realmente.
	for (DWORD n = 0; n < MAX_SERVER_WORKER_THREAD; n++)
	{
		m_ServerWorkerThread[n] = nullptr;
	}
}

CSocketManager::~CSocketManager()
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
bool CSocketManager::Init(WORD port)
{
	m_Port = port;

	// Evento de parada manual-reset: una vez señalado (SetEvent), permanece
	// señalado para todos los hilos que lo consulten (no se auto-resetea).
	if ((m_ShutdownEvent = CreateEvent(nullptr, true, false, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - Init] Error al crear evento de shutdown: %d", GetLastError());
		Clean();
		return false;
	}

	// Socket de escucha (bind + listen).
	if (CreateListenSocket() == 0)
	{
		Clean();
		return false;
	}

	// Puerto de finalizacion de E/S (IOCP).
	if (CreateCompletionPort() == 0)
	{
		Clean();
		return false;
	}

	// Hilo que acepta nuevas conexiones entrantes.
	if (CreateAcceptThread() == 0)
	{
		Clean();
		return false;
	}

	// Hilos de trabajo que procesan los eventos del IOCP (recv/send).
	if (CreateWorkerThread() == 0)
	{
		Clean();
		return false;
	}

	// Cola interna de paquetes ya parseados + hilo que los despacha
	// hacia ConnectServerProtocolCore.
	if (CreateServerQueue() == 0)
	{
		Clean();
		return false;
	}

	Log.ToDisp(LOG_BLACK, "[SocketManager - Init] Servidor TCP iniciado en el puerto [%d]", m_Port);
	return true;
}

// Crea el socket de escucha (overlapped) para aceptar conexiones entrantes.
//
// Pasos:
//   - WSASocket() con WSA_FLAG_OVERLAPPED para poder usarlo con IOCP.
//   - setsockopt(SO_REUSEADDR) para permitir reutilizar el puerto
//     inmediatamente despues de reiniciar el proceso (evita el tipico
//     error "Only one usage of each socket address is normally permitted"
//     cuando el socket anterior queda en TIME_WAIT).
//   - bind() sobre INADDR_ANY:m_Port.
//   - listen() con el backlog por defecto.
bool CSocketManager::CreateListenSocket()
{
	if ((m_Listen = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateListenSocket] WSASocket() fallo con el error: %d", WSAGetLastError());
		return false;
	}

	// FIX: SO_REUSEADDR permite volver a hacer bind() sobre el mismo puerto
	// inmediatamente despues de cerrar el servidor (muy util al recompilar
	// y relanzar el servidor seguido durante el desarrollo en VS2026).
	BOOL reuseAddr = true;
	if (setsockopt(m_Listen, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR)
	{
		// No es fatal: solo se informa y se continua.
		Log.ToDisp(LOG_RED, "[SocketManager - CreateListenSocket] setsockopt(SO_REUSEADDR) fallo con el error: %d", WSAGetLastError());
	}

	SOCKADDR_IN SocketAddr{};
	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	SocketAddr.sin_port = htons(m_Port);

	if (bind(m_Listen, (sockaddr*)&SocketAddr, sizeof(SocketAddr)) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateListenSocket] bind() fallo en el puerto %d con el error: %d", m_Port, WSAGetLastError());
		return false;
	}

	if (listen(m_Listen, DEFAULT_BACKLOG) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateListenSocket] listen() fallo con el error: %d", WSAGetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, "[SocketManager - CreateListenSocket] Socket de escucha creado exitosamente en el puerto: %d", m_Port);

	return true;
}

// Crea el puerto de finalizacion de E/S (IOCP) que se usara para asociar
// todos los sockets de los clientes y recibir notificaciones de recv/send
// completados en los hilos de trabajo.
//
// Se crea "vacio" (sin asociar ningun handle todavia); cada cliente se
// asocia individualmente en ServerAcceptThread mediante
// CreateIoCompletionPort((HANDLE)socket, m_CompletionPort, index, 0).
bool CSocketManager::CreateCompletionPort()
{
	m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (m_CompletionPort == nullptr)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateCompletionPort] Error al crear el puerto de finalizacion de E/S.", GetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, "[SocketManager - CreateCompletionPort] Puerto de finalizacion de E/S creado exitosamente.");
	return true;
}

// Crea y configura el hilo que acepta conexiones entrantes (WSAAccept en
// bucle dentro de ServerAcceptThread). Se le asigna la prioridad mas alta
// para minimizar la latencia al aceptar nuevos clientes.
bool CSocketManager::CreateAcceptThread()
{
	m_ServerAcceptThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ServerAcceptThread, this, 0, nullptr);
	if (m_ServerAcceptThread == nullptr)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateAcceptThread] Error al crear el hilo de aceptacion de conexiones.", GetLastError());
		return false;
	}

	if (SetThreadPriority(m_ServerAcceptThread, THREAD_PRIORITY_HIGHEST) == 0)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateAcceptThread] Error al establecer la prioridad del hilo de aceptacion de conexiones.", GetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, "[SocketManager - CreateAcceptThread] Hilo de aceptacion de conexiones creado y configurado exitosamente.");
	return true;
}

// Crea los hilos de trabajo (workers) que procesan los eventos completados
// del IOCP (recv/send de cada cliente).
//
// La cantidad de hilos se ajusta al numero de procesadores logicos del
// sistema, con un tope de MAX_SERVER_WORKER_THREAD (esta es la
// recomendacion clasica de IOCP: 1 hilo por nucleo evita cambios de
// contexto innecesarios). Todos se crean con prioridad maxima.
bool CSocketManager::CreateWorkerThread()
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	m_ServerWorkerThreadCount = (SystemInfo.dwNumberOfProcessors > MAX_SERVER_WORKER_THREAD)
		? MAX_SERVER_WORKER_THREAD
		: SystemInfo.dwNumberOfProcessors;

	for (DWORD n = 0; n < m_ServerWorkerThreadCount; n++)
	{
		m_ServerWorkerThread[n] = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CSocketManager::ServerWorkerThread, this, 0, nullptr);

		if (m_ServerWorkerThread[n] == nullptr)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - CreateWorkerThread] Error al crear el hilo de trabajo %u. Codigo: %lu", n, GetLastError());
			return false;
		}
		// FIX: THREAD_PRIORITY_HIGHEST por THREAD_PRIORITY_NORMAL
		if (SetThreadPriority(m_ServerWorkerThread[n], THREAD_PRIORITY_NORMAL) == 0)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - CreateWorkerThread] Error al establecer la prioridad del hilo de trabajo %u. Codigo: %lu", n, GetLastError());
			return false;
		}
	}

	Log.ToDisp(LOG_BLACK, "[SocketManager - CreateWorkerThread] %d hilos de trabajo creados exitosamente.", m_ServerWorkerThreadCount);
	return true;
}

// Crea el semaforo de la cola del servidor (contador de paquetes
// pendientes de procesar) y el hilo que los consume y los despacha
// hacia ConnectServerProtocolCore.
bool CSocketManager::CreateServerQueue()
{
	// El semaforo arranca en 0 (sin paquetes pendientes) y permite hasta
	// MAX_QUEUE_SIZE incrementos simultaneos (uno por cada AddToQueue).
	if ((m_ServerQueueSemaphore = CreateSemaphore(nullptr, 0, MAX_QUEUE_SIZE, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateServerQueue] Error al crear el semaforo de la cola del servidor. Codigo: %lu", GetLastError());
		return false;
	}

	if ((m_ServerQueueThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CSocketManager::ServerQueueThread, this, 0, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateServerQueue] Error al crear el hilo de la cola del servidor. Codigo: %lu", GetLastError());
		return false;
	}

	if (SetThreadPriority(m_ServerQueueThread, THREAD_PRIORITY_HIGHEST) == 0)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateServerQueue] Error al establecer la prioridad del hilo de la cola del servidor. Codigo: %lu", GetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, "[SocketManager - CreateServerQueue] Semaforo y hilo de cola del servidor creados exitosamente.");
	return true;
}

// =====================================================================
// Recepcion / parsing del protocolo
// =====================================================================

// Procesa los datos crudos recibidos de un cliente (lpIoBuffer->buff,
// lpIoBuffer->size bytes) extrayendo todos los paquetes completos del
// protocolo de MU y encolandolos para su procesamiento.
//
// Formato de cabecera soportado:
//   - C1 (0xC1): [0xC1][size: 1 byte][head: 1 byte][payload...]
//                tamaño minimo de cabecera = 3 bytes.
//   - C2 (0xC2): [0xC2][size_hi: 1 byte][size_lo: 1 byte][head: 1 byte][payload...]
//                tamaño minimo de cabecera = 4 bytes, size = (size_hi << 8) | size_lo.
//
// Retorno:
//   - true  : el buffer se proceso sin errores de protocolo (puede haber
//             quedado un fragmento incompleto al final, que se conserva
//             al comienzo del buffer para completarse con el proximo recv).
//   - false : error real de protocolo (cabecera desconocida o tamaño
//             fuera de rango). El llamador (OnRecv) desconecta al cliente.
//
// FIX: anteriormente, si llegaba un fragmento TCP mas chico que el tamaño
// minimo de cabecera (1 o 2 bytes sueltos, algo perfectamente normal en
// TCP), la funcion retornaba "false" y el cliente era desconectado por
// error. Ahora esos casos se tratan como "falta mas informacion" y se
// hace "break" para esperar el resto en el siguiente WSARecv.
bool CSocketManager::DataRecv(int index, IO_RECV_BUFFER* lpIoBuffer)
{
	BYTE* lpMsg = lpIoBuffer->Buffer;

	int count = 0;

	BYTE DecBuff[MAX_RECV_PACKET_SIZE];

	while (true)
	{
		int available = lpIoBuffer->Size - count;

		if (available <= 0)
		{
			break;
		}

		BYTE header = 0;
		BYTE head = 0;
		int size = 0;

		if (lpMsg[count] == PACKET_C1 || lpMsg[count] == PACKET_C3)
		{
			if (available < 3)
			{
				break;
			}

			header = lpMsg[count];
			size = lpMsg[count + 1];
			head = lpMsg[count + 2];
		}
		else if (lpMsg[count] == PACKET_C2 || lpMsg[count] == PACKET_C4)
		{
			if (available < 4)
			{
				break;
			}

			header = lpMsg[count];
			size = (static_cast<WORD>(lpMsg[count + 1]) << 8) | lpMsg[count + 2];
			head = lpMsg[count + 3];
		}
		else
		{
			// Byte de cabecera desconocido: esto si es un error de protocolo real.
			Log.ToDisp(LOG_RED, "[SocketManager - DataRecv] Error de cabecera del protocolo (Index: %d, Header: %02X)", index, lpMsg[count]);
			return false;
		}

		const int minSize = ((header == PACKET_C1) || (header == PACKET_C3)) ? 3 : 4;

		if (size < minSize || size > MAX_RECV_PACKET_SIZE)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - DataRecv] Error de tamaño del protocolo (Index: %d, Size: %d, ProtocolHead: %02X)", index, size, header);
			return false;

			return false;
		}

		if (size > available)
		{
			break;
		}

		QUEUE_INFO QueueInfo{};

		if (header == PACKET_C3)
		{
			int DecSize = gPacketManager.Decrypt(&DecBuff[1], &lpMsg[count + 2], size - 2) + 1;

			int DecSerial = DecBuff[1];

			DecBuff[0] = 0xC1;
			DecBuff[1] = static_cast<BYTE>(DecSize);

			head = DecBuff[2];

			if (gPacketManager.AddData(DecBuff, DecSize) == 0 ||
				gPacketManager.ExtractPacket(DecBuff) == 0)
			{
				return false;
			}

			QueueInfo.Index = index;
			QueueInfo.ProtocolHead = head;
			QueueInfo.Size = static_cast<WORD>(DecSize);
			QueueInfo.Encrypt = 1;
			QueueInfo.Serial = DecSerial;

			std::memcpy(QueueInfo.Buffer, DecBuff, static_cast<size_t>(DecSize));
		}
		else if (header == 0xC4)
		{
			int DecSize = gPacketManager.Decrypt(&DecBuff[2], &lpMsg[count + 3], size - 3) + 2;

			int DecSerial = DecBuff[2];

			DecBuff[0] = 0xC2;
			DecBuff[1] = HIBYTE(DecSize);
			DecBuff[2] = LOBYTE(DecSize);

			head = DecBuff[3];

			if (gPacketManager.AddData(DecBuff, DecSize) == 0 ||
				gPacketManager.ExtractPacket(DecBuff) == 0)
			{
				return false;
			}

			QueueInfo.Index = index;
			QueueInfo.ProtocolHead = head;
			QueueInfo.Size = static_cast<WORD>(DecSize);
			QueueInfo.Encrypt = 1;
			QueueInfo.Serial = DecSerial;

			std::memcpy(QueueInfo.Buffer, DecBuff, static_cast<size_t>(DecSize));
		}
		else
		{
			if (gPacketManager.AddData(&lpMsg[count], size) == 0 ||
				gPacketManager.ExtractPacket(DecBuff) == 0)
			{
				return false;
			}

			QueueInfo.Index = index;
			QueueInfo.ProtocolHead = head;
			QueueInfo.Size = static_cast<WORD>(size);
			QueueInfo.Encrypt = 0;
			QueueInfo.Serial = -1;

			std::memcpy(QueueInfo.Buffer, DecBuff, static_cast<size_t>(size));
		}

		if (m_ServerQueue.AddToQueue(&QueueInfo) != 0)
		{
			ReleaseSemaphore(m_ServerQueueSemaphore, 1, nullptr);
		}
		else
		{
			Log.ToDisp(LOG_RED, "[SocketManager - DataRecv] Cola del servidor llena, paquete descartado (Index: %d, ProtocolHead: %02X)", index, head);
			return false;
		}

		count += size;

		if (count >= lpIoBuffer->Size)
		{
			count = 0;
			lpIoBuffer->Size = 0;
			break;
		}
	}

	if (count > 0)
	{
		int remaining = lpIoBuffer->Size - count;

		if (remaining > 0)
		{
			std::memmove(lpMsg, &lpMsg[count], static_cast<size_t>(remaining));
		}

		lpIoBuffer->Size = remaining;
	}

	return true;
}

// =====================================================================
// Envio de datos
// =====================================================================

// Envia datos al servidor especificado, gestionando un buffer secundario
// ("side buffer") para no perder informacion si ya hay un WSASend en
// curso para ese servidor.
//
// Flujo:
//   - Valida indice/estado del servidor y tamaño del mensaje.
//   - Si ya hay un envio pendiente (IoSize > 0), los nuevos datos se
//     acumulan en IoSideBuffer (se desconecta si no entran).
//   - Si no hay envio pendiente, los datos se copian al buffer principal
//     y se dispara WSASend de inmediato.
bool CSocketManager::DataSend(int index, BYTE* lpMsg, int size)
{
	if (!OBJECT_USER_RANGE(index))
	{
		return false;
	}

	if (gObj[index].Socket == INVALID_SOCKET)
	{
		return false;
	}

	if (gObj[index].Connected == OBJECT_OFFLINE)
	{
		return false;
	}

	if (size > MAX_SEND_PACKET_SIZE)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - DataSend] Tamaño maximo de mensaje excedido (Tipo: 1, indice: %d, Tamaño: %d)", index, size);
		Disconnect(index);
		return false;
	}

	BYTE SendBuffer[MAX_SEND_PACKET_SIZE];

	memcpy(SendBuffer, lpMsg, size);

	switch (lpMsg[PACKET_TYPE_OFFSET])
	{
	case PACKET_C3:
	{
		const BYTE serial = gSerialCheck[index].GetSendSerial();

		SendBuffer[C1_PACKET_SIZE_OFFSET] = serial;

		size = gPacketManager.Encrypt(
			&SendBuffer[2],
			&SendBuffer[1],
			size - 1) + 2;

		SendBuffer[PACKET_TYPE_OFFSET] = PACKET_C3;
		SendBuffer[C1_PACKET_SIZE_OFFSET] = static_cast<BYTE>(size);
		break;
	}
	case PACKET_C4:
	{
		const BYTE serial = gSerialCheck[index].GetSendSerial();

		SendBuffer[C2_PACKET_SIZEL_OFFSET] = serial;

		size = gPacketManager.Encrypt(
			&SendBuffer[3],
			&SendBuffer[2],
			size - 2) + 3;

		SendBuffer[PACKET_TYPE_OFFSET] = PACKET_C4;
		SendBuffer[C2_PACKET_SIZEH_OFFSET] = HIBYTE(size);
		SendBuffer[C2_PACKET_SIZEL_OFFSET] = LOBYTE(size);
		break;
	}
	}

#if (ENCRYPT_STATE == 1)

	EncryptData(SendBuffer, size);

#endif

	IO_SEND_CONTEXT* lpIoContext = &gObj[index].PerSocketContext->IoSendContext;

	if (lpIoContext->IoSize > 0)
	{
		// Ya hay un envio en curso: acumulamos en el buffer secundario.
		int IoSideBufferSizeTotal = lpIoContext->IoSideBuffer.Size + size;

		if (IoSideBufferSizeTotal > MAX_SEND_SIDE_PACKET_SIZE)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - DataSend] Tamaño maximo de mensaje excedido (Tipo: 2, Índice: %d, Tamaño: %d)", index, IoSideBufferSizeTotal);
			Disconnect(index);
			return false;
		}

		memcpy(
			&lpIoContext->IoSideBuffer.Buffer[lpIoContext->IoSideBuffer.Size],
			SendBuffer,
			size);

		lpIoContext->IoSideBuffer.Size += size;

		return true;
	}

	memcpy(lpIoContext->IoSendBuffer.Buffer, SendBuffer, size);

	lpIoContext->WSAbuf.buf = reinterpret_cast<char*>(lpIoContext->IoSendBuffer.Buffer);
	lpIoContext->WSAbuf.len = size;

	lpIoContext->IoType = IO_SEND;
	lpIoContext->IoSize = size;
	lpIoContext->IoSendBuffer.Size = 0;

	DWORD SendSize = 0, Flags = 0;

	if (WSASend(gObj[index].Socket, &lpIoContext->WSAbuf, 1, &SendSize, Flags, &lpIoContext->OverLapped, nullptr) == SOCKET_ERROR)
	{
		const int WSAerror = WSAGetLastError();

		if (WSAerror != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - DataSend] WSASend() fallo con error: %d", WSAerror);
			Disconnect(index);
			return false;
		}
	}

	return true;
}

// =====================================================================
// Desconexion
// =====================================================================
//
// Disconnect() SOLO cierra el socket. No toca los IO contexts.
//
// Razonamiento IOCP:
//   closesocket() cancela de forma ASÍNCRONA todas las operaciones
//   WSARecv/WSASend pendientes. El kernel entregará cada una de esas
//   cancelaciones al IOCP con IoSize = 0 y un error de conexión
//   (ERROR_NETNAME_DELETED, etc.). Es en ese momento —dentro de
//   OnRecv/OnSend con IoSize == 0— donde es seguro llamar a DelClient()
//   y liberar los IO contexts, porque el kernel ya no los usará más.
//
//   Si liberásemos los IO contexts aquí (como hacía antes), el kernel
//   podría escribir en memoria ya liberada → heap corruption / crash.
//
// Esta función es idempotente: si el socket ya es INVALID_SOCKET
// (porque ya se llamó Disconnect antes), IsOnline() retorna false
// y salimos sin hacer nada.
void CSocketManager::Disconnect(int index)
{
	if (!OBJECT_USER_RANGE(index))
	{
		return;
	}

	if (gObj[index].Socket == INVALID_SOCKET)
	{
		return;
	}

	if (gObj[index].Connected == OBJECT_OFFLINE)
	{
		return;
	}

	if (closesocket(gObj[index].Socket) == SOCKET_ERROR)
	{
		const int WSAerror = WSAGetLastError();

		if (WSAerror != WSAENOTSOCK)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - Disconnect] closesocket() fallo con el error: %d", WSAerror);
			return;
		}
	}

	gObj[index].Socket = INVALID_SOCKET;

#if (OFFLINE_MODE)

	if (!gObj[index].m_OfflineSocket)
	{
		gObjDel(index);
	}

#else

	gObjDel(index);

#endif
}

// =====================================================================
// Callbacks de E/S completada (recv / send)
// =====================================================================

// Maneja la finalizacion de una operacion de recepcion (WSARecv) para
// el servidor "index".
//
// Flujo:
//   - IoSize == 0  -> el peer cerro la conexion de forma ordenada (FIN):
//                     se desconecta al servidor.
//   - Se acumulan los bytes recibidos en IoMainBuffer y se intenta
//     parsear paquetes completos con DataRecv().
//   - Si DataRecv detecta un error real de protocolo, se desconecta.
//   - Se programa el siguiente WSARecv, continuando a partir del
//     remanente que dejo DataRecv en el buffer.
void CSocketManager::OnRecv(int index, DWORD IoSize, IO_RECV_CONTEXT* lpIoContext)
{
	if (!OBJECT_USER_RANGE(index))
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];
	PER_SOCKET_CONTEXT* lpSocketContext = lpObj->PerSocketContext;

	CCriticalSection::CLock lock(lpSocketContext->Lock);

	if (IoSize == 0)
	{
		// Conexión cerrada por el cliente.
		Disconnect(index);
		return;
	}

	if (lpObj->Connected == OBJECT_OFFLINE)
	{
		return;
	}

	// Validación de seguridad: evita desbordar el buffer de recepción.
	if (lpIoContext->IoRecvBuffer.Size < 0 ||
		lpIoContext->IoRecvBuffer.Size > MAX_RECV_PACKET_SIZE ||
		IoSize > static_cast<DWORD>(MAX_RECV_PACKET_SIZE - lpIoContext->IoRecvBuffer.Size))
	{
		Log.ToDisp(LOG_RED,
			"[SocketManager - OnRecv] Se detectó un desbordamiento del buffer de recepción (Index: %d, Size: %d, Recv: %u)",
			index,
			lpIoContext->IoRecvBuffer.Size,
			IoSize);

		Disconnect(index);
		return;
	}

#if (ENCRYPT_STATE == 1)

	DecryptData(&lpIoContext->IoRecvBuffer.Buffer[lpIoContext->IoRecvBuffer.Size], IoSize);

#endif

	lpIoContext->IoRecvBuffer.Size += IoSize;

	if (!DataRecv(index, &lpIoContext->IoRecvBuffer))
	{
		// Error de protocolo.
		Disconnect(index);
		return;
	}

	if (lpSocketContext->Socket == INVALID_SOCKET)
	{
		return;
	}

	// Prepara el siguiente WSARecv utilizando el remanente dejado por DataRecv.
	lpIoContext->WSAbuf.buf = reinterpret_cast<char*>(&lpIoContext->IoRecvBuffer.Buffer[lpIoContext->IoRecvBuffer.Size]);
	lpIoContext->WSAbuf.len = MAX_RECV_PACKET_SIZE - lpIoContext->IoRecvBuffer.Size;
	lpIoContext->IoType = IO_RECV;

	DWORD RecvSize = 0;
	DWORD Flags = 0;

	if (WSARecv(lpSocketContext->Socket,
		&lpIoContext->WSAbuf,
		1,
		&RecvSize,
		&Flags,
		&lpIoContext->OverLapped,
		nullptr) == SOCKET_ERROR)
	{
		int WSAerror = WSAGetLastError();

		if (WSAerror != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED,
				"[SocketManager - OnRecv] WSARecv() falló con error: %d",
				WSAerror);

			Disconnect(index);
			return;
		}
	}
}

// Maneja la finalizacion de una operacion de envio (WSASend) para el
// servidor "index".
//
// Flujo:
//   - IoSize == 0 -> error de envio: se desconecta al servidor.
//   - Se acumula lo enviado en IoMainBuffer.size.
//   - Si ya se envio todo lo que habia que enviar (IoMainBuffer.size >=
//     IoSize):
//       * Si no hay nada en el side buffer, se marca el envio como
//         finalizado (IoSize = 0) y no se programa un nuevo WSASend.
//       * Si hay datos en el side buffer, se mueven al buffer principal
//         (en bloques de hasta MAX_RECV_PACKET_SIZE) y se programa un
//         nuevo WSASend para vaciarlo.
//   - Si todavia falta enviar parte del mensaje actual, se programa un
//     WSASend con el resto pendiente.
void CSocketManager::OnSend(int index, DWORD IoSize, IO_SEND_CONTEXT* lpIoContext)
{
	if (!OBJECT_USER_RANGE(index))
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];
	PER_SOCKET_CONTEXT* lpSocketContext = lpObj->PerSocketContext;

	CCriticalSection::CLock lock(lpSocketContext->Lock);

	if (IoSize == 0)
	{
		Disconnect(index);
		return;
	}

	if (lpObj->Connected == OBJECT_OFFLINE)
	{
		return;
	}

	if (lpSocketContext->Socket == INVALID_SOCKET)
	{
		return;
	}

	lpIoContext->IoSendBuffer.Size += IoSize;

	if (lpIoContext->IoSendBuffer.Size >= lpIoContext->IoSize)
	{
		if (lpIoContext->IoSideBuffer.Size <= 0)
		{
			lpIoContext->IoSize = 0;
			return;
		}

		if (lpIoContext->IoSideBuffer.Size > MAX_SEND_PACKET_SIZE)
		{
			memcpy(lpIoContext->IoSendBuffer.Buffer,
				lpIoContext->IoSideBuffer.Buffer,
				MAX_SEND_PACKET_SIZE);

			lpIoContext->WSAbuf.buf = reinterpret_cast<char*>(lpIoContext->IoSendBuffer.Buffer);
			lpIoContext->WSAbuf.len = MAX_SEND_PACKET_SIZE;
			lpIoContext->IoType = IO_SEND;
			lpIoContext->IoSize = MAX_SEND_PACKET_SIZE;
			lpIoContext->IoSendBuffer.Size = 0;

			memmove(lpIoContext->IoSideBuffer.Buffer,
				&lpIoContext->IoSideBuffer.Buffer[MAX_SEND_PACKET_SIZE],
				lpIoContext->IoSideBuffer.Size - MAX_SEND_PACKET_SIZE);

			lpIoContext->IoSideBuffer.Size -= MAX_SEND_PACKET_SIZE;
		}
		else
		{
			memcpy(lpIoContext->IoSendBuffer.Buffer,
				lpIoContext->IoSideBuffer.Buffer,
				lpIoContext->IoSideBuffer.Size);

			lpIoContext->WSAbuf.buf = reinterpret_cast<char*>(lpIoContext->IoSendBuffer.Buffer);
			lpIoContext->WSAbuf.len = lpIoContext->IoSideBuffer.Size;
			lpIoContext->IoType = IO_SEND;
			lpIoContext->IoSize = lpIoContext->IoSideBuffer.Size;
			lpIoContext->IoSendBuffer.Size = 0;
			lpIoContext->IoSideBuffer.Size = 0;
		}
	}
	else
	{
		lpIoContext->WSAbuf.buf = reinterpret_cast<char*>(&lpIoContext->IoSendBuffer.Buffer[lpIoContext->IoSendBuffer.Size]);
		lpIoContext->WSAbuf.len = lpIoContext->IoSize - lpIoContext->IoSendBuffer.Size;
		lpIoContext->IoType = IO_SEND;
	}

	DWORD SendSize = 0;
	DWORD Flags = 0;

	if (WSASend(lpSocketContext->Socket,
		&lpIoContext->WSAbuf,
		1,
		&SendSize,
		Flags,
		&lpIoContext->OverLapped,
		nullptr) == SOCKET_ERROR)
	{
		int WSAerror = WSAGetLastError();

		if (WSAerror != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED,
				"[SocketManager - OnSend] WSASend() falló con error: %d",
				WSAerror);

			Disconnect(index);
			return;
		}
	}
}

int CALLBACK CSocketManager::ServerAcceptCondition(
	IN LPWSABUF lpCallerId,
	IN LPWSABUF lpCallerData,
	IN OUT LPQOS lpSQOS,
	IN OUT LPQOS lpGQOS,
	IN LPWSABUF lpCalleeId,
	OUT LPWSABUF lpCalleeData,
	OUT GROUP FAR* g,
	DWORD_PTR dwCallbackData)
{
	UNREFERENCED_PARAMETER(lpCallerData);
	UNREFERENCED_PARAMETER(lpSQOS);
	UNREFERENCED_PARAMETER(lpGQOS);
	UNREFERENCED_PARAMETER(lpCalleeId);
	UNREFERENCED_PARAMETER(lpCalleeData);
	UNREFERENCED_PARAMETER(g);
	UNREFERENCED_PARAMETER(dwCallbackData);

	SOCKADDR_IN* lpSocketAddr = reinterpret_cast<SOCKADDR_IN*>(lpCallerId->buf);

	char IpAddress[INET_ADDRSTRLEN]{};

	if (InetNtopA(AF_INET, &lpSocketAddr->sin_addr, IpAddress, INET_ADDRSTRLEN) == nullptr)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - ServerAcceptCondition] InetNtopA() falló con error: %lu",	GetLastError());

		return CF_REJECT;
	}

	if (!gIpManager.CheckIpAddress(IpAddress))
	{
		return CF_REJECT;
	}

	return CF_ACCEPT;
}

// Hilo principal de aceptacion de conexiones.
//
// Bucle:
//   1) Si m_ShutdownEvent esta señalado, termina el hilo.
//   2) WSAAccept() (bloqueante) espera una nueva conexion, validandola
//      con ServerAcceptCondition (filtro por IP).
//   3) Si WSAAccept falla:
//        - Si fue porque cerramos el listen socket durante el shutdown,
//          se revisa el evento de parada y se sale del bucle.
//        - Otros errores se loggean y se continua esperando.
//   4) Si se acepto una conexion:
//        - Se busca un slot libre en gServerManager.
//        - Se asocia el socket al IOCP con CreateIoCompletionPort,
//          usando "index" como clave de finalizacion (completion key).
//        - Se registra el servidor (AddServer) con su IP.
//        - Se dispara el primer WSARecv para empezar a recibir datos.
DWORD WINAPI CSocketManager::ServerAcceptThread(CSocketManager* lpSocketManager)
{
	SOCKADDR_IN SocketAddr{};
	int SocketAddrSize = sizeof(SocketAddr);

	while (true)
	{
		// Finalización solicitada.
		if (lpSocketManager->m_ShutdownEvent &&	WaitForSingleObject(lpSocketManager->m_ShutdownEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}

		SOCKET socket = WSAAccept(lpSocketManager->m_Listen, (sockaddr*)&SocketAddr, &SocketAddrSize, (LPCONDITIONPROC)&lpSocketManager->ServerAcceptCondition, (DWORD_PTR)lpSocketManager);

		if (socket == INVALID_SOCKET)
		{
			if (lpSocketManager->m_ShutdownEvent && WaitForSingleObject(lpSocketManager->m_ShutdownEvent, 0) == WAIT_OBJECT_0)
			{
				break;
			}

			const int WSAerror = WSAGetLastError();

			if (WSAerror != WSAEWOULDBLOCK)
			{
				Log.ToDisp(LOG_RED,	"[SocketManager - ServerAcceptThread] WSAAccept() fallo con error: %d",	WSAerror);
			}

			continue;
		}

		char ipAddress[INET_ADDRSTRLEN]{};

		if (InetNtopA(AF_INET, &SocketAddr.sin_addr, ipAddress, INET_ADDRSTRLEN) == nullptr)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - ServerAcceptThread] InetNtopA() fallo con error: %d", GetLastError());

			closesocket(socket);
			continue;
		}

		int index = -1;

		index = gObjAddSearch(socket, ipAddress);

		if (index != -1)
		{
			if (CreateIoCompletionPort((HANDLE)socket, lpSocketManager->m_CompletionPort, index, 0) == nullptr)
			{
				Log.ToDisp(LOG_RED,
					"[SocketManager - ServerAcceptThread] CreateIoCompletionPort() fallo con error: %d",
					GetLastError());

				index = -1;
			}
			else if (gObjAdd(socket, ipAddress, index) == -1)
			{
				Log.ToDisp(LOG_RED,
					"[SocketManager - ServerAcceptThread] gObjAdd() fallo.");

				index = -1;
			}
		}

		if (index == -1)
		{
			closesocket(socket);
			continue;
		}

		LPOBJ lpObj = &gObj[index];
		PER_SOCKET_CONTEXT* lpSocketContext = lpObj->PerSocketContext;

		lpSocketContext->Socket = socket;
		lpSocketContext->Index = index;

		memset(&lpSocketContext->IoRecvContext.OverLapped, 0, sizeof(lpSocketContext->IoRecvContext.OverLapped));

		lpSocketContext->IoRecvContext.WSAbuf.buf = reinterpret_cast<char*>(lpSocketContext->IoRecvContext.IoRecvBuffer.Buffer);
		lpSocketContext->IoRecvContext.WSAbuf.len = MAX_RECV_PACKET_SIZE;
		lpSocketContext->IoRecvContext.IoType = IO_RECV;
		lpSocketContext->IoRecvContext.IoSize = 0;
		lpSocketContext->IoRecvContext.IoRecvBuffer.Size = 0;

		memset(&lpSocketContext->IoSendContext.OverLapped, 0, sizeof(lpSocketContext->IoSendContext.OverLapped));

		lpSocketContext->IoSendContext.WSAbuf.buf = reinterpret_cast<char*>(lpSocketContext->IoSendContext.IoSendBuffer.Buffer);
		lpSocketContext->IoSendContext.WSAbuf.len = MAX_SEND_PACKET_SIZE;
		lpSocketContext->IoSendContext.IoType = IO_SEND;
		lpSocketContext->IoSendContext.IoSize = 0;
		lpSocketContext->IoSendContext.IoSendBuffer.Size = 0;
		lpSocketContext->IoSendContext.IoSideBuffer.Size = 0;

		DWORD RecvSize = 0;
		DWORD Flags = 0;

		if (WSARecv(socket, &lpSocketContext->IoRecvContext.WSAbuf, 1, &RecvSize, &Flags, &lpSocketContext->IoRecvContext.OverLapped, nullptr) == SOCKET_ERROR)
		{
			const int WSAerror = WSAGetLastError();

			if (WSAerror != WSA_IO_PENDING)
			{
				Log.ToDisp(LOG_RED, "[SocketManager - ServerAcceptThread] WSARecv() fallo con error: %d", WSAerror);

				lpSocketManager->Disconnect(index);
				continue;
			}
		}

		GCConnectClientSend(index, 1);
	}

	return 0;
}

// Hilo de trabajo del IOCP. Cada uno de estos hilos (uno por CPU, hasta
// MAX_SERVER_WORKER_THREAD) espera eventos de E/S completados con
// GetQueuedCompletionStatus y los despacha a OnRecv/OnSend segun
// corresponda.
//
// Casos de salida del hilo:
//   - GQCS retorna 0 (fallo) con lpOverlapped == nullptr, o con un error
//     no relacionado a una desconexion esperada: error real, se loggea
//     y el hilo termina.
//   - GQCS retorna 0 con lpOverlapped != nullptr y el error es uno de
//     los esperables al desconectarse un cliente (ERROR_NETNAME_DELETED,
//     ERROR_CONNECTION_ABORTED, ERROR_OPERATION_ABORTED,
//     ERROR_SEM_TIMEOUT): se continua el procesamiento normal; el
//     IoSize quedara en 0 y OnRecv/OnSend desconectaran al cliente.
//   - "Paquete vacio" de apagado (IoSize==0, index==0, lpOverlapped==
//     nullptr) publicado por Clean(): el hilo termina limpiamente.
DWORD WINAPI CSocketManager::ServerWorkerThread(CSocketManager* lpSocketManager)
{
	DWORD IoSize = 0;
	ULONG_PTR CompletionKey = 0;
	LPOVERLAPPED lpOverlapped = nullptr;

	while (true)
	{
		if (GetQueuedCompletionStatus(
			lpSocketManager->m_CompletionPort,
			&IoSize,
			&CompletionKey,
			&lpOverlapped,
			INFINITE) == FALSE)
		{
			DWORD Error = GetLastError();

			// Señal de finalización del servidor.
			if (lpSocketManager->m_ShutdownEvent &&
				WaitForSingleObject(lpSocketManager->m_ShutdownEvent, 0) == WAIT_OBJECT_0)
			{
				break;
			}

			if (lpOverlapped == nullptr)
			{
				Log.ToDisp(LOG_RED,
					"[SocketManager - ServerWorkerThread] GetQueuedCompletionStatus() falló con error: %lu",
					Error);

				continue;
			}

			if (Error != ERROR_NETNAME_DELETED &&
				Error != ERROR_CONNECTION_ABORTED &&
				Error != ERROR_OPERATION_ABORTED &&
				Error != ERROR_SEM_TIMEOUT)
			{
				Log.ToDisp(LOG_RED,
					"[SocketManager - ServerWorkerThread] GetQueuedCompletionStatus() falló con error: %lu",
					Error);
			}
		}

		if (IoSize == 0 && CompletionKey == 0 && lpOverlapped == nullptr)
		{
			break;
		}

		IO_CONTEXT* lpIoContext = reinterpret_cast<IO_CONTEXT*>(lpOverlapped);

		switch (lpIoContext->IoType)
		{
		case IO_RECV:
		{
			lpSocketManager->OnRecv(
				static_cast<int>(CompletionKey),
				IoSize,
				reinterpret_cast<IO_RECV_CONTEXT*>(lpIoContext));

			break;
		}

		case IO_SEND:
		{
			lpSocketManager->OnSend(
				static_cast<int>(CompletionKey),
				IoSize,
				reinterpret_cast<IO_SEND_CONTEXT*>(lpIoContext));

			break;
		}

		default:
		{
			Log.ToDisp(LOG_RED,
				"[SocketManager - ServerWorkerThread] Tipo de IO desconocido (%d)",
				lpIoContext->IoType);

			break;
		}
		}
	}

	return 0;
}

// Hilo que consume la cola interna de paquetes ya parseados por DataRecv
// y los despacha hacia JoinServerProtocolCore.
//
// Espera simultaneamente (WaitForMultipleObjects) en dos handles:
//   - m_ServerQueueSemaphore: se libera una vez por cada paquete
//     encolado (y una vez mas durante Clean() para destrabar el hilo).
//   - m_ShutdownEvent: señal de apagado cooperativo.
//
// Si el handle señalado es el evento de parada, el hilo termina. En
// caso contrario, intenta sacar un paquete de la cola y, si el cliente
// asociado sigue conectado, lo procesa.
DWORD WINAPI CSocketManager::ServerQueueThread(CSocketManager* lpSocketManager)
{
	while (true)
	{
		// Finalización solicitada.
		if (lpSocketManager->m_ShutdownEvent &&
			WaitForSingleObject(lpSocketManager->m_ShutdownEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}

		if (WaitForSingleObject(lpSocketManager->m_ServerQueueSemaphore, INFINITE) == WAIT_FAILED)
		{
			Log.ToDisp(LOG_RED,
				"[SocketManager - ServerQueueThread] WaitForSingleObject() falló con error: %lu",
				GetLastError());

			break;
		}

		QUEUE_INFO QueueInfo{};

		if (!lpSocketManager->m_ServerQueue.GetFromQueue(&QueueInfo))
		{
			continue;
		}

		if (!OBJECT_RANGE(QueueInfo.Index))
		{
			continue;
		}

		if (gObj[QueueInfo.Index].Connected == OBJECT_OFFLINE)
		{
			continue;
		}

		ProtocolCore(
			QueueInfo.ProtocolHead,
			QueueInfo.Buffer,
			QueueInfo.Size,
			QueueInfo.Index,
			QueueInfo.Encrypt,
			QueueInfo.Serial);
	}

	return 0;
}

// Utilidades

// Devuelve la cantidad actual de paquetes pendientes en la cola interna.
DWORD CSocketManager::GetQueueSize()
{
	return m_ServerQueue.GetQueueSize();
}

// =====================================================================
// Apagado / liberacion de recursos
// =====================================================================

// Libera de forma ordenada todos los recursos utilizados por el gestor
// de sockets. Es seguro llamarla aunque Init() no haya llegado a
// completarse (todos los miembros se chequean contra su valor "vacio").
//
// Secuencia de apagado cooperativo:
//   1) Señaliza m_ShutdownEvent: cualquier hilo que este esperando en el
//      (ServerAcceptThread, ServerQueueThread) lo detecta y termina.
//   2) Cierra el socket de escucha: hace que WSAAccept() falle de
//      inmediato si ServerAcceptThread estaba bloqueado en el.
//   3) Publica un paquete "vacio" por cada worker en el IOCP: cada
//      ServerWorkerThread recibe IoSize=0/index=0/lpOverlapped=nullptr,
//      lo reconoce como señal de apagado y termina.
//   4) Libera el semaforo de la cola una vez para destrabar
//      ServerQueueThread (que tambien esta esperando m_ShutdownEvent).
//   5) Espera (con timeout) a que cada hilo termine y cierra sus handles.
//   6) Libera el semaforo, vacia la cola, cierra el IOCP y el evento.
void CSocketManager::Clean()
{
	// 1) Señalizar parada cooperativa.
	if (m_ShutdownEvent != nullptr)
	{
		SetEvent(m_ShutdownEvent);
	}

	// 2) Cerrar socket de escucha para hacer fallar WSAAccept y permitir
	//    la salida del hilo de aceptacion.
	if (m_Listen != INVALID_SOCKET)
	{
		closesocket(m_Listen);
		m_Listen = INVALID_SOCKET;
	}

	// 3) Despertar hilos worker publicando paquetes vacios en el completion port.
	if (m_CompletionPort != nullptr)
	{
		for (DWORD n = 0; n < m_ServerWorkerThreadCount; ++n)
		{
			PostQueuedCompletionStatus(m_CompletionPort, 0, 0, nullptr);
		}
	}

	// 4) Señalizar hilo de cola (el evento de parada tambien hara que
	//    WaitForMultipleObjects termine, pero liberamos el semaforo por
	//    si el hilo esta justo evaluando la cola).
	if (m_ServerQueueSemaphore != nullptr)
	{
		ReleaseSemaphore(m_ServerQueueSemaphore, 1, nullptr);
	}

	// 5) Esperar a que los hilos terminen correctamente, en orden:
	//    cola -> workers -> accept.
	if (m_ServerQueueThread != nullptr)
	{
		if (WaitForSingleObject(m_ServerQueueThread, DEFAULT_TIME_WAIT) == WAIT_TIMEOUT)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - Clean] Timeout esperando ServerQueueThread al detenerse.");
		}
		CloseHandle(m_ServerQueueThread);
		m_ServerQueueThread = nullptr;
	}

	for (DWORD n = 0; n < m_ServerWorkerThreadCount; ++n)
	{
		if (m_ServerWorkerThread[n] != nullptr)
		{
			if (WaitForSingleObject(m_ServerWorkerThread[n], DEFAULT_TIME_WAIT) == WAIT_TIMEOUT)
			{
				Log.ToDisp(LOG_RED, "[SocketManager - Clean] Timeout esperando ServerWorkerThread %lu al detenerse.", n);
			}
			CloseHandle(m_ServerWorkerThread[n]);
			m_ServerWorkerThread[n] = nullptr;
		}
	}

	if (m_ServerAcceptThread != nullptr)
	{
		if (WaitForSingleObject(m_ServerAcceptThread, DEFAULT_TIME_WAIT) == WAIT_TIMEOUT)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - Clean] Timeout esperando ServerAcceptThread al detenerse.");
		}
		CloseHandle(m_ServerAcceptThread);
		m_ServerAcceptThread = nullptr;
	}

	// 6) Liberar el resto de los recursos.
	if (m_ServerQueueSemaphore != nullptr)
	{
		CloseHandle(m_ServerQueueSemaphore);
		m_ServerQueueSemaphore = nullptr;
	}

	m_ServerQueue.ClearQueue();

	if (m_CompletionPort != nullptr)
	{
		CloseHandle(m_CompletionPort);
		m_CompletionPort = nullptr;
	}

	if (m_ShutdownEvent != nullptr)
	{
		CloseHandle(m_ShutdownEvent);
		m_ShutdownEvent = nullptr;
	}
}
