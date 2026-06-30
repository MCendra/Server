// BufferHandle.cpp
#include "BufferHandle.h"

BufferHandle::BufferHandle(size_t size)
	: buffer(std::make_unique<char[]>(size)),
	bufferSize(size)
{}

BufferHandle::BufferHandle(BufferHandle&& other) noexcept
	: buffer(std::move(other.buffer)),
	bufferSize(other.bufferSize)
{
	other.bufferSize = 0;
}

BufferHandle& BufferHandle::operator=(BufferHandle&& other) noexcept
{
	if (this != &other)
	{
		buffer = std::move(other.buffer);
		bufferSize = other.bufferSize;

		other.bufferSize = 0;
	}

	return *this;
}

char* BufferHandle::data()
{
	return buffer.get();
}

const char* BufferHandle::data() const
{
	return buffer.get();
}

size_t BufferHandle::getSize() const
{
	return bufferSize;
}
