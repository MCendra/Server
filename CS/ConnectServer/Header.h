#pragma once

// Incluir archivos de inclusion estandar del sistema y archivos especificos del proyecto
#include "TargetVer.h"

// Excluir material rara vez utilizado de encabezados de Windows
#define WIN32_LEAN_AND_MEAN             

// Constantes del proyecto
constexpr size_t MAX_LOADSTRING = 100;

// Define la version del servidor de conexion
constexpr auto CONNECTSERVER_VERSION = "1.0.0";

constexpr auto SERVER_PART = "CONNECT SERVER";

// Define el tipo de servidor de juegos
#define GAMESERVER_TYPE2 3

// Define el nombre del cliente basado en el tipo de servidor de juegos
#if (GAMESERVER_TYPE2 == 0)
#define CONNECTSERVER_CLIENT "VIERJA&HIGHLEVEL"
#elif (GAMESERVER_TYPE2 == 1)
#define CONNECTSERVER_CLIENT "HAI DAI SU"
#elif (GAMESERVER_TYPE2 == 2)
#define CONNECTSERVER_CLIENT "REAL GAMING"
#else
#define CONNECTSERVER_CLIENT "ARKANIA"
#endif

// Define la version de actualizacion del servidor de conexion si no se ha definido previamente
#ifndef CONNECTSERVER_UPDATE
#define CONNECTSERVER_UPDATE 803
#endif

// Necesario por el uso de WSASocketA 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>		// Necesario para inet_ntop

// Archivos de encabezado de Windows
#include <windows.h>    // Archivo de encabezado de la API de Windows

#include <Rpc.h>
#include <Psapi.h>

// Archivos de encabezado en tiempo de ejecucion de C
// #include <stdlib.h>     // Utilidades generales: memoria dinamica, random, exit, etc.
// #include <malloc.h>     // Asignacion de memoria: malloc, free, etc. (especifico de algunas plataformas)
// #include <memory.h>     // Manipulacion de memoria (alias de <string.h> en algunas plataformas)
// #include <tchar.h>      // Soporte para caracteres Unicode/ANSI (util en aplicaciones que usan TCHAR)

// Archivos de encabezado en tiempo de ejecucion de C++
#include <string>       // Manejo de cadenas en C++
#include <cstdint>		// Proporciona definiciones de tipos enteros con tamaños fijos

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Rpcrt4.lib")
#pragma comment(lib,"Psapi.lib")

// Define un tipo de dato especifico (QWORD es un entero sin signo de 64 bits)
// using BYTE = uint8_t;
// using WORD = uint16_t;
// No redefinir DWORD aqui, usar el de windows.h
// using DWORD = uint32_t;
using QWORD = uint64_t;

// Definicion del path de trabajo
extern char* WorkingPath;

// Definicion de archivos de configuracion
extern char ConfigFilePath[MAX_PATH];
extern char ServerListFilePath[MAX_PATH];

// Declaraciones de variables de configuracion
extern char CustomerName[32];
extern char CustomerHardwareId[36];
extern long MaxIpConnection;
extern WORD ConnectServerPortTCP;
extern WORD ConnectServerPortUDP;

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
