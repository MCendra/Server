// SocketManager.cpp
//
// Gestor de sockets basado en I/O Completion Ports (IOCP) para Mu Online
// Season 6 Episodio 5 (ConnectServer / JoinServer).
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
//     C1/C2) y encolarlos para su procesamiento por ConnectServerProtocolCore.
//   - Enviar datos a los clientes usando WSASend con buffer secundario
//     (side buffer) para no perder datos si hay un envio en curso.
//   - Apagado cooperativo y ordenado de todos los hilos (Clean()).
#include "SocketManager.h"
#include "ServerManager.h"
#include "AllowableIpList.h"
#include "Log.h"

CSocketManager gSocketManager;

// Construccion / Destruccion

CSocketManager::CSocketManager()
	: m_listen(INVALID_SOCKET),         // Socket de escucha invalido hasta CreateListenSocket().
	m_CompletionPort(nullptr),          // Puerto de finalizacion aun no creado.
	m_port(0),                          // Puerto TCP, se asigna en Init().
	m_ServerAcceptThread(nullptr),      // Hilo de aceptacion, se crea en CreateAcceptThread().
	m_ServerQueueSemaphore(nullptr),    // Semaforo de la cola, se crea en CreateServerQueue().
	m_ServerQueueThread(nullptr),       // Hilo de la cola, se crea en CreateServerQueue().
	m_ServerWorkerThreadCount(0),       // Cantidad real de hilos de trabajo (segun CPUs).
	m_shutdownEvent(nullptr)            // Evento de apagado cooperativo (manual-reset).
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
	m_port = port;

	// Evento de parada manual-reset: una vez señalado (SetEvent), permanece
	// señalado para todos los hilos que lo consulten (no se auto-resetea).
	if ((m_shutdownEvent = CreateEvent(nullptr, true, false, nullptr)) == nullptr)
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

	Log.ToDisp(LOG_BLACK, "[SocketManager - Init] Servidor TCP iniciado en el puerto [%d]", m_port);
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
//   - bind() sobre INADDR_ANY:m_port.
//   - listen() con el backlog por defecto.
bool CSocketManager::CreateListenSocket()
{
	if ((m_listen = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateListenSocket] WSASocket() fallo con el error: %d", WSAGetLastError());
		return false;
	}

	// FIX: SO_REUSEADDR permite volver a hacer bind() sobre el mismo puerto
	// inmediatamente despues de cerrar el servidor (muy util al recompilar
	// y relanzar el servidor seguido durante el desarrollo en VS2026).
	BOOL reuseAddr = true;
	if (setsockopt(m_listen, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR)
	{
		// No es fatal: solo se informa y se continua.
		Log.ToDisp(LOG_RED, "[SocketManager - CreateListenSocket] setsockopt(SO_REUSEADDR) fallo con el error: %d", WSAGetLastError());
	}

	SOCKADDR_IN SocketAddr{};
	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	SocketAddr.sin_port = htons(m_port);

	if (bind(m_listen, (sockaddr*)&SocketAddr, sizeof(SocketAddr)) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateListenSocket] bind() fallo en el puerto %d con el error: %d", m_port, WSAGetLastError());
		return false;
	}

	if (listen(m_listen, DEFAULT_BACKLOG) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - CreateListenSocket] listen() fallo con el error: %d", WSAGetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, "[SocketManager - CreateListenSocket] Socket de escucha creado exitosamente en el puerto: %d", m_port);

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
// Apagado / liberacion de recursos
// =====================================================================

// Libera de forma ordenada todos los recursos utilizados por el gestor
// de sockets. Es seguro llamarla aunque Init() no haya llegado a
// completarse (todos los miembros se chequean contra su valor "vacio").
//
// Secuencia de apagado cooperativo:
//   1) Señaliza m_shutdownEvent: cualquier hilo que este esperando en el
//      (ServerAcceptThread, ServerQueueThread) lo detecta y termina.
//   2) Cierra el socket de escucha: hace que WSAAccept() falle de
//      inmediato si ServerAcceptThread estaba bloqueado en el.
//   3) Publica un paquete "vacio" por cada worker en el IOCP: cada
//      ServerWorkerThread recibe IoSize=0/index=0/lpOverlapped=nullptr,
//      lo reconoce como señal de apagado y termina.
//   4) Libera el semaforo de la cola una vez para destrabar
//      ServerQueueThread (que tambien esta esperando m_shutdownEvent).
//   5) Espera (con timeout) a que cada hilo termine y cierra sus handles.
//   6) Libera el semaforo, vacia la cola, cierra el IOCP y el evento.
void CSocketManager::Clean()
{
	// 1) Señalizar parada cooperativa.
	if (m_shutdownEvent != nullptr)
	{
		SetEvent(m_shutdownEvent);
	}

	// 2) Cerrar socket de escucha para hacer fallar WSAAccept y permitir
	//    la salida del hilo de aceptacion.
	if (m_listen != INVALID_SOCKET)
	{
		closesocket(m_listen);
		m_listen = INVALID_SOCKET;
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
		for (int n = 0; n < MAX_SERVER; n++)
		{
			if (gServerManager[n].IsOnline())
			{
				Disconnect(n);
			}
		}
		CloseHandle(m_CompletionPort);
		m_CompletionPort = nullptr;
	}

	if (m_shutdownEvent != nullptr)
	{
		CloseHandle(m_shutdownEvent);
		m_shutdownEvent = nullptr;
	}
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
bool CSocketManager::DataRecv(int serverIndex, IO_RECV_BUFFER* lpIoBuffer)
{
	BYTE* lpMsg = lpIoBuffer->Buffer; // Puntero al inicio de los datos pendientes.

	int count = 0;

	while (true)
	{
		int available = lpIoBuffer->Size - count;

		// ¿Queda al menos 1 byte para leer la cabecera?
		if (available <= 0)
		{
			break;
		}

		int size = 0;
		BYTE protocolhead = 0;

		if (lpMsg[count] == PACKET_HEADER_C1)
		{
			// Cabecera C1: necesitamos al menos 3 bytes (header+size+head).
			if (available < 3)
			{
				// FIX: datos insuficientes para leer la cabecera completa.
				// No es un error de protocolo: esperamos al proximo recv.
				break;
			}

			size = lpMsg[count + 1];
			protocolhead = lpMsg[count + 2];
		}
		else if (lpMsg[count] == PACKET_HEADER_C2)
		{
			// Cabecera C2: necesitamos al menos 4 bytes (header+size_hi+size_lo+head).
			if (available < 4)
			{
				// FIX: igual que el caso anterior, esperar mas datos.
				break;
			}

			// Forma explicita: size = (size_hi << 8) | size_lo. Se evita
			// MAKEWORD() para que el orden de bytes del protocolo C2
			// (sizeH luego sizeL) quede explicito sin depender de la
			// convencion de parametros de esa macro.
			size = ((WORD)lpMsg[count + 1] << 8) | lpMsg[count + 2];
			protocolhead = lpMsg[count + 3];
		}
		else
		{
			// Byte de cabecera desconocido: esto si es un error de protocolo real.
			Log.ToDisp(LOG_RED, "[SocketManager - DataRecv] Error de cabecera del protocolo (Header: %02X)", lpMsg[count]);
			return false;
		}

		// Tamaño de paquete fuera de rango: error de protocolo real.
		// FIX: el minimo valido depende del tipo de cabecera. C1 requiere
		// al menos 3 bytes (header+size+head). C2 requiere un tamaño
		// minimo de 4 bytes para contener la cabecera completa
		// (header + sizeH + sizeL + head); un "size" menor a 4 ni
		// siquiera alcanza para la propia cabecera.
		int minSize = (lpMsg[count] == PACKET_HEADER_C1) ? 3 : 4;

		if (size < minSize || size > MAX_RECV_PACKET_SIZE)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - DataRecv] Error de tamaño del protocolo (Index: %d, Size: %d, ProtocolHead: %02X)", serverIndex, size, protocolhead);
			return false;
		}

		// ¿Llego el paquete completo (incluyendo payload)?
		if (size <= available)
		{
			QUEUE_INFO QueueInfo{};
			if (!SERVER_RANGE(serverIndex))
			{
				Log.ToDisp(LOG_RED, "[SocketManager - DataRecv] Servidor invalido (Index: %d)", serverIndex);
				return false;
			}
			QueueInfo.ServerIndex = static_cast<WORD>(serverIndex);
			QueueInfo.ProtocolHead = protocolhead;
			memcpy(QueueInfo.Buffer, &lpMsg[count], size);
			QueueInfo.Size = static_cast<WORD>(size);

			// Encola el paquete para que ServerQueueThread lo procese.
			if (m_ServerQueue.AddToQueue(&QueueInfo) != 0)
			{
				ReleaseSemaphore(m_ServerQueueSemaphore, 1, nullptr);
			}
			else
			{
				// La cola esta llena: no podemos seguir aceptando paquetes
				// de este servidor, se desconecta.
				Log.ToDisp(LOG_RED, "[SocketManager - DataRecv] Cola del servidor llena, paquete descartado (Index: %d, ProtocolHead: %02X)", serverIndex, protocolhead);
				return false;
			}

			count += size; // Avanza el cursor de lectura dentro del buffer.

			// Si ya no queda nada mas por procesar, terminamos.
			if (count >= lpIoBuffer->Size)
			{
				count = 0;
				lpIoBuffer->Size = 0;
				break;
			}

			// Si queda mas data, seguimos el while para intentar parsear
			// otro paquete dentro del mismo buffer (varios paquetes
			// pueden llegar concatenados en un mismo recv).
		}
		else
		{
			// Paquete incompleto: aun falta payload. Salimos del bucle y
			// esperamos mas datos en el proximo WSARecv.
			break;
		}
	}

	// Si quedo un fragmento incompleto al final del buffer (count > 0),
	// lo movemos al comienzo para que el proximo recv lo complete.
	if (count > 0)
	{
		int remaining = lpIoBuffer->Size - count;

		if (remaining > 0)
		{
			memmove(lpMsg, &lpMsg[count], remaining);
		}

		lpIoBuffer->Size = remaining;
	}

	return true; // Datos procesados correctamente (con o sin remanente).
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
bool CSocketManager::DataSend(int serverIndex, BYTE* lpMsg, int size)
{
	if (SERVER_RANGE(serverIndex) == 0)
	{
		return false;
	}

	CServerManager* lpServerManager = &gServerManager[serverIndex];

	CCriticalSection::CLock lock(lpServerManager->m_lock);

	if (lpServerManager->IsOnline() == false)
	{
		return false;
	}

	if (size > MAX_SEND_PACKET_SIZE)
	{
		Log.ToDisp(LOG_RED, "[SocketManager - DataSend] Tamaño maximo de mensaje excedido (Tipo: 1, indice: %d, Tamaño: %d)", serverIndex, size);
		return false;
	}

	IO_SEND_CONTEXT* lpIoContext = lpServerManager->m_IoSendContext;

	if (lpIoContext->IoSize > 0)
	{
		// Ya hay un envio en curso: acumulamos en el buffer secundario.
		int IoSideBufferSizeTotal = lpIoContext->IoSideBuffer.Size + size;

		if (IoSideBufferSizeTotal > MAX_SEND_SIDE_PACKET_SIZE)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - DataSend] Tamaño maximo de mensaje excedido (Tipo: 2, Índice: %d, Tamaño: %d)", serverIndex, IoSideBufferSizeTotal);
			Disconnect(serverIndex);
			return false;
		}

		memcpy(&lpIoContext->IoSideBuffer.Buffer[lpIoContext->IoSideBuffer.Size], lpMsg, size);
		lpIoContext->IoSideBuffer.Size += size;
		return true;
	}

	// Arma y envia desde el buffer principal.
	memcpy(lpIoContext->IoSendBuffer.Buffer, lpMsg, size);

	lpIoContext->WSAbuf.buf = (char*)lpIoContext->IoSendBuffer.Buffer;
	lpIoContext->WSAbuf.len = size;
	lpIoContext->IoType = IO_SEND;
	lpIoContext->IoSize = size;
	lpIoContext->IoSendBuffer.Size = 0;

	DWORD SendSize = 0, Flags = 0;

	if (WSASend(lpServerManager->m_socket, &lpIoContext->WSAbuf, 1, &SendSize, Flags, &lpIoContext->OverLapped, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - DataSend] WSASend() fallo con error: %d", WSAGetLastError());
			Disconnect(serverIndex);
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
	if (SERVER_RANGE(index) == 0)
	{
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	// En x86, SOCKET = UINT_PTR = 32 bits = mismo tamaño que LONG.
	// InterlockedExchange garantiza que solo un hilo obtiene el socket
	// válido y ejecuta closesocket(), sin necesidad de tomar m_lock.
	SOCKET s = (SOCKET)InterlockedExchange(
		reinterpret_cast<volatile LONG*>(&lpServerManager->m_socket),
		(LONG)INVALID_SOCKET
	);

	if (s != INVALID_SOCKET)
	{
		if (closesocket(s) == SOCKET_ERROR && WSAGetLastError() != WSAENOTSOCK)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - Disconnect] closesocket() fallo con el error: %d", WSAGetLastError());
		}
		// El IOCP entregará las cancelaciones con IoSize=0.
		// OnRecv/OnSend llamarán a DelServer() cuando las reciban.
	}
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
void CSocketManager::OnRecv(int serverIndex, DWORD IoSize, IO_RECV_CONTEXT* lpIoContext)
{
	if (SERVER_RANGE(serverIndex) == 0)
	{
		return;
	}

	CServerManager* lpServerManager = &gServerManager[serverIndex];

	// Lock por servidor: protege m_socket, m_state y el contenido de
	// lpIoContext (IoMainBuffer, wsabuf) contra DataSend/DelClient
	// ejecutándose en paralelo para este mismo servidor.
	CCriticalSection::CLock lock(lpServerManager->m_lock);

	if (IoSize == 0)
	{
		// Conexión cerrada por el peer, o cancelación de closesocket().
		// AHORA es seguro liberar: el IOCP ya no emitirá más eventos
		// para este overlapped. Llamamos DelServer SIN pasar por
		// Disconnect (el socket ya fue cerrado antes de llegar aquí).
		lpServerManager->DelServer();
		return;
	}

	// Verificación adicional: si el estado ya es OFFLINE Disconnect()
	// fue llamado entre el WSARecv y esta completo, ignoramos.
	if (lpServerManager->m_state == SERVER_OFFLINE)
		return;

	// Validación de seguridad: Si los nuevos bytes superan la capacidad del buffer, desconectamos.
	if (lpIoContext->IoRecvBuffer.Size < 0 ||
		lpIoContext->IoRecvBuffer.Size > MAX_RECV_PACKET_SIZE ||
		IoSize > static_cast<DWORD>(MAX_RECV_PACKET_SIZE - lpIoContext->IoRecvBuffer.Size))
	{
		Log.ToDisp(LOG_RED, "[SocketManager - OnRecv] Buffer overflow detectado (Index: %d, Size: %d, Recv: %d)",
			serverIndex, lpIoContext->IoRecvBuffer.Size, IoSize);
		Disconnect(serverIndex);
		return;
	}

	lpIoContext->IoRecvBuffer.Size += IoSize;

	if (DataRecv(serverIndex, &lpIoContext->IoRecvBuffer) == 0)
	{
		// Error real de protocolo (cabecera invalida, tamaño fuera de
		// rango o cola llena): se desconecta al servidor.
		Disconnect(serverIndex);
		return;
	}

	// Prepara el siguiente recv a partir del remanente dejado por
	// DataRecv (que ya quedo alineado al comienzo del buffer).
	lpIoContext->WSAbuf.buf = (char*)&lpIoContext->IoRecvBuffer.Buffer[lpIoContext->IoRecvBuffer.Size];
	lpIoContext->WSAbuf.len = MAX_RECV_PACKET_SIZE - lpIoContext->IoRecvBuffer.Size;
	lpIoContext->IoType = IO_RECV;

	DWORD RecvSize = 0, Flags = 0;

	if (WSARecv(lpServerManager->m_socket, &lpIoContext->WSAbuf, 1, &RecvSize, &Flags, &lpIoContext->OverLapped, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - OnRecv] WSARecv() failed with error: %d", WSAGetLastError());
			Disconnect(serverIndex);
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
void CSocketManager::OnSend(int serverIndex, DWORD IoSize, IO_SEND_CONTEXT* lpIoContext)
{
	if (SERVER_RANGE(serverIndex) == 0)
	{
		return;
	}

	CServerManager* lpServerManager = &gServerManager[serverIndex];

	// Lock por servidor: protege m_socket y lpIoContext (IoMainBuffer,
	// IoSideBuffer, wsabuf, IoSize) frente a DataSend del mismo servidor.
	CCriticalSection::CLock lock(lpServerManager->m_lock);

	if (IoSize == 0)
	{
		// Ídem OnRecv: cancelación confirmada por IOCP → seguro liberar.
		// ATENCIÓN: solo llamamos DelClient si OnRecv no lo hizo ya.
		// DelClient es idempotente (chequea m_state antes de hacer nada).
		lpServerManager->DelServer();
		return;
	}

	if (lpServerManager->m_state == SERVER_OFFLINE)
		return;

	lpIoContext->IoSendBuffer.Size += IoSize;

	if (lpIoContext->IoSendBuffer.Size >= lpIoContext->IoSize)
	{
		// Se completo el envio del mensaje actual.
		if (lpIoContext->IoSideBuffer.Size <= 0)
		{
			// Nada más por enviar: IoSize=0 marca que no hay envío en curso,
			// permitiendo que el próximo DataSend() dispare un WSASend directo
			// en vez de acumular en el side buffer.
			lpIoContext->IoSize = 0;
			return;
		}

		if (lpIoContext->IoSideBuffer.Size > MAX_SEND_PACKET_SIZE)
		{
			// El side buffer no entra entero: enviamos solo el primer
			// bloque de MAX_SEND_PACKET_SIZE bytes y dejamos el resto
			// para la proxima vuelta.
			memcpy(lpIoContext->IoSendBuffer.Buffer, lpIoContext->IoSideBuffer.Buffer, MAX_SEND_PACKET_SIZE);

			lpIoContext->WSAbuf.buf = (char*)lpIoContext->IoSendBuffer.Buffer;
			lpIoContext->WSAbuf.len = MAX_SEND_PACKET_SIZE;
			lpIoContext->IoType = IO_SEND;
			lpIoContext->IoSize = MAX_SEND_PACKET_SIZE;
			lpIoContext->IoSendBuffer.Size = 0;

			// Desplaza el remanente del side buffer al comienzo.
			memmove(lpIoContext->IoSideBuffer.Buffer, &lpIoContext->IoSideBuffer.Buffer[MAX_SEND_PACKET_SIZE], (lpIoContext->IoSideBuffer.Size - MAX_SEND_PACKET_SIZE));
			lpIoContext->IoSideBuffer.Size = lpIoContext->IoSideBuffer.Size - MAX_SEND_PACKET_SIZE;
		}
		else
		{
			// El side buffer entra completo: lo movemos entero al
			// buffer principal y lo vaciamos.
			memcpy(lpIoContext->IoSendBuffer.Buffer, lpIoContext->IoSideBuffer.Buffer, lpIoContext->IoSideBuffer.Size);

			lpIoContext->WSAbuf.buf = (char*)lpIoContext->IoSendBuffer.Buffer;
			lpIoContext->WSAbuf.len = lpIoContext->IoSideBuffer.Size;
			lpIoContext->IoType = IO_SEND;
			lpIoContext->IoSize = lpIoContext->IoSideBuffer.Size;
			lpIoContext->IoSendBuffer.Size = 0;
			lpIoContext->IoSideBuffer.Size = 0;
		}
	}
	else
	{
		// Envio parcial: queda pendiente el resto del mensaje actual.
		lpIoContext->WSAbuf.buf = (char*)&lpIoContext->IoSendBuffer.Buffer[lpIoContext->IoSendBuffer.Size];
		lpIoContext->WSAbuf.len = lpIoContext->IoSize - lpIoContext->IoSendBuffer.Size;
		lpIoContext->IoType = IO_SEND;
	}

	DWORD SendSize = 0, Flags = 0;

	if (WSASend(lpServerManager->m_socket, &lpIoContext->WSAbuf, 1, &SendSize, Flags, &lpIoContext->OverLapped, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - OnSend] WSASend() failed with error: %d", WSAGetLastError());
			Disconnect(serverIndex);
			return;
		}
	}
}

// =====================================================================
// Aceptacion de conexiones
// =====================================================================

// Condicion de aceptacion de WSAAccept(): se ejecuta de forma sincrona
// por cada intento de conexion entrante, ANTES de que el socket sea
// aceptado, y permite rechazarlo (CF_REJECT) segun la IP de origen.
//
// FIX: el ultimo parametro (dwCallbackData) se declara como DWORD_PTR
// para que coincida con el tipo real que usa WSAAccept en compilaciones
// de 64 bits. Antes se truncaba el puntero "this" a 32 bits al castearlo
// a DWORD; aunque este parametro no se usa actualmente (queda
// reservado para futuro uso, p.ej. loggear desde que instancia se
// llamo), evita un cast con perdida de datos y warnings del compilador.
int CALLBACK CSocketManager::ServerAcceptCondition(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN OUT LPQOS lpSQOS, IN OUT LPQOS lpGQOS, IN LPWSABUF lpCalleeId, OUT LPWSABUF lpCalleeData, OUT GROUP FAR* g, DWORD_PTR dwCallbackData)
{
	UNREFERENCED_PARAMETER(lpCallerData);
	UNREFERENCED_PARAMETER(lpSQOS);
	UNREFERENCED_PARAMETER(lpGQOS);
	UNREFERENCED_PARAMETER(lpCalleeId);
	UNREFERENCED_PARAMETER(lpCalleeData);
	UNREFERENCED_PARAMETER(g);
	UNREFERENCED_PARAMETER(dwCallbackData);

	SOCKADDR_IN* SocketAddr = (SOCKADDR_IN*)lpCallerId->buf;

	char ipStr[INET_ADDRSTRLEN];
	if (InetNtopA(AF_INET, &SocketAddr->sin_addr, ipStr, INET_ADDRSTRLEN) == nullptr)
	{
		return CF_REJECT;
	}

	// Consulta al IpManager (listas blancas/negras, limite de
	// conexiones por IP, etc.) para decidir si se acepta la conexion.
	if (gAllowableIpList.CheckAllowableIp(ipStr) == 0)
	{
		return CF_REJECT;
	}
	else
	{
		return CF_ACCEPT;
	}
}

// Hilo principal de aceptacion de conexiones.
//
// Bucle:
//   1) Si m_shutdownEvent esta señalado, termina el hilo.
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
		if (lpSocketManager->m_shutdownEvent && WaitForSingleObject(lpSocketManager->m_shutdownEvent, 0) == WAIT_OBJECT_0)
		{
			break;
		}

		// FIX: se castea el puntero a DWORD_PTR (no DWORD) para que sea
		// valido tanto en x86 como en x64.
		SOCKET socket = WSAAccept(lpSocketManager->m_listen, (sockaddr*)&SocketAddr, &SocketAddrSize, (LPCONDITIONPROC)&lpSocketManager->ServerAcceptCondition, (DWORD_PTR)lpSocketManager);

		if (socket == INVALID_SOCKET)
		{
			// Si el listen socket fue cerrado durante el shutdown, WSAAccept
			// falla de inmediato: comprobamos el evento de parada.
			if (lpSocketManager->m_shutdownEvent && WaitForSingleObject(lpSocketManager->m_shutdownEvent, 0) == WAIT_OBJECT_0)
			{
				break;
			}

			int Error = WSAGetLastError();

			if (Error != WSAEWOULDBLOCK)
			{
				Log.ToDisp(LOG_RED, "[SocketManager - ServerAcceptThread] WSAAccept() fallo con error: %d", Error);
			}
			continue;
		}

		char ipAddress[INET_ADDRSTRLEN];

		if (InetNtopA(AF_INET, &SocketAddr.sin_addr, ipAddress, INET_ADDRSTRLEN) == nullptr)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - ServerAcceptThread] InetNtopA() fallo con error: %d", GetLastError());
			closesocket(socket);
			continue;
		}

		// Busca un indice (slot) libre para el nuevo servidor.
		// GetFreeServerIndex se sincroniza internamente con gServerArrayLock.
		int index = gServerManager[0].GetFreeServerIndex();

		if (index == -1)
		{
			closesocket(socket);
			continue;
		}

		// Asocia el socket recien aceptado al IOCP. La "completion key"
		// (tercer parametro) sera el indice del servidor, y se recibira
		// en GetQueuedCompletionStatus para identificar a que servidor
		// corresponde cada evento de E/S.
		if (CreateIoCompletionPort((HANDLE)socket, lpSocketManager->m_CompletionPort, index, 0) == nullptr)
		{
			Log.ToDisp(LOG_RED, "[SocketManager - ServerAcceptThread] CreateIoCompletionPort() fallo con error: %d", GetLastError());
			closesocket(socket);
			continue;
		}

		CServerManager* lpServerManager = &gServerManager[index];

		// AddServer toma internamente m_lock (del servidor) y
		// gServerArrayLock (para gServerSearchStart); deja los IO contexts
		// inicializados y listos para WSARecv.
		lpServerManager->AddServer(index, ipAddress, socket);

		DWORD RecvSize = 0, Flags = 0;

		// Dispara la primera recepcion asincrona para este cliente.
		if (WSARecv(socket, &lpServerManager->m_IoRecvContext->WSAbuf, 1, &RecvSize, &Flags, &lpServerManager->m_IoRecvContext->OverLapped, 0) == SOCKET_ERROR)
		{
			int Error = WSAGetLastError();

			if (Error != WSA_IO_PENDING)
			{
				Log.ToDisp(LOG_RED, "[SocketManager - ServerAcceptThread] WSARecv() fallo con error: %d", Error);
				lpSocketManager->Disconnect(index);
				continue;
			}
		}
	}
	return 0;
}

