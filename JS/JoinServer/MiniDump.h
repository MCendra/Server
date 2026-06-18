// MiniDump.h
#pragma once
#include <windows.h>
#include <strsafe.h>    // Funciones seguras de manipulacion de cadenas
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

class CMiniDump
{
public:
	static void Start();
	static void Clean();
};
