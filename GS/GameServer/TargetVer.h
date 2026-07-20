#pragma once

// Especifica que el target minimo es Windows 10
#define _WIN32_WINNT _WIN32_WINNT_WIN10

// La inclusion de SDKDDKVer.h define la plataforma Windows mas avanzada disponible.
// Si desea compilar la aplicacion para una plataforma Windows anterior, incluya WinSDKVer.h y
// establezca la macro _WIN32_WINNT en la plataforma que desea admitir antes de incluir SDKDDKVer.h.
#include <SDKDDKVer.h>