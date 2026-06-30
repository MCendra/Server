// BufferHandle.h
#pragma once
#include <memory> // Para std::unique_ptr

class BufferHandle
{
public:
	explicit BufferHandle(size_t size);

	BufferHandle(BufferHandle&& other) noexcept;
	BufferHandle& operator=(BufferHandle&& other) noexcept;

	char* data();
	const char* data() const;

	size_t getSize() const;

private:
	std::unique_ptr<char[]> buffer;
	size_t bufferSize;

	BufferHandle(const BufferHandle&) = delete;
	BufferHandle& operator=(const BufferHandle&) = delete;
};
