// ServerDisplayer.h
#pragma once
#include "CriticalSection.h"
#include <strsafe.h>
#include <Richedit.h>

// Tamaño maximo en caracteres para cada linea individual de texto en la ventana de logs
#define MAX_LOG_TEXT_SIZE 110
#define MAX_LOG_TEXT_LINE 41

#define MAX_LOGCONNECT_TEXT_LINE 13
#define MAX_LOGCONNECT_TEXT_SIZE 55

#define MAX_LOGGLOBAL_TEXT_LINE 8
#define MAX_LOGGLOBAL_TEXT_SIZE 100

enum LogColor
{
	LOG_BLACK = 0, // Mensajes generales, informativos o de sistema standar
	LOG_RED = 1, // Errores criticos, caidas de conexion o excepciones
	LOG_GREEN = 2, // Conexiones exitosas, encendido de servicios o estados OK
	LOG_BLUE = 3, // Acciones de usuarios, recargas de configuracion o debug especializado
	//MC bot
	LOG_BOT = 4,
	LOG_USER = 5,
	LOG_EVENT = 6,
	LOG_ALERT = 7,
	//MC bot
};

struct LOG_DISPLAY_INFO
{
	char Text[MAX_LOG_TEXT_SIZE];
	LogColor Color;
};

struct LOGCONNECT_DISPLAY_INFO
{
	char Text[MAX_LOGCONNECT_TEXT_SIZE];
	LogColor Color;
};

struct LOGGLOBAL_DISPLAY_INFO
{
	char Text[MAX_LOGGLOBAL_TEXT_SIZE];
	LogColor Color;
};

class CServerDisplayer
{
public:
	// Constructor y destructor publicos
	CServerDisplayer();
	virtual ~CServerDisplayer();
	// Inicializa la clase con el HWND de la ventana principal
	void Init(HWND hWnd);
	void Run();
	void background();
	void SetWindowName();
	void PaintAllInfo();
	void LogTextPaint();
	void LogTextPaintConnect();
	void LogTextPaintGlobalMessage();
	void PaintName();
	void PaintEventTime();
	void PaintInvasionTime();
	void PaintCustomArenaTime();
	void LogAddText(eLogColor color, char* text, int size);
	void LogAddTextConnect(eLogColor color, char* text, int size);
	void LogAddTextGlobal(eLogColor color, char* text, int size);
	void PaintOnline();
	void PaintPremium();
	void PaintSeason();
	int EventBc;
	int EventDs;
	int EventCc;
	int EventIt;
	int EventCustomLottery;
	int EventCustomBonus;
	//int EventCustomArena;
	int EventCustomQuiz;
	int EventMoss;
	int EventKing;
	int EventDrop;
	int EventTvT;
	int EventInvasion[30];
	int EventCustomArena[30];
	int EventCs;
	int EventCsState;
	int EventCastleDeep;
	int EventCryWolf;
	int EventCryWolfState;

#if(SKY_EVENT == 1)
	int EventSkyEvent;
#endif
#if(THANMA)
	int BEventThanMa;
#endif

//#if(BOSS_GUILD == 1)
	int EventBossGuild;
//#endif

#if(HONCHIENCLASS == 1)
	int ReadPKEventInfo;
#endif

	int EventCTCMini;

	// Restriccion explicita mediante C++ moderno: se deshabilitan copias y movimientos de la instancia
	CServerDisplayer(const CServerDisplayer&) = delete;
	CServerDisplayer& operator=(const CServerDisplayer&) = delete;
	CServerDisplayer(CServerDisplayer&&) = delete;
	CServerDisplayer& operator=(CServerDisplayer&&) = delete;

private:
	HWND m_hwnd;                                // Handle de la ventana principal del servidor (Win32)
	HWND m_hrichedit;						    // Control RICHEDIT nativo: scroll, historial y mouse wheel gratis
	HFONT m_font;                               // Fuente tipografica principal utilizada para el texto de la interfaz
	HFONT m_font2;								// Fuente tipografica secundaria para listas compactas
	HFONT m_font3;								// Fuente tipografica secundaria para listas compactas
	HFONT m_font4;								// Fuente tipografica secundaria para listas compactas
	HFONT m_font5;								// Fuente tipografica secundaria para listas compactas
	HMODULE m_richeditmodule;                   // Handle del modulo DLL cargado en memoria para dar soporte al Rich Edit
	HBRUSH m_brush[5];							// Brochas GDI para pintar los fondos de la ventana segun el color activo
	LOG_DISPLAY_INFO m_log[MAX_LOG_TEXT_LINE];
	LOGCONNECT_DISPLAY_INFO m_logConnect[MAX_LOGCONNECT_TEXT_LINE];
	LOGGLOBAL_DISPLAY_INFO m_logGlobal[MAX_LOGGLOBAL_TEXT_LINE];
	int m_count;
	int m_countConnect;
	int m_countGlobal;
	char m_displayertext[2][64];                // Buffers estaticos para almacenar strings informativos de corta longitud

	// Protege m_log / m_count contra accesos concurrentes desde
	// distintos hilos del servidor (ServerWorkerThread, AcceptThread, etc.)
	mutable CCriticalSection m_logLock;
};

extern CServerDisplayer gServerDisplayer;
