// ServerConfig.cpp
#include "Header.h"
#include "ServerConfig.h"
#include "FileHandle.h"
#include "Log.h"

// Instancia global del visualizador de servidor
CServerConfig gServerConfig;

// Definición de las constantes
constexpr char CONFIG_FILE_NAME[] = "JoinServer.ini";
constexpr char ALLOWABLE_IP_LIST_FILE_NAME[] = "AllowableIpList.txt";

constexpr char SECTION_JOIN_SERVER_INFO[] = "JoinServerInfo";

constexpr char KEY_CUSTOMER_NAME[] = "CustomerName";
constexpr char KEY_CUSTOMER_HARDWARE_ID[] = "CustomerHardwareId";
constexpr char KEY_CONNECT_SERVER_ADDRESS[] = "ConnectServerAddress";
constexpr char KEY_CONNECT_SERVER_PORT_UDP[] = "ConnectServerPort";
constexpr char KEY_JOIN_SERVER_PORT[] = "JoinServerPort";
constexpr char KEY_JOIN_SERVER_ODBC[] = "JoinServerODBC";
constexpr char KEY_JOIN_SERVER_USER[] = "JoinServerUSER";
constexpr char KEY_JOIN_SERVER_PASS[] = "JoinServerPASS";
constexpr char KEY_GLOBAL_PASSWORD[] = "GlobalPassword";
constexpr char KEY_CASE_SENSITIVE[] = "CaseSensitive";
constexpr char KEY_MD5_ENCRYPTIONS[] = "MD5Encryptions";

constexpr char DEFAULT_CONNECT_SERVER_ADDRESS[] = "127.0.0.1";
constexpr int DEFAULT_CONNECT_SERVER_PORT_UDP = 55557;
constexpr int DEFAULT_JOIN_SERVER_TCP_PORT = 55970;
constexpr char DEFAULT_JOIN_SERVER_ODBC[] = "MuOnlineS6";
constexpr char DEFAULT_JOIN_SERVER_USER[] = "sa";
constexpr char DEFAULT_JOIN_SERVER_PASS[] = "$Magda314$$$";
constexpr char DEFAULT_GLOBAL_PASSWORD[] = "";
constexpr int DEFAULT_CASE_SENSITIVE= 1;
constexpr int DEFAULT_MD5_ENCRYPTIONS = 1;

constexpr char DEFAULT_CONFIG[] = "[JoinServerInfo]\n"
"CustomerName=\n"
"CustomerHardwareId=\n"
"ConnectServerAddress=%ld\n"
"ConnectServerPort=%d\n"
"JoinServerPort=%d\n"
"JoinServerODBC=%ld\n"
"JoinServerUSER=%ld\n"
"JoinServerPASS=%ld\n"
"GlobalPassword=%ld\n"
"CaseSensitive=%d\n"
"MD5Encryptions=%d\n";

constexpr char DEFAULT_ALLOWABLE_IP_LIST[] = "0\n"
"\"127.0.0.1\"\n"
"end\n";

// Definición de las variables externas
char ConfigFilePath[MAX_PATH];
char AlloweableIpListFilePath[MAX_PATH];
char CustomerName[32];
char CustomerHardwareId[36];
char ConnectServerAddress[16];
WORD ConnectServerPortUDP;
WORD JoinServerPort;
char JoinServerODBC[32];
char JoinServerUSER[32];
char JoinServerPASS[32];
char GlobalPassword[32];
BYTE CaseSensitive;
BYTE MD5Encryptions;

// Construction/Destruction

