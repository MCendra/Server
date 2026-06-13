// BufferHandle.cpp
#include "BufferHandle.h"

BufferHandle::BufferHandle(size_t size)
    : buffer(std::make_unique<char[]>(size)), bufferSize(size) {}

BufferHandle::BufferHandle(BufferHandle&& other) noexcept
    : buffer(std::move(other.buffer)), bufferSize(other.bufferSize) {}

BufferHandle& BufferHandle::operator=(BufferHandle&& other) noexcept {
    if (this != &other) {
        buffer = std::move(other.buffer);
        bufferSize = other.bufferSize;
    }
    return *this;
}

const std::unique_ptr<char[]>& BufferHandle::getBuffer() const {
    return buffer;
}

size_t BufferHandle::getSize() const {
    return bufferSize;
}
