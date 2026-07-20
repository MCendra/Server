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
	CHAT = 1,
	COMMAND = 2,
	TRADE = 3,
	CONNECT = 4,
	HACK = 5,
	CASH_SHOP = 6,
	CHAOS_MIX = 7,
	ANTIFLOOD = 8,
	RESET = 9,
	TIEN_TE = 10,
	KET_NOI = 11,
	THU_MUA = 12,
	MAX_LOG = 20
};

struct LogInitInfo
{
	LogType type;
	const char* directory;
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
