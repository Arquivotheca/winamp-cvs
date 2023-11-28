#include "MessageLoop.h"
#include <assert.h>
#include <sched.h>
#include <time.h>


/* Android ARMv7 implementation */

lifo_t nu::MessageLoop::message_cache = {0,};
lifo_t nu::MessageLoop::cache_bases= {0,};

#define MESSAAGE_CACHE_SEED 64

typedef uint8_t message_data_t[64]; // ensure all messages are this size

nu::MessageLoop::MessageLoop()
{
	mpscq_init(&message_queue);
	sem_init(&message_notification, 0, 0);
}

nu::MessageLoop::~MessageLoop()
{
	sem_destroy(&message_notification);
}

void nu::MessageLoop::RefillCache()
{
	message_data_t *cache_seed = (message_data_t *)memalign(64, MESSAAGE_CACHE_SEED*sizeof(message_data_t));
	
	if (cache_seed)
	{
		int i=MESSAAGE_CACHE_SEED;
		while (--i)
		{
			lifo_push(&message_cache, (queue_node_t *)&cache_seed[i]);
		}
		lifo_push(&cache_bases, (queue_node_t *)cache_seed);
	}
	else
	{
		sched_yield(); // yield and hope that someone else pops something off soon		
	}
}

nu::message_node_t *nu::MessageLoop::AllocateMessage()
{
	message_node_t *apc = 0;

	do 
	{
		apc = (message_node_t *)lifo_pop(&message_cache);
		if (!apc)
			RefillCache();
	} while (!apc);
	return apc;
}

void nu::MessageLoop::PostMessage(nu::message_node_t *message)
{
	if (mpscq_push(&message_queue, message) == 0)
		sem_post(&message_notification);
}

void nu::MessageLoop::FreeMessage(nu::message_node_t *message)
{
	lifo_push(&message_cache, message);
}

nu::message_node_t *nu::MessageLoop::GetMessage()
{
	message_node_t *message = PeekMessage();
	if (message)
		return message;

	for (;;)
	{
		sem_wait(&message_notification);
		message = PeekMessage();
		if (message)
			return message;
	}
	return 0;
}

nu::message_node_t *nu::MessageLoop::PeekMessage()
{
	for (;;) // loop because we need to handle 'busy' from the queue
	{
		message_node_t *message = (message_node_t *)mpscq_pop(&message_queue);
		if (message == (message_node_t *)1) /* special return value that indicates a busy list */
		{
			// benski> although it's tempting to return 0 here, doing so will mess up the Event logic
			sched_yield(); // yield so that the thread that got pre-empted during push can finish
		}
		else
		{
			if (message)
			{
				return message;
			}
			else
			{
				return 0;
			}
		}
	}				
}

nu::message_node_t *nu::MessageLoop::PeekMessage(unsigned int milliseconds)
{
	message_node_t *message = PeekMessage();
	if (message)
		return message;

	unsigned int seconds = milliseconds/1000;
	milliseconds -= seconds*1000;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += seconds;
	ts.tv_nsec += milliseconds*1000000;

	if (sem_timedwait(&message_notification, &ts) == 0)
	{
		message = PeekMessage();
		if (message)
			return message;
	}
	return 0;
}
