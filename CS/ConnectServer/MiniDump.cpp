// MiniDump.cpp
#include "MiniDump.h"

LPTOP_LEVEL_EXCEPTION_FILTER PreviousExceptionFilter = nullptr;

// Funcion de filtro para el manejo de excepciones y generacion de minidumps
LONG WINAPI DumpExceptionFilter(EXCEPTION_POINTERS* info)
{
    char path[MAX_PATH] = { 0 };
    SYSTEMTIME systemTime;

    GetLocalTime(&systemTime);

    CreateDirectoryA("dumps", nullptr); // Crear carpeta si no existe

    // Utilizando StringCchPrintf para evitar desbordamientos de buffer
    StringCchPrintfA(path, MAX_PATH, "dumps\\%d-%d-%d_%dh%dm%ds.dmp",
        systemTime.wYear, systemTime.wMonth, systemTime.wDay,
        systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

    // Creacion del archivo de minidump
    HANDLE file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (file != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION mdei;
        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = info;
        mdei.ClientPointers = false;

        if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file,
            (MINIDUMP_TYPE)(MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory),
            &mdei, nullptr, nullptr))
        {
            CloseHandle(file);
            return EXCEPTION_EXECUTE_HANDLER;
        }

        // Cerrar el archivo si no se pudo escribir el minidump
        CloseHandle(file);
    }

    // Si el archivo no se creo o fallo la escritura del minidump
    return EXCEPTION_CONTINUE_SEARCH;
}

// Inicia el manejo de excepciones con minidump
void CMiniDump::Start()
{
    SetErrorMode(SEM_FAILCRITICALERRORS);
    PreviousExceptionFilter = SetUnhandledExceptionFilter(DumpExceptionFilter);
}

// Restaura el filtro de excepciones previo
void CMiniDump::Clean()
{
    if (PreviousExceptionFilter != nullptr)
    {
        SetUnhandledExceptionFilter(PreviousExceptionFilter);
		PreviousExceptionFilter = nullptr;
    }
}
