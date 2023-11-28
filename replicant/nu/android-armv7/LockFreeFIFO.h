#pragma once
#include "queue_node.h"

#ifdef __cplusplus
extern "C" {
#endif

	
struct FIFO_POINTER
{
	queue_node_t *fifo_node_t;
	size_t count;
};

struct fifo_t
{
	queue_node_t *head;
	queue_node_t *tail;
};

void fifo_init(fifo_t *fifo);
void fifo_push(fifo_t *fifo, queue_node_t *cl);
queue_node_t *fifo_pop(fifo_t *fifo);

#ifdef __cplusplus
}
#endif