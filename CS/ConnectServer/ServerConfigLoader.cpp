// ServerConfigLoader.cpp
#include "Header.h"
#include "ServerConfigLoader.h"

CServerConfigLoader::CServerConfigLoader()
{
    m_buff = 0;
    m_size = 0;
    memset(m_path, 0, sizeof(m_path));
    SetLastError(4);
}

CServerConfigLoader::~CServerConfigLoader()
{
    if (m_buff != 0)
    {
        delete[] m_buff;
        m_buff = 0;
    }
    m_size = 0;
}

bool CServerConfigLoader::SetBuffer(const char* path)
{
    strcpy_s(m_path, path);

    HANDLE file = CreateFileA(m_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, 0);

    if (file == INVALID_HANDLE_VALUE)
    {
        SetLastError(0);
        return 0;
    }

    m_size = GetFileSize(file, 0);

    if (m_buff != 0)
    {
        delete[] m_buff;
        m_buff = 0;
    }

    m_buff = new char[m_size];

    if (m_buff == 0)
    {
        SetLastError(1);
        CloseHandle(file);
        return 0;
    }

    DWORD OutSize = 0;

    if (ReadFile(file, m_buff, m_size, &OutSize, 0) == 0)
    {
        SetLastError(2);
        CloseHandle(file);
        return 0;
    }

    CloseHandle(file);

    m_count = 0;

    m_tick = GetTickCount64(); // Modificado para GetTickCount64()

    return 1;
}

bool CServerConfigLoader::GetBuffer(char* buff, DWORD* size)
{
    if (m_buff == 0)
    {
        SetLastError(3);
        return 0;
    }
    memcpy(buff, m_buff, m_size);
    (*size) = m_size;
    return 1;
}

char CServerConfigLoader::GetChar()
{
    if (m_count >= m_size)
    {
        return -1;
    }
    return m_buff[m_count++];
}

void CServerConfigLoader::UnGetChar(char ch)
{
    if (m_count == 0)
    {
        return;
    }

    m_buff[--m_count] = ch;
}

char CServerConfigLoader::CheckComment(char ch)
{
    if (ch != '/' || (ch = GetChar()) != '/')
    {
        return ch;
    }
    while (true)
    {
        if ((ch = GetChar()) == -1)
        {
            return ch;
        }

        if (ch == '\n')
        {
            return ch;
        }
    }
    return ch;
}

eTokenResult CServerConfigLoader::GetToken()
{
    if ((GetTickCount64() - m_tick) > 1000)
    {
        SetLastError(4);
        throw 1;
    }

    m_number = 0;

    memset(m_string, 0, sizeof(m_string));

    char ch;

    while (true)
    {
        if ((ch = GetChar()) == -1)
        {
            return TOKEN_END;
        }

        if (isspace(ch) != 0)
        {
            continue;
        }

        if ((ch = CheckComment(ch)) == -1)
        {
            return TOKEN_END;
        }
        else if (ch != '\n')
        {
            break;
        }
    }

    if (ch == '-' || ch == '.' || ch == '*' || ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9')
    {
        return GetTokenNumber(ch);
    }

    if (ch == '"')
    {
        return GetTokenString(ch);
    }

    return GetTokenCommon(ch);
}

eTokenResult CServerConfigLoader::GetTokenNumber(char ch)
{
    int count = 0;

    UnGetChar(ch);

    while ((ch = GetChar()) != -1 && (ch == '-' || ch == '.' || ch == '*' || isdigit(ch) != 0))
    {
        m_string[count++] = ch;
    }

    if (strcmp(m_string, "*") == 0)
    {
        m_number = -1;
    }
    else
    {
        m_number = (float)atof(m_string);
    }

    m_string[count] = 0;

    return TOKEN_NUMBER;
}

eTokenResult CServerConfigLoader::GetTokenString(char ch)
{
    int count = 0;

    while ((ch = GetChar()) != -1 && ch != '"')
    {
        m_string[count++] = ch;
    }

    if (ch != '"')
    {
        UnGetChar(ch);
    }

    m_string[count] = 0;

    return TOKEN_STRING;
}

eTokenResult CServerConfigLoader::GetTokenCommon(char ch)
{
    if (isalpha(ch) == 0)
    {
        return TOKEN_ERROR;
    }

    int count = 0;

    m_string[count++] = ch;

	while ((ch = GetChar()) != -1 && (ch == '.' || ch == '_' || ch == '-' || isalnum(ch) != 0))
    {
        m_string[count++] = ch;
    }

    UnGetChar(ch);

    m_string[count] = 0;

    return TOKEN_STRING;
}

void CServerConfigLoader::SetLastError(int error)
{
    switch (error)
    {
    case 0:
        wsprintfA(m_LastError, SERVER_CONFIG_ERROR_CODE0, m_path);
        break;
    case 1:
        wsprintfA(m_LastError, SERVER_CONFIG_ERROR_CODE1, m_path);
        break;
    case 2:
        wsprintfA(m_LastError, SERVER_CONFIG_ERROR_CODE2, m_path);
        break;
    case 3:
        wsprintfA(m_LastError, SERVER_CONFIG_ERROR_CODE3, m_path);
        break;
    case 4:
        wsprintfA(m_LastError, SERVER_CONFIG_ERROR_CODE4, m_path);
        break;
    default:
        wsprintfA(m_LastError, SERVER_CONFIG_ERROR_CODEX, m_path, error);
        break;
    }
}

char* CServerConfigLoader::GetLastError()
{
    return m_LastError;
}

int CServerConfigLoader::GetNumber()
{
    return (int)m_number;
}

int CServerConfigLoader::GetAsNumber()
{
    GetToken();

    return (int)m_number;
}

float CServerConfigLoader::GetFloatNumber()
{
    return m_number;
}

float CServerConfigLoader::GetAsFloatNumber()
{
    GetToken();

    return m_number;
}

char* CServerConfigLoader::GetString()
{
    return m_string;
}

char* CServerConfigLoader::GetAsString()
{
    GetToken();

    return m_string;
}
