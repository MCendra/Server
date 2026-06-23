// QueryManager.cpp
#include "Header.h"
#include "QueryManager.h"
#include "Log.h"
#include "Util.h"

CQueryManager gQueryManager;

// Construction/Destruction

CQueryManager::CQueryManager()
	: m_SQLEnvironment(SQL_NULL_HANDLE),
	m_SQLConnection(SQL_NULL_HANDLE),
	m_STMT(SQL_NULL_HANDLE),
	m_RowCount(-1),
	m_ColCount(-1)
{
	// Inicialización a cero de todos los buffers de texto y credenciales
	memset(m_odbc, 0, sizeof(m_odbc));
	memset(m_user, 0, sizeof(m_user));
	memset(m_pass, 0, sizeof(m_pass));

	// Inicialización de las estructuras de datos de las columnas de SQL
	memset(m_SQLColName, 0, sizeof(m_SQLColName));
	memset(m_SQLData, 0, sizeof(m_SQLData));
	memset(m_SQLDataLen, 0, sizeof(m_SQLDataLen));
	memset(m_SQLBindValue, 0, sizeof(m_SQLBindValue));

	// Asignación del entorno básico de ODBC e indicación de versión nativa (ODBC 3)
	// NOTA: Se remueven los "" para un código limpio
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_SQLEnvironment);
	SQLSetEnvAttr(m_SQLEnvironment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
}

CQueryManager::~CQueryManager()
{
	Disconnect();
}


bool CQueryManager::Connect(char* odbc, char* user, char* pass)
{
	strcpy_s(m_odbc, odbc);
	strcpy_s(m_user, user);
	strcpy_s(m_pass, pass);

	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, m_SQLEnvironment, &m_SQLConnection)) == 0)
	{
		m_SQLConnection = SQL_NULL_HANDLE;
		return 0;
	}

	if (SQL_SUCCEEDED(SQLConnect(m_SQLConnection,
		(SQLCHAR*)m_odbc,
		SQL_NTS,
		(SQLCHAR*)m_user,
		SQL_NTS,
		(SQLCHAR*)m_pass,
		SQL_NTS)) == 0)
	{
		SQLFreeHandle(SQL_HANDLE_DBC, m_SQLConnection);
		m_SQLConnection = SQL_NULL_HANDLE;
		return 0;
	}

	if (SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, m_SQLConnection, &m_STMT)) == 0)
	{
		SQLDisconnect(m_SQLConnection);
		SQLFreeHandle(SQL_HANDLE_DBC, m_SQLConnection);

		m_SQLConnection = SQL_NULL_HANDLE;
		m_STMT = SQL_NULL_HANDLE;

		return 0;
	}

	return 1;
}

void CQueryManager::Disconnect()
{
	// 1. Liberar el Statement (Esto ya cierra cursores y libera bindings internamente)
	if (m_STMT != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_STMT, m_STMT);
		m_STMT = SQL_NULL_HANDLE;
	}

	// 2. Desconectar y liberar la Conexión
	if (m_SQLConnection != SQL_NULL_HANDLE)
	{
		SQLDisconnect(m_SQLConnection);
		SQLFreeHandle(SQL_HANDLE_DBC, m_SQLConnection);
		m_SQLConnection = SQL_NULL_HANDLE;
	}

	// 3. Liberar el Environment
	if (m_SQLEnvironment != SQL_NULL_HANDLE)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, m_SQLEnvironment);
		m_SQLEnvironment = SQL_NULL_HANDLE;
	}
}

