// ServerConfig.cpp
#include "Header.h"
#include "ServerConfig.h"

// Instancia global del visualizador de servidor
CServerConfig gServerConfig;

// Definicion de las constantes
constexpr char COMMON_CONFIG_FILE_NAME[] = "GameServerInfo - Common.ini";
constexpr char CHAOSMIX_CONFIG_FILE_NAME[] = "GameServerInfo - ChaosMix.ini";
constexpr char CUSTOM_CONFIG_FILE_NAME[] = "GameServerInfo - Custom.ini";
constexpr char SKILL_CONFIG_FILE_NAME[] = "GameServerInfo - Skill.ini";
constexpr char EVENT_CONFIG_FILE_NAME[] = "GameServerInfo - Event.ini";
constexpr char CHARACTER_CONFIG_FILE_NAME[] = "GameServerInfo - Character.ini";
constexpr char COMMAND_CONFIG_FILE_NAME[] = "GameServerInfo - Command.ini";

constexpr char ALLOWABLE_IP_LIST_FILE_NAME[] = "AllowableIpList.txt";

constexpr char SECTION_JOIN_SERVER_INFO[] = "GameServerInfo";

constexpr char KEY_CUSTOMER_NAME[] = "CustomerName";
constexpr char KEY_CUSTOMER_HARDWARE_ID[] = "CustomerHardwareId";
constexpr char KEY_SERVER_NAME[] = "ServerName";
constexpr char KEY_SERVER_CODE[] = "ServerCode";
constexpr char KEY_SERVER_LOCK[] = "ServerLock";
constexpr char KEY_SERVER_PORT[] = "ServerPort";
constexpr char KEY_SERVER_VERSION[] = "ServerVersion";
constexpr char KEY_SERVER_SERIAL[] = "ServerSerial";
constexpr char KEY_SERVER_MAX_USER_NUMBER[] = "ServerMaxUserNumber";
constexpr char KEY_MD5_ENCRYPTIONS[] = "MD5Encryption";
constexpr char KEY_SERVER_GUARD_MESSSAGE[] = "GuardMessage";

constexpr char DEFAULT_CONNECT_SERVER_ADDRESS[] = "127.0.0.1";
constexpr int DEFAULT_CONNECT_SERVER_PORT_UDP = 55557;
constexpr int DEFAULT_JOIN_SERVER_TCP_PORT = 55970;
constexpr char DEFAULT_JOIN_SERVER_ODBC[] = "MuOnlineS6";
constexpr char DEFAULT_JOIN_SERVER_USER[] = "sa";
constexpr char DEFAULT_JOIN_SERVER_PASS[] = "$Magda314$$$";
constexpr char DEFAULT_GLOBAL_PASSWORD[] = "";
constexpr int DEFAULT_CASE_SENSITIVE = 1;
constexpr int DEFAULT_MD5_ENCRYPTIONS = 1;

