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

constexpr char SECTION_GAME_SERVER_INFO[] = "GameServerInfo";

constexpr char KEY_CUSTOMER_NAME[] = "CustomerName";
constexpr char KEY_CUSTOMER_HARDWARE_ID[] = "CustomerHardwareId";
constexpr char KEY_SERVER_NAME[] = "ServerName";
constexpr char KEY_SERVER_CODE[] = "ServerCode";
constexpr char KEY_SERVER_LOCK[] = "ServerLock";
constexpr char KEY_SERVER_PORT[] = "ServerPort";
constexpr char KEY_SERVER_VERSION[] = "ServerVersion";
constexpr char KEY_SERVER_SERIAL[] = "ServerSerial";
constexpr char KEY_SERVER_MAX_USER_NUMBER[] = "ServerMaxUserNumber";
constexpr char KEY_SERVER_MIN_LEVEL[] = "ServerMinLevel";
constexpr char KEY_SERVER_MAX_LEVEL[] = "ServerMaxLevel";
constexpr char KEY_SERVER_MIN_RESET[] = "ServerMinReset";
constexpr char KEY_SERVER_MAX_RESET[] = "ServerMaxReset";
constexpr char KEY_SERVER_MIN_MASTER_RESET[] = "ServerMinMasterReset";
constexpr char KEY_SERVER_MAX_MASTER_RESET[] = "ServerMaxMasterReset";
constexpr char KEY_SERVER_GUARD_MESSSAGE[] = "GuardMessage";

constexpr char KEY_CONNECTSERVER_ADDRESS[] = "ConnectServerAddress";
constexpr char KEY_CONNECTSERVER_PORT[] = "ConnectServerPort";
constexpr char KEY_JOINSERVER_ADDRESS[] = "JoinServerAddress";
constexpr char KEY_JOINSERVER_PORT[] = "JoinServerPort";
constexpr char KEY_DATASERVER_ADDRESS[] = "DataServerAddress";
constexpr char KEY_DATASERVER_PORT[] = "DataServerPort";

constexpr char STRING_MUTEX[] = "WZ_MU_GS_MUTEX_%d";

