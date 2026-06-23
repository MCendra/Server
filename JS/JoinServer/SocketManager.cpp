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
//
// CORRECCIONES APLICADAS EN ESTA REVISIÓN (ver comentarios "// FIX:"):
//   1) DataRecv: ya no desconecta al cliente cuando llega un fragmento TCP
//      mas pequeño que la cabecera del protocolo (bug critico de estabilidad).
//   2) ServerAcceptCondition / WSAAccept: se evita el truncamiento del
//      puntero "this" en compilaciones de 64 bits (DWORD -> DWORD_PTR).
//   3) CreateListenSocket: se agrega SO_REUSEADDR para permitir reinicios
//      rapidos del servidor durante el desarrollo/pruebas.
#include "SocketManager.h"
#include "ServerManager.h"
#include "AllowableIpList.h"
#include "Log.h"
#include "Util.h"

CSocketManager gSocketManager;

// =====================================================================
// Mensajes de log (constantes para evitar strings "magicos" repetidos)
// =====================================================================

constexpr char INIT_CREATE_SHUTDOWN_EVENT_ERROR_MSG[] = "[JS] CSocketManager::Init() - Error al crear evento de shutdown: %d";
constexpr char INIT_SUCCESS_MSG[] = "[JS] CSocketManager::Init() - Servidor iniciado en el puerto [%d]";

constexpr char CREATELISTENSOCKET_ERROR_WSA_SOCKET[] = "[SocketManager] WSASocket() fallo con el error: %d";
constexpr char CREATELISTENSOCKET_ERROR_SETSOCKOPT[] = "[SocketManager] setsockopt(SO_REUSEADDR) fallo con el error: %d";
constexpr char CREATELISTENSOCKET_ERROR_BIND[] = "[SocketManager] bind() fallo en el puerto %d con el error: %d";
constexpr char CREATELISTENSOCKET_ERROR_LISTEN[] = "[SocketManager] listen() fallo con el error: %d";
constexpr char CREATELISTENSOCKET_SUCCESS_MSG[] = "[SocketManager] Socket de escucha creado exitosamente en el puerto: %d";

constexpr char CREATECOMPLETIONPORT_ERROR_MSG[] = "[SocketManager] Error al crear el puerto de finalizacion de E/S.";
constexpr char CREATECOMPLETIONPORT_SUCCESS_MSG[] = "[SocketManager] Puerto de finalizacion de E/S creado exitosamente.";

constexpr char CREATEACCEPTTHREAD_ERROR_CREATETHREAD[] = "[SocketManager] Error al crear el hilo de aceptacion de conexiones.";
constexpr char CREATEACCEPTTHREAD_ERROR_SETTHREADPRIORITY[] = "[SocketManager] Error al establecer la prioridad del hilo de aceptacion de conexiones.";
constexpr char CREATEACCEPTTHREAD_SUCCESS_MSG[] = "[SocketManager] Hilo de aceptacion de conexiones creado y configurado exitosamente.";

constexpr char CREATEWORKERTHREAD_ERROR_CREATETHREAD[] = "[SocketManager] Error al crear el hilo de trabajo %u. Codigo: %lu";
constexpr char CREATEWORKERTHREAD_ERROR_SETTHREADPRIORITY[] = "[SocketManager] Error al establecer la prioridad del hilo de trabajo %u. Codigo: %lu";
constexpr char CREATEWORKERTHREAD_SUCCESS_MSG[] = "[SocketManager] %d hilos de trabajo creados exitosamente.";

constexpr char CREATESERVERQUEUE_ERROR_CREATESEMAPHORE[] = "[SocketManager] Error al crear el semaforo de la cola del servidor. Codigo: %lu";
constexpr char CREATESERVERQUEUE_ERROR_CREATETHREAD[] = "[SocketManager] Error al crear el hilo de la cola del servidor. Codigo: %lu";
constexpr char CREATESERVERQUEUE_ERROR_SETTHREADPRIORITY[] = "[SocketManager] Error al establecer la prioridad del hilo de la cola del servidor. Codigo: %lu";
constexpr char CREATESERVERQUEUE_SUCCESS_MSG[] = "[SocketManager] Semaforo y hilo de cola del servidor creados exitosamente.";

