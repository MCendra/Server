// Log.cpp
#include "Header.h"
#include "Log.h"

CLog Log;

std::string CLog::FormatMessage(const char* text, va_list args) const
{
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), text, args);
	return std::string(buffer);
}

void CLog::ToFile(LogType type, const char* text, ...)
{
	va_list args;
	va_start(args, text);
	std::string formattedMessage = FormatMessage(text, args);
	va_end(args);

	gServerLog.Output(type, formattedMessage);
}

void CLog::ToDisp(LogColor color, const char* text, ...)
{
	va_list args;
	va_start(args, text);
	std::string formattedMessage = FormatMessage(text, args);
	va_end(args);

	gServerDisplayer.LogAddText(color, formattedMessage);

	gServerLog.Output(LogType::GENERAL, formattedMessage);
}

void CLog::ToDispAndFile(LogColor color, LogType type, const char* text, ...)
{
	va_list args;
	va_start(args, text);
	std::string formattedMessage = FormatMessage(text, args);
	va_end(args);

	gServerDisplayer.LogAddText(color, formattedMessage);

	gServerLog.Output(type, formattedMessage);
}