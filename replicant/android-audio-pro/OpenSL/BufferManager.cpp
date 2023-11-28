#include "BufferManager.h"
#include "foundation/error.h"
#include <stdlib.h>

int BufferManager::BufferHeader::Create(size_t bytes, BufferHeader **out_buffer)
{
	size_t total_size = sizeof(BufferHeader) + bytes;
	if (total_size < sizeof(BufferHeader)) // check for overflow
		return NErr_IntegerOverflow;

	BufferHeader *buffer = (BufferHeader *)malloc(total_size);
	if (!buffer)
		return NErr_OutOfMemory;

	buffer->bytes_used=0;
	buffer->bytes = bytes;
	*out_buffer = buffer;
	return NErr_Success;
}

BufferManager::BufferHeader *BufferManager::BufferHeader::GetHeader(void *data)
{
	if (!data)
		return 0;

	return (BufferHeader *)((uint8_t *)data - sizeof(BufferHeader));
}

void *BufferManager::BufferHeader::GetData()
{
	return this+1;
}

BufferManager::BufferManager()
{
	bytes_per_buffer=0;
	buffer_count=0;
}

BufferManager::~BufferManager()
{
	FreeBuffers();
}

bool BufferManager::Empty(LockFreeRingBuffer &buffer_pool)
{
	return buffer_pool.empty();
}

size_t BufferManager::BuffersInPool(LockFreeRingBuffer &buffer_pool)
{
	return buffer_pool.size()/sizeof(BufferHeader *);
}

BufferManager::BufferHeader *BufferManager::GetBuffer(LockFreeRingBuffer &buffer_pool)
{
	BufferHeader *buffer;
	size_t bytes_read =	buffer_pool.read(&buffer, sizeof(BufferHeader *));
	if (bytes_read)
		return buffer;
	else
		return 0;
}

BufferManager::BufferHeader *BufferManager::PeekBuffer(LockFreeRingBuffer &buffer_pool)
{
	BufferHeader *buffer;
	size_t bytes_read =	buffer_pool.peek(&buffer, sizeof(BufferHeader *));
	if (bytes_read)
		return buffer;
	else
		return 0;
}

void BufferManager::Advance(LockFreeRingBuffer &buffer_pool)
{
	buffer_pool.advance(sizeof(BufferHeader *));
}

void BufferManager::PutBuffer(LockFreeRingBuffer &buffer_pool, BufferManager::BufferHeader *buffer)
{
	buffer_pool.write(&buffer, sizeof(buffer));
}

void BufferManager::OnClear()
{
	BufferHeader *buffer;

	while (buffer = GetBuffer(in_use))
		available.write(&buffer, sizeof(buffer));

	while (buffer = GetBuffer(filled))
		available.write(&buffer, sizeof(buffer));
}

void BufferManager::OnRestart()
{
	BufferHeader *buffer;

	while (buffer = GetBuffer(in_use))
		available.write(&buffer, sizeof(buffer));
}

int BufferManager::Initialize(size_t new_bytes_per_buffer, size_t new_buffer_count)
{
	available.expand(new_buffer_count*sizeof(BufferHeader));
	filled.expand(new_buffer_count*sizeof(BufferHeader));
	in_use.expand(new_buffer_count*sizeof(BufferHeader));

	if (new_bytes_per_buffer != bytes_per_buffer)
	{
		FreeBuffers();
		bytes_per_buffer=new_bytes_per_buffer;
		buffer_count=0;
	}
	else
	{
		OnClear(); 
	}

	while (new_buffer_count < buffer_count)
	{
		free(GetBuffer(available));
		buffer_count--;
	}

	while (new_buffer_count > buffer_count)
	{
		BufferHeader *buffer;
		int ret = BufferHeader::Create(bytes_per_buffer, &buffer);
		if (ret != NErr_Success)
			return ret;

		PutBuffer(available, buffer);
		buffer_count++;
	}
	return NErr_Success;
}

void BufferManager::FreeBuffers()
{
	BufferHeader *buffer;

	while (buffer = GetBuffer(in_use))
		free(buffer);

	while (buffer = GetBuffer(filled))
		free(buffer);

	while (buffer = GetBuffer(available))
		free(buffer);
}