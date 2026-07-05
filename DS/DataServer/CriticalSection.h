// CriticalSection.h
#pragma once
#include <windows.h>

// Clase para manejar la sincronizacion entre hilos usando CRITICAL_SECTION de Windows
class CCriticalSection
{
public:
	// Constructor: Inicializa la seccion critica
	CCriticalSection();

	// Destructor: Libera los recursos asociados a la seccion critica
	virtual ~CCriticalSection();

	// Bloquea la seccion critica para acceso exclusivo
	void lock();

	// Desbloquea la seccion critica para permitir acceso a otros hilos
	void unlock();

	// Clase interna para gestionar el bloqueo automatico de la seccion critica
	class CLock
	{
	public:
		// Constructor: Bloquea la seccion critica
		explicit CLock(CCriticalSection& cs)
			: m_cs(cs)
		{
			m_cs.lock();
		}
		
		// Destructor: Desbloquea la seccion critica
		~CLock()
		{
			m_cs.unlock();
		}
		CLock(const CLock&) = delete;
		CLock& operator=(const CLock&) = delete;

	private:
		CCriticalSection& m_cs;  // Referencia a la seccion critica
	};

private:
	// Variable miembro que representa la seccion critica
	CRITICAL_SECTION m_critical;
};
