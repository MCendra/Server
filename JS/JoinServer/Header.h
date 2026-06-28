// Header.h
#pragma once

// Incluir archivos de inclusión estándar del sistema y archivos específicos del proyecto
#include "TargetVer.h"

// Excluir material rara vez utilizado de encabezados de Windows
#define WIN32_LEAN_AND_MEAN   

// Constantes del proyecto
constexpr size_t MAX_LOADSTRING = 100;

// Define la versión del servidor de conexión
constexpr auto JOINSERVER_VERSION = "1.0.0";

constexpr auto SERVER_PART = "JOIN SERVER";

// Define el tipo de servidor de juegos
#define GAMESERVER_TYPE2 3

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
#ifndef JOINSERVER_UPDATE
#define JOINSERVER_UPDATE 803
#endif

// Necesario por el uso de WSASocketA 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>		// Necesario para inet_ntop

// Archivos de encabezado de Windows
#include <Windows.h>    // Archivo de encabezado de la API de Windows

#include <Rpc.h>
#include <Psapi.h>

// Archivos de encabezado en tiempo de ejecución de C
// #include <stdlib.h>     // Utilidades generales: memoria dinámica, random, exit, etc.
// #include <malloc.h>     // Asignación de memoria: malloc, free, etc. (específico de algunas plataformas)
// #include <memory.h>     // Manipulación de memoria (alias de <string.h> en algunas plataformas)
// #include <tchar.h>      // Soporte para caracteres Unicode/ANSI (útil en aplicaciones que usan TCHAR)

// Archivos de encabezado en tiempo de ejecución de C++
#include <string>       // Manejo de cadenas en C++
#include <cstdint>		// Proporciona definiciones de tipos enteros con tamaños fijos

//#include <commctrl.h>
#include <sql.h>
#include <sqltypes.h>
#include <sqlext.h>
#include <process.h>
#include <iostream>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>
#include <math.h>
#include <map>
#include <vector>
#include <queue>
#include <algorithm>
#include <assert.h>
#include <fstream>
//#include <dbghelp.h>

#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"Rpcrt4.lib")
//#pragma comment(lib,"dbghelp.lib")
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

// Declaraciones de variables de configuracion
extern char CustomerName[32];
extern char CustomerHardwareId[36];
extern char ConnectServerAddress[16];
extern WORD ConnectServerPortUDP;
extern WORD JoinServerPort;
extern char JoinServerODBC[32];
extern char JoinServerUSER[32];
extern char JoinServerPASS[32];
extern char GlobalPassword[32];
extern BOOL CaseSensitive;
extern BOOL MD5Encryption;
