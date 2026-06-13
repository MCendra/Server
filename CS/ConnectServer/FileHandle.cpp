// FileHandle.cpp
#include "FileHandle.h"

// Constructor que abre un archivo en el modo especificado.
FileHandle::FileHandle(const char* path, DWORD accessMode, DWORD creationDisposition) {
    handle = CreateFileA(path, accessMode, 0, nullptr, creationDisposition, FILE_ATTRIBUTE_NORMAL, nullptr);
}

// Destructor que cierra el handle si es valido.
FileHandle::~FileHandle() {
    if (handle != INVALID_HANDLE_VALUE) {
        CloseHandle(handle);
    }
}

// Constructor de movimiento que transfiere la propiedad del handle de `other` a esta instancia.
FileHandle::FileHandle(FileHandle&& other) noexcept : handle(other.handle) {
    other.handle = INVALID_HANDLE_VALUE;
}

// Operador de asignacion por movimiento que transfiere la propiedad del handle de `other` a esta instancia.
FileHandle& FileHandle::operator=(FileHandle&& other) noexcept {
    if (this != &other) {
        if (handle != INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
        }
        handle = other.handle;
        other.handle = INVALID_HANDLE_VALUE;
    }
    return *this;
}

// Funcion que devuelve el handle almacenado.
HANDLE FileHandle::getHandle() const {
    return handle;
}

// Metodo para leer datos del archivo.
bool FileHandle::read(void* buffer, DWORD numberOfBytesToRead, DWORD& bytesRead) const {
    if (handle == INVALID_HANDLE_VALUE) {
        return false; // Retorna false si el handle no es valido.
    }
    return ReadFile(handle, buffer, numberOfBytesToRead, &bytesRead, nullptr);
}

// Metodo para escribir datos en el archivo.
bool FileHandle::write(const void* buffer, DWORD numberOfBytesToWrite, DWORD& bytesWritten) const {
    if (handle == INVALID_HANDLE_VALUE) {
        return false; // Retorna false si el handle no es valido.
    }
    return WriteFile(handle, buffer, numberOfBytesToWrite, &bytesWritten, nullptr);
}
