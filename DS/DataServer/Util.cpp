// Util.cpp
#include "Header.h"
#include "CharacterManager.h"
#include "Util.h"

//#include "CharacterManager.h"
//#include "ServerManager.h"


// Instancia global
CUtil gUtil;

char* WorkingPath = nullptr;                 // Path del ejecutable declarado en Header.h

constexpr char ERROR_TITLE[] = "Error";


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

// IMPORTANTE: "message" debe ser SIEMPRE un string literal o constante interna.
// NUNCA pasar aquí texto proveniente de un cliente, archivo de configuración,
// o cualquier fuente externa: es un formato de printf y un %s/%n malicioso
// podría leer/escribir memoria fuera de buff.
void CUtil::ErrorMessageBox(const char* message, ...)
{

	if (message == nullptr)
	{
		MessageBoxA(nullptr, "Unknown error", ERROR_TITLE, MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	char buff[512] = {};

	va_list arg;
	va_start(arg, message);

	vsnprintf_s(buff, sizeof(buff), _TRUNCATE, message, arg);

	va_end(arg);

	MessageBoxA(nullptr, buff, ERROR_TITLE, MB_OK | MB_ICONERROR);

	ExitProcess(0);

}

bool CUtil::CheckTextSyntax(const char* text, int size)
{
	for (int n = 0;n < size;n++)
	{
		if (text[n] == 0x20 || text[n] == 0x22 || text[n] == 0x27)
		{
			return false;
		}
	}

	return true;
}

std::string NormalizeToLower(const char* text)
{
	std::string key(text);

	std::transform(
		key.begin(),
		key.end(),
		key.begin(),
		[](unsigned char c)
		{
			return static_cast<char>(std::tolower(c));
		});

	return key;
}

bool GetCharacterSlot(char CharacterName[5][11],char* name,BYTE* slot)
{
	for(int n=0;n < 5;n++)
	{
		if(_stricmp(CharacterName[n],name) == 0)
		{
			*slot = static_cast<BYTE>(n);
			return true;
		}
	}

	return false;
}

WORD GetServerCodeByName(const char* name)
{
	CHARACTER_INFO CharacterInfo{};

	if (!gCharacterManager.GetCharacterInfo(&CharacterInfo, name)) 
	{
		return 0xFFFF;
	}
	else
	{
		return CharacterInfo.GameServerCode;
	}
}


