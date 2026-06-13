// CriticalSection.cpp
#include "CriticalSection.h"

// Constructor: Inicializa la seccion critica
CCriticalSection::CCriticalSection()
{
    InitializeCriticalSection(&m_critical);
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


// Use:
// void SomeFunction()
// {
//     CCriticalSection cs;
//     {
//         CCriticalSection::CLock lock(cs); // Bloquea automaticamente
 
         // Codigo critico que necesita sincronizacion

//     } // `lock` se destruye aqui y desbloquea automaticamente
// }