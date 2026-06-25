// ScriptParser.h
#pragma once
#include "FileHandle.h"
#include "BufferHandle.h"

#define SERVER_CONFIG_ALLOC_ERROR "[%s] No se pudo asignar memoria para ScriptParser"
#define SERVER_CONFIG_ERROR_CODE0 "[%s] No se pudo abrir el archivo"
#define SERVER_CONFIG_ERROR_CODE1 "[%s] No se pudo asignar el buffer del archivo"
#define SERVER_CONFIG_ERROR_CODE2 "[%s] No se pudo leer el archivo"
#define SERVER_CONFIG_ERROR_CODE3 "[%s] No se pudo obtener el buffer del archivo"
#define SERVER_CONFIG_ERROR_CODE4 "[%s] El archivo no se configuro correctamente"
#define SERVER_CONFIG_ERROR_CODEX "[%s] Codigo de error desconocido: %d"

enum eTokenResult
{
    TOKEN_NUMBER = 0,
    TOKEN_STRING = 1,
    TOKEN_END = 2,
    TOKEN_ERROR = 3,
};

class CScriptParser
{
public:
    CScriptParser();
    virtual ~CScriptParser();
    bool SetBuffer(const char* path);
    bool GetBuffer(char* buff, DWORD* size);
    char GetChar();
    void UnGetChar(char ch);
    char CheckComment(char ch);
    eTokenResult GetToken();
    eTokenResult GetTokenNumber(char ch);
    eTokenResult GetTokenString(char ch);
    eTokenResult GetTokenCommon(char ch);
    void SetLastError(int error);
    char* GetLastError();
    int GetNumber();
    int GetAsNumber();
    float GetFloatNumber();
    float GetAsFloatNumber();
    char* GetString();
    char* GetAsString();
private:
    char* m_buff;
    DWORD m_size;
    char m_path[256];
    DWORD m_count;
    float m_number;
    char m_string[256];
    ULONGLONG m_tick;
    char m_LastError[256];
};
