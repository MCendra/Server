// Log.cpp
#include "Header.h"
#include "Log.h"

// Definición de la instancia global de ServerLog
CLog Log;

// Constructor de CServerLog
CLog::CLog() {
{
	this->m_count = 0;
}

// Destructor de CServerLog
CLog::~CLog() {
	// Destructor - libera cualquier recurso si es necesario
	for(int n=0;n < this->m_count;n++)
	{
		if(this->m_LogInfo[n].Active != 0)
		{
			CloseHandle(this->m_LogInfo[n].File);
		}
	}
}

std::string CLog::FormatMessage(const char* text, va_list args) const {
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), text, args);
	return std::string(buffer);
}

void CLog::ToFile(const char* text, ...) {
	va_list args;
	va_start(args, text);
	// Formatear el mensaje usando la función de utilidad `FormatMessage`
	std::string formattedmessage = FormatMessage(text, args);
	va_end(args);

	gServerLog.Output(LogType::GENERAL, formattedmessage.c_str());
}

void CLog::ToDisp(LogColor color, const char* text, ...) {
	va_list args;
	va_start(args, text);
	// Formatear el mensaje usando la función de utilidad `FormatMessage`
	std::string formattedmessage = FormatMessage(text, args);
	va_end(args);

	// Llamar a LogAddText para agregar el log al array m_log
	gServerDisplayer.LogAddText(color, formattedmessage);

	gServerLog.Output(LogType::GENERAL, formattedmessage.c_str());
}

//void CLog::AddLog(BOOL active,char* directory) // OK
//{
//	if(this->m_count < 0 || this->m_count >= MAX_LOG)
//	{
//		return;
//	}
//
//	LOG_INFO* lpInfo = &this->m_LogInfo[this->m_count++];
//
//	lpInfo->Active = active;
//
//	strcpy_s(lpInfo->Directory,directory);
//
//	if(lpInfo->Active != 0)
//	{
//		CreateDirectory(lpInfo->Directory,0);
//
//		SYSTEMTIME time;
//
//		GetLocalTime(&time);
//
//		lpInfo->Day = time.wDay;
//
//		lpInfo->Month = time.wMonth;
//
//		lpInfo->Year = time.wYear;
//
//		wsprintf(lpInfo->Filename,".\\%s\\%04d-%02d-%02d.txt",lpInfo->Directory,lpInfo->Year,lpInfo->Month,lpInfo->Day);
//
//		lpInfo->File = CreateFile(lpInfo->Filename,GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
//
//		if(lpInfo->File == INVALID_HANDLE_VALUE)
//		{
//			lpInfo->Active = 0;
//			return;
//		}
//
//		if(SetFilePointer(lpInfo->File,0,0,FILE_END) == INVALID_SET_FILE_POINTER)
//		{
//			lpInfo->Active = 0;
//			CloseHandle(lpInfo->File);
//			return;
//		}
//	}
//}
//
//void CLog::Output(eLogType type,char* text,...) // OK
//{
//	if(type < 0 || type >= this->m_count)
//	{
//		return;
//	}
//
//	LOG_INFO* lpInfo = &this->m_LogInfo[type];
//
//	if(lpInfo->Active == 0)
//	{
//		return;
//	}
//
//	SYSTEMTIME time;
//
//	GetLocalTime(&time);
//
//	if(time.wDay != lpInfo->Day || time.wMonth != lpInfo->Month || time.wYear != lpInfo->Year)
//	{
//		CloseHandle(lpInfo->File);
//
//		lpInfo->Day = time.wDay;
//
//		lpInfo->Month = time.wMonth;
//
//		lpInfo->Year = time.wYear;
//
//		wsprintf(lpInfo->Filename,".\\%s\\%04d-%02d-%02d.txt",lpInfo->Directory,lpInfo->Year,lpInfo->Month,lpInfo->Day);
//
//		lpInfo->File = CreateFile(lpInfo->Filename,GENERIC_WRITE,FILE_SHARE_READ,0,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
//
//		if(lpInfo->File == INVALID_HANDLE_VALUE)
//		{
//			lpInfo->Active = 0;
//			return;
//		}
//
//		if(SetFilePointer(lpInfo->File,0,0,FILE_END) == INVALID_SET_FILE_POINTER)
//		{
//			lpInfo->Active = 0;
//			CloseHandle(lpInfo->File);
//			return;
//		}
//	}
//
//	char temp[1024] = {0};
//
//	va_list arg;
//	va_start(arg,text);
//	vsprintf_s(temp,text,arg);
//	va_end(arg);
//
//	char buff[1024] = {0};
//
//	wsprintf(buff,"%02d:%02d:%02d %s\r\n",time.wHour,time.wMinute,time.wSecond,temp);
//
//	DWORD OutSize;
//
//	WriteFile(lpInfo->File,buff,strlen(buff),&OutSize,0);
//}