constexpr char COMMON_DEFAULT_CONFIG[] = R"ini([GameServerInfo]
;==================================================
; No modificar si no se tiene conocimiento
;==================================================
CustomerName=
CustomerHardwareId=
;==================================================
; Configuracion del servidor
;==================================================
ServerName=Sub-1
ServerCode=0
ServerLock=0
ServerPort=55901
ServerVersion=1.04.05
ServerSerial=TbYehR2hFUPBKgZj
ServerMaxUserNumber=100
; Condiciones para entrar al Sub
ServerMinLevel=0            ; Nivel minimo para entrar al Sub
ServerMaxLevel=0            ; Nivel maximo para entrar al Sub (0 significa sin limite)
ServerMinReset=0            ; Reinicio minimo para entrar al Sub
ServerMaxReset=0            ; Reinicio maximo para entrar al Sub (0 significa sin limite)
ServerMinMasterReset=0
ServerMaxMasterReset=0
; Dialogo de los guardias
GuardMessage=Bienvenido al continente de Mu
;==================================================
; Configuracion de conexion
;==================================================
DataServerAddress=127.0.0.1
DataServerPort=55960
JoinServerAddress=127.0.0.1
JoinServerPort=55972
ConnectServerAddress=127.0.0.1
ConnectServerPort=55559
;==================================================
; Activar/Desactivar registro de logs del Sub
;==================================================
WriteChatLog=1                                ; Log de chat
WriteCommandLog=1                             ; Log de comandos
WriteTradeLog=1                               ; Log de transacciones
WriteConnectLog=1                             ; Log de conexiones
WriteHackLog=1                                ; Log de hacks
WriteCashShopLog=1                            ; Log de compras en xshop
WriteChaosMixLog=1                            ; Log de combinaciones en Golbin
WriteSystemAntifloodLog=1                     ; Registro del sistema antiflood
WriteResetLog=1                               ; Log de reinicios
WriteTienTeLog=1                              ; Log de moneda
WriteKetNoiLog=1                              ; Log de conexiones
WriteThuMuaExcLog=1                           ; Log de compras de items excelentes
;==================================================
; Configuracion de AntiFlood
;==================================================
MaxIpConnection=20                            ; Numero maximo de cuentas por IP que pueden conectarse al Sub
IpLimitConnectionTime=1000
IpConnectionBlockedTime=0
MaxHwidConnection_AL0=5                       ; Maximo numero de ventanas de juego por dispositivo.
MaxHwidConnection_AL1=5                       ; Maximo numero de ventanas de juego por dispositivo.
MaxHwidConnection_AL2=5                       ; Maximo numero de ventanas de juego por dispositivo.
MaxHwidConnection_AL3=5                       ; Maximo numero de ventanas de juego por dispositivo.
;==================================================
; Usar top 1 ghrs
GioiHanResetTop1=1
;==================================================
;==================================================
; Quest para item duplicado
; No apagar
DeleteDupeItem=1
DisconectDupeItem=1
;==================================================
; Verificacion de CRC
;==================================================
GameGuardCRC=0x0 // 0 - Desactivado
VerifyCRC=0x0 // 0 - Desactivado
MainDLLCRC=0x0 // 0 - Desactivado
MainEXECRC=0x0 // 0 - Desactivado
;==================================================
; Tiempo de salida del juego y cambio de personaje
;==================================================
TimeOutGame=3
;==================================================
ActivarCambioGenero=1
;==================================================
; Configuracion de Antihack
;==================================================
CheckSpeedHack=1                            ; Sistema de deteccion de hacks de velocidad (0=Off/1=On).
CheckSpeedHackTolerance=250                 ; Tolerancia maxima para el uso incorrecto de velocidad de ataque.
CheckLatencyHack=1                          ; Sistema de deteccion de latencia (0=Off/1=On).
CheckLatencyHackTolerance=2000              ; Tolerancia maxima para el uso incorrecto de latencia.
CheckAutoPotionHack=1                       ; Sistema de deteccion de uso automatico de pociones (0=Off/1=On).
CheckAutoPotionHackTolerance=200            ; Tiempo de espera entre el uso de pociones (Milisegundos).
CheckAutoComboHack=1                        ; Sistema de deteccion de uso automatico de combinaciones (0=Off/1=On).
CheckAutoComboHackTolerance=100             ; Tiempo de espera entre el uso de combinaciones (Milisegundos).
;==================================================
; Configuracion de Hack de velocidad de habilidad (HackSkillCheck.txt)
; Tipos:
; 0: Solo mostrar mensaje en el log - Solo mostrar notificacion al personaje
; 1: Desconectar al usuario - Desconectar al personaje
; 2: Deshabilitar ataque - Deshabilitar ataque
; 3: Deshabilitar ataque por x milisegundos - Deshabilitar ataque por x milisegundos
; 4: Desconectar al usuario y bloquear el personaje
;==================================================
SpeedHackSkillEnable=1                      ; Activar:1 Desactivar:0
SpeedHackSkillType=3                        ; Tipo indicado arriba
SpeedHackSkillPenalty=1000                  ; Tiempo de deshabilitacion del ataque
SpeedHackSkillDialog=1                      ; Mostrar notificacion al jugador
SpeedHackSkillDebug=0                       ; Solo activar para pruebas
SpeedHackSkillBlockDays=30                  ; Bloqueo de dias para la habilidad
;==================================================
; Anti Xdame, configurado en Hack/HackXDameCheck.txt
; Se requiere antihack DragonCheat para que funcione
;XdameType =2 requiere salir del juego y volver a entrar.
;XdameType =3 deshabilita por xxx segundos
;==================================================
XdameEnable=1                               ; Activar/Desactivar
XdameType=3
XdameDetectionTime=3000                     ; Tiempo de bloqueo de la habilidad al detectar xdame
XdameNotice=1                               ; Notificar al jugador
XdameTest=0                                 ; Solo activar para pruebas
XdameBlock=3                                ; Tiempo de bloqueo al detectar xdame (tiempo en segundos)
;23-03-2022
; 1=Notificacion original
; 2=Notificacion a todos los Sub
ThongBaoOnline=2
;25-03-2022
; 1=Notificacion original
; 2=Notificacion a todos los Sub
ThongBaoReset=2
;==================================================
; Configuracion general
;==================================================
MaxItemOption=7                             ; Maxima opcion al mejorar items con la joya de la vida
PersonalCodeCheck=0                         ; Usar codigo personal de 7 digitos (1=Activar/0=Desactivar)
SetItemAcceptHarmonySwitch=1                ; Permitir items de set con opcion dorada
;==================================================
; Configuracion de monstruos
; MonsterLifeRate: Porcentaje de vida de los monstruos en todo el Sub
; MonsterDamageRate: Porcentaje de daño de los monstruos en todo el Sub
; MonsterDefenseRate: Porcentaje de defensa de los monstruos en todo el Sub
;==================================================
MonsterLifeRate=100
MonsterDamageRate=100
MonsterDefenseRate=100
;==================================================
; Activar/Desactivar barra de salud de jugadores y monstruos
;==================================================
UserHealthBarSwitch=1
MonsterHealthBarSwitch=1
;==================================================
; Configuracion de PK
;==================================================
NonPK=0                                     ; 0: Sub PK, 1: Sub sin PK
PKLimitCount=1000                           ; Limite de asesinatos PK
PKLimitFree=0
PKDownTime1=3600
PKDownTime2=7200
PKDisableShop=1
PKDisableTrade=1
;==================================================
; Configuracion de comercio (1=activar / 0=desactivar)
;==================================================
TradeSwitch=1                               ; Sistema de comercio (0=Desactivar / 1=Activar)
;==================================================
; Configuracion de tienda personal (1=activar / 0=desactivar)
;==================================================
PersonalShopSwitch=1                        ; Sistema de tienda personal (0=Desactivar / 1=Activar)
;==================================================
; Configuracion de duelos (1=activar / 0=desactivar)
;==================================================
DuelSwitch=1                                ; Activar/Desactivar duelos
DuelArenaAnnounceSwitch=1                   ; Activar/Desactivar notificaciones
DuelArenaDisableRestoreHP=1                 ; Desactivar recuperacion de HP en la arena de duelos
DuelArenaDisableRestoreSD=1                 ; Desactivar recuperacion de SD en la arena de duelos
DuelArenaDisableRespawn=0
;==================================================
; Configuracion de gremios
;==================================================
GuildCreateSwitch=1                         ; Crear gremio
GuildDeleteSwitch=1                         ; Eliminar gremio
GuildCreateMinLevel=100                     ; Nivel minimo para crear un gremio
GuildInsertMaxUser1=40                      ; Maximo de miembros
GuildInsertMaxUser2=60
GuildInsertMaxUser3=80
GuildInsertMaxUser4=100
GuildInsertMaxUser5=120
GuildAllianceMinLevel=100                   ; Nivel minimo para aliarse con otro gremio
;==================================================
; Configuracion de Mapa
;==================================================
LorenciaEnable=1                            ; Habilitar mapa
NoriaEnable=1
DeviasEnable=1
DungeonEnable=1
AtlansEnable=1
LostTowerEnable=1
ExileEnable=1
ExileV2Enable=1
IcarusEnable=1
TarkanEnable=1
ArenaEnable=1
AidaEnable=1
CrywolfEnable=1
KalimaEnable=1
KantruEnable=1
Kantru2Enable=1
Kantru3Enable=1
RaklionEnable=1
VulcanusEnable=1
SantaTownEnable=1
KarutanEnable=1
Karutan2Enable=1
IllusionTempleEnable=1
ValleyOfLorenEnable=1
LandOfTrialsEnable=1
AcheronEnable=1
ElbelandEnable=1
;==================================================
; Sistema de malla
;==================================================
UseAntiWallSwitch=1                         ; Sistema anti-malla (0=Desactivar / 1=Activar)
UseAntiWallRange=6                          ; Rango de deteccion
;==================================================
; Sistema anti-pared
;==================================================
UseAntiGateSwitch=1                         ; Sistema anti-pared (0=Desactivar / 1=Activar)
UseAntiGateRange=6                          ; Rango de deteccion
)ini";

