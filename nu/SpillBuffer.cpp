#include "SpillBuffer.h"
#include <stdlib.h>
#include <string.h>
#ifndef min
#define min(a,b) ((a<b)?(a):(b))
#endif

SpillBuffer::SpillBuffer()
{
	spillBufferUsed=0;
	spillBufferSize=0;
	spillBuffer=0;
}

SpillBuffer::~SpillBuffer()
{
	free(spillBuffer);
}

void SpillBuffer::reset()
{
	free(spillBuffer);
	spillBuffer=0;
	spillBufferUsed=0;
	spillBufferSize=0;
}

bool SpillBuffer::reserve(size_t bytes)
{
	spillBufferSize=bytes;
	spillBuffer = (char *)realloc(spillBuffer, spillBufferSize*2);
	if (!spillBuffer) return false;
	clear();
	return true;
}

void SpillBuffer::clear()
{
	spillBufferUsed=0;
}

size_t SpillBuffer::write(const void *buffer, size_t bytes)
{
	size_t avail = spillBufferSize - spillBufferUsed;
	bytes = min(avail, bytes);
	memcpy(spillBuffer + spillBufferUsed, buffer, bytes);
	spillBufferUsed+=bytes;
	return bytes;
}

bool SpillBuffer::get(void **buffer, size_t *len)
{
	*buffer = spillBuffer;
	*len = spillBufferUsed;
	spillBufferUsed = 0;
	return true;
}

bool SpillBuffer::full() const
{
	return spillBufferUsed == spillBufferSize;
}

bool SpillBuffer::empty() const
{
	return spillBufferUsed == 0;
}

void SpillBuffer::remove(size_t len)
{
	if (len > spillBufferUsed)
		len=spillBufferUsed;

	if (len) 
	{
		memmove(spillBuffer, spillBuffer + len, spillBufferUsed - len);
	}
}

size_t SpillBuffer::remaining() const
{
	return spillBufferSize - spillBufferUsed;
}

size_t SpillBuffer::length() const
{
	return spillBufferSize;
}
