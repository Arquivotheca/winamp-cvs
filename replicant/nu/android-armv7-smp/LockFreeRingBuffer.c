#include "LockFreeRingBuffer.h"
#include <stdlib.h>

lockfree_ringbuffer_t *ringbuffer_malloc(size_t bytes)
{
	lockfree_ringbuffer_t *buffer = malloc(16 + bytes);
	buffer->size = bytes;
	buffer->available = bytes;
	buffer->write_head = 0;
	buffer->read_head = 0;
	return buffer;
}

void ringbuffer_free(lockfree_ringbuffer_t *buffer)
{
	free(buffer);
}