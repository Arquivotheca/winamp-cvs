#include "LockFreeLIFO.h"
#include "foundation/atomics.h"
#include <stdlib.h>
#include <assert.h>
void lifo_init(lifo_t *lifo)
{
	assert(sizeof(lifo_t) == 8);
	lifo->head = 0;
	lifo->aba = 0;
}

queue_node_t *lifo_malloc(size_t bytes)
{
	return malloc(bytes); // malloc's default is 8 byte alignment which is more than what we need
}

void lifo_free(queue_node_t *ptr)
{
	free(ptr);
}
