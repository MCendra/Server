// ClientManager.h
#pragma once
#include "SocketManager.h"
#include "CriticalSection.h"

// Definiciones
#define MAX_CLIENT 10000           // Numero maximo de clientes permitidos
#define MAX_ONLINE_TIME 300000     // Tiempo maximo en linea en milisegundos (5 minutos)

// Macro para verificar si el indice del cliente esta dentro del rango valido
#define CLIENT_RANGE(x) (((x)<0)?0:((x)>=MAX_CLIENT)?0:1)

// Enum para representar el estado del cliente
enum eClientState
{
	CLIENT_OFFLINE = 0,  // El cliente esta desconectado
	CLIENT_ONLINE = 1,   // El cliente esta conectado
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
	bool CheckState();

	// Verifica si los contextos de IO estan correctamente asignados
	bool CheckAlloc();

	// Verifica si el tiempo en linea del cliente esta dentro del limite permitido
	bool CheckOnlineTime() const;

	// Agrega un nuevo cliente con un indice, IP y socket especificos
	void AddClient(int index, char* ip, SOCKET socket);

	// Elimina un cliente y libera los recursos asociados
	void DelClient();

	int GetFreeClientIndex();
	int SearchFreeClientIndex(int MinIndex, int MaxIndex, ULONGLONG MinTime);

public:
	CCriticalSection m_lock;			// CORRECCIÓN: Se agregó un mutex para proteger el acceso a los datos del cliente

	int m_index;                        // Índice del cliente
	eClientState m_state;               // Estado del cliente (en linea o desconectado)
	char m_IpAddr[16];                  // Direccion IP del cliente
	SOCKET m_socket;                    // Socket del cliente
	IO_RECV_CONTEXT* m_IoRecvContext;   // Contexto de recepcion de IO
	IO_SEND_CONTEXT* m_IoSendContext;   // Contexto de envio de IO
	ULONGLONG m_OnlineTime;             // Tiempo en linea del cliente
	ULONGLONG m_PacketTime;             // Tiempo del ultimo paquete recibido/enviado
};

// Objeto global para gestionar multiples clientes
extern CClientManager gClientManager[MAX_CLIENT];
extern int gClientCount;
