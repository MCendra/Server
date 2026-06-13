#pragma once

// Especifica que el target mínimo es Windows 10
#define _WIN32_WINNT _WIN32_WINNT_WIN10

// La inclusión de SDKDDKVer.h define la plataforma Windows más avanzada disponible.
// Si desea compilar la aplicación para una plataforma Windows anterior, incluya WinSDKVer.h y
// establezca la macro _WIN32_WINNT en la plataforma que desea admitir antes de incluir SDKDDKVer.h.
#include <SDKDDKVer.h>
