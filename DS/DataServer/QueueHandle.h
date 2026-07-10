// QueueHandle.h
#pragma once
#include "CriticalSection.h"
#include <queue>

#define MAX_QUEUE_SIZE 2048
#define MAX_BUFFER_QUEUE_SIZE 2048

struct QUEUE_INFO
{
	WORD Index;
	WORD Size;
	BYTE Head;
	BYTE Buff[MAX_BUFFER_QUEUE_SIZE];
};

class CQueue
{
public:
	CQueue();
	virtual ~CQueue();
	void ClearQueue();
	size_t GetQueueSize();
	bool AddToQueue(const QUEUE_INFO* lpInfo);
	bool GetFromQueue(QUEUE_INFO* lpInfo);
private:
	CCriticalSection m_critical;
	std::queue<QUEUE_INFO> m_QueueInfo;
};