constexpr char CLEAN_TIMEOUT_SERVER_QUEUE_THREAD[] = "[SocketManager] Timeout esperando ServerQueueThread al detenerse.";
constexpr char CLEAN_TIMEOUT_SERVER_WORKER_THREAD[] = "[SocketManager] Timeout esperando ServerWorkerThread %u al detenerse.";
constexpr char CLEAN_TIMEOUT_SERVER_ACCEPT_THREAD[] = "[SocketManager] Timeout esperando ServerAcceptThread al detenerse.";

constexpr char CLOSESOCKET_ERROR_MSG[] = "[SocketManager] closesocket() fallo con el error: %d";

constexpr char SERVERACCEPTTHREAD_ERROR_WSAACCEPT[] = "[SocketManager - ServerAcceptThread] WSAAccept() fallo con error: %d";
constexpr char SERVERACCEPTTHREAD_ERROR_INETNTOPA[] = "[SocketManager - ServerAcceptThread] InetNtopA() fallo con error: %d";
constexpr char SERVERACCEPTTHREAD_ERROR_CREATEIOCP[] = "[SocketManager - ServerAcceptThread] CreateIoCompletionPort() fallo con error: %d";
constexpr char SERVERACCEPTTHREAD_ERROR_WSARECV[] = "[SocketManager - ServerAcceptThread] WSARecv() fallo con error: %d";

constexpr char DATARECV_PROTOCOL_HEADER_ERROR[] = "[SocketManager] Error de cabecera del protocolo (Index: %d, Header: %02X)";
constexpr char DATARECV_PROTOCOL_SIZE_ERROR[] = "[SocketManager] Error de tamaño del protocolo (Index: %d, Size: %d, Head: %02X)";
constexpr char DATARECV_SERVER_QUEUE_FULL[] = "[SocketManager] Server queue full(Index:% d, Head : % 02X)";
constexpr char DATARECV_INVALID_SERVER[] = "[SocketManager - DataRecv] Servidor invalido (Index: %d)";

constexpr char SERVERWORKERTHREAD_GETQUEUEDCOMPLETIONSTATUS_ERROR[] = "[SocketManager - ServerWorkerThread] GetQueuedCompletionStatus() fallo con error: %d";

constexpr char SERVERQUEUETHREAD_WAITFORMULTIPLEOBJECTS_ERROR[] = "[SocketManager - ServerQueueThread] WaitForMultipleObjects() fallo con error: %d";

// =====================================================================
// Construccion / Destruccion
// =====================================================================

