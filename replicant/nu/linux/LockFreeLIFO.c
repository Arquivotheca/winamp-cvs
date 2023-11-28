#include "LockFreeLIFO.h"
#include "foundation/atomics.h"
#include <stdlib.h>

void lifo_init(lifo_t *lifo)
{
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
#if 0
void lifo_push(lifo_t *lifo, queue_node_t *cl)
{
	queue_node_t *new_head = cl;
	queue_node_t *old_head = 0;
	do
	{
		old_head = (queue_node_t *)lifo->Next;
		new_head->Next = old_head;
	} while (!nx_atomic_cmpxchg_pointer(old_head, new_head, (void * volatile *)&lifo->Next));
}
#endif
#if 0
queue_node_t *lifo_pop(lifo_t *lifo)
{
	lifo_t *old_head, *new_head;
	do
	{
		old_head = lifo->Next;
		if (!old_head)
			return 0;

		new_head = old_head->Next;
	} while (!nx_atomic_cmpxchg_pointer(old_head, new_head, (void * volatile *)&lifo->Next));
	return old_head;
}
#endif
