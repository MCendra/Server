// Header.h
#pragma once

// Incluir archivos de inclusion estandar del sistema y archivos especificos del proyecto
#include "TargetVer.h"

// Excluir material rara vez utilizado de encabezados de Windows
#define WIN32_LEAN_AND_MEAN             

// Constantes del proyecto
constexpr size_t MAX_LOADSTRING = 100;

// Define la version del servidor de conexion
constexpr auto DATASERVER_VERSION = "1.0.0";

constexpr auto SERVER_PART = "DATA SERVER";

constexpr size_t MAX_CHARACTER_NAME = 11;
constexpr size_t MAX_ACCOUNT_NAME = 11;
constexpr size_t MAX_GUILD_NAME = 9;
constexpr size_t GUILD_MARK_SIZE = 32;
constexpr size_t GUILD_NOTICE_SIZE = 60;
constexpr size_t GUILD_CHAT_SIZE = 60;
constexpr size_t MAX_MSG_CHAT_SIZE = 60;
constexpr size_t MAX_UNION_GUILDS = 100;

// Define el tipo de servidor de juegos
#define GAMESERVER_TYPE2 3

// Define el nombre del cliente basado en el tipo de servidor de juegos
#if (GAMESERVER_TYPE2 == 0)
#define DATASERVER_CLIENT "VIERJA&HIGHLEVEL"
#elif (GAMESERVER_TYPE2 == 1)
#define DATASERVER_CLIENT "HAI DAI SU"
#elif (GAMESERVER_TYPE2 == 2)
#define DATASERVER_CLIENT "REAL GAMING"
#else
#define DATASERVER_CLIENT "ARKANIA"
#endif

// Define la version de actualizacion del servidor de conexion si no se ha definido previamente
#ifndef DATASERVER_UPDATE
#define DATASERVER_UPDATE 803
#endif

#define MOCNAP				1
#define DANHHIEU_NEW		1
#define TULUYEN_NEW			1
#define CHIEN_TRUONG_CO		1
#define NEWBOSSGUILD		1
#define RANKINGGOC			0
#define RANKING_NEW			1
#define GHRS_TOP1_NEW		1
#define HIDE_VT				0 
#define CHIENTRUONGCO		1
#define SACHTHUOCTINH_NEW	1
#define CHONPHEDOILAP		1
#define B_HON_HOAN			1
#define EVENT_END_LESS		1
#define FLAG_SKIN			1
#ifndef BOT_STATUE
	#define BOT_STATUE 1
#endif

// Necesario por el uso de WSASocketA 
// Necesario por el uso de WSASocketA 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>		// Necesario para inet_ntop

// Archivos de encabezado de Windows
#include <Windows.h>    // Archivo de encabezado de la API de Windows

#include <Rpc.h>
#include <Psapi.h>

// Archivos de encabezado en tiempo de ejecución de C
#include <stdlib.h>     // Utilidades generales: memoria dinámica, random, exit, etc.
#include <malloc.h>     // Asignación de memoria: malloc, free, etc. (específico de algunas plataformas)
#include <memory.h>     // Manipulación de memoria (alias de <string.h> en algunas plataformas)
#include <tchar.h>      // Soporte para caracteres Unicode/ANSI (útil en aplicaciones que usan TCHAR)

// Archivos de encabezado en tiempo de ejecución de C++
#include <string>       // Manejo de cadenas en C++
#include <cstdint>		// Proporciona definiciones de tipos enteros con tamaños fijos

#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <process.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
#include <assert.h>
#include <fstream>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"Psapi.lib")

// Define un tipo de dato específico (QWORD es un entero sin signo de 64 bits)
// using BYTE = uint8_t;
// using WORD = uint16_t;
// No redefinir DWORD aquí, usar el de windows.h
// using DWORD = uint32_t;
using QWORD = uint64_t;

// Definición del path de trabajo
extern char* WorkingPath;

// Definicion de archivos de configuracion
extern char ConfigFilePath[MAX_PATH];
extern char AllowableIpListFilePath[MAX_PATH];
extern char BadSyntaxFilePath[MAX_PATH];

// Declaraciones de variables de configuracion
extern char CustomerName[32];
extern char CustomerHardwareId[36];
extern WORD DataServerPort;
extern char DataServerODBC[32];
extern char DataServerUSER[32];
extern char DataServerPASS[32];
extern BOOL AdvancedLog;
extern WORD RSTimeCTC;

//==============================================================
// Common Packet Offsets
//==============================================================

constexpr std::size_t PACKET_TYPE_OFFSET = 0;

