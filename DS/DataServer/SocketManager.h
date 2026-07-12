// SocketManager.h
#pragma once
#include "DataServerProtocol.h"
#include "CriticalSection.h"
#include "QueueHandle.h"

#define MAX_SERVER_WORKER_THREAD 8
#define MAX_IO_OPERATION 2
#define IO_RECV 0
#define IO_SEND 1

struct IO_RECV_BUFFER
{
	BYTE Buffer[MAX_RECV_PACKET_SIZE];
	int Size;
};

struct IO_SEND_BUFFER
{
	BYTE Buffer[MAX_SEND_PACKET_SIZE];
	int Size;
};

struct IO_SIDE_BUFFER
{
	BYTE Buffer[MAX_SEND_SIDE_PACKET_SIZE];
	int Size;
};

struct IO_CONTEXT
{
	WSAOVERLAPPED OverLapped;
	WSABUF WSAbuf;
	int IoType;
	int IoSize;
};

struct IO_RECV_CONTEXT
{
	WSAOVERLAPPED OverLapped;
	WSABUF WSAbuf;
	int IoType;
	int IoSize;
	IO_RECV_BUFFER IoRecvBuffer;
};

struct IO_SEND_CONTEXT
{
	WSAOVERLAPPED OverLapped;
	WSABUF WSAbuf;
	int IoType;
	int IoSize;
	IO_SEND_BUFFER IoSendBuffer;
	IO_SIDE_BUFFER IoSideBuffer;
};

class CSocketManager
{
public:
	CSocketManager();
	virtual ~CSocketManager();
	bool Init(WORD port);
	void Clean();
	bool CreateListenSocket();
	bool CreateCompletionPort();
	bool CreateAcceptThread();
	bool CreateWorkerThread();
	bool CreateServerQueue();
	bool DataRecv(int serverIndex, IO_RECV_BUFFER* lpIoBuffer);
	bool DataSend(int index, BYTE* lpMsg, int size);
	void Disconnect(int index);
	void OnRecv(int serverIndex, DWORD IoSize, IO_RECV_CONTEXT* lpIoContext);
	void OnSend(int serverIndex, DWORD IoSize, IO_SEND_CONTEXT* lpIoContext);
	// FIX: el ultimo parametro pasa de "CSocketManager*" a "DWORD_PTR" para
	// coincidir con el tipo real de dwCallbackData en WSAAccept y evitar
	// truncamiento de puntero en compilaciones x64 (ver SocketManager.cpp).
	static int CALLBACK ServerAcceptCondition(
		IN LPWSABUF lpCallerId,
		IN LPWSABUF lpCallerData,
		IN OUT LPQOS lpSQOS,
		IN OUT LPQOS lpGQOS,
		IN LPWSABUF lpCalleeId,
		OUT LPWSABUF lpCalleeData,
		OUT GROUP FAR* g,
		DWORD_PTR dwCallbackData);
	static DWORD WINAPI ServerAcceptThread(CSocketManager* lpSocketManager);
	static DWORD WINAPI ServerWorkerThread(CSocketManager* lpSocketManager);
	static DWORD WINAPI ServerQueueThread(CSocketManager* lpSocketManager);
	DWORD GetQueueSize();
private:
	SOCKET m_listen;
	HANDLE m_CompletionPort;
	WORD m_port;
	HANDLE m_ServerAcceptThread;
	HANDLE m_ServerWorkerThread[MAX_SERVER_WORKER_THREAD];
	DWORD m_ServerWorkerThreadCount;
	CQueue m_ServerQueue;
	HANDLE m_ServerQueueSemaphore;
	HANDLE m_ServerQueueThread;
	HANDLE m_shutdownEvent;
};

extern CSocketManager gSocketManager;

