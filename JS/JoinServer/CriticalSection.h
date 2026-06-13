// CriticalSection.h
#pragma once
#include <windows.h>

// Clase para manejar la sincronización entre hilos usando CRITICAL_SECTION de Windows
class CCriticalSection
{
public:
	// Constructor: Inicializa la sección crítica
	CCriticalSection();

	// Destructor: Libera los recursos asociados a la sección crítica
	virtual ~CCriticalSection();

	// Bloquea la sección crítica para acceso exclusivo
	void lock();

	// Desbloquea la sección crítica para permitir acceso a otros hilos
	void unlock();

	// Clase interna para gestionar el bloqueo automático de la sección crítica
	class CLock
	{
	public:
		// Constructor: Bloquea la sección crítica
		CLock(CCriticalSection& cs) : m_cs(cs) { m_cs.lock(); }

		// Destructor: Desbloquea la sección crítica
		~CLock() { m_cs.unlock(); }

	private:
		CCriticalSection& m_cs;  // Referencia a la sección crítica
	};

private:
	// Variable miembro que representa la sección crítica
	CRITICAL_SECTION m_critical;
};
