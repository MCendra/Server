// Util.h
#pragma once

#include "JoinServerProtocol.h"
#include "ServerDisplayer.h"
#include "ServerManager.h"

struct SDHP_JOIN_SERVER_LIVE_SEND
{
	PBMSG_HEAD header; // C1:02
	DWORD QueueSize;
};

// Clase Util para funciones utilitarias
class CUtil
{
public:
	// Obtiene el path del ejecutable y lo convierte a formato extendido
	void GetExecutablePath();
	// Mensaje de error para terminar el proceso
	void ErrorMessageBox(const char* message, ...);
	void LogAdd(eLogColor color, const char* text, ...);
	void JoinServerLiveProc();
	bool CheckTextSyntax(char* text, int size);
	LONG CheckAccountCaseSensitive(int value);
	int GetFreeServerIndex();
	int SearchFreeServerIndex(int MinIndex, int MaxIndex, DWORD MinTime);
	CServerManager* FindServerByCode(WORD ServerCode);
	DWORD MakeAccountKey(char* account);
};

extern CUtil gUtil;

extern int gServerCount;