constexpr char DEFAULT_ALLOWABLE_IP_LIST[] = "0\n"
"\"127.0.0.1\"\n"
"end\n";

// Definicion de las variables externas
char CommonConfigFilePath[MAX_PATH];
char AllowableIpListFilePath[MAX_PATH];
char CustomerName[32];
char CustomerHardwareId[36];
char ConnectServerAddress[16];
WORD ConnectServerPortUDP;
WORD JoinServerPort;
char JoinServerODBC[32];
char JoinServerUSER[32];
char JoinServerPASS[32];
char GlobalPassword[32];
BOOL CaseSensitive;
BOOL MD5Encryption;

// Construction/Destruction

bool CServerConfig::Init() {

	// Establece paths a los archivos
	sprintf_s(CommonConfigFilePath, "%s%s", WorkingPath, COMMON_CONFIG_FILE_NAME);
	// sprintf_s(AllowableIpListFilePath, "%s%s", WorkingPath, ALLOWABLE_IP_LIST_FILE_NAME);

	// Asegura que los archivos existan
	bool configResult = EnsureCommonConfigFileExists();
	bool alloweableIpListResult = EnsureServerListFileExists();

	return configResult && alloweableIpListResult;
}

bool CServerConfig::EnsureCommonConfigFileExists() {
	// Verificar si el archivo existe utilizando GetFileAttributes
	DWORD fileAttributes = GetFileAttributesA(CommonConfigFilePath);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		// El archivo no existe, lo creamos con valores por defecto
		{
			FileHandle file(CommonConfigFilePath, GENERIC_WRITE, CREATE_ALWAYS);
			if (file.getHandle() != INVALID_HANDLE_VALUE) {
				char buffer[2048];
				sprintf_s(buffer, sizeof(buffer), COMMON_DEFAULT_CONFIG);

				DWORD bytesWritten;
				if (file.write(buffer, static_cast<DWORD>(strlen(buffer)), bytesWritten)) {
					Log.ToFile(LogType::GENERAL, "[InitConfig] Archivo %s creado con valores por defecto.", COMMON_CONFIG_FILE_NAME);
				}
				else {
					Log.ToFile(LogType::GENERAL, "[InitConfig] Error al escribir en el archivo %s.", COMMON_CONFIG_FILE_NAME);
					return false;
				}
			}
			else {
				Log.ToFile(LogType::GENERAL, "[InitConfig] Error al crear el archivo %s.", COMMON_CONFIG_FILE_NAME);
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
	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_CUSTOMER_NAME, "", CustomerName, sizeof(CustomerName), CommonConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerName: %s", CustomerName);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerName desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	// Leer CustomerHardwareId directamente en CustomerHardwareId
	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_CUSTOMER_HARDWARE_ID, "", CustomerHardwareId, sizeof(CustomerHardwareId), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerHardwareId: %s", CustomerHardwareId);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerHardwareId desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_CONNECT_SERVER_ADDRESS, "", ConnectServerAddress, sizeof(ConnectServerAddress), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Direccion IP de ConnectServer: %s", ConnectServerAddress);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ConnectServerAddress desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ConnectServerPortUDP = static_cast<WORD>(GetPrivateProfileIntA(SECTION_JOIN_SERVER_INFO, KEY_CONNECT_SERVER_PORT_UDP, 0, ConfigFilePath));
	if (ConnectServerPortUDP > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Puerto UDP: %d", ConnectServerPortUDP);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ConnectServerPortUDP desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	JoinServerPort = static_cast<WORD>(GetPrivateProfileIntA(SECTION_JOIN_SERVER_INFO, KEY_JOIN_SERVER_PORT, 0, ConfigFilePath));
	if (JoinServerPort > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Puerto servidor de cuentas: %d", JoinServerPort);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar JoinServerPort desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_JOIN_SERVER_ODBC, "", JoinServerODBC, sizeof(JoinServerODBC), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Conectividad abierta de para la base datos: %s", JoinServerODBC);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar JoinServerODBC desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_JOIN_SERVER_USER, "", JoinServerUSER, sizeof(JoinServerUSER), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Usuario para la base datos: %s", JoinServerUSER);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar JoinServerUSER desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_JOIN_SERVER_PASS, "", JoinServerPASS, sizeof(JoinServerPASS), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Password para la base datos leida correctamente.");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar JoinServerPASS desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_JOIN_SERVER_INFO, KEY_GLOBAL_PASSWORD, "", GlobalPassword, sizeof(GlobalPassword), ConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Password global para la base datos leida correctamente.");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar GlobalPassword desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	CaseSensitive = static_cast<BYTE>(GetPrivateProfileIntA(SECTION_JOIN_SERVER_INFO, KEY_CASE_SENSITIVE, 0, ConfigFilePath));
	if (CaseSensitive == 0 || CaseSensitive == 1) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Sensible a mayusculas y minusculas: %s", (CaseSensitive == 1) ? "SI" : "NO");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CaseSensitive desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	MD5Encryption = static_cast<BYTE>(GetPrivateProfileIntA(SECTION_JOIN_SERVER_INFO, KEY_MD5_ENCRYPTIONS, 0, ConfigFilePath));
	if (MD5Encryption == 0 || MD5Encryption == 1) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Encriptacion MD5: %s", (MD5Encryption == 1) ? "SI" : "NO");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar MD5Encryption desde %s.", COMMON_CONFIG_FILE_NAME);
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
		Log.ToFile(LogType::GENERAL, "[InitConfig] Se utilizara %s existente.", ALLOWABLE_IP_LIST_FILE_NAME);
		return true;
	}
}

const char* CServerConfig::getIniPath() {
	return ConfigFilePath;
}

const char* CServerConfig::getServerListPath() {
	return AllowableIpListFilePath;
}
