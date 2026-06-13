// BufferHandle.h
#pragma once
#include <memory> // Para std::unique_ptr

class BufferHandle {
public:
    // Constructor que asigna un buffer de tamaño dado.
    BufferHandle(size_t size);
    BufferHandle(BufferHandle&& other) noexcept;
    BufferHandle& operator=(BufferHandle&& other) noexcept;

    const std::unique_ptr<char[]>& getBuffer() const;
    size_t getSize() const;  // Nueva funcion para obtener el tamaño del buffer

private:
    std::unique_ptr<char[]> buffer;
    size_t bufferSize;  // Almacena el tamaño del buffer
};
