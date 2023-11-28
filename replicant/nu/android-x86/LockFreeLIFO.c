#include "LockFreeLIFO.h"
#include "foundation/atomics.h"

/* Mac OS X x86 implementation */

void lifo_init(lifo_t *lifo)
{
	lifo->head = 0;
	lifo->aba = 0;
}

queue_node_t *lifo_malloc(size_t bytes)
{
	return malloc(bytes);
}

void lifo_free(queue_node_t *ptr)
{
    free(ptr);
}