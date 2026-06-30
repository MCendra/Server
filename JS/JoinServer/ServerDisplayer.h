// ServerDisplayer.h
#pragma once
#include <strsafe.h>
#include <Richedit.h>
#include "CriticalSection.h"

// Tamaño maximo en caracteres para cada linea individual de texto en la ventana de logs
#define MAX_LOG_TEXT_SIZE 110

// Enumeracion para la gestion de colores de texto visualizados en pantalla
enum LogColor
{
	LOG_BLACK = 0, // Mensajes generales, informativos o de sistema standar
	LOG_RED = 1, // Errores criticos, caidas de conexion o excepciones
	LOG_GREEN = 2, // Conexiones exitosas, encendido de servicios o estados OK
	LOG_BLUE = 3, // Acciones de usuarios, recargas de configuracion o debug especializado
};

class CServerDisplayer
{
public:
	// Constructor y destructor publicos
	CServerDisplayer();
	virtual ~CServerDisplayer();
	// Inicializa la clase con el HWND de la ventana principal
	void Init(HWND hWnd);
	// Actualiza el titulo de la ventana
	void UpdateWindowTitle(int accountCount, int queueSize) const;
	// Pintar el nombre del cliente en la ventana (const)
	void PaintName(HDC hdc) const;
	// Pintar el estado del servidor en la ventana (const)
	void PaintJoinServerState(HDC hdc) const;
	// Agrega texto al log y lo pinta
	void LogAddText(LogColor color, const std::string& text);
	// Actualiza el estado del servidor y repinta la ventana
	void Refresh();
	// Actualiza el layout de la ventana
	void UpdateLayout();

	void SetActiveState(bool active) { m_isactive = active; }

	// Restriccion explicita mediante C++ moderno: se deshabilitan copias y movimientos de la instancia
	CServerDisplayer(const CServerDisplayer&) = delete;
	CServerDisplayer& operator=(const CServerDisplayer&) = delete;
	CServerDisplayer(CServerDisplayer&&) = delete;
	CServerDisplayer& operator=(CServerDisplayer&&) = delete;

private:
	HWND m_hwnd;                                // Handle de la ventana principal del servidor (Win32)
	HWND m_hrichedit;						    // Control RICHEDIT nativo: scroll, historial y mouse wheel gratis
	HFONT m_font;                               // Fuente tipografica principal utilizada para el texto de la interfaz
	HFONT m_smallfont;                          // Fuente tipografica secundaria (menor tamano) para listas compactas
	HMODULE m_richeditmodule;                   // Handle del modulo DLL cargado en memoria para dar soporte al Rich Edit
	HBRUSH m_brush[5];                          // Brochas GDI para pintar los fondos de la ventana segun el color activo
	RECT m_rect;								// Almacena las coordenadas del rectangulo de visualizacion de la ventana
	bool m_isactive;
	int m_serverlistbottom;                     // Coordenada inferior limite asignada al bloque de la lista de servidores
	int m_lineheight;                           // Altura en pixeles calculada para cada linea de texto renderizada
	char m_displayertext[2][32];                // Buffers estaticos para almacenar strings informativos de corta longitud


	// Protege m_log / m_count contra accesos concurrentes desde
	// distintos hilos del servidor (ServerWorkerThread, AcceptThread, etc.)
	mutable CCriticalSection m_logLock;
};

extern CServerDisplayer gServerDisplayer;
