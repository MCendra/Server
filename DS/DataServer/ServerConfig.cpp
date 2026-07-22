// ServerConfig.cpp
#include "Header.h"
#include "ServerConfig.h"

// Instancia global del visualizador de servidor
CServerConfig gServerConfig;

// Definición de las constantes
constexpr char CONFIG_FILE_NAME[] = "DataServer.ini";
constexpr char ALLOWABLE_IP_LIST_FILE_NAME[] = "AllowableIpList.txt";
constexpr char BADSYNTAX_FILE_NAME[] = "BadSyntax.txt";

constexpr char SECTION_DATA_SERVER_INFO[] = "DataServerInfo";

constexpr char KEY_CUSTOMER_NAME[] = "CustomerName";
constexpr char KEY_CUSTOMER_HARDWARE_ID[] = "CustomerHardwareId";
constexpr char KEY_DATA_SERVER_PORT[] = "DataServerPort";
constexpr char KEY_DATA_SERVER_ODBC[] = "DataServerODBC";
constexpr char KEY_DATA_SERVER_USER[] = "DataServerUSER";
constexpr char KEY_DATA_SERVER_PASS[] = "DataServerPASS";
constexpr char KEY_DATA_ADVANCED_LOG[] = "AdvancedLog";
constexpr char KEY_RSTIME_CTC[] = "RSTimeCTC";

constexpr char DEFAULT_CONFIG[] = R"ini([DataServerInfo]
CustomerName=
CustomerHardwareId=
DataServerPort=55960
DataServerODBC=MuOnlineS6
DataServerUSER=sa
DataServerPASS=$Magda314$$$
AdvancedLog=1
RSTimeCTC=40)ini";

constexpr char DEFAULT_ALLOWABLE_IP_LIST[] = R"ini(0
"127.0.0.1"
end
)ini";

constexpr char DEFAULT_BADSYNTAX[] = R"ini(end)ini";

// Definición de las variables externas
char ConfigFilePath[MAX_PATH];
char AllowableIpListFilePath[MAX_PATH];
char BadSyntaxFilePath[MAX_PATH];
char CustomerName[32];
char CustomerHardwareId[36];
WORD DataServerPort;
char DataServerODBC[32];
char DataServerUSER[32];
char DataServerPASS[32];
BOOL AdvancedLog;
WORD RSTimeCTC;

// Construction/Destruction

bool CServerConfig::Init() {

	// Establece paths a los archivos
	sprintf_s(ConfigFilePath, "%s%s", WorkingPath, CONFIG_FILE_NAME);
	sprintf_s(AllowableIpListFilePath, "%s%s", WorkingPath, ALLOWABLE_IP_LIST_FILE_NAME);
	sprintf_s(BadSyntaxFilePath, "%s%s", WorkingPath, BADSYNTAX_FILE_NAME);

	// Asegura que los archivos existan
	bool configResult = EnsureConfigFileExists();
	bool alloweableIpListResult = EnsureServerListFileExists();
	bool badSyntaxResult = EnsureBadSyntaxFileExists();

	return configResult && alloweableIpListResult && badSyntaxResult;
}

bool CServerConfig::EnsureConfigFileExists() {
	// Verificar si el archivo existe utilizando GetFileAttributes
	DWORD fileAttributes = GetFileAttributesA(ConfigFilePath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		// El archivo no existe, lo creamos con valores por defecto
		{
			FileHandle file(ConfigFilePath, GENERIC_WRITE, CREATE_ALWAYS);
			if (file.getHandle() != INVALID_HANDLE_VALUE) {
				char buffer[2048];
				sprintf_s(buffer, sizeof(buffer), DEFAULT_CONFIG);

				DWORD bytesWritten;
				if (file.write(buffer, static_cast<DWORD>(strlen(buffer)), bytesWritten)) {
					Log.ToFile(LogType::GENERAL, "[InitConfig] Archivo %s creado con valores por defecto.", CONFIG_FILE_NAME);
				}
				else {
					Log.ToFile(LogType::GENERAL, "[InitConfig] Error al escribir en el archivo %s.", CONFIG_FILE_NAME);
					return false;
				}
			}
			else {
				Log.ToFile(LogType::GENERAL, "[InitConfig] Error al crear el archivo %s.", CONFIG_FILE_NAME);
				return false;
			}
		} // El archivo se cierra aquí.
	}

	// El archivo ya existe o ha sido creado correctamente, lo cargamos
	return LoadConfig();
}

