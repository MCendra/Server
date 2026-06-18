// Log.h
#pragma once
#include "ServerLog.h"
#include "ServerDisplayer.h"

class CLog
{
public:
	CLog();
	virtual ~CLog();

	// Registra un mensaje en el archivo de log (GENERAL o ACCOUNT segun "type").
	void ToFile(LogType type, const char* text, ...);

	// Registra y muestra un mensaje en la ventana del programa.
	// Siempre se escribe tambien al log GENERAL en disco.
	void ToDisp(LogColor color, const char* text, ...);

private:
	// Funcion auxiliar para formatear el mensaje.
	std::string FormatMessage(const char* text, va_list args) const;
};

// Declaracion global de la instancia de Log
extern CLog Log;
