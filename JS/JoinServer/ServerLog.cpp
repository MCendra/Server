// ServerLog.cpp
#include "Header.h"
#include "ServerLog.h"

// Instancia global
CServerLog gServerLog;

constexpr char LOG_DIRECTORY_NAME[] = "LOG";
constexpr char LOG_ACCOUNT_DIRECTORY_NAME[] = "LOG_ACCOUNT";

// Constructor
CServerLog::CServerLog() noexcept
{
	// No es necesario inicializar m_criticalSection, ya que se maneja automáticamente por el RAII.
	for (auto& logInfo : m_logInfo)
	{
		logInfo.active.store(false, std::memory_order_relaxed);
		logInfo.file = INVALID_HANDLE_VALUE;
		logInfo.lastWrite = { 0 };
	}
}

// Destructor
CServerLog::~CServerLog() noexcept
{
	for (auto& logInfo : m_logInfo)
	{
		if (logInfo.active.load(std::memory_order_relaxed) && logInfo.file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(logInfo.file);
		}
	}
	// No es necesario inicializar m_criticalSection, ya que se maneja automáticamente por el RAII.
}

// Método para agregar un nuevo log
void CServerLog::Init(bool active)
{
	CCriticalSection::CLock lock(m_criticalSection); // Bloquea automáticamente

	// --- Log general ---
	auto& generalLog = m_logInfo[static_cast<size_t>(LogType::GENERAL)];
	generalLog.active.store(active, std::memory_order_relaxed);

	char generalDirectory[MAX_PATH];
	snprintf(generalDirectory, MAX_PATH, "%s\\%s", WorkingPath, LOG_DIRECTORY_NAME);
	strncpy_s(generalLog.directory, sizeof(generalLog.directory), generalDirectory, _TRUNCATE);

	if (active)
	{
		OpenLogFile(generalLog);
	}

	// --- Log de cuentas ---
	auto& accountLog = m_logInfo[static_cast<size_t>(LogType::ACCOUNT)];
	accountLog.active.store(active, std::memory_order_relaxed);

	char accountDirectory[MAX_PATH];
	snprintf(accountDirectory, MAX_PATH, "%s\\%s", WorkingPath, LOG_ACCOUNT_DIRECTORY_NAME);
	strncpy_s(accountLog.directory, sizeof(accountLog.directory), accountDirectory, _TRUNCATE);

	if (active)
	{
		OpenLogFile(accountLog);
	}

	// Al salir del scope, el destructor de CLock libera la sección crítica
}

void CServerLog::Output(LogType type, const std::string& text)
{
	if (type < LogType::GENERAL || type >= LogType::MAX_LOG)
	{
		return;
	}

	auto& logInfo = m_logInfo[static_cast<size_t>(type)];

	if (!logInfo.active.load(std::memory_order_relaxed))
	{
		return;
	}

	// Usar RAII para la sección crítica
	CCriticalSection::CLock lock(m_criticalSection);

	SYSTEMTIME currentTime;
	GetLocalTime(&currentTime);

	if (currentTime.wDay != logInfo.lastWrite.wDay || currentTime.wMonth != logInfo.lastWrite.wMonth ||	currentTime.wYear != logInfo.lastWrite.wYear)
	{
		if (logInfo.file != INVALID_HANDLE_VALUE)
		{
			CloseHandle(logInfo.file);
		}
		OpenLogFile(logInfo);
	}

	// Usar stringstream para construir la línea final del log
	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << currentTime.wHour << ':'
		<< std::setw(2) << std::setfill('0') << currentTime.wMinute << ':'
		<< std::setw(2) << std::setfill('0') << currentTime.wSecond << ' '
		<< text << "\r\n";

	std::string finalMessage = oss.str();

	DWORD bytesWritten = 0;
	if (!WriteFile(logInfo.file, finalMessage.c_str(),
		static_cast<DWORD>(finalMessage.size()), &bytesWritten, nullptr))
	{
		// El handle puede haberse invalidado externamente (ej. archivo
		// borrado/movido mientras el proceso seguia corriendo).
		// Reintentamos una vez reabriendo el archivo; si vuelve a
		// fallar, OpenLogFile desactiva el log automaticamente.
		CloseHandle(logInfo.file);
		OpenLogFile(logInfo);
	}
}

// Crea el directorio si no existe y abre (o crea) el archivo del dia
// actual para el LogInfo dado. Funciona igual para GENERAL y ACCOUNT:
// la unica diferencia entre ambos es el "directory" ya resuelto por
// Init(), esta funcion es agnostica al tipo de log.
void CServerLog::OpenLogFile(LogInfo& logInfo)
{
	CreateDirectoryA(logInfo.directory, NULL);

	SYSTEMTIME currentTime;
	GetLocalTime(&currentTime);

	snprintf(logInfo.filename, MAX_PATH, "%s\\%04d-%02d-%02d.txt",
		logInfo.directory, currentTime.wYear, currentTime.wMonth, currentTime.wDay);

	logInfo.file = CreateFileA(logInfo.filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (logInfo.file != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(logInfo.file, 0, NULL, FILE_END);
		logInfo.lastWrite = currentTime;
	}
	else
	{
		logInfo.active.store(false, std::memory_order_relaxed);
	}
}

// Metodo para obtener la marca de tiempo actual
void CServerLog::GetCurrentTimestamp(char* buffer, size_t bufferSize)
{
	SYSTEMTIME currentTime;
	GetLocalTime(&currentTime);
	snprintf(buffer, bufferSize, "%02d:%02d:%02d", currentTime.wHour, currentTime.wMinute, currentTime.wSecond);
}