//==============================================================
// C1 / C3 Header
//==============================================================

constexpr std::size_t C1_PACKET_SIZE_OFFSET = 1;
constexpr std::size_t C1_PACKET_HEAD_OFFSET = 2;
constexpr std::size_t C1_PACKET_DATA_OFFSET = 3;

//==============================================================
// C2 / C4 Header
//==============================================================

constexpr std::size_t C2_PACKET_SIZEH_OFFSET = 1;
constexpr std::size_t C2_PACKET_SIZEL_OFFSET = 2;
constexpr std::size_t C2_PACKET_HEAD_OFFSET = 3;
constexpr std::size_t C2_PACKET_DATA_OFFSET = 4;

//==============================================================
// Packet Types
//==============================================================

constexpr BYTE PACKET_C1 = 0xC1;
constexpr BYTE PACKET_C2 = 0xC2;
constexpr BYTE PACKET_C3 = 0xC3;
constexpr BYTE PACKET_C4 = 0xC4;

//==============================================================
// DataServer Main Heads
//==============================================================

constexpr BYTE DS_HEAD_SERVER_INFO = 0x00;
constexpr BYTE DS_HEAD_EXDB_PROTOCOL = 0xE0;
constexpr BYTE DS_HEAD_CONNECT_SERVER = 0xE1;

//==============================================================
// ExDB Protocol Heads (0xE0)
//==============================================================

constexpr BYTE EXDB_HEAD_CHAR_CLOSE = 0x02;

constexpr BYTE EXDB_HEAD_GUILD_CREATE = 0x30;
constexpr BYTE EXDB_HEAD_GUILD_DESTROY = 0x31;
constexpr BYTE EXDB_HEAD_GUILD_MEMBER_ADD = 0x32;
constexpr BYTE EXDB_HEAD_GUILD_MEMBER_DEL = 0x33;
constexpr BYTE EXDB_HEAD_GUILD_MEMBER_INFO = 0x35;
constexpr BYTE EXDB_HEAD_GUILD_MASTER_LIST = 0x36;
constexpr BYTE EXDB_HEAD_GUILD_SCORE_UPDATE = 0x37;
constexpr BYTE EXDB_HEAD_GUILD_NOTICE = 0x38;

#if (NEWBOSSGUILD == 1)
constexpr BYTE EXDB_HEAD_GUILD_SCORE_UPDATE_EX = 0x39;
#endif

constexpr BYTE EXDB_HEAD_GUILD_CHAT = 0x50;
constexpr BYTE EXDB_HEAD_UNION_CHAT = 0x51;

constexpr BYTE EXDB_HEAD_ASSIGN_STATUS = 0xE1;
constexpr BYTE EXDB_HEAD_ASSIGN_TYPE = 0xE2;

constexpr BYTE EXDB_HEAD_RELATIONSHIP_JOIN = 0xE5;
constexpr BYTE EXDB_HEAD_RELATIONSHIP_BREAK = 0xE6;
constexpr BYTE EXDB_HEAD_RELATIONSHIP_LIST = 0xE7;
constexpr BYTE EXDB_HEAD_RELATIONSHIP_NOTIFICATION = 0xE8;
constexpr BYTE EXDB_HEAD_UNION_LIST = 0xE9;

constexpr BYTE EXDB_HEAD_KICKOUT_UNION_MEMBER = 0xEB;

//==============================================================
// ExDB SubHeads
//==============================================================

constexpr BYTE EXDB_SUB_HEAD_KICKOUT_UNION_MEMBER = 0x01;

//==============================================================
// ConnectServer Protocol Heads (0xE1)
//==============================================================

constexpr BYTE CS_HEAD_FRIEND_LIST = 0x60;
constexpr BYTE CS_HEAD_WAIT_FRIEND_LIST = 0x61;
constexpr BYTE CS_HEAD_FRIEND_STATE = 0x62;
constexpr BYTE CS_HEAD_FRIEND_ADD = 0x63;
constexpr BYTE CS_HEAD_WAIT_FRIEND_ADD = 0x64;
constexpr BYTE CS_HEAD_FRIEND_DELETE = 0x65;
constexpr BYTE CS_HEAD_CHATROOM_CREATE = 0x66;

constexpr BYTE CS_HEAD_MEMO_SEND = 0x70;
constexpr BYTE CS_HEAD_MEMO_LIST = 0x71;
constexpr BYTE CS_HEAD_MEMO_READ = 0x72;
constexpr BYTE CS_HEAD_MEMO_DELETE = 0x73;
constexpr BYTE CS_HEAD_FRIEND_INVITATION = 0x74;