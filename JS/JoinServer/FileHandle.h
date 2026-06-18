// FileHandle.h
#pragma once
#include <windows.h>

class FileHandle {
public:
	// Constructor que abre un archivo en el modo especificado.
	FileHandle(const char* path, DWORD accessMode = GENERIC_READ, DWORD creationDisposition = OPEN_EXISTING);

	// Destructor que cierra el handle si es valido.
	~FileHandle();

	// Constructor de movimiento.
	FileHandle(FileHandle&& other) noexcept;

	// Operador de asignacion por movimiento.
	FileHandle& operator=(FileHandle&& other) noexcept;

	// Funcion que devuelve el handle almacenado.
	HANDLE getHandle() const;

	// Metodo para leer datos del archivo.
	bool read(void* buffer, DWORD numberOfBytesToRead, DWORD& bytesRead) const;

	// Metodo para escribir datos en el archivo.
	bool write(const void* buffer, DWORD numberOfBytesToWrite, DWORD& bytesWritten) const;

private:
	HANDLE handle{ INVALID_HANDLE_VALUE };

	// Prohibir copias para evitar manejar dos handles para el mismo recurso.
	FileHandle(const FileHandle&) = delete;
	FileHandle& operator=(const FileHandle&) = delete;
};
