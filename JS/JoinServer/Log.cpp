// Log.cpp
#include "Header.h"
#include "Log.h"

// Definicion de la instancia global de Log
CLog Log;

// CLog ya no mantiene estado propio: toda la persistencia a disco
// vive en gServerLog (ServerLog.h/.cpp), y la visualizacion en
// gServerDisplayer. El constructor/destructor quedan vacios.
CLog::CLog()
{}

CLog::~CLog()
{}

std::string CLog::FormatMessage(const char* text, va_list args) const
{
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), text, args);
	return std::string(buffer);
}

// Escribe directamente a disco, sin pasar por la ventana. Se usa para
// eventos que no necesitan mostrarse en pantalla (ej. el log de
// cuentas, que puede ser muy verboso para mostrarlo en tiempo real).
void CLog::ToFile(LogType type, const char* text, ...)
{
	va_list args;
	va_start(args, text);
	std::string formattedMessage = FormatMessage(text, args);
	va_end(args);

	gServerLog.Output(type, formattedMessage);
}

// Muestra el mensaje en la ventana del servidor Y lo persiste en el
// log GENERAL en disco. Se usa para eventos relevantes de operacion
// (inicio, errores, conexiones, etc) que el operador del servidor
// quiere ver en tiempo real.
void CLog::ToDisp(LogColor color, const char* text, ...)
{
	va_list args;
	va_start(args, text);
	std::string formattedMessage = FormatMessage(text, args);
	va_end(args);

	gServerDisplayer.LogAddText(color, formattedMessage);

	gServerLog.Output(LogType::GENERAL, formattedMessage);
}
