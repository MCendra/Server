#pragma once
#include "ServerLog.h"
#include "ServerDisplayer.h"

//
//#define MAX_LOG 2
//
//enum eLogType
//{
//	LOG_GENERAL = 0,
//	LOG_ACCOUNT = 1,
//};

struct LOG_INFO
{
	BOOL Active;
	char Directory[256];
	int Day;
	int Month;
	int Year;
	char Filename[256];
	HANDLE File;
};

class CLog
{
public:
	CLog();  // Constructor
	virtual ~CLog(); // Destructor

	// Registra un mensaje en el archivo de log.
	void ToFile(LogType type, const char* text, ...);

	// Registra y muestra un mensaje en la ventana del programa.
	void ToDisp(LogColor color, const char* text, ...);

private:
	// Función auxiliar para formatear el mensaje.
	std::string FormatMessage(const char* text, va_list args) const;
	//LOG_INFO m_LogInfo[MAX_LOG];
	//int m_count;
};

// Declaración global de la instancia de ServerLog
extern CLog Log;

//// Log.h: interface for the CLog class.
////
////////////////////////////////////////////////////////////////////////
//
//#pragma once
//
//#define MAX_LOG 2
//
//enum eLogType
//{
//	LOG_GENERAL = 0,
//	LOG_ACCOUNT = 1,
//};
//
//struct LOG_INFO
//{
//	BOOL Active;
//	char Directory[256];
//	int Day;
//	int Month;
//	int Year;
//	char Filename[256];
//	HANDLE File;
//};
//
//class CLog
//{
//public:
//	CLog();
//	virtual ~CLog();
//	void AddLog(BOOL active,char* directory);
//	void Output(eLogType type,char* text,...);
//private:
//	LOG_INFO m_LogInfo[MAX_LOG];
//	int m_count;
//};
//
//extern CLog gLog;
// Log.h
