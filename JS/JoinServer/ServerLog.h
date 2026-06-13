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
	ACCOUNT = 1,
	MAX_LOG = 2
};

class CServerLog
{
public:
	// Constructor y destructor públicos
	CServerLog() noexcept;
	virtual ~CServerLog() noexcept;

	// Métodos de la clase
	void Init(bool active);
	void Output(LogType type, const std::string& text);

private:
	struct LogInfo
	{
		std::atomic<bool> active;
		char directory[MAX_PATH] = { '\0' }; // Inicialización predeterminada
		char filename[MAX_PATH] = { '\0' };  // Inicialización predeterminada
		HANDLE file{ INVALID_HANDLE_VALUE };
		SYSTEMTIME lastWrite{ 0 };           // Inicialización predeterminada
	};

	static constexpr size_t MAX_LOG_TYPES = static_cast<size_t>(LogType::MAX_LOG);
	std::array<LogInfo, MAX_LOG_TYPES> m_logInfo;
	CCriticalSection m_criticalSection;

	void OpenLogFile(LogInfo& logInfo);
	static void GetCurrentTimestamp(char* buffer, size_t bufferSize);
};

// Instancia global
extern CServerLog gServerLog;
