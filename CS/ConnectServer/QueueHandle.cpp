// QueueHandle.cpp
#include "QueueHandle.h"

// Construction/Destruction

CQueue::CQueue()
{
    // No es necesario inicializar aqui m_critical; se inicializa automaticamente en su constructor
}

CQueue::~CQueue()
{
    this->ClearQueue();
}

void CQueue::ClearQueue()
{
    CCriticalSection::CLock lock(this->m_critical);

    this->m_QueueInfo = std::queue<QUEUE_INFO>();  // Asignacion directa de una cola vacia
}

size_t CQueue::GetQueueSize()
{
    CCriticalSection::CLock lock(this->m_critical);

    return this->m_QueueInfo.size();
}

bool CQueue::AddToQueue(QUEUE_INFO* lpInfo)
{

	if (lpInfo == nullptr)
	{
		return false;
	}

	if (lpInfo->size > MAX_BUFFER_QUEUE_SIZE)
	{
		return false;
	}

	CCriticalSection::CLock lock(this->m_critical);

    if (this->m_QueueInfo.size() < MAX_BUFFER_QUEUE_SIZE)
    {
        this->m_QueueInfo.push(*lpInfo);
        return true;
    }

    return false;
}

bool CQueue::GetFromQueue(QUEUE_INFO* lpInfo)
{

	if (lpInfo == nullptr)
	{
		return false;
	}

    CCriticalSection::CLock lock(this->m_critical);

    if (!this->m_QueueInfo.empty())
    {
        *lpInfo = this->m_QueueInfo.front();
        this->m_QueueInfo.pop();
        return true;
    }

    return false;
}
