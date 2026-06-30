// ServerManager.h
#pragma once
#include "SocketManager.h"
#include "CriticalSection.h"

// Numero maximo de clientes permitidos
#define MAX_SERVER 20
// Macro para verificar si el indice del cliente esta dentro del rango valido
#define SERVER_RANGE(x) (((x)<0)?0:((x)>=MAX_SERVER)?0:1)
// Tiempo maximo (ms) que un slot debe llevar offline para ser
// considerado "reutilizable" por SearchFreeServerIndex.
#define MAX_SERVER_OFFLINE_TIME_FOR_REUSE 10000

// Enum para representar el estado del servidor
enum eServerState
{
	SERVER_OFFLINE = 0,
	SERVER_ONLINE = 1
};

// Clase para gestionar la informacion y el estado de los servidores
class CServerManager
{
public:
	// Constructor: Inicializa un nuevo objeto CServerManager
	CServerManager();
	// Destructor: Limpia los recursos del objeto CServerManager
	virtual ~CServerManager();
	// Verifica si el servidor esta en un estado valido (en linea y con socket valido)
	bool IsOnline();
	// Verifica si los contextos de IO estan correctamente asignados
	bool CheckAlloc();
	// Agrega un nuevo servidor con un indice, IP y socket especificos
	void AddServer(int index,char* ip,SOCKET socket);
	// Elimina un servidor y libera los recursos asociados
	void DelServer();
	// Busca un indice de servidor libre para asignar un nuevo servidor
	int GetFreeServerIndex();
	// Busca un indice de servidor libre dentro de un rango especifico, considerando el tiempo offline para reutilizacion
	int SearchFreeServerIndex(int MinIndex, int MaxIndex, DWORD MinTime);

	void SetServerInfo(char* name,WORD port,WORD code);
public:
	CCriticalSection m_lock;			// CORRECCIÓN: Se agregó un mutex para proteger el acceso a los datos del servidor
	int m_index;						// Índice del servidor
	eServerState m_state;				// Estado del servidor (en linea o desconectado)
	char m_IpAddr[16];					// Direccion IP del servidor
	SOCKET m_socket;					// Socket del cliente
	IO_RECV_CONTEXT* m_IoRecvContext;	// Contexto de recepcion de IO
	IO_SEND_CONTEXT* m_IoSendContext;	// Contexto de envio de IO
	char m_ServerName[32];				// Nombre del servidor
	WORD m_ServerPort;					// Puerto del servidor
	int m_ServerCode;					// Codigo del servidor
	ULONGLONG m_LastStateChangeTime;	// Tiempo del ultimo cambio de estado
	ULONGLONG m_LastPacketTime;			// Tiempo del ultimo paquete recibido/enviado
	int m_CurUserCount;					// Contador de usuarios actuales conectados al servidor
	int m_MaxUserCount;					// Contador del maximo de usuarios permitidos en el servidor
};

// Objeto global para gestionar multiples servidores
extern CServerManager gServerManager[MAX_SERVER];
// Cursor circular para iniciar la busqueda de slots libres. NO es un contador
extern int gServerSearchStart;   

// Recorre gServerManager[] y retorna el primero cuyo ServerCode coincida.
// Retorna nullptr si ningún servidor activo tiene ese código.
CServerManager* FindServerByCode(int ServerCode);
