// CriticalSection.cpp
#include "CriticalSection.h"

// Constructor: Inicializa la sección crítica
CCriticalSection::CCriticalSection()
{
	InitializeCriticalSection(&m_critical);
}

// Destructor: Libera los recursos asociados a la sección crítica
CCriticalSection::~CCriticalSection()
{
	DeleteCriticalSection(&m_critical);
}

// Bloquea la sección crítica para acceso exclusivo
void CCriticalSection::lock()
{
	EnterCriticalSection(&m_critical);
}

// Desbloquea la sección crítica para permitir acceso a otros hilos
void CCriticalSection::unlock()
{
	LeaveCriticalSection(&m_critical);
}


// Use:
// void SomeFunction()
// {
//     CCriticalSection cs;
//     {
//         CCriticalSection::CLock lock(cs); // Bloquea automáticamente

		 // Código crítico que necesita sincronización

//     } // `lock` se destruye aquí y desbloquea automáticamente
// }
