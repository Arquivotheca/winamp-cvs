#pragma once
#define USE_LOCKFREE_FIFO

#include "nu/lfmpscq.h"

#include "nu/LockFreeLIFO.h"
#include <semaphore.h>
#include "foundation/align.h"

typedef queue_node_t fifo_node_t;
struct threadloop_node_t : public fifo_node_t
{
	void (*func)(void *param1, void *param2, double real_value);

	void *param1;
	void *param2;
	double real_value;
	uint8_t cache_line_padding[8]; // this makes this an even 32 bytes
};

class ThreadLoop
{
public:
	ThreadLoop();
	~ThreadLoop();
	threadloop_node_t *GetAPC(); // returns a node for you to fill out
	void Schedule(threadloop_node_t *apc);
	void Run();
	void Try();
	void Step();
	void Step(unsigned int milliseconds);
	void uStep(unsigned int microseconds);
	void Kill();
private:
	void RefillCache();
	void ProcessMessages();

	sem_t procedure_notification;
	volatile size_t kill_switch;
	mpscq_t procedure_queue;

	/* Memory cache to be able to run APCs without having the memory manager lock 
	we'll allocate 100 at a time (#defined by PROCEDURE_CACHE_SEED)
	and allocate new ones only if the cache is empty (which unfortunately will lock)
	cache_bases holds the pointers we've allocated (to free on destruction of this object)
	and procedure_cache holds the individual pointers */
};