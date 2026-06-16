// ServerList.h
#pragma once
#include "ServerProtocol.h"
#include <map>

// Definicion del tamaño maximo de la cola de servidores para unirse
#define MAX_JOIN_SERVER_QUEUE_SIZE 100

// Estructura para recibir informacion sobre el estado de un servidor de juego
struct SDHP_GAME_SERVER_LIVE_RECV
{
	PBMSG_HEAD header; // Encabezado del mensaje (C1:01)
	WORD ServerCode; // Codigo del servidor
	BYTE UserTotal; // Total de usuarios en el servidor
	WORD UserCount; // Conteo actual de usuarios
	WORD AccountCount; // Conteo de cuentas
	WORD PCPointCount; // Conteo de puntos de PC
	WORD MaxUserCount; // Maximo numero de usuarios
};

// Estructura para recibir informacion sobre el estado de la cola de servidores para unirse
struct SDHP_JOIN_SERVER_LIVE_RECV
{
	PBMSG_HEAD header; // Encabezado del mensaje (C1:02)
	DWORD QueueSize; // Tamaño de la cola de servidores para unirse
};

// Estructura que contiene la informacion de un servidor en la lista
struct SERVER_LIST_INFO
{
	WORD ServerCode; // Codigo del servidor
	char ServerName[32]; // Nombre del servidor
	char ServerAddress[16]; // Direccion IP del servidor
	WORD ServerPort; // Puerto del servidor
	bool ServerShow; // Indica si el servidor debe mostrarse en la lista
	bool ServerState; // Estado del servidor (en linea o fuera de linea)
	ULONGLONG ServerStateTime; // Tiempo del ultimo cambio de estado del servidor
	BYTE UserTotal; // Total de usuarios en el servidor
	WORD UserCount; // Conteo actual de usuarios
	WORD AccountCount; // Conteo de cuentas
	WORD PCPointCount; // Conteo de puntos de PC
	WORD MaxUserCount; // Maximo numero de usuarios
};

class CServerList
{
public:
	// Constructor de la clase CServerList
	CServerList();

	// Destructor de la clase CServerList
	virtual ~CServerList();

	// Carga la informacion de los servidores desde un archivo
	void Init(const char* path);

	// Maneja el mensaje de estado de un servidor de juego
	void CheckServerTimeouts();

	// Verifica el estado de la cola de servidores para unirse
	bool IsJoinServerOnline() const;

	// Genera la lista de servidores y la almacena en lpMsg
	long GenerateServerList(BYTE* lpMsg, int* size);

	// Obtiene la informacion del servidor basado en el codigo del servidor
	SERVER_LIST_INFO* GetGameServerInfo(int serverCode);

	// Maneja la logica del protocolo del servidor basado en el encabezado del mensaje
	void ProcessServerStatusPacket(BYTE head, BYTE* lpMsg, int size);

	// Maneja el mensaje de estado del servidor de juego
	void ProcessGameServerHeartbeat(SDHP_GAME_SERVER_LIVE_RECV* lpMsg);

	// Maneja el mensaje de estado de la cola de servidores para unirse
	void ProcessJoinServerHeartbeat(SDHP_JOIN_SERVER_LIVE_RECV* lpMsg);

	// Obtiene el conteo de servidores en linea
	int GetOnlineGameServerCount() const;

	// Obtiene el conteo total de usuarios en todos los servidores
	const std::map<int, SERVER_LIST_INFO>& GetGameServerList() const;

private:
	// Estado del servidor de union (0 = fuera de linea, 1 = en linea)
	bool m_JoinServerState;

	// Tiempo del ultimo cambio de estado del servidor de union
	ULONGLONG m_JoinServerStateTime;

	// Tamaño de la cola de servidores para unirse
	DWORD m_JoinServerQueueSize;

	// Mapa que almacena la informacion de la lista de servidores
	std::map<int, SERVER_LIST_INFO> m_ServerListInfo;
		
};

// Instancia global de CServerList
extern CServerList gServerList;
