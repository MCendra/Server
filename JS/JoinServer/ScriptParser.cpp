// ScriptParser.cpp
#include "Header.h"
#include "ScriptParser.h"

constexpr DWORD MAX_SCRIPT_SIZE = 10 * 1024 * 1024; // 10 MB

CScriptParser::CScriptParser()
	: m_buffer(nullptr),
	m_size(0),
	m_count(0),
	m_number(0.0f),
	m_tick(0)
{
	memset(m_path, 0, sizeof(m_path));
	memset(m_string, 0, sizeof(m_string));
	memset(m_LastError, 0, sizeof(m_LastError));

	SetLastError(4);
}

CScriptParser::~CScriptParser() = default;

bool CScriptParser::SetBuffer(const char* path)
{
	strcpy_s(m_path, path);

	FileHandle file(m_path);

	if (file.getHandle() == INVALID_HANDLE_VALUE)
	{
		SetLastError(0);
		return false;
	}

	DWORD high = 0;

	m_size = GetFileSize(file.getHandle(), &high);

	if (m_size == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)
	{
		SetLastError(2);
		return false;
	}

	if (high != 0 || m_size > MAX_SCRIPT_SIZE)
	{
		SetLastError(2);
		return false;
	}

	try
	{
		m_buffer = std::make_unique<BufferHandle>(m_size + 1);
	}
	catch (const std::bad_alloc&)
	{
		SetLastError(1);
		return false;
	}

	if (!m_buffer)
	{
		SetLastError(1);
		return false;
	}

	DWORD outSize = 0;

	if (!file.read(m_buffer->data(), m_size, outSize) || outSize != m_size)
	{
		SetLastError(2);
		return false;
	}

	// Terminador para trabajar como string cuando haga falta
	m_buffer->data()[m_size] = '\0';

	m_count = 0;
	m_tick = GetTickCount64();

	return true;
}

bool CScriptParser::GetBuffer(char* buff, DWORD* size)
{
	if (m_buffer == nullptr || buff == nullptr || size == nullptr || *size < m_size)
	{
		SetLastError(3);
		return false;
	}

	memcpy(buff, m_buffer->data(), m_size);

	*size = m_size;

	return true;
}

char CScriptParser::GetChar()
{
	if (m_buffer == nullptr || m_count >= m_size)
	{
		return static_cast<char>(-1);
	}

	return m_buffer->data()[m_count++];
}

void CScriptParser::UnGetChar(char)
{
	if (m_count > 0)
	{
		--m_count;
	}
}

char CScriptParser::CheckComment(char ch)
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
}

eTokenResult CScriptParser::GetToken()
{
	ULONGLONG currentTick = GetTickCount64();

	if ((currentTick - m_tick) > 1000)
	{
		// Evita que un script malformado provoque un bucle infinito.
		SetLastError(4);
		throw 1;
	}

	m_tick = currentTick;

	m_number = 0.0f;
	std::memset(m_string, 0, sizeof(m_string));

	char ch;

	while (true)
	{
		ch = GetChar();

		if (ch == static_cast<char>(-1))
		{
			return TOKEN_END;
		}

		if (std::isspace(static_cast<unsigned char>(ch)))
		{
			continue;
		}

		ch = CheckComment(ch);

		if (ch == static_cast<char>(-1))
		{
			return TOKEN_END;
		}

		if (ch != '\n')
		{
			break;
		}
	}

	if (ch == '*' || ch == '-' || ch == '.' || std::isdigit(static_cast<unsigned char>(ch)))
	{
		return GetTokenNumber(ch);
	}

	if (ch == '"')
	{
		return GetTokenString(ch);
	}

	return GetTokenCommon(ch);
}

eTokenResult CScriptParser::GetTokenNumber(char ch)
{
	UnGetChar(ch);

	int count = 0;
	const int maxLen = sizeof(m_string) - 1;

	while ((ch = GetChar()) != -1 && (ch == '-' || ch == '.' || ch == '*' || std::isdigit(static_cast<unsigned char>(ch))))
	{
		if (count < maxLen)
		{
			m_string[count++] = ch;
		}
	}

	m_string[count] = 0;

	if (strcmp(m_string, "*") == 0)
	{
		m_number = -1.0f;
	}
	else
	{
		m_number = static_cast<float>(strtod(m_string, nullptr));
	}

	return TOKEN_NUMBER;
}

eTokenResult CScriptParser::GetTokenString(char ch)
{
	int count = 0;
	const int maxLen = sizeof(m_string) - 1;

	while ((ch = GetChar()) != -1 && ch != '"')
	{
		if (count < maxLen)
		{
			m_string[count++] = ch;
		}
	}

	if (ch != '"')
	{
		UnGetChar(ch);
	}

	m_string[count] = 0;

	return TOKEN_STRING;
}

eTokenResult CScriptParser::GetTokenCommon(char ch)
{
	if (!std::isalpha(static_cast<unsigned char>(ch)))
	{
		return TOKEN_ERROR;
	}

	int count = 0;
	const int maxLen = sizeof(m_string) - 1;

	m_string[count++] = ch;

	while ((ch = GetChar()) != -1 && (ch == '.' || ch == '_' || ch == '-' || std::isalnum(static_cast<unsigned char>(ch))))
	{
		if (count < maxLen)
		{
			m_string[count++] = ch;
		}
	}

	UnGetChar(ch);

	m_string[count] = 0;

	return TOKEN_STRING;
}

void CScriptParser::SetLastError(int error)
{
	switch (error)
	{
	case 0:
		sprintf_s(m_LastError, SCRIPTPARSER_ERROR_CODE0, m_path);
		break;

	case 1:
		sprintf_s(m_LastError, SCRIPTPARSER_ERROR_CODE1, m_path);
		break;

	case 2:
		sprintf_s(m_LastError, SCRIPTPARSER_ERROR_CODE2, m_path);
		break;

	case 3:
		sprintf_s(m_LastError, SCRIPTPARSER_ERROR_CODE3, m_path);
		break;

	case 4:
		sprintf_s(m_LastError, SCRIPTPARSER_ERROR_CODE4, m_path);
		break;

	default:
		sprintf_s(m_LastError, SCRIPTPARSER_ERROR_CODEX, m_path, error);
		break;
	}
}

char* CScriptParser::GetLastError()
{
	return m_LastError;
}

int CScriptParser::GetNumber()
{
	return static_cast<int>(m_number);
}

int CScriptParser::GetAsNumber()
{
	GetToken();
	return static_cast<int>(m_number);
}

float CScriptParser::GetFloatNumber()
{
	return m_number;
}

float CScriptParser::GetAsFloatNumber()
{
	GetToken();
	return m_number;
}

char* CScriptParser::GetString()
{
	return m_string;
}

char* CScriptParser::GetAsString()
{
	GetToken();
	return m_string;
}
