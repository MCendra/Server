// Log.h
#pragma once
#include "ServerLog.h"
#include "ServerDisplayer.h"

class CLog
{
	public:
		CLog() = default;
		~CLog() = default;

		// Registra un mensaje en el archivo de log.
		void ToFile(const char* text, ...);

		// Registra y muestra un mensaje en la ventana del programa.
		void ToDisp(LogColor color, const char* text, ...);

	private:
		// Funcion auxiliar para formatear el mensaje.
		std::string FormatMessage(const char* text, va_list args) const;
};

// Declaracion global de la instancia de ServerLog
extern CLog Log;
