// Util.cpp
#include "Header.h"
#include "Util.h"
#include "Protect.h"
#include "QueryManager.h"
#include "SocketManager.h"
#include "SocketManagerUDP.h"
#include "ThemidaSDK.h"

// Instancia global
CUtil gUtil;

char* WorkingPath = nullptr;                 // Path del ejecutable declarado en Header.h

constexpr char ERROR_TITLE[] = "Error";

int gServerCount = 0;

void CUtil::GetExecutablePath()
{
	DWORD bufferSize = MAX_PATH;
	std::vector<char> buffer;
	buffer.resize(bufferSize);

	while (true)
	{
		DWORD result = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
		if (result == 0)
		{
			// No se pudo obtener la ruta
			if (WorkingPath)
			{
				free(WorkingPath);
				WorkingPath = nullptr;
			}
			return;
		}

		// Si el buffer fue insuficiente, GetModuleFileName devuelve igual al tamaño del buffer
		if (result >= buffer.size())
		{
			// Aumentar y volver a intentar
			buffer.resize(buffer.size() * 2);
			continue;
		}

		// Encontrar la última barra invertida para mantener solo el directorio
		char* lastSlash = strrchr(buffer.data(), '\\');
		if (lastSlash)
		{
			*(lastSlash + 1) = '\0';
		}

		// Liberar anterior si existe y duplicar
		if (WorkingPath)
		{
			free(WorkingPath);
		}
		WorkingPath = _strdup(buffer.data());
		return;
	}
}

void CUtil::ErrorMessageBox(const char* message, ...) {
	char buff[256];

	// Inicializa y formatea el mensaje en el buffer
	va_list arg;
	va_start(arg, message);

	// Formatea el mensaje usando los argumentos variables
	vsprintf_s(buff, sizeof(buff), message, arg);

	// Termina el manejo de argumentos variables
	va_end(arg);

	// Muestra el mensaje en un MessageBox
	MessageBoxA(0, buff, ERROR_TITLE, MB_OK | MB_ICONERROR);

	// Termina el proceso
	ExitProcess(0);
}

void LogAdd(eLogColor color,char* text,...) // OK
{
	tm today;
	time_t ltime;
	time(&ltime);

	if(localtime_s(&today,&ltime) != 0)
	{
		return;
	}

	char time[32];

	if(asctime_s(time,sizeof(time),&today) != 0)
	{
		return;
	}

	char temp[1024];

	va_list arg;
	va_start(arg,text);
	vsprintf_s(temp,text,arg);
	va_end(arg);

	char log[1024];

	wsprintf(log,"%.8s %s",&time[11],temp);

	gServerDisplayer.LogAddText(color,log,strlen(log));
}

void JoinServerLiveProc() // OK
{
	PROTECT_START

	SDHP_JOIN_SERVER_LIVE_SEND pMsg;

	pMsg.header.set(0x02,sizeof(pMsg));

	pMsg.QueueSize = gSocketManager.GetQueueSize();

	gSocketManagerUDP.DataSend((BYTE*)&pMsg,pMsg.header.size);

	PROTECT_FINAL
}

bool CheckTextSyntax(char* text,int size) // OK
{
	for(int n=0;n < size;n++)
	{
		if(text[n] == 0x20 || text[n] == 0x22 || text[n] == 0x27)
		{
			return 0;
		}
	}

	return 1;
}

LONG CheckAccountCaseSensitive(int value) // OK
{
	if(CaseSensitive == 0)
	{
		return tolower(value);
	}
	else
	{
		return value;
	}
}

int GetFreeServerIndex() // OK
{
	int index = -1;
	int count = gServerCount;

	if(SearchFreeServerIndex(&index,0,MAX_SERVER,10000) != 0)
	{
		return index;
	}

	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[count].CheckState() == 0)
		{
			return count;
		}
		else
		{
			count = (((++count)>=MAX_SERVER)?0:count);
		}
	}

	return -1;
}

int SearchFreeServerIndex(int* index,int MinIndex,int MaxIndex,DWORD MinTime) // OK
{
	DWORD CurOnlineTime = 0;
	DWORD MaxOnlineTime = 0;

	for(int n=MinIndex;n < MaxIndex;n++)
	{
		if(gServerManager[n].CheckState() == 0 && gServerManager[n].CheckAlloc() != 0)
		{
			if((CurOnlineTime=(GetTickCount()-gServerManager[n].m_OnlineTime)) > MinTime && CurOnlineTime > MaxOnlineTime)
			{
				(*index) = n;
				MaxOnlineTime = CurOnlineTime;
			}
		}
	}

	return (((*index)==-1)?0:1);
}

CServerManager* FindServerByCode(WORD ServerCode) // OK
{
	for(int n=0;n < MAX_SERVER;n++)
	{
		if(gServerManager[n].CheckState() != 0 && gServerManager[n].m_ServerCode == ServerCode)
		{
			return &gServerManager[n];
		}
	}

	return 0;
}

DWORD MakeAccountKey(char* account) // OK
{
	int size = strlen(account);

	DWORD key = 0;

	for(int n=0;n < size;n++)
	{
		key += account[n]+17;
	}

	return ((key+((10-size)*17))%256);
}
