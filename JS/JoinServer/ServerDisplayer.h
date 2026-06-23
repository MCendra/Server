// ServerDisplayer.h
#pragma once
#include <strsafe.h>    // Funciones seguras de manipulación de cadenas
#include "CriticalSection.h"

#define MAX_LOG_TEXT_LINE 28
#define MAX_LOG_TEXT_SIZE 100

enum LogColor
{
	LOG_BLACK = 0,
	LOG_RED = 1,
	LOG_GREEN = 2,
	LOG_BLUE = 3,
};

struct LogDisplayInfo
{
	std::string text;
	LogColor color = LOG_BLACK;  // Inicialización por defecto

};

class CServerDisplayer
{
public:
	// Constructor y destructor públicos
	CServerDisplayer();
	virtual ~CServerDisplayer();

	// Inicializa la clase con el HWND de la ventana principal
	void Init(HWND hWnd);

	// Actualiza el título de la ventana
	void UpdateWindowTitle(int accountCount,int queueSize) const;

	// Pintar el nombre del cliente en la ventana (const)
	void PaintName(HDC hdc) const;

	// Pintar el estado del servidor en la ventana (const)
	void PaintJoinServerState(HDC hdc) const;

	// Funcion para pintar los logs de texto
	void PaintLogText(HDC hdc);

	// Agrega texto al log y lo pinta
	void LogAddText(LogColor color, const std::string& text);

	// Actualiza el estado del servidor y repinta la ventana
	void Refresh();

	void SetActiveState(bool active) { m_isActive = active; }

	// Elimina la posibilidad de copiar o mover la instancia
	CServerDisplayer(const CServerDisplayer&) = delete;
	CServerDisplayer& operator=(const CServerDisplayer&) = delete;
	CServerDisplayer(CServerDisplayer&&) = delete;
	CServerDisplayer& operator=(CServerDisplayer&&) = delete;

private:
	HWND m_hwnd;
	HFONT m_font;
	HBRUSH m_brush[5];
	LogDisplayInfo m_log[MAX_LOG_TEXT_LINE];
	bool m_isActive;
	int m_count;
	//int m_servercode;                           // Almacena el estado del servidor
	char m_displayertext[2][32];
	RECT m_rect;								// Almacena las coordenadas del rectángulo de visualización
	RECT m_logRect;								// FIX: area del log (subrect de m_rect, top = 100)

	// Protege m_log / m_count contra accesos concurrentes desde
	// distintos hilos del servidor (ServerWorkerThread, AcceptThread, etc.)
	mutable CCriticalSection m_logLock;
};

extern CServerDisplayer gServerDisplayer;
