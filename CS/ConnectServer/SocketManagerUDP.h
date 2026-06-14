// SocketManagerUDP.h
// SocketManagerUDP.h
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

#pragma once
#include "ServerProtocol.h"
#include "CriticalSection.h"

class CSocketManagerUdp
{
public:
	CSocketManagerUdp();
	virtual ~CSocketManagerUdp();
	bool Init(WORD port);
	bool Connect(char* IpAddress, WORD port);
	void Clean();
	bool DataRecv();
	bool DataSend(BYTE* lpMsg, int size);
	static DWORD WINAPI ServerRecvThread(LPVOID lpParam);
private:
	SOCKET m_socket;
	SOCKADDR_IN m_SocketAddr;
	HANDLE m_ServerRecvThread;
	BYTE m_RecvBuff[MAX_UDP_PACKET_SIZE];
	int m_RecvSize;
	BYTE m_SendBuff[MAX_UDP_PACKET_SIZE];
	int m_SendSize;

	// Evento para parada cooperativa del hilo de recepcion UDP
	HANDLE m_shutdownEvent;

	// Mutex para proteger m_socket, m_SendBuff/m_SendSize y m_RecvSize
	// frente a accesos concurrentes desde otros hilos (DataSend, Clean).
	CCriticalSection m_lock;
};

extern CSocketManagerUdp gSocketManagerUdp;
