#pragma once

// Incluir archivos de inclusion estandar del sistema y archivos especificos del proyecto
#include "TargetVer.h"

// Excluir material rara vez utilizado de encabezados de Windows
#define WIN32_LEAN_AND_MEAN             

// Constantes del proyecto
constexpr size_t MAX_LOADSTRING = 100;

// Define la version del servidor de conexion
constexpr auto DATASERVER_VERSION = "1.0.0";

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