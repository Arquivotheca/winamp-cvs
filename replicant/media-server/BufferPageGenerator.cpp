#include "BufferPageGenerator.h"
#include "foundation/error.h"

BufferPageGenerator::BufferPageGenerator()
{
	buffer=0;
	length=0;
}

BufferPageGenerator::~BufferPageGenerator()
{
	free(buffer);
}


int BufferPageGenerator::Initialize(const void *buffer, size_t length)
{
	this->buffer = (uint8_t *)malloc(length);
	if (!this->buffer)
		return NErr_OutOfMemory;
	this->length = length;
	memcpy(this->buffer, buffer, length);
	return NErr_Success;
}

size_t BufferPageGenerator::PageGenerator_GetData(void *buf, size_t size)
{
	if (size > length)
		size = length;
	memcpy(buf, buffer, size);
	length -= size;
	buffer += size;
	return size;
}