constexpr char COMMON_DEFAULT_CONFIG[] = R"ini([GameServerInfo]
;==================================================
; No modificar si no se tiene conocimiento
;==================================================
CustomerName=
CustomerHardwareId=
;==================================================
; Configuracion del servidor
;==================================================
ServerName=Sala-1
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
ConnectServerAddress=127.0.0.1
ConnectServerPort=55559
JoinServerAddress=127.0.0.1
JoinServerPort=55972
DataServerAddress=127.0.0.1
DataServerPort=55960
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
char ServerName[32];
int ServerCode;
int ServerLock;
int ServerPort;
char ServerMutex[32];
char ServerVersion[6];
char ServerSerial[17];
int ServerMaxUserNumber;
int ServerMinLevel;
int ServerMaxLevel;
int ServerMinReset;
int ServerMaxReset;
int ServerMinMasterReset;
int ServerMaxMasterReset;
#if(GUARDMESSAGE)
char GuardMessage[64];
#endif
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

	if (GetPrivateProfileStringA(SECTION_GAME_SERVER_INFO, KEY_CUSTOMER_NAME, "", CustomerName, sizeof(CustomerName), CommonConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerName: %s", CustomerName);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerName desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_GAME_SERVER_INFO, KEY_CUSTOMER_HARDWARE_ID, "", CustomerHardwareId, sizeof(CustomerHardwareId), CommonConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] CustomerHardwareId: %s", CustomerHardwareId);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar CustomerHardwareId desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_GAME_SERVER_INFO, KEY_SERVER_NAME, "", ServerName, sizeof(ServerName), CommonConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Nombre del GameServer: %s", ServerName);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerName desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerCode = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_CODE, 0, CommonConfigFilePath));
	if (ServerCode > 0) {
		sprintf_s(ServerMutex, STRING_MUTEX, ServerCode);

		Log.ToDisp(LOG_BLACK, "[InitConfig] Codigo del GameServer: %d", ServerCode);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerCode desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerLock = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_LOCK, 0, CommonConfigFilePath));
	if (ServerLock == 0 || ServerLock == 1) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] GameServer bloqueado para ingresos: %s", (ServerLock == 1) ? "SI" : "NO");
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerLock desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerPort = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_PORT, 0, CommonConfigFilePath));
	if (ServerPort > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig]  Puerto TCP: %d", ServerCode);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerPort desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_GAME_SERVER_INFO, KEY_SERVER_VERSION, "", ServerVersion, sizeof(ServerVersion), CommonConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Version GameServer: %s", ServerVersion);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerVersion desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	if (GetPrivateProfileStringA(SECTION_GAME_SERVER_INFO, KEY_SERVER_SERIAL, "", ServerSerial, sizeof(ServerSerial), CommonConfigFilePath) > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Serial: %s", ServerSerial);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerSerial desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerMaxUserNumber = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_MAX_USER_NUMBER, 0, CommonConfigFilePath));
	if (ServerMaxUserNumber > 0) {
		Log.ToDisp(LOG_BLACK, "[InitConfig] Cantidad de usuarios maxima: %d", ServerMaxUserNumber);
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerMaxUserNumber desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerMinLevel = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_MIN_LEVEL, 0, CommonConfigFilePath));
	if (ServerMinLevel >= 0) {
		if (ServerLock == 0) {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Nivel minimo para entrar al servidor: ninguno");
		}
		else {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Nivel minimo para entrar al servidor: %d", ServerMinLevel);
		}
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerMinLevel desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerMaxLevel = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_MAX_LEVEL, 0, CommonConfigFilePath));
	if (ServerMaxLevel >= 0) {
		if (ServerMaxLevel == 0) {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Nivel maximo para entrar al servidor: ninguno");
		}
		else {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Nivel maximo para entrar al servidor: %d", ServerMaxLevel);
		}
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerMaxLevel desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerMinReset = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_MIN_RESET, 0, CommonConfigFilePath));
	if (ServerMinReset >= 0) {
		if (ServerMinReset == 0) {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Resets minimos para entrar al servidor: ninguno");
		}
		else {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Resets minimos para entrar al servidor: %d", ServerMinReset);
		}
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerMinReset desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerMaxReset = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_MAX_RESET, 0, CommonConfigFilePath));
	if (ServerMaxReset >= 0) {
		if (ServerMaxReset == 0) {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Resets maximos para entrar al servidor: ninguno");
		}
		else {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Resets maximos para entrar al servidor: %d", ServerMaxReset);
		}
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerMaxReset desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerMinMasterReset = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_MIN_MASTER_RESET, 0, CommonConfigFilePath));
	if (ServerMinMasterReset >= 0) {
		if (ServerMinMasterReset == 0) {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Master reset minimo: ninguno");
		}
		else {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Master reset minimo: %d", ServerMinMasterReset);
		}
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerMinMasterReset desde %s.", COMMON_CONFIG_FILE_NAME);
		allValuesLoaded = false;
	}

	ServerMaxMasterReset = static_cast<int>(GetPrivateProfileIntA(SECTION_GAME_SERVER_INFO, KEY_SERVER_MAX_MASTER_RESET, 0, CommonConfigFilePath));
	if (ServerMaxMasterReset >= 0) {
		if (ServerMaxMasterReset == 0) {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Master reset maximo: ninguno");
		}
		else {
			Log.ToDisp(LOG_BLACK, "[InitConfig] Master reset maximo: %d", ServerMaxMasterReset);
		}
	}
	else {
		Log.ToDisp(LOG_RED, "[InitConfig] Error al cargar ServerMaxMasterReset desde %s.", COMMON_CONFIG_FILE_NAME);
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
	return CommonConfigFilePath;
}

const char* CServerConfig::getServerListPath() {
	return AllowableIpListFilePath;
}