bool CServerConfig::LoadConfig() {

	bool allValuesLoaded = true;

	// Leer CustomerName directamente en CustomerName
	if (GetPrivateProfileStringA(SECTION_DATA_SERVER_INFO, KEY_CUSTOMER_NAME, "", CustomerName, sizeof(CustomerName), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerName: %s", CustomerName);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerName desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	// Leer CustomerHardwareId directamente en CustomerHardwareId
	if (GetPrivateProfileStringA(SECTION_DATA_SERVER_INFO, KEY_CUSTOMER_HARDWARE_ID, "", CustomerHardwareId, sizeof(CustomerHardwareId), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerHardwareId: %s", CustomerHardwareId);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerHardwareId desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	DataServerPort = static_cast<WORD>(GetPrivateProfileIntA(SECTION_DATA_SERVER_INFO, KEY_DATA_SERVER_PORT, 0, ConfigFilePath));
	if (DataServerPort > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Puerto servidor de cuentas: %d", DataServerPort);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar DataServerPort desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_DATA_SERVER_INFO, KEY_DATA_SERVER_ODBC, "", DataServerODBC, sizeof(DataServerODBC), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Conectividad abierta de para la base datos: %s", DataServerODBC);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar DataServerODBC desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_DATA_SERVER_INFO, KEY_DATA_SERVER_USER, "", DataServerUSER, sizeof(DataServerUSER), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Usuario para la base datos: %s", DataServerUSER);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar DataServerUSER desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_DATA_SERVER_INFO, KEY_DATA_SERVER_PASS, "", DataServerPASS, sizeof(DataServerPASS), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Password para la base datos leida correctamente.");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar DataServerPASS desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	AdvancedLog = static_cast<BYTE>(GetPrivateProfileIntA(SECTION_DATA_SERVER_INFO, KEY_DATA_ADVANCED_LOG, 0, ConfigFilePath));
	if (AdvancedLog == 0 || AdvancedLog == 1) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Registro de acciones avanzados: %s", (AdvancedLog == 1) ? "SI" : "NO");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar AdvancedLog desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

#if(CHIEN_TRUONG_CO)
	RSTimeCTC = static_cast<WORD>(GetPrivateProfileIntA(SECTION_DATA_SERVER_INFO, KEY_RSTIME_CTC, 0, ConfigFilePath));
	Log.ToDisp(LOG_BLACK, "[InitConfig] RSTimeCTC: %d", RSTimeCTC);
#endif

	// Informar si todos los valores fueron leídos correctamente
	if (allValuesLoaded) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Todos los valores de configuración se cargaron correctamente.");
	}
	return allValuesLoaded;
}

bool CServerConfig::EnsureServerListFileExists() {
	// Verificar si el archivo existe utilizando GetFileAttributes
	DWORD fileAttributes = GetFileAttributesA(AllowableIpListFilePath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		// El archivo no existe, lo creamos con valores por defecto
		FileHandle file(AllowableIpListFilePath, GENERIC_WRITE, CREATE_ALWAYS);
		if (file.getHandle() != INVALID_HANDLE_VALUE) {
			DWORD bytesWritten;
			// Usar el método `write` de `FileHandle` para escribir datos
			if (file.write(DEFAULT_ALLOWABLE_IP_LIST, static_cast<DWORD>(strlen(DEFAULT_ALLOWABLE_IP_LIST)), bytesWritten)) {
				Log.ToFile(LogType::GENERAL, "[InitConfig] Archivo de lista %s creado con valores por defecto.", ALLOWABLE_IP_LIST_FILE_NAME);
				return true;
			}
			else {
				Log.ToFile(LogType::GENERAL, "[InitConfig] Error al escribir en el archivo de lista %s.", ALLOWABLE_IP_LIST_FILE_NAME);
				return false;
			}
		}
		else {
			Log.ToFile(LogType::GENERAL, "[InitConfig] Error al crear el archivo de lista %s.", ALLOWABLE_IP_LIST_FILE_NAME);
			return false;
		}
	}
	else {
		// El archivo ya existe, confirmamos que existe
		Log.ToFile(LogType::GENERAL, "[InitConfig] Se utilizará %s existente.", ALLOWABLE_IP_LIST_FILE_NAME);
		return true;
	}
}

bool CServerConfig::EnsureBadSyntaxFileExists() {
	// Verificar si el archivo existe utilizando GetFileAttributes
	DWORD fileAttributes = GetFileAttributesA(BadSyntaxFilePath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		// El archivo no existe, lo creamos con valores por defecto
		FileHandle file(BadSyntaxFilePath, GENERIC_WRITE, CREATE_ALWAYS);
		if (file.getHandle() != INVALID_HANDLE_VALUE) {
			DWORD bytesWritten;
			// Usar el método `write` de `FileHandle` para escribir datos
			if (file.write(DEFAULT_BADSYNTAX, static_cast<DWORD>(strlen(DEFAULT_BADSYNTAX)), bytesWritten)) {
				Log.ToFile(LogType::GENERAL, "[InitConfig] Archivo de lista %s creado con valores por defecto.", BADSYNTAX_FILE_NAME);
				return true;
			}
			else {
				Log.ToFile(LogType::GENERAL, "[InitConfig] Error al escribir en el archivo de lista %s.", BADSYNTAX_FILE_NAME);
				return false;
			}
		}
		else {
			Log.ToFile(LogType::GENERAL, "[InitConfig] Error al crear el archivo de lista %s.", BADSYNTAX_FILE_NAME);
			return false;
		}
	}
	else {
		// El archivo ya existe, confirmamos que existe
		Log.ToFile(LogType::GENERAL, "[InitConfig] Se utilizará %s existente.", BADSYNTAX_FILE_NAME);
		return true;
	}
}

const char* CServerConfig::getIniPath() {
	return ConfigFilePath;
}

const char* CServerConfig::getServerListPath() {
	return AllowableIpListFilePath;
}

const char* CServerConfig::getBadSyntaxPath() {
	return BadSyntaxFilePath;
}
