// ServerConfig.h
#pragma once

class CServerConfig {
public:
    bool Init(); // Obtiene el path del ejecutable y devuelve true o false

    const char* getIniPath();  // Cambiado a estático
    const char* getServerListPath();  // Cambiado a estático

private:
    bool EnsureConfigFileExists(); // Devuelve true o false según el éxito de la operación
    bool EnsureServerListFileExists(); // Devuelve true o false según el éxito de la operación
    bool LoadConfig();
};

// Instancia global del visualizador de servidor
extern CServerConfig gServerConfig;