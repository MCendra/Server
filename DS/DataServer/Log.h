// Log.h
#pragma once
#include "ServerLog.h"      // Define LogType (GENERAL) y gServerLog
#include "ServerDisplayer.h" // Define LogColor y gServerDisplayer

// La diferencia con ConnectServer es que ToFile recibe LogType como
// primer parametro, permitiendo dirigir el mensaje al archivo correcto.
// ToDisp siempre va a GENERAL (los eventos de cuenta son demasiado
// verbosos para mostrarlos en pantalla en tiempo real).

class CLog
{
public:
	CLog() = default;
	~CLog() = default;

	// Escribe a disco en el archivo correspondiente al tipo indicado.
	// No muestra nada en pantalla.
	void ToFile(LogType type, const char* text, ...);

	// Muestra en pantalla Y escribe en el log GENERAL a disco.
	// Para eventos de operacion que el operador necesita ver en tiempo real.
	void ToDisp(LogColor color, const char* text, ...);

private:
	std::string FormatMessage(const char* text, va_list args) const;
};

extern CLog Log;