// =====================================================================
// Hilos de trabajo (IOCP) y de cola de paquetes
// =====================================================================

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
	DWORD IoSize;
	DWORD Index;
	LPOVERLAPPED lpOverlapped;

	while (true)
	{
		if (GetQueuedCompletionStatus(lpSocketManager->m_CompletionPort, &IoSize, &Index, &lpOverlapped, INFINITE) == 0)
		{
			DWORD Error = GetLastError();

			if (lpOverlapped == nullptr ||
				(Error != ERROR_NETNAME_DELETED &&
					Error != ERROR_CONNECTION_ABORTED &&
					Error != ERROR_OPERATION_ABORTED &&
					Error != ERROR_SEM_TIMEOUT))
			{
				Log.ToDisp(LOG_RED, "[SocketManager - ServerWorkerThread] GetQueuedCompletionStatus() fallo con error: %d", Error);
				return 1;
			}
			// Error esperado de desconexion: se sigue procesando con
			// IoSize == 0 para que OnRecv/OnSend desconecten al cliente.
		}

		// Señal de apagado publicada por Clean(): IoSize=0, index=0,
		// lpOverlapped=nullptr (GQCS retorna exito para este post).
		if (IoSize == 0 && Index == 0)
		{
			if (lpOverlapped == nullptr) return 0;
		}

		IO_CONTEXT* lpIoContext = (IO_CONTEXT*)lpOverlapped;

		switch (lpIoContext->IoType)
		{
		case IO_RECV:
			lpSocketManager->OnRecv(Index, IoSize, (IO_RECV_CONTEXT*)lpIoContext);
			break;
		case IO_SEND:
			lpSocketManager->OnSend(Index, IoSize, (IO_SEND_CONTEXT*)lpIoContext);
			break;
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
//   - m_shutdownEvent: señal de apagado cooperativo.
//
// Si el handle señalado es el evento de parada, el hilo termina. En
// caso contrario, intenta sacar un paquete de la cola y, si el cliente
// asociado sigue conectado, lo procesa.
DWORD WINAPI CSocketManager::ServerQueueThread(CSocketManager* lpSocketManager)
{
	HANDLE handles[2] = { lpSocketManager->m_ServerQueueSemaphore, lpSocketManager->m_shutdownEvent };

	while (true)
	{
		DWORD waitResult = WaitForMultipleObjects(2, handles, false, INFINITE);

		switch (waitResult)
		{
		case WAIT_OBJECT_0: // Semaphore señalado
		{
			QUEUE_INFO QueueInfo;

			if (lpSocketManager->m_ServerQueue.GetFromQueue(&QueueInfo) != 0)
			{
				if (SERVER_RANGE(QueueInfo.ServerIndex) != 0 && gServerManager[QueueInfo.ServerIndex].IsOnline() != false)
				{
					DataServerProtocolCore(QueueInfo.ServerIndex, QueueInfo.ProtocolHead, QueueInfo.Buffer, QueueInfo.Size);
				}
			}
		}
		break;

		case WAIT_OBJECT_0 + 1: // ShutdownEvent señalado
		{
			Log.ToDisp(LOG_BLUE, "[SocketManager - ServerQueueThread] Finalizando hilo de cola.");

			return 0;
		}
		case WAIT_FAILED:
		{
			Log.ToDisp(LOG_RED, "[SocketManager - ServerQueueThread] WaitForMultipleObjects() fallo con error: %lu", GetLastError());

			return 0;
		}
		default:
		{
			Log.ToDisp(LOG_RED, "[SocketManager - ServerQueueThread] Resultado inesperado: %lu", waitResult);

			return 0;
		}
		}
	}
	return 0;
}

// Utilidades

// Devuelve la cantidad actual de paquetes pendientes en la cola interna.
DWORD CSocketManager::GetQueueSize()
{
	return m_ServerQueue.GetQueueSize();
}
