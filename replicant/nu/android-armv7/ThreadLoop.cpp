#include "ThreadLoop.h"
#include "foundation/atomics.h"
#include <string.h>
#include <assert.h>
#include <android/log.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>

static lifo_t procedure_cache = {0,0};
static lifo_t cache_bases= {0,0};

#define PROCEDURE_CACHE_SEED 64

ThreadLoop::ThreadLoop()
{
	assert(sizeof(threadloop_node_t) == 32); 

	mpscq_init(&procedure_queue);
	sem_init(&procedure_notification, 0, 0);

	kill_switch=0;
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[ThreadLoop] ARMv7 implementation");
}

ThreadLoop::~ThreadLoop()
{
	sem_destroy(&procedure_notification);
}

void ThreadLoop::RefillCache()
{
	threadloop_node_t *cache_seed = (threadloop_node_t *)memalign(32, PROCEDURE_CACHE_SEED*sizeof(threadloop_node_t));

	if (cache_seed)
	{
		int i=PROCEDURE_CACHE_SEED;
		while (--i)
		{
			lifo_push(&procedure_cache, (queue_node_t *)&cache_seed[i]);
		}
		lifo_push(&cache_bases, (queue_node_t *)cache_seed);
	}
	else
	{
		sched_yield(); // yield and hope that someone else pops something off soon		
	}

}

void ThreadLoop::Run()
{
	#if 0
	__android_log_print(ANDROID_LOG_INFO, "libreplicant", "[ThreadLoop] threadid=%u (0x%x)", gettid(), gettid());
#endif
	for (;;)
	{
		sem_wait(&procedure_notification);
		if (kill_switch)
		{
			return;
		}

		for (;;)
		{
			threadloop_node_t *apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);

			if (apc == (threadloop_node_t *)1) /* special return value that indicates a busy list */
			{
				sched_yield(); // yield so that the thread that got pre-empted during push can finish
			}
			else
			{
				if (apc)
				{
					apc->func(apc->param1, apc->param2, apc->real_value);
					lifo_push(&procedure_cache, (queue_node_t *)apc);
				}
				else
				{
					break;	
				}				
			}
		}
	}
}

void ThreadLoop::Step(unsigned int milliseconds)
{
	unsigned int seconds = milliseconds/1000;
	milliseconds -= seconds*1000;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += seconds;
	ts.tv_nsec += milliseconds*1000000;

	if (sem_timedwait(&procedure_notification, &ts) == 0)
	{
		if (kill_switch)
		{
			return;
		}

		for (;;)
		{
			threadloop_node_t *apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);

			if (apc == (threadloop_node_t *)1) /* special return value that indicates a busy list */
			{
				sched_yield(); // yield so that the thread that got pre-empted during push can finish
			}
			else
			{
				if (apc)
				{
					apc->func(apc->param1, apc->param2, apc->real_value);
					lifo_push(&procedure_cache, (queue_node_t *)apc);
				}
				else
				{
					break;	
				}				
			}
		}
	}
}

void ThreadLoop::uStep(unsigned int microseconds)
{
	unsigned int seconds = microseconds/1000000;
	microseconds -= seconds*1000000;
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += seconds;
	ts.tv_nsec += microseconds*1000;

	if (sem_timedwait(&procedure_notification, &ts) == 0)
	{
		if (kill_switch)
		{
			return;
		}

		for (;;)
		{
			threadloop_node_t *apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);

			if (apc == (threadloop_node_t *)1) /* special return value that indicates a busy list */
			{
				sched_yield(); // yield so that the thread that got pre-empted during push can finish
			}
			else
			{
				if (apc)
				{
					apc->func(apc->param1, apc->param2, apc->real_value);
					lifo_push(&procedure_cache, (queue_node_t *)apc);
				}
				else
				{
					break;	
				}				
			}
		}
	}
}

void ThreadLoop::Step()
{
	if (sem_wait(&procedure_notification) == 0)
	{
		if (kill_switch)
		{
			return;
		}

		for (;;)
		{
			threadloop_node_t *apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);

			if (apc == (threadloop_node_t *)1) /* special return value that indicates a busy list */
			{
				sched_yield(); // yield so that the thread that got pre-empted during push can finish
			}
			else
			{
				if (apc)
				{
					apc->func(apc->param1, apc->param2, apc->real_value);
					lifo_push(&procedure_cache, (queue_node_t *)apc);
				}
				else
				{
					break;	
				}				
			}
		}
	}
}

void ThreadLoop::Try()
{
	if (sem_trywait(&procedure_notification) == 0)
	{
		if (kill_switch)
		{
			return;
		}

		for (;;)
		{
			threadloop_node_t *apc = (threadloop_node_t *)mpscq_pop(&procedure_queue);

			if (apc == (threadloop_node_t *)1) /* special return value that indicates a busy list */
			{
				sched_yield(); // yield so that the thread that got pre-empted during push can finish
			}
			else
			{
				if (apc)
				{
					apc->func(apc->param1, apc->param2, apc->real_value);
					lifo_push(&procedure_cache, (queue_node_t *)apc);
				}
				else
				{
					break;	
				}				
			}
		}
	}
}

threadloop_node_t *ThreadLoop::GetAPC()
{
	threadloop_node_t *apc = 0;
	do 
	{
		apc = (threadloop_node_t *)lifo_pop(&procedure_cache);
		if (!apc)
			RefillCache();
	} while (!apc);
	return apc;
}

void ThreadLoop::Schedule(threadloop_node_t *apc)
{
	if (mpscq_push(&procedure_queue, apc) == 0)
		sem_post(&procedure_notification);
}

void ThreadLoop::Kill()
{
	nx_atomic_write(1,&kill_switch);
	sem_post(&procedure_notification);
}
