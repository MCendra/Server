// ServerConfig.cpp
#include "Header.h"
#include "ServerConfig.h"
#include "FileHandle.h"
#include "Log.h"

// Instancia global del visualizador de servidor
CServerConfig gServerConfig;

// Definicion de las constantes
constexpr char CONFIG_FILE_NAME[] = "ConnectServer.ini";
constexpr char SERVER_LIST_FILE_NAME[] = "ServerList.dat";

constexpr char SECTION_CONNECT_SERVER_INFO[] = "ConnectServerInfo";

constexpr char KEY_CUSTOMER_NAME[] = "CustomerName";
constexpr char KEY_CUSTOMER_HARDWARE_ID[] = "CustomerHardwareId";
constexpr char KEY_CONNECT_SERVER_PORT_TCP[] = "ConnectServerPortTCP";
constexpr char KEY_CONNECT_SERVER_PORT_UDP[] = "ConnectServerPortUDP";
constexpr char KEY_MAX_IP_CONNECTION[] = "MaxIpConnection";

constexpr int DEFAULT_TCP_PORT = 44405;
constexpr int DEFAULT_UDP_PORT = 55557;
constexpr int DEFAULT_MAX_IP_CONNECTION = 1;

constexpr char DEFAULT_CONFIG[] = "[ConnectServerInfo]\n"
"CustomerName=\n"
"CustomerHardwareId=\n"
"ConnectServerPortTCP=%d\n"
"ConnectServerPortUDP=%d\n"
"MaxIpConnection=%ld\n";

constexpr char DEFAULT_SERVER_LIST[] = "\\\\ServerCode\tServerName\tServerAddress\t\tServerPort\tServerType\n"
"0\t\tServerA\t\t127.0.0.1\t\t55901\t\tSHOW\n"
"19\t\tServer-CS\t\t127.1.0.1\t\t55919\t\tSHOW\n"
"end\n";

// Definicion de las variables externas
char ConfigFilePath[MAX_PATH];
char ServerListFilePath[MAX_PATH];
char CustomerName[32];
char CustomerHardwareId[36];
long MaxIpConnection;
WORD ConnectServerPortTCP;
WORD ConnectServerPortUDP;

// Construction/Destruction

bool CServerConfig::Init() {

	// Establece paths a los archivos
	sprintf_s(ConfigFilePath, "%s%s", WorkingPath, CONFIG_FILE_NAME);
	sprintf_s(ServerListFilePath, "%s%s", WorkingPath, SERVER_LIST_FILE_NAME);

	// Asegura que los archivos existan
	bool configResult = EnsureConfigFileExists();
	bool serverListResult = EnsureServerListFileExists();

	return configResult && serverListResult;
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
				sprintf_s(buffer, sizeof(buffer), DEFAULT_CONFIG, DEFAULT_TCP_PORT, DEFAULT_UDP_PORT, DEFAULT_MAX_IP_CONNECTION);

				DWORD bytesWritten;
				if (file.write(buffer, static_cast<DWORD>(strlen(buffer)), bytesWritten)) {
					Log.ToFile("[InitConfig] Archivo %s creado con valores por defecto.", CONFIG_FILE_NAME);
				}
				else {
					Log.ToFile("[InitConfig] Error al escribir en el archivo %s.", CONFIG_FILE_NAME);
					return false;
				}
			}
			else {
				Log.ToFile("[InitConfig] Error al crear el archivo %s.", CONFIG_FILE_NAME);
				return false;
			}
		} // El archivo se cierra aqui.
	}

	// El archivo ya existe o ha sido creado correctamente, lo cargamos
	return LoadConfig();
}

bool CServerConfig::LoadConfig() {
	bool allValuesLoaded = true;

	// Leer CustomerName directamente en CustomerName
	if (GetPrivateProfileStringA(SECTION_CONNECT_SERVER_INFO, KEY_CUSTOMER_NAME, "", CustomerName, sizeof(CustomerName), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerName: %s", CustomerName);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerName desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	// Leer CustomerHardwareId directamente en CustomerHardwareId
	if (GetPrivateProfileStringA(SECTION_CONNECT_SERVER_INFO, KEY_CUSTOMER_HARDWARE_ID, "", CustomerHardwareId, sizeof(CustomerHardwareId), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerHardwareId: %s", CustomerHardwareId);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerHardwareId desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	// Leer los puertos y conexiones maximas (cast explicito a WORD)
	ConnectServerPortTCP = static_cast<WORD>(GetPrivateProfileIntA(SECTION_CONNECT_SERVER_INFO, KEY_CONNECT_SERVER_PORT_TCP, 0, ConfigFilePath));
	if (ConnectServerPortTCP > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Puerto TCP: %d", ConnectServerPortTCP);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar el puerto TCP desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ConnectServerPortUDP = static_cast<WORD>(GetPrivateProfileIntA(SECTION_CONNECT_SERVER_INFO, KEY_CONNECT_SERVER_PORT_UDP, 0, ConfigFilePath));
	if (ConnectServerPortUDP > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Puerto UDP: %d", ConnectServerPortUDP);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar el puerto UDP desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	MaxIpConnection = GetPrivateProfileIntA(SECTION_CONNECT_SERVER_INFO, KEY_MAX_IP_CONNECTION, 0, ConfigFilePath);
	if (MaxIpConnection > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Conexiones maximas por IP: %d", MaxIpConnection);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar las conexiones maximas por IP desde %s.", CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	// Informar si todos los valores fueron leidos correctamente
	if (allValuesLoaded) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Todos los valores de configuracion se cargaron correctamente.");
	}
	return allValuesLoaded;
}

bool CServerConfig::EnsureServerListFileExists() {
	// Verificar si el archivo existe utilizando GetFileAttributes
	DWORD fileAttributes = GetFileAttributesA(ServerListFilePath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		// El archivo no existe, lo creamos con valores por defecto
		FileHandle file(ServerListFilePath, GENERIC_WRITE, CREATE_ALWAYS);
		if (file.getHandle() != INVALID_HANDLE_VALUE) {
			DWORD bytesWritten;
			// Usar el metodo `write` de `FileHandle` para escribir datos
			if (file.write(DEFAULT_SERVER_LIST, static_cast<DWORD>(strlen(DEFAULT_SERVER_LIST)), bytesWritten)) {
				Log.ToFile("[InitConfig] Archivo de lista de servidores %s creado con valores por defecto.", SERVER_LIST_FILE_NAME);
				return true;
			}
			else {
				Log.ToFile("[InitConfig] Error al escribir en el archivo de lista de servidores %s.", SERVER_LIST_FILE_NAME);
				return false;
			}
		}
		else {
			Log.ToFile("[InitConfig] Error al crear el archivo de lista de servidores %s.", SERVER_LIST_FILE_NAME);
			return false;
		}
	}
	else {
		// El archivo ya existe, confirmamos que existe
		Log.ToFile("[InitConfig] Se utilizara %s existente.", SERVER_LIST_FILE_NAME);
		return true;
	}
}

const char* CServerConfig::getIniPath() {
	return ConfigFilePath;
}

const char* CServerConfig::getServerListPath() {
	return ServerListFilePath;
}
