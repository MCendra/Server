// CriticalSection.cpp
#include "CriticalSection.h"

// Constructor: Inicializa la seccion critica
CCriticalSection::CCriticalSection()
{
	// Spin count moderado: antes de bloquear el hilo e ir al kernel,
	// intenta 2000 veces en user-mode. Reduce cambios de contexto
	// en locks muy cortos (colas, IO contexts, listas pequeñas),
	// que es exactamente el patrón de uso en este servidor IOCP.
	InitializeCriticalSectionAndSpinCount(&m_critical, 2000);
}

// Destructor: Libera los recursos asociados a la seccion critica
CCriticalSection::~CCriticalSection()
{
	DeleteCriticalSection(&m_critical);
}

// Bloquea la seccion critica para acceso exclusivo
void CCriticalSection::lock()
{
	EnterCriticalSection(&m_critical);
}

// Desbloquea la seccion critica para permitir acceso a otros hilos
void CCriticalSection::unlock()
{
	LeaveCriticalSection(&m_critical);
}
