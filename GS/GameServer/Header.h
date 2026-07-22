// Header.h
#pragma once

// Incluir archivos de inclusion estandar del sistema y archivos especificos del proyecto
#include "TargetVer.h"

// Excluir material rara vez utilizado de encabezados de Windows
#define WIN32_LEAN_AND_MEAN             

// Constantes del proyecto
constexpr size_t MAX_LOADSTRING = 100;

// Define la versión del servidor de conexión
constexpr auto GAMESERVER_VERSION = "1.0.0";

constexpr auto SERVER_PART = "GAME SERVER";

// Define el nombre del cliente basado en el tipo de servidor de juegos
#if (GAMESERVER_TYPE2 == 0)
#define JOINSERVER_CLIENT "VIERJA&HIGHLEVEL"
#elif (GAMESERVER_TYPE2 == 1)
#define JOINSERVER_CLIENT "HAI DAI SU"
#elif (GAMESERVER_TYPE2 == 2)
#define JOINSERVER_CLIENT "REAL GAMING"
#else
#define JOINSERVER_CLIENT "ARKANIA"
#endif

// Define la versión de actualización del servidor de conexión si no se ha definido previamente
#ifndef GAMESERVER_UPDATE
#define GAMESERVER_UPDATE 803
#endif

// Define el tipo de servidor de juegos
#define GAMESERVER_TYPE2 3

#define	MOVE_ITEM			true
#define USE_FAKE_ONLINE		true
#define OFFLINE_MODE_NEW	true
#ifndef OANTUTI
#define OANTUTI				true
#endif
#define DANHHIEU_NEW		true
#define TULUYEN_NEW			true
#define TEAMVSTEAM			true
#define HONCHIENCLASS		true
#define NGAN_HANG_NGOC		true
#define MOCNAP				true
#define CHONPHEDOILAP		true
#define B_HON_HOAN			true
#define EVENT_END_LESS		true
#define SACHTHUOCTINH_NEW	true
#define FLAG_SKIN			true
#define ALLBOTSSTRUC		true
#define BOT_BUFFER			true
#define BOT_STORE			true
#define VONGQUAY_NEW		true
#define MOVETAP				true
#define NEWBOSSGUILD		false

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

// Definicion del path de trabajo
extern char* WorkingPath;

// Declaraciones de variables de configuracion
extern char CustomerName[32];
extern char CustomerHardwareId[36];
extern char ServerName[32];
extern int ServerCode;
extern int ServerLock;
extern int ServerPort;
extern char ServerMutex[32];
extern char ServerVersion[6];
extern char ServerSerial[17];
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

constexpr BYTE SET_NUMBERHB(DWORD x) {
	return static_cast<BYTE>(x >> 8);
}

constexpr BYTE SET_NUMBERLB(DWORD x) {
	return static_cast<BYTE>(x & 0xFF);
}

constexpr WORD SET_NUMBERHW(DWORD x) {
	return static_cast<WORD>(x >> 16);
}

constexpr WORD SET_NUMBERLW(DWORD x) {
	return static_cast<WORD>(x & 0xFFFF);
}

constexpr DWORD SET_NUMBERHDW(QWORD x) {
	return static_cast<DWORD>(x >> 32);
}

constexpr DWORD SET_NUMBERLDW(QWORD x) {
	return static_cast<DWORD>(x & 0xFFFFFFFF);
}

constexpr WORD MAKE_NUMBERW(BYTE x, BYTE y) {
	return static_cast<WORD>((static_cast<WORD>(y) & 0xFF) | (static_cast<WORD>(x) << 8));
}

constexpr DWORD MAKE_NUMBERDW(WORD x, WORD y) {
	return static_cast<DWORD>((static_cast<DWORD>(y) & 0xFFFF) | (static_cast<DWORD>(x) << 16));
}

constexpr QWORD MAKE_NUMBERQW(DWORD x, DWORD y) {
	return (static_cast<QWORD>(y) & 0xFFFFFFFF) | (static_cast<QWORD>(x) << 32);
}