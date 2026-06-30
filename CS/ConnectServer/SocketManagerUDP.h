// SocketManagerUDP.h
#pragma once
#include "ConnectServerProtocol.h"
#include "CriticalSection.h"

class CSocketManagerUdp
{
public:
	CSocketManagerUdp();
	virtual ~CSocketManagerUdp();
	bool Init(WORD port);
	bool Connect(char* IpAddress, WORD port);
	void Clean();
	//  Antes: bool DataRecv();
	bool DataRecv(int recvSize);
	bool DataSend(BYTE* lpMsg, int size);
	static DWORD WINAPI ServerRecvThread(LPVOID lpParam);
private:
	SOCKET m_socket;
	SOCKADDR_IN m_SocketAddr;
	HANDLE m_ServerRecvThread;
	BYTE m_RecvBuff[MAX_UDP_PACKET_SIZE];

	// Mutex para proteger m_socket, m_SendBuff/m_SendSize y m_RecvSize
	// frente a accesos concurrentes desde otros hilos (DataSend, Clean).
	CCriticalSection m_lock;
};

extern CSocketManagerUdp gSocketManagerUdp;
