// ServerConfigLoader.cpp
#include "Header.h"
#include "ServerConfigLoader.h"

CServerConfigLoader::CServerConfigLoader()
{
    this->m_buff = 0;
    this->m_size = 0;
    memset(this->m_path, 0, sizeof(this->m_path));
    this->SetLastError(4);
}

CServerConfigLoader::~CServerConfigLoader()
{
    if (this->m_buff != 0)
    {
        delete[] this->m_buff;
        this->m_buff = 0;
    }
    this->m_size = 0;
}

bool CServerConfigLoader::SetBuffer(const char* path)
{
    strcpy_s(this->m_path, path);

    HANDLE file = CreateFileA(this->m_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_ARCHIVE, 0);

    if (file == INVALID_HANDLE_VALUE)
    {
        this->SetLastError(0);
        return 0;
    }

    this->m_size = GetFileSize(file, 0);

    if (this->m_buff != 0)
    {
        delete[] this->m_buff;
        this->m_buff = 0;
    }

    this->m_buff = new char[this->m_size];

    if (this->m_buff == 0)
    {
        this->SetLastError(1);
        CloseHandle(file);
        return 0;
    }

    DWORD OutSize = 0;

    if (ReadFile(file, this->m_buff, this->m_size, &OutSize, 0) == 0)
    {
        this->SetLastError(2);
        CloseHandle(file);
        return 0;
    }

    CloseHandle(file);

    this->m_count = 0;

    this->m_tick = GetTickCount64(); // Modificado para GetTickCount64()

    return 1;
}

bool CServerConfigLoader::GetBuffer(char* buff, DWORD* size)
{
    if (this->m_buff == 0)
    {
        this->SetLastError(3);
        return 0;
    }
    memcpy(buff, this->m_buff, this->m_size);
    (*size) = this->m_size;
    return 1;
}

char CServerConfigLoader::GetChar()
{
    if (this->m_count >= this->m_size)
    {
        return -1;
    }
    return this->m_buff[this->m_count++];
}

void CServerConfigLoader::UnGetChar(char ch)
{
    if (this->m_count == 0)
    {
        return;
    }

    this->m_buff[--this->m_count] = ch;
}

char CServerConfigLoader::CheckComment(char ch)
{
    if (ch != '/' || (ch = this->GetChar()) != '/')
    {
        return ch;
    }
    while (true)
    {
        if ((ch = this->GetChar()) == -1)
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
    if ((GetTickCount64() - this->m_tick) > 1000)
    {
        this->SetLastError(4);
        throw 1;
    }

    this->m_number = 0;

    memset(this->m_string, 0, sizeof(this->m_string));

    char ch;

    while (true)
    {
        if ((ch = this->GetChar()) == -1)
        {
            return TOKEN_END;
        }

        if (isspace(ch) != 0)
        {
            continue;
        }

        if ((ch = this->CheckComment(ch)) == -1)
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
        return this->GetTokenNumber(ch);
    }

    if (ch == '"')
    {
        return this->GetTokenString(ch);
    }

    return this->GetTokenCommon(ch);
}

eTokenResult CServerConfigLoader::GetTokenNumber(char ch)
{
    int count = 0;

    this->UnGetChar(ch);

    while ((ch = this->GetChar()) != -1 && (ch == '-' || ch == '.' || ch == '*' || isdigit(ch) != 0))
    {
        this->m_string[count++] = ch;
    }

    if (strcmp(this->m_string, "*") == 0)
    {
        this->m_number = -1;
    }
    else
    {
        this->m_number = (float)atof(this->m_string);
    }

    this->m_string[count] = 0;

    return TOKEN_NUMBER;
}

eTokenResult CServerConfigLoader::GetTokenString(char ch)
{
    int count = 0;

    while ((ch = this->GetChar()) != -1 && ch != '"')
    {
        this->m_string[count++] = ch;
    }

    if (ch != '"')
    {
        this->UnGetChar(ch);
    }

    this->m_string[count] = 0;

    return TOKEN_STRING;
}

eTokenResult CServerConfigLoader::GetTokenCommon(char ch)
{
    if (isalpha(ch) == 0)
    {
        return TOKEN_ERROR;
    }

    int count = 0;

    this->m_string[count++] = ch;

    while ((ch = this->GetChar()) != -1 && (ch == '.' || ch == '_' || isalnum(ch) != 0))
    {
        this->m_string[count++] = ch;
    }

    this->UnGetChar(ch);

    this->m_string[count] = 0;

    return TOKEN_STRING;
}

void CServerConfigLoader::SetLastError(int error)
{
    switch (error)
    {
    case 0:
        wsprintfA(this->m_LastError, SERVER_CONFIG_ERROR_CODE0, this->m_path);
        break;
    case 1:
        wsprintfA(this->m_LastError, SERVER_CONFIG_ERROR_CODE1, this->m_path);
        break;
    case 2:
        wsprintfA(this->m_LastError, SERVER_CONFIG_ERROR_CODE2, this->m_path);
        break;
    case 3:
        wsprintfA(this->m_LastError, SERVER_CONFIG_ERROR_CODE3, this->m_path);
        break;
    case 4:
        wsprintfA(this->m_LastError, SERVER_CONFIG_ERROR_CODE4, this->m_path);
        break;
    default:
        wsprintfA(this->m_LastError, SERVER_CONFIG_ERROR_CODEX, this->m_path, error);
        break;
    }
}

char* CServerConfigLoader::GetLastError()
{
    return this->m_LastError;
}

int CServerConfigLoader::GetNumber()
{
    return (int)this->m_number;
}

int CServerConfigLoader::GetAsNumber()
{
    this->GetToken();

    return (int)this->m_number;
}

float CServerConfigLoader::GetFloatNumber()
{
    return this->m_number;
}

float CServerConfigLoader::GetAsFloatNumber()
{
    this->GetToken();

    return this->m_number;
}

char* CServerConfigLoader::GetString()
{
    return this->m_string;
}

char* CServerConfigLoader::GetAsString()
{
    this->GetToken();

    return this->m_string;
}
