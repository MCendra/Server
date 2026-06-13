// ServerConfig.h
#pragma once

class CServerConfig {
public:
    bool Init(); // Obtiene el path del ejecutable y devuelve true o false

    const char* getIniPath();  // Cambiado a estatico
    const char* getServerListPath();  // Cambiado a estatico

private:
    bool EnsureConfigFileExists(); // Devuelve true o false segun el exito de la operacion
    bool EnsureServerListFileExists(); // Devuelve true o false segun el exito de la operacion
    bool LoadConfig();
};

// Instancia global del visualizador de servidor
extern CServerConfig gServerConfig;
