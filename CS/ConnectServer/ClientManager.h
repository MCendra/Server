// ClientManager.h
#pragma once
#include "SocketManager.h"
#include "CriticalSection.h"

// Numero maximo de clientes permitidos
#define MAX_CLIENT 10000
// Macro para verificar si el indice del cliente esta dentro del rango valido
#define CLIENT_RANGE(x) (((x)<0)?0:((x)>=MAX_CLIENT)?0:1)
// Tiempo minimo (ms) que un slot debe llevar offline para ser
// considerado "reutilizable" por SearchFreeClientIndex.
#define MIN_CLIENT_OFFLINE_TIME_FOR_REUSE 10000

// Enum para representar el estado del cliente
enum eClientState
{
	CLIENT_OFFLINE = 0,
	CLIENT_ONLINE = 1
};

// Clase para gestionar la informacion y el estado de los clientes
class CClientManager
{
public:
	// Constructor: Inicializa un nuevo objeto CClientManager
	CClientManager();
	// Destructor: Limpia los recursos del objeto CClientManager
	virtual ~CClientManager();
	// Verifica si el cliente esta en un estado valido (en linea y con socket valido)
	bool IsOnline();
	// Verifica si los contextos de IO estan correctamente asignados
	bool CheckAlloc();
	// Verifica si el tiempo en linea del cliente esta dentro del limite permitido
	bool CheckOnlineTime() const;
	// Agrega un nuevo cliente con un indice, IP y socket especificos
	void AddClient(int index, char* ip, SOCKET socket);
	// Elimina un cliente y libera los recursos asociados
	void DelClient();
	// Busca un indice de cliente libre para asignar un nuevo cliente
	int GetFreeClientIndex();
	// Busca un indice de cliente libre dentro de un rango especifico, considerando el tiempo offline para reutilizacion
	int SearchFreeClientIndex(int MinIndex, int MaxIndex, ULONGLONG MinTime);
	// Actualiza el estado del cliente y registra el tiempo del cambio de estado
	static void CheckClientTimeouts();

public:
	CCriticalSection m_lock;			// CORRECCIÓN: Se agregó un mutex para proteger el acceso a los datos del cliente
	int m_index;                        // Índice del cliente
	eClientState m_state;               // Estado del cliente (en linea o desconectado)
	char m_IpAddr[16];                  // Direccion IP del cliente
	SOCKET m_socket;                    // Socket del cliente
	IO_RECV_CONTEXT* m_IoRecvContext;   // Contexto de recepcion de IO
	IO_SEND_CONTEXT* m_IoSendContext;   // Contexto de envio de IO
	ULONGLONG m_LastStateChangeTime;	// Tiempo del ultimo cambio de estado
	ULONGLONG m_LastPacketTime;         // Tiempo del ultimo paquete recibido/enviado
};

// Objeto global para gestionar multiples clientes
extern CClientManager gClientManager[MAX_CLIENT];
// Cursor circular para iniciar la busqueda de slots libres. NO es un contador
extern int gClientSearchStart;
