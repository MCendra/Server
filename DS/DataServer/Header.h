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
extern int AdvancedLog;
extern WORD RSTimeCTC;