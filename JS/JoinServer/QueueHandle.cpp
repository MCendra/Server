// QueueHandle.cpp
#include "QueueHandle.h"

// Construction/Destruction

CQueue::CQueue() = default;

CQueue::~CQueue() = default;

void CQueue::ClearQueue()
{
	CCriticalSection::CLock lock(m_critical);

	std::queue<QUEUE_INFO> empty;
	m_QueueInfo.swap(empty);
}

size_t CQueue::GetQueueSize()
{
	CCriticalSection::CLock lock(m_critical);
	return m_QueueInfo.size();
}

bool CQueue::AddToQueue(const QUEUE_INFO* lpInfo)
{
	if (lpInfo == nullptr || lpInfo->size > MAX_BUFFER_QUEUE_SIZE)
	{
		return false;
	}

	CCriticalSection::CLock lock(m_critical);

	if (m_QueueInfo.size() >= MAX_QUEUE_SIZE)
	{
		return false;
	}

	m_QueueInfo.push(*lpInfo);

	return true;
}

bool CQueue::GetFromQueue(QUEUE_INFO* lpInfo)
{
	if (lpInfo == nullptr)
	{
		return false;
	}

	CCriticalSection::CLock lock(m_critical);

	if (m_QueueInfo.empty())
	{
		return false;
	}

	*lpInfo = m_QueueInfo.front();
	m_QueueInfo.pop();

	return true;
}
