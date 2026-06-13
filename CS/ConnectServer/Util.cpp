// Util.cpp
#include "Header.h"
#include "Util.h"

// Instancia global
CUtil gUtil;

char* WorkingPath = nullptr;                 // Path del ejecutable declarado en Header.h

constexpr char ERROR_TITLE[] = "Error";

void CUtil::GetExecutablePath()
{
    DWORD bufferSize = MAX_PATH;
	std::vector<char> buffer;
	buffer.resize(bufferSize);

	while (true)
	{
		DWORD result = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
		if (result == 0)
		{
			// No se pudo obtener la ruta
			if (WorkingPath)
			{
				free(WorkingPath);
				WorkingPath = nullptr;
			}
			return;
		}

		// Si el buffer fue insuficiente, GetModuleFileName devuelve igual al tamaño del buffer
		if (result >= buffer.size())
		{
			// Aumentar y volver a intentar
			buffer.resize(buffer.size() * 2);
			continue;
		}

		// Encontrar la ultima barra invertida para mantener solo el directorio
		char* lastSlash = strrchr(buffer.data(), '\\');
		if (lastSlash)
		{
			*(lastSlash + 1) = '\0';
		}

		// Liberar anterior si existe y duplicar
		if (WorkingPath)
		{
			free(WorkingPath);
		}
		WorkingPath = _strdup(buffer.data());
		return;
	}
}

void CUtil::ErrorMessageBox(const char* message, ...) {
    char buff[256];

    // Inicializa y formatea el mensaje en el buffer
    va_list arg;
    va_start(arg, message);

    // Formatea el mensaje usando los argumentos variables
    vsprintf_s(buff, sizeof(buff), message, arg);

    // Termina el manejo de argumentos variables
    va_end(arg);

    // Muestra el mensaje en un MessageBox
    MessageBoxA(0, buff, ERROR_TITLE, MB_OK | MB_ICONERROR);

    // Termina el proceso
    ExitProcess(0);
}

