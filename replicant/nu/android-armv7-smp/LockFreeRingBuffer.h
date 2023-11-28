#pragma once

#include "foundation/types.h"
#include "foundation/align.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef struct lockfree_ringbuffer_t
	{
		size_t available;
		size_t size;
		size_t write_head;
		size_t read_head;
		uint8_t data[4];
	}  lockfree_ringbuffer_t;
	
	/* use this to allocate an object that will go into this */
	lockfree_ringbuffer_t *ringbuffer_malloc(size_t bytes); 
	void ringbuffer_free(lockfree_ringbuffer_t *ptr);
	
	size_t ringbuffer_write(lockfree_ringbuffer_t *ring_buffer, const void *bytes, size_t length);
	size_t ringbuffer_read(lockfree_ringbuffer_t *ring_buffer, void *bytes, size_t length);

	
#ifdef __cplusplus
}
#endif
