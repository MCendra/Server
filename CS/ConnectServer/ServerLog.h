// ServerLog.h
#pragma once
#include "CriticalSection.h"
#include <atomic>
#include <array>
#include <sstream>
#include <iomanip>

enum class LogType : uint8_t
{
    GENERAL = 0,
    MAX_LOG
};

class CServerLog
{
public:
    // Constructor y destructor publicos
    CServerLog() noexcept;
    virtual ~CServerLog() noexcept;

    // Metodos de la clase
    void Init(bool active);
    void Output(LogType type, const std::string& text);

private:
    struct LogInfo
    {
        std::atomic<bool> active;
        char directory[MAX_PATH] = { '\0' }; // Inicializacion predeterminada
        char filename[MAX_PATH] = { '\0' };  // Inicializacion predeterminada
        HANDLE file{ INVALID_HANDLE_VALUE };
        SYSTEMTIME lastWrite{ 0 };           // Inicializacion predeterminada
    };

    static constexpr size_t MAX_LOG_TYPES = static_cast<size_t>(LogType::MAX_LOG);
    std::array<LogInfo, MAX_LOG_TYPES> m_logInfo;
    CCriticalSection m_criticalSection;

    void OpenLogFile(LogInfo& logInfo);
    static void GetCurrentTimestamp(char* buffer, size_t bufferSize);
};

// Instancia global
extern CServerLog gServerLog;