// Ejecuta una consulta SQL de diagnóstico y muestra el resultado en la consola
void CQueryManager::Diagnostic(const char* query)
{
	Log.ToDisp(LOG_BLACK, "%s", query);

	SQLINTEGER NativeError = 0;
	SQLSMALLINT BufferLength = 0;
	SQLCHAR SqlState[6] = { 0 };
	SQLCHAR MessageText[SQL_MAX_MESSAGE_LENGTH] = { 0 };

	bool reconnect = false;

	for (SQLSMALLINT RecNumber = 1;; RecNumber++)
	{
		SQLRETURN result = SQLGetDiagRec(
			SQL_HANDLE_STMT,
			m_STMT,
			RecNumber,
			SqlState,
			&NativeError,
			MessageText,
			sizeof(MessageText),
			&BufferLength);

		if (result == SQL_NO_DATA)
		{
			break;
		}

		Log.ToDisp(LOG_RED, "[QueryManager - Diagnostic] Estado (%s), Diagnostico: %s", SqlState, MessageText);

		if (strcmp((char*)SqlState, "08S01") == 0)
		{
			reconnect = true;
		}
	}

	if (reconnect)
	{
		Log.ToDisp(
			LOG_RED,"[QueryManager - Diagnostic] Falla en la conexion detectada. Reconectando...");

		if (m_STMT != SQL_NULL_HANDLE)
		{
			SQLFreeHandle(SQL_HANDLE_STMT, m_STMT);
			m_STMT = SQL_NULL_HANDLE;
		}

		if (m_SQLConnection != SQL_NULL_HANDLE)
		{
			SQLFreeHandle(SQL_HANDLE_DBC, m_SQLConnection);
			m_SQLConnection = SQL_NULL_HANDLE;
		}

		if (Connect(m_odbc, m_user, m_pass) == 0)
		{
			Log.ToDisp(LOG_RED, "[QueryManager - Diagnostic] Fallo al reconectarse al servidor SQL.");
		}
	}
}

bool CQueryManager::ExecQuery(char* query,...) // OK
{
	char buff[4096];

	va_list arg;
	va_start(arg,query);
	vsprintf_s(buff,query,arg);
	va_end(arg);

	SQLRETURN result;

	if(SQL_SUCCEEDED((result=SQLExecDirect(m_STMT,(SQLCHAR*)buff,SQL_NTS))) == 0 && result != SQL_NO_DATA)
	{
		Diagnostic(buff);
		return 0;
	}

	SQLRowCount(m_STMT,&m_RowCount);

	if(m_RowCount == 0){return 1;}

	SQLNumResultCols(m_STMT,&m_ColCount);

	if(m_ColCount == 0){return 1;}

	if(m_ColCount > MAX_COLUMNS){return 0;}

	memset(m_SQLColName,0,sizeof(m_SQLColName));

	memset(m_SQLData,0,sizeof(m_SQLData));

	for(int n=0;n < m_ColCount;n++)
	{
		SQLDescribeCol(m_STMT,(n+1),m_SQLColName[n],sizeof(m_SQLColName[n]),0,0,0,0,0);
		SQLBindCol(m_STMT,(n+1),SQL_C_CHAR,m_SQLData[n],sizeof(m_SQLData[n]),&m_SQLDataLen[n]);
	}

	return 1;
}

void CQueryManager::Close()
{
	SQLCloseCursor(m_STMT);
	SQLFreeStmt(m_STMT,SQL_UNBIND);
}

SQLRETURN CQueryManager::Fetch()
{
	return SQLFetch(m_STMT);
}

int CQueryManager::FindIndex(char* ColName)
{
	for(int n=0;n < m_ColCount;n++)
	{
		if(_stricmp(ColName,(char*)m_SQLColName[n]) == 0)
		{
			return n;
		}
	}

	return -1;
}

int CQueryManager::GetResult(int index)
{
	return atoi(m_SQLData[index]);
}

int CQueryManager::GetAsInteger(char* ColName)
{
	int index = FindIndex(ColName);

	if(index == -1)
	{
		return index;
	}
	else
	{
		return atoi(m_SQLData[index]);
	}
}

float CQueryManager::GetAsFloat(char* ColName)
{
	int index = FindIndex(ColName);

	if(index == -1)
	{
		return (float)index;
	}
	else
	{
		return (float)atof(m_SQLData[index]);
	}
}

__int64 CQueryManager::GetAsInteger64(char* ColName)
{
	int index = FindIndex(ColName);

	if(index == -1)
	{
		return index;
	}
	else
	{
		return _atoi64(m_SQLData[index]);
	}
}

void CQueryManager::GetAsString(char* ColName,char* OutBuffer,int OutBufferSize)
{
	int index = FindIndex(ColName);

	if(index == -1)
	{
		memset(OutBuffer,0,OutBufferSize);
	}
	else
	{
		strncpy_s(OutBuffer,OutBufferSize,m_SQLData[index],(OutBufferSize-1));
	}
}