// Constructor: inicializa todos los miembros a valores "vacios" / invalidos
// para que CSocketManager::Clean() pueda ejecutarse de forma segura en
// cualquier momento (incluso si Init() falla a mitad de camino).
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
bool CSocketManager::Init(WORD port)
{
	this->m_port = port;

	// Evento de parada manual-reset: una vez señalado (SetEvent), permanece
	// señalado para todos los hilos que lo consulten (no se auto-resetea).
	if ((this->m_shutdownEvent = CreateEvent(nullptr, true, false, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, INIT_CREATE_SHUTDOWN_EVENT_ERROR_MSG, GetLastError());
		this->Clean();
		return false;
	}

	// Socket de escucha (bind + listen).
	if (this->CreateListenSocket() == 0)
	{
		this->Clean();
		return false;
	}

	// Puerto de finalizacion de E/S (IOCP).
	if (this->CreateCompletionPort() == 0)
	{
		this->Clean();
		return false;
	}

	// Hilo que acepta nuevas conexiones entrantes.
	if (this->CreateAcceptThread() == 0)
	{
		this->Clean();
		return false;
	}

	// Hilos de trabajo que procesan los eventos del IOCP (recv/send).
	if (this->CreateWorkerThread() == 0)
	{
		this->Clean();
		return false;
	}

	// Cola interna de paquetes ya parseados + hilo que los despacha
	// hacia ConnectServerProtocolCore.
	if (this->CreateServerQueue() == 0)
	{
		this->Clean();
		return false;
	}

	Log.ToDisp(LOG_BLACK, INIT_SUCCESS_MSG, this->m_port);
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
	if ((this->m_listen = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		Log.ToDisp(LOG_RED, CREATELISTENSOCKET_ERROR_WSA_SOCKET, WSAGetLastError());
		return false;
	}

	// FIX: SO_REUSEADDR permite volver a hacer bind() sobre el mismo puerto
	// inmediatamente despues de cerrar el servidor (muy util al recompilar
	// y relanzar el servidor seguido durante el desarrollo en VS2026).
	BOOL reuseAddr = true;
	if (setsockopt(this->m_listen, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseAddr, sizeof(reuseAddr)) == SOCKET_ERROR)
	{
		// No es fatal: solo se informa y se continua.
		Log.ToDisp(LOG_RED, CREATELISTENSOCKET_ERROR_SETSOCKOPT, WSAGetLastError());
	}

	SOCKADDR_IN SocketAddr;
	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	SocketAddr.sin_port = htons(this->m_port);

	if (bind(this->m_listen, (sockaddr*)&SocketAddr, sizeof(SocketAddr)) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, CREATELISTENSOCKET_ERROR_BIND, this->m_port, WSAGetLastError());
		return false;
	}

	if (listen(this->m_listen, DEFAULT_BACKLOG) == SOCKET_ERROR)
	{
		Log.ToDisp(LOG_RED, CREATELISTENSOCKET_ERROR_LISTEN, WSAGetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, CREATELISTENSOCKET_SUCCESS_MSG, this->m_port);

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
	this->m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	if (this->m_CompletionPort == nullptr)
	{
		Log.ToDisp(LOG_RED, CREATECOMPLETIONPORT_ERROR_MSG, GetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, CREATECOMPLETIONPORT_SUCCESS_MSG);
	return true;
}

// Crea y configura el hilo que acepta conexiones entrantes (WSAAccept en
// bucle dentro de ServerAcceptThread). Se le asigna la prioridad mas alta
// para minimizar la latencia al aceptar nuevos clientes.
bool CSocketManager::CreateAcceptThread()
{
	this->m_ServerAcceptThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)this->ServerAcceptThread, this, 0, nullptr);
	if (this->m_ServerAcceptThread == nullptr)
	{
		Log.ToDisp(LOG_RED, CREATEACCEPTTHREAD_ERROR_CREATETHREAD, GetLastError());
		return false;
	}

	if (SetThreadPriority(this->m_ServerAcceptThread, THREAD_PRIORITY_HIGHEST) == 0)
	{
		Log.ToDisp(LOG_RED, CREATEACCEPTTHREAD_ERROR_SETTHREADPRIORITY, GetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, CREATEACCEPTTHREAD_SUCCESS_MSG);
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

	this->m_ServerWorkerThreadCount = (SystemInfo.dwNumberOfProcessors > MAX_SERVER_WORKER_THREAD)
		? MAX_SERVER_WORKER_THREAD
		: SystemInfo.dwNumberOfProcessors;

	for (DWORD n = 0; n < this->m_ServerWorkerThreadCount; n++)
	{
		this->m_ServerWorkerThread[n] = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CSocketManager::ServerWorkerThread, this, 0, nullptr);

		if (this->m_ServerWorkerThread[n] == nullptr)
		{
			Log.ToDisp(LOG_RED, CREATEWORKERTHREAD_ERROR_CREATETHREAD, n, GetLastError());
			return false;
		}
		// FIX: THREAD_PRIORITY_HIGHEST por THREAD_PRIORITY_NORMAL
		if (SetThreadPriority(this->m_ServerWorkerThread[n], THREAD_PRIORITY_NORMAL) == 0)
		{
			Log.ToDisp(LOG_RED, CREATEWORKERTHREAD_ERROR_SETTHREADPRIORITY, n, GetLastError());
			return false;
		}
	}

	Log.ToDisp(LOG_BLACK, CREATEWORKERTHREAD_SUCCESS_MSG, this->m_ServerWorkerThreadCount);
	return true;
}

// Crea el semaforo de la cola del servidor (contador de paquetes
// pendientes de procesar) y el hilo que los consume y los despacha
// hacia ConnectServerProtocolCore.
bool CSocketManager::CreateServerQueue()
{
	// El semaforo arranca en 0 (sin paquetes pendientes) y permite hasta
	// MAX_QUEUE_SIZE incrementos simultaneos (uno por cada AddToQueue).
	if ((this->m_ServerQueueSemaphore = CreateSemaphore(nullptr, 0, MAX_QUEUE_SIZE, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, CREATESERVERQUEUE_ERROR_CREATESEMAPHORE, GetLastError());
		return false;
	}

	if ((this->m_ServerQueueThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CSocketManager::ServerQueueThread, this, 0, nullptr)) == nullptr)
	{
		Log.ToDisp(LOG_RED, CREATESERVERQUEUE_ERROR_CREATETHREAD, GetLastError());
		return false;
	}

	if (SetThreadPriority(this->m_ServerQueueThread, THREAD_PRIORITY_HIGHEST) == 0)
	{
		Log.ToDisp(LOG_RED, CREATESERVERQUEUE_ERROR_SETTHREADPRIORITY, GetLastError());
		return false;
	}

	Log.ToDisp(LOG_BLACK, CREATESERVERQUEUE_SUCCESS_MSG);
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
	if (this->m_shutdownEvent != nullptr)
	{
		SetEvent(this->m_shutdownEvent);
	}

	// 2) Cerrar socket de escucha para hacer fallar WSAAccept y permitir
	//    la salida del hilo de aceptacion.
	if (this->m_listen != INVALID_SOCKET)
	{
		closesocket(this->m_listen);
		this->m_listen = INVALID_SOCKET;
	}

	// 3) Despertar hilos worker publicando paquetes vacios en el completion port.
	if (this->m_CompletionPort != nullptr)
	{
		for (DWORD n = 0; n < this->m_ServerWorkerThreadCount; ++n)
		{
			PostQueuedCompletionStatus(this->m_CompletionPort, 0, 0, nullptr);
		}
	}

	// 4) Señalizar hilo de cola (el evento de parada tambien hara que
	//    WaitForMultipleObjects termine, pero liberamos el semaforo por
	//    si el hilo esta justo evaluando la cola).
	if (this->m_ServerQueueSemaphore != nullptr)
	{
		ReleaseSemaphore(this->m_ServerQueueSemaphore, 1, nullptr);
	}

	// 5) Esperar a que los hilos terminen correctamente, en orden:
	//    cola -> workers -> accept.
	if (this->m_ServerQueueThread != nullptr)
	{
		if (WaitForSingleObject(this->m_ServerQueueThread, DEFAULT_TIME_WAIT) == WAIT_TIMEOUT)
		{
			Log.ToDisp(LOG_RED, CLEAN_TIMEOUT_SERVER_QUEUE_THREAD);
		}
		CloseHandle(this->m_ServerQueueThread);
		this->m_ServerQueueThread = nullptr;
	}

	for (DWORD n = 0; n < this->m_ServerWorkerThreadCount; ++n)
	{
		if (this->m_ServerWorkerThread[n] != nullptr)
		{
			if (WaitForSingleObject(this->m_ServerWorkerThread[n], DEFAULT_TIME_WAIT) == WAIT_TIMEOUT)
			{
				Log.ToDisp(LOG_RED, CLEAN_TIMEOUT_SERVER_WORKER_THREAD, n);
			}
			CloseHandle(this->m_ServerWorkerThread[n]);
			this->m_ServerWorkerThread[n] = nullptr;
		}
	}

	if (this->m_ServerAcceptThread != nullptr)
	{
		if (WaitForSingleObject(this->m_ServerAcceptThread, DEFAULT_TIME_WAIT) == WAIT_TIMEOUT)
		{
			Log.ToDisp(LOG_RED, CLEAN_TIMEOUT_SERVER_ACCEPT_THREAD);
		}
		CloseHandle(this->m_ServerAcceptThread);
		this->m_ServerAcceptThread = nullptr;
	}

	// 6) Liberar el resto de los recursos.
	if (this->m_ServerQueueSemaphore != nullptr)
	{
		CloseHandle(this->m_ServerQueueSemaphore);
		this->m_ServerQueueSemaphore = nullptr;
	}

	this->m_ServerQueue.ClearQueue();

	if (this->m_CompletionPort != nullptr)
	{
		for (int n = 0; n < MAX_SERVER; n++)
		{
			if (gServerManager[n].IsOnline())
			{
				this->Disconnect(n);
			}
		}
		CloseHandle(this->m_CompletionPort);
		this->m_CompletionPort = nullptr;
	}

	if (this->m_shutdownEvent != nullptr)
	{
		CloseHandle(this->m_shutdownEvent);
		this->m_shutdownEvent = nullptr;
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
bool CSocketManager::DataRecv(int index, IO_MAIN_BUFFER* lpIoBuffer)
{
	BYTE* lpMsg = lpIoBuffer->buff; // Puntero al inicio de los datos pendientes.

	int count = 0;

	while (true)
	{
		int available = lpIoBuffer->size - count;

		// ¿Queda al menos 1 byte para leer la cabecera?
		if (available <= 0)
		{
			break;
		}

		int size = 0;
		BYTE head = 0;

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
			head = lpMsg[count + 2];
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
			head = lpMsg[count + 3];
		}
		else
		{
			// Byte de cabecera desconocido: esto si es un error de protocolo real.
			Log.ToDisp(LOG_RED, DATARECV_PROTOCOL_HEADER_ERROR, index, lpMsg[count]);
			return false;
		}

		// Tamaño de paquete fuera de rango: error de protocolo real.
		// FIX: el minimo valido depende del tipo de cabecera. C1 requiere
		// al menos 3 bytes (header+size+head). C2 requiere un tamaño
		// minimo de 4 bytes para contener la cabecera completa
		// (header + sizeH + sizeL + head); un "size" menor a 4 ni
		// siquiera alcanza para la propia cabecera.
		int minSize = (lpMsg[count] == PACKET_HEADER_C1) ? 3 : 4;

		if (size < minSize || size > MAX_MAIN_PACKET_SIZE)
		{
			Log.ToDisp(LOG_RED, DATARECV_PROTOCOL_SIZE_ERROR, index, size, head);
			return false;
		}

		// ¿Llego el paquete completo (incluyendo payload)?
		if (size <= available)
		{
			QUEUE_INFO QueueInfo;
			if (index < 0 || index > MAX_SERVER)
			{
				Log.ToDisp(LOG_RED, DATARECV_INVALID_SERVER, index);
				return false;
			}
			QueueInfo.index = static_cast<WORD>(index);
			QueueInfo.head = head;
			memcpy(QueueInfo.buff, &lpMsg[count], size);
			QueueInfo.size = static_cast<WORD>(size);

			// Encola el paquete para que ServerQueueThread lo procese.
			if (this->m_ServerQueue.AddToQueue(&QueueInfo) != 0)
			{
				ReleaseSemaphore(this->m_ServerQueueSemaphore, 1, nullptr);
			}
			else
			{
				// La cola esta llena: no podemos seguir aceptando paquetes
				// de este cliente, se desconecta.
				Log.ToDisp(LOG_RED, DATARECV_SERVER_QUEUE_FULL, index, head);
				return false;
			}

			count += size; // Avanza el cursor de lectura dentro del buffer.

			// Si ya no queda nada mas por procesar, terminamos.
			if (count >= lpIoBuffer->size)
			{
				count = 0;
				lpIoBuffer->size = 0;
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
		int remaining = lpIoBuffer->size - count;

		if (remaining > 0)
		{
			memmove(lpMsg, &lpMsg[count], remaining);
		}

		lpIoBuffer->size = remaining;
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
bool CSocketManager::DataSend(int index, BYTE* lpMsg, int size)
{
	this->m_critical.lock();

	if (SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return false;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	if(lpServerManager->IsOnline() == false)
	{
		this->m_critical.unlock();
		return false;
	}

	if(size > MAX_MAIN_PACKET_SIZE)
	{
		Log.ToDisp(LOG_RED, "[SocketManager] Tamaño maximo de mensaje excedido (Tipo: 1, indice: %d, Tamaño: %d)", index, size);
		this->m_critical.unlock();
		return false;
	}

	IO_SEND_CONTEXT* lpIoContext = lpServerManager->m_IoSendContext;

	if (lpIoContext->IoSize > 0)
	{
		// Ya hay un envio en curso: acumulamos en el buffer secundario.
		if ((lpIoContext->IoSideBuffer.size + size) > MAX_SIDE_PACKET_SIZE)
		{
			Log.ToDisp(LOG_RED, "[SocketManager] Tamaño maximo de mensaje excedido (Tipo: 2, Índice: %d, Tamaño: %d)", index, (lpIoContext->IoSideBuffer.size + size));
			this->Disconnect(index);
			this->m_critical.unlock();
			return false;
		}

		memcpy(&lpIoContext->IoSideBuffer.buff[lpIoContext->IoSideBuffer.size], lpMsg, size);
		lpIoContext->IoSideBuffer.size += size;
		this->m_critical.unlock();
		return true;
	}

	// No hay envio pendiente: enviamos de inmediato desde el buffer principal.
	memcpy(lpIoContext->IoMainBuffer.buff, lpMsg, size);

	lpIoContext->wsabuf.buf = (char*)lpIoContext->IoMainBuffer.buff;
	lpIoContext->wsabuf.len = size;
	lpIoContext->IoType = IO_SEND;
	lpIoContext->IoSize = size;
	lpIoContext->IoMainBuffer.size = 0;

	DWORD SendSize = 0, Flags = 0;
	
	if (WSASend(lpServerManager->m_socket,&lpIoContext->wsabuf,1,&SendSize,Flags,&lpIoContext->overlapped,0) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED, "[SocketManager] WSASend() fallo con error: %d", WSAGetLastError());
			this->Disconnect(index);
			this->m_critical.unlock();
			return false;
		}
	}

	this->m_critical.unlock();
	return true;
}

// =====================================================================
// Desconexion
// =====================================================================

// Cierra el socket del servidor y libera su slot en gServerManager.
// Usa un lock automatico (RAII) sobre la seccion critica para garantizar
// que se libere incluso si hay returns tempranos.
void CSocketManager::Disconnect(int index)
{
	CCriticalSection::CLock lock(this->m_critical);

	if(SERVER_RANGE(index) == 0)
	{
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	if(lpServerManager->IsOnline() == false)
	{
		return;
	}

	if(closesocket(lpServerManager->m_socket) == SOCKET_ERROR && WSAGetLastError() != WSAENOTSOCK)
	{
		Log.ToDisp(LOG_RED, CLOSESOCKET_ERROR_MSG, WSAGetLastError());
		return;
	}

	lpServerManager->DelServer();
}

void CSocketManager::OnRecv(int index,DWORD IoSize,IO_RECV_CONTEXT* lpIoContext) // OK
{
	this->m_critical.lock();

	if(SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	if(IoSize == 0)
	{
		this->Disconnect(index);
		this->m_critical.unlock();
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	lpIoContext->IoMainBuffer.size += IoSize;

	if(this->DataRecv(index,&lpIoContext->IoMainBuffer) == 0)
	{
		this->Disconnect(index);
		this->m_critical.unlock();
		return;
	}

	lpIoContext->wsabuf.buf = (char*)&lpIoContext->IoMainBuffer.buff[lpIoContext->IoMainBuffer.size];

	lpIoContext->wsabuf.len = MAX_MAIN_PACKET_SIZE-lpIoContext->IoMainBuffer.size;

	lpIoContext->IoType = IO_RECV;

	DWORD RecvSize=0,Flags=0;

	if(WSARecv(lpServerManager->m_socket,&lpIoContext->wsabuf,1,&RecvSize,&Flags,&lpIoContext->overlapped,0) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED,"[SocketManager] WSARecv() failed with error: %d",WSAGetLastError());
			this->Disconnect(index);
			this->m_critical.unlock();
			return;
		}
	}

	this->m_critical.unlock();
}

void CSocketManager::OnSend(int index,DWORD IoSize,IO_SEND_CONTEXT* lpIoContext) // OK
{
	this->m_critical.lock();

	if(SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	if(IoSize == 0)
	{
		this->Disconnect(index);
		this->m_critical.unlock();
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	lpIoContext->IoMainBuffer.size += IoSize;

	if(lpIoContext->IoMainBuffer.size >= lpIoContext->IoSize)
	{
		if(lpIoContext->IoSideBuffer.size <= 0)
		{
			lpIoContext->IoSize = 0;
			this->m_critical.unlock();
			return;
		}

		if(lpIoContext->IoSideBuffer.size > MAX_MAIN_PACKET_SIZE)
		{
			memcpy(lpIoContext->IoMainBuffer.buff,lpIoContext->IoSideBuffer.buff,MAX_MAIN_PACKET_SIZE);

			lpIoContext->wsabuf.buf = (char*)lpIoContext->IoMainBuffer.buff;

			lpIoContext->wsabuf.len = MAX_MAIN_PACKET_SIZE;

			lpIoContext->IoType = IO_SEND;

			lpIoContext->IoSize = MAX_MAIN_PACKET_SIZE;

			lpIoContext->IoMainBuffer.size = 0;

			memmove(lpIoContext->IoSideBuffer.buff,&lpIoContext->IoSideBuffer.buff[MAX_MAIN_PACKET_SIZE],(lpIoContext->IoSideBuffer.size-MAX_MAIN_PACKET_SIZE));

			lpIoContext->IoSideBuffer.size = lpIoContext->IoSideBuffer.size-MAX_MAIN_PACKET_SIZE;
		}
		else
		{
			memcpy(lpIoContext->IoMainBuffer.buff,lpIoContext->IoSideBuffer.buff,lpIoContext->IoSideBuffer.size);

			lpIoContext->wsabuf.buf = (char*)lpIoContext->IoMainBuffer.buff;

			lpIoContext->wsabuf.len = lpIoContext->IoSideBuffer.size;

			lpIoContext->IoType = IO_SEND;

			lpIoContext->IoSize = lpIoContext->IoSideBuffer.size;

			lpIoContext->IoMainBuffer.size = 0;

			lpIoContext->IoSideBuffer.size = 0;
		}
	}
	else
	{
		lpIoContext->wsabuf.buf = (char*)&lpIoContext->IoMainBuffer.buff[lpIoContext->IoMainBuffer.size];

		lpIoContext->wsabuf.len = lpIoContext->IoSize-lpIoContext->IoMainBuffer.size;

		lpIoContext->IoType = IO_SEND;
	}

	DWORD SendSize=0,Flags=0;

	if(WSASend(lpServerManager->m_socket,&lpIoContext->wsabuf,1,&SendSize,Flags,&lpIoContext->overlapped,0) == SOCKET_ERROR)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
		{
			Log.ToDisp(LOG_RED,"[SocketManager] WSASend() failed with error: %d",WSAGetLastError());
			this->Disconnect(index);
			this->m_critical.unlock();
			return;
		}
	}

	this->m_critical.unlock();
}

int CALLBACK CSocketManager::ServerAcceptCondition(IN LPWSABUF lpCallerId, IN LPWSABUF lpCallerData, IN OUT LPQOS lpSQOS, IN OUT LPQOS lpGQOS, IN LPWSABUF lpCalleeId, OUT LPWSABUF lpCalleeData, OUT GROUP FAR* g, DWORD_PTR dwCallbackData)
{
	UNREFERENCED_PARAMETER(lpCallerData);
	UNREFERENCED_PARAMETER(lpSQOS);
	UNREFERENCED_PARAMETER(lpGQOS);
	UNREFERENCED_PARAMETER(dwCallbackData);

	SOCKADDR_IN* SocketAddr = (SOCKADDR_IN*)lpCallerId->buf;

	if (gAllowableIpList.CheckAllowableIp(inet_ntoa(SocketAddr->sin_addr)) == 0)
	{
		return CF_REJECT;
	}

	return CF_ACCEPT;
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
//        - Se busca un slot libre en gClientManager.
//        - Se asocia el socket al IOCP con CreateIoCompletionPort,
//          usando "index" como clave de finalizacion (completion key).
//        - Se registra el cliente (AddClient) con su IP.
//        - Se dispara el primer WSARecv para empezar a recibir datos.
DWORD WINAPI CSocketManager::ServerAcceptThread(CSocketManager* lpSocketManager)
{
	SOCKADDR_IN SocketAddr;
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
				Log.ToDisp(LOG_RED, SERVERACCEPTTHREAD_ERROR_WSAACCEPT, Error);
			}
			continue;
		}

		char ipAddress[INET_ADDRSTRLEN];

		if (InetNtopA(AF_INET, &SocketAddr.sin_addr, ipAddress, INET_ADDRSTRLEN) == nullptr)
		{
			Log.ToDisp(LOG_RED, SERVERACCEPTTHREAD_ERROR_INETNTOPA, GetLastError());
			closesocket(socket);
			continue;
		}

		// ANTES tenía:
		// { CCriticalSection::CLock lock(lpSocketManager->m_critical); ... }
		// AHORA: sin lock extra. GetFreeClientIndex y AddClient ya se
		// sincronizan internamente vía gClientArrayLock / m_lock.

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
			Log.ToDisp(LOG_RED, SERVERACCEPTTHREAD_ERROR_CREATEIOCP, GetLastError());
			closesocket(socket);
			continue;
		}

		CServerManager* lpServerManager = &gServerManager[index];

		// AddClient toma internamente m_lock (del servidor) y
		// gServerArrayLock (para gServerSearchStart); deja los IO contexts
		// inicializados y listos para WSARecv.
		lpServerManager->AddServer(index, ipAddress, socket);

		DWORD RecvSize=0,Flags=0;

		// Dispara la primera recepcion asincrona para este cliente.
		if (WSARecv(socket, &lpServerManager->m_IoRecvContext->wsabuf, 1, &RecvSize, &Flags, &lpServerManager->m_IoRecvContext->overlapped, 0) == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSA_IO_PENDING)
			{
				Log.ToDisp(LOG_RED, SERVERACCEPTTHREAD_ERROR_WSARECV, WSAGetLastError());
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
	DWORD index;
	LPOVERLAPPED lpOverlapped;

	while(true)
	{
		if (GetQueuedCompletionStatus(lpSocketManager->m_CompletionPort, &IoSize, &index, &lpOverlapped, INFINITE) == 0)
		{
			DWORD Error = GetLastError();

			if (lpOverlapped == nullptr ||
				(Error != ERROR_NETNAME_DELETED &&
					Error != ERROR_CONNECTION_ABORTED &&
					Error != ERROR_OPERATION_ABORTED &&
					Error != ERROR_SEM_TIMEOUT))
			{
				Log.ToDisp(LOG_RED, SERVERWORKERTHREAD_GETQUEUEDCOMPLETIONSTATUS_ERROR, Error);
				return 1;
			}
			// Error esperado de desconexion: se sigue procesando con
			// IoSize == 0 para que OnRecv/OnSend desconecten al cliente.
		}

		// Señal de apagado publicada por Clean(): IoSize=0, index=0,
		// lpOverlapped=nullptr (GQCS retorna exito para este post).
		if (IoSize == 0 && index == 0)
		{
			if (lpOverlapped == nullptr) return 0;
		}

		IO_CONTEXT* lpIoContext = (IO_CONTEXT*)lpOverlapped;

		switch (lpIoContext->IoType)
		{
		case IO_RECV:
			lpSocketManager->OnRecv(index, IoSize, (IO_RECV_CONTEXT*)lpIoContext);
			break;
		case IO_SEND:
			lpSocketManager->OnSend(index, IoSize, (IO_SEND_CONTEXT*)lpIoContext);
			break;
		}
	}

	return 0;
}

DWORD WINAPI CSocketManager::ServerQueueThread(CSocketManager* lpSocketManager) // OK
{
	while(true)
	{
		if(WaitForSingleObject(lpSocketManager->m_ServerQueueSemaphore,INFINITE) == WAIT_FAILED)
		{
			Log.ToDisp(LOG_RED,"[SocketManager] WaitForSingleObject() failed with error: %d",GetLastError());
			break;
		}

		static QUEUE_INFO QueueInfo;

		if(lpSocketManager->m_ServerQueue.GetFromQueue(&QueueInfo) != 0)
		{
			if(SERVER_RANGE(QueueInfo.index) != 0 && gServerManager[QueueInfo.index].IsOnline() != false)
			{
				JoinServerProtocolCore(QueueInfo.index,QueueInfo.head,QueueInfo.buff,QueueInfo.size);
			}
		}
	}

	return 0;
}

// =====================================================================
// Utilidades
// =====================================================================

// Devuelve la cantidad actual de paquetes pendientes en la cola interna.
DWORD CSocketManager::GetQueueSize()
{
	return this->m_ServerQueue.GetQueueSize();
}