bool CServerConfig::Init() {

	// Establece paths a los archivos
	sprintf_s(ConfigFilePath, "%s%s", WorkingPath, CONFIG_FILE_NAME);
	sprintf_s(AlloweableIpListFilePath, "%s%s", WorkingPath, ALLOWABLE_IP_LIST_FILE_NAME);

	// Asegura que los archivos existan
	bool configResult = EnsureConfigFileExists();
	bool alloweableIpListResult = EnsureServerListFileExists();

	return configResult && alloweableIpListResult;
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
				sprintf_s(buffer, sizeof(buffer), DEFAULT_CONFIG, DEFAULT_CONNECT_SERVER_ADDRESS, DEFAULT_CONNECT_SERVER_PORT_UDP,
					DEFAULT_JOIN_SERVER_TCP_PORT, DEFAULT_JOIN_SERVER_ODBC, DEFAULT_JOIN_SERVER_USER, DEFAULT_JOIN_SERVER_PASS,
					DEFAULT_GLOBAL_PASSWORD, DEFAULT_CASE_SENSITIVE, DEFAULT_MD5_ENCRYPTIONS);

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
	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_CUSTOMER_NAME, "", CustomerName, sizeof(CustomerName), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerName: %s", CustomerName);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerName desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	// Leer CustomerHardwareId directamente en CustomerHardwareId
	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_CUSTOMER_HARDWARE_ID, "", CustomerHardwareId, sizeof(CustomerHardwareId), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerHardwareId: %s", CustomerHardwareId);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerHardwareId desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_CONNECT_SERVER_ADDRESS, "", ConnectServerAddress, sizeof(ConnectServerAddress), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Dirección del servidor de conexiones: %s", ConnectServerAddress);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ConnectServerAddress desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ConnectServerPortUDP = static_cast<WORD>(GetPrivateProfileIntA(SECTION_JOIN_SERVER_INFO, KEY_CONNECT_SERVER_PORT_UDP, 0, ConfigFilePath));
	if (ConnectServerPortUDP > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Puerto UDP: %d", ConnectServerPortUDP);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ConnectServerPortUDP desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	JoinServerPort = static_cast<WORD>(GetPrivateProfileIntA(SECTION_JOIN_SERVER_INFO, KEY_JOIN_SERVER_PORT, 0, ConfigFilePath));
	if (JoinServerPort > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Puerto servidor de cuentas: %d", JoinServerPort);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar JoinServerPort desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_JOIN_SERVER_ODBC, "", JoinServerODBC, sizeof(JoinServerODBC), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Conectividad abierta de para la base datos: %s", JoinServerODBC);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar JoinServerODBC desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, DEFAULT_JOIN_SERVER_USER, "", JoinServerUSER, sizeof(JoinServerUSER), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Usuario para la base datos: %s", JoinServerUSER);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar JoinServerODBC desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_JOIN_SERVER_PASS, "", JoinServerPASS, sizeof(JoinServerPASS), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Contraseña para la base datos leida correctamente.");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar JoinServerPASS desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_GLOBAL_PASSWORD, "", GlobalPassword, sizeof(GlobalPassword), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Contraseña global para la base datos leida correctamente.");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar GlobalPassword desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	CaseSensitive = static_cast<BYTE>(GetPrivateProfileIntA(SECTION_JOIN_SERVER_INFO, KEY_CASE_SENSITIVE, 0, ConfigFilePath));
	if (CaseSensitive == 0 || CaseSensitive == 1) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Sensible a mayúsculas y minúsculas: %s", (CaseSensitive == 1) ? "SI" : "NO");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CaseSensitive desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	MD5Encryptions = static_cast<BYTE>(GetPrivateProfileIntA(SECTION_JOIN_SERVER_INFO, KEY_MD5_ENCRYPTIONS, 0, ConfigFilePath));
	if (MD5Encryptions == 0 || MD5Encryptions == 1) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Encriptacion MD5: %s", (MD5Encryptions == 1) ? "SI" : "NO");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar MD5Encryptions desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	// Informar si todos los valores fueron leídos correctamente
	if (allValuesLoaded) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Todos los valores de configuración se cargaron correctamente.");
	}
	return allValuesLoaded;
}

bool CServerConfig::EnsureServerListFileExists() {
	// Verificar si el archivo existe utilizando GetFileAttributes
	DWORD fileAttributes = GetFileAttributesA(AlloweableIpListFilePath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		// El archivo no existe, lo creamos con valores por defecto
		FileHandle file(AlloweableIpListFilePath, GENERIC_WRITE, CREATE_ALWAYS);
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

const char* CServerConfig::getIniPath() {
	return ConfigFilePath;
}

const char* CServerConfig::getServerListPath() {
	return AlloweableIpListFilePath;
}