void CQueryManager::GetAsBinary(char* ColName,BYTE* OutBuffer,int OutBufferSize)
{
	int index = FindIndex(ColName);

	if(index == -1)
	{
		memset(OutBuffer,0,OutBufferSize);
	}
	else
	{
		ConvertStringToBinary(m_SQLData[index], (int)strlen(m_SQLData[index]), OutBuffer, OutBufferSize);
	}
}

void CQueryManager::BindParameterAsString(int ParamNumber,void* InBuffer,int ColumnSize)
{
	m_SQLBindValue[(ParamNumber-1)] = SQL_NTS;

	SQLBindParameter(m_STMT,ParamNumber,SQL_PARAM_INPUT,SQL_C_CHAR,SQL_VARCHAR,ColumnSize,0,InBuffer,0,&m_SQLBindValue[(ParamNumber-1)]);
}

void CQueryManager::BindParameterAsBinary(int ParamNumber,void* InBuffer,int ColumnSize)
{
	m_SQLBindValue[(ParamNumber-1)] = ColumnSize;

	SQLBindParameter(m_STMT,ParamNumber,SQL_PARAM_INPUT,SQL_C_BINARY,SQL_VARBINARY,ColumnSize,0,InBuffer,0,&m_SQLBindValue[(ParamNumber-1)]);
}

// CORRECCIÓN CRÍTICA DE ALGORITMO: El original fallaba con letras minúsculas (ej: 'a'-'f')
// y tenía instrucciones nulas (size = size + 0). Reescrito usando operaciones bit a bit.
void CQueryManager::ConvertStringToBinary(const char* InBuff, int InSize, BYTE* OutBuff, int OutSize)
{
	// 1. Validaciones básicas de seguridad indispensables
	if (InBuff == nullptr || InSize <= 0 || OutSize <= 0)
	{
		return;
	}

	// 2. Limpieza inicial obligatoria del buffer de salida
	memset(OutBuff, 0, OutSize);


	// 3. El MD5 tiene 32 caracteres. Si InSize es impar, la cadena está corrupta.
	if ((InSize % 2) != 0)
	{
		return;
	}

	// 4. Determinar el límite real basado en el espacio físico del destino
	int bytesToProcess = InSize / 2;
	if (bytesToProcess > OutSize)
	{
		bytesToProcess = OutSize;
	}

	// 5. Tabla de búsqueda (LUT) compacta de 256 bytes para máxima seguridad.
	// Garantiza que cualquier caracter inválido (incluido el ASCII alto) aborte el login.
	static const BYTE HexTable[256] = {
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // '0'-'9'
		0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // 'A'-'F'
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // 'a'-'f'
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // Bloque ASCII Alto (128-255)
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
	};

	// 6. Bucle de conversión lineal seguro
	for (int outIdx = 0; outIdx < bytesToProcess; ++outIdx)
	{
		int n = outIdx << 1;

		BYTE val1 = HexTable[static_cast<unsigned char>(InBuff[n])];
		BYTE val2 = HexTable[static_cast<unsigned char>(InBuff[n + 1])];

		// Si la base de datos devuelve un hash corrupto o caracteres extraños,
		// limpiamos el buffer de salida y abortamos de inmediato. El login fallará de forma segura.
		if ((val1 | val2) == 0xFF)
		{
			memset(OutBuff, 0, OutSize);
			return;
		}

		OutBuff[outIdx] = static_cast<BYTE>((val1 << 4) | val2);
	}
}


// OPTIMIZACIÓN: Reemplazadas las divisiones (/ 16 y % 16) por corrimientos de bits (>> 4 y & 0x0F)
// Es mucho más rápido en procesadores x86 de 32 bits.
void CQueryManager::ConvertBinaryToString(BYTE* InBuff, int InSize, char* OutBuff, int OutSize)
{
	static const char Hex[] = "0123456789ABCDEF";

	memset(OutBuff, 0, OutSize);

	int out = 0;

	for (int n = 0; n < InSize && (out + 1) < OutSize; n++)
	{
		BYTE b = InBuff[n];

		OutBuff[out++] = Hex[(b >> 4) & 0x0F];
		OutBuff[out++] = Hex[b & 0x0F];
	}
}
