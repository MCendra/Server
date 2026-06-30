// Util.h
#pragma once
#include <vector>

// Clase Util para funciones utilitarias
class CUtil
{
public:
	// Obtiene el path del ejecutable y lo convierte a formato extendido
	void GetExecutablePath();
	// Mensaje de error para terminar el proceso
	void ErrorMessageBox(const char* message, ...);
	// Verifica la sintaxis de un texto
	bool CheckTextSyntax(const char* text, int size);
};

extern CUtil gUtil;
