#include "ThreadLoop.h"
#include "foundation/atomics.h"
#include <stdlib.h>
#include <sched.h>
#include <android/log.h>
#include <assert.h>
#include <sched.h>
#include <time.h>

/* ARMv5 implementation */
ThreadLoop::ThreadLoop()
{
#ifdef USE_LOCKFREE_FIFO
	mpscq_init(&procedure_queue);
#endif
	sem_init(&procedure_notification, 0, 0);
	kill_switch=0;
}

void ThreadLoop::Run()
{
	for (;;)
	{
		sem_wait(&procedure_notification);
		if (kill_switch)
		{
			return;
		}
#ifdef USE_LOCKFREE_FIFO
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
					free(apc);			
				}
				else
				{
					break;	
				}				
			}
		}
#else
		queue_guard.Lock();
		threadloop_node_t *apc = procedure_queue.back();
		if (apc)
			procedure_queue.pop_back();
		queue_guard.Unlock();


		if (apc)
		{
			apc->func(apc->param1, apc->param2, apc->real_value);
			free(apc);		
		}
		else
		{
		}
#endif
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

#ifdef USE_LOCKFREE_FIFO
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
					free(apc);			
				}
				else
				{
					break;	
				}				
			}
		}
#else
		queue_guard.Lock();
		threadloop_node_t *apc = procedure_queue.back();
		if (apc)
			procedure_queue.pop_back();
		queue_guard.Unlock();


		if (apc)
		{
			apc->func(apc->param1, apc->param2, apc->real_value);
			free(apc);		
		}
		else
		{
		}
#endif
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

#ifdef USE_LOCKFREE_FIFO
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
					free(apc);			
				}
				else
				{
					break;	
				}				
			}
		}
#else
		queue_guard.Lock();
		threadloop_node_t *apc = procedure_queue.back();
		if (apc)
			procedure_queue.pop_back();
		queue_guard.Unlock();


		if (apc)
		{
			apc->func(apc->param1, apc->param2, apc->real_value);
			free(apc);		
		}
		else
		{
		}
#endif
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

#ifdef USE_LOCKFREE_FIFO
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
					free(apc);			
				}
				else
				{
					break;	
				}				
			}
		}
#else
		queue_guard.Lock();
		threadloop_node_t *apc = procedure_queue.back();
		if (apc)
			procedure_queue.pop_back();
		queue_guard.Unlock();


		if (apc)
		{
			apc->func(apc->param1, apc->param2, apc->real_value);
			free(apc);		
		}
		else
		{
		}
#endif
	}
}

threadloop_node_t *ThreadLoop::GetAPC()
{
	return (threadloop_node_t *)malloc(sizeof(threadloop_node_t));
}

void ThreadLoop::Schedule(threadloop_node_t *apc)
{
#ifdef USE_LOCKFREE_FIFO
	if (mpscq_push(&procedure_queue, apc) == 0)
		sem_post(&procedure_notification);
#else
	queue_guard.Lock();
	procedure_queue.push_front(apc);
	queue_guard.Unlock();
	sem_post(&procedure_notification);
#endif
}

void ThreadLoop::Kill()
{
	nx_atomic_write(1,&kill_switch);
	sem_post(&procedure_notification);
}
