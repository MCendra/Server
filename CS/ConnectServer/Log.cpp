// Log.cpp
#include "Header.h"
#include "Log.h"

// Definicion de la instancia global de ServerLog
CLog Log;

// Constructor de CServerLog
CLog::CLog() {

}

// Destructor de CServerLog
CLog::~CLog() {
    // Destructor - libera cualquier recurso si es necesario
}

std::string CLog::FormatMessage(const char* text, va_list args) const {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), text, args);
    return std::string(buffer);
}

void CLog::ToFile(const char* text, ...) {
    va_list args;
    va_start(args, text);
    // Formatear el mensaje usando la funcion de utilidad `FormatMessage`
    std::string formattedmessage = FormatMessage(text, args);
    va_end(args);

    gServerLog.Output(LogType::GENERAL, formattedmessage.c_str());
}

void CLog::ToDisp(LogColor color, const char* text, ...) {
    va_list args;
    va_start(args, text);
    // Formatear el mensaje usando la funcion de utilidad `FormatMessage`
    std::string formattedmessage = FormatMessage(text, args);
    va_end(args);

    // Llamar a LogAddText para agregar el log al array m_log
    gServerDisplayer.LogAddText(color, formattedmessage);
    
    gServerLog.Output(LogType::GENERAL, formattedmessage.c_str());
}