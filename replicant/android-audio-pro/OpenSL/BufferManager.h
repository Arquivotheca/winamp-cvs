#pragma once
#include "nu/LockFreeRingBuffer.h"

class BufferManager
{
public:
	BufferManager();
	~BufferManager();

	void OnClear(); // moves everything from filled and in_use back to available
	void OnRestart(); // moves everything from in_use back to available
	LockFreeRingBuffer available, filled, in_use;

	struct BufferHeader
	{
		size_t bytes_used;
		size_t bytes;
		static int Create(size_t bytes, BufferHeader **out_header);
		static BufferHeader *GetHeader(void *data);
		void *GetData();
	};
	static BufferHeader *GetBuffer(LockFreeRingBuffer &buffer_pool);
	static BufferHeader *PeekBuffer(LockFreeRingBuffer &buffer_pool);
	static void Advance(LockFreeRingBuffer &buffer_pool);
	static void PutBuffer(LockFreeRingBuffer &buffer_pool, BufferHeader *buffer);
	static bool Empty(LockFreeRingBuffer &buffer_pool);
	static size_t BuffersInPool(LockFreeRingBuffer &buffer_pool);
	int Initialize(size_t bytes_per_buffer, size_t buffer_count);
private:
	void FreeBuffers();
	size_t bytes_per_buffer;
	size_t buffer_count;
};