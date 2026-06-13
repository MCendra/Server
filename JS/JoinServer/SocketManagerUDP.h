// SocketManagerUDP.h: interface for the CSocketManagerUDP class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_UDP_PACKET_SIZE 4096

class CSocketManagerUDP
{
public:
	CSocketManagerUDP();
	virtual ~CSocketManagerUDP();
	bool Start(WORD port);
	bool Connect(char* IpAddress,WORD port);
	void Clean();
	bool DataRecv();
	bool DataSend(BYTE* lpMsg,int size);
	static DWORD WINAPI ServerRecvThread(CSocketManagerUDP* lpSocketManagerUDP);
private:
	SOCKET m_socket;
	SOCKADDR_IN m_SocketAddr;
	HANDLE m_ServerRecvThread;
	BYTE m_RecvBuff[MAX_UDP_PACKET_SIZE];
	int m_RecvSize;
	BYTE m_SendBuff[MAX_UDP_PACKET_SIZE];
	int m_SendSize;
};

extern CSocketManagerUDP gSocketManagerUDP;
