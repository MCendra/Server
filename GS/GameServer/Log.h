// Log.h
#pragma once
#include "ServerLog.h"      // Define LogType (GENERAL, ACCOUNT) y gServerLog
#include "ServerDisplayer.h" // Define LogColor y gServerDisplayer

// CLog es la fachada unificada de logging para JoinServer.
//
// Cualquier archivo que incluya Log.h puede:
//   Log.ToFile(LogType::GENERAL,  "mensaje %d", valor);   → solo a disco, carpeta LOG
//   Log.ToFile(LogType::ACCOUNT,  "login %s",   usuario); → solo a disco, carpeta LOG_ACCOUNT
//   Log.ToDisp(LOG_GREEN, "mensaje %d", valor);            → pantalla + disco (GENERAL)
//
// La diferencia con ConnectServer es que ToFile recibe LogType como
// primer parametro, permitiendo dirigir el mensaje al archivo correcto.
// ToDisp siempre va a GENERAL (los eventos de cuenta son demasiado
// verbosos para mostrarlos en pantalla en tiempo real).

//struct LOG_INFO
//{
//	BOOL Active;
//	char Directory[256];
//	int Day;
//	int Month;
//	int Year;
//	char Filename[256];
//	HANDLE File;
//};

class CLog
{
public:
	CLog() = default;
	~CLog() = default;

	// Escribe a disco en el archivo correspondiente al tipo indicado.
	// No muestra nada en pantalla.
	void ToFile(LogType type, const char* text, ...);

	// Para eventos de operacion que el operador necesita ver en tiempo real.

	// Muestra en pantalla Y escribe en el log GENERAL a disco.
	void ToDisp(LogColor color, const char* text, ...);
	// Muestra en pantalla Y escribe en el log determinado por LogType a disco.
	void ToDispAndFile(LogColor color, LogType type, const char* text, ...);

private:
	std::string FormatMessage(const char* text, va_list args) const;
};

extern CLog Log;
