#include "ThreadLoop.h"
#include "foundation/atomics.h"
#include <stdlib.h>
#include <sched.h>
#include <assert.h>
#include <sched.h>
#include <time.h>
#include <sys/time.h>

/* ARMv5 implementation */
ThreadLoop::ThreadLoop()
{
#ifdef USE_LOCKFREE_FIFO
	mpscq_init(&procedure_queue);
#endif
	pthread_mutexattr_t procedure_mutex_attributes;
	pthread_mutexattr_init(&procedure_mutex_attributes);
	pthread_mutexattr_settype(&procedure_mutex_attributes, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&procedure_mutex, &procedure_mutex_attributes);
	pthread_mutexattr_destroy(&procedure_mutex_attributes);
	
	pthread_cond_init(&procedure_condition, NULL);
	
	kill_switch=0;
}

void ThreadLoop::Run()
{
	for (;;)
	{
		pthread_mutex_lock(&procedure_mutex);
		pthread_cond_wait(&procedure_condition, &procedure_mutex);
		pthread_mutex_unlock(&procedure_mutex);
		
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
	
	struct timespec ts;
	
	ts.tv_sec = milliseconds / 1000;
	ts.tv_nsec = (milliseconds % 1000) * 1000000;

	pthread_mutex_lock(&procedure_mutex);
	int wait_result = pthread_cond_timedwait_relative_np(&procedure_condition, &procedure_mutex, &ts);
	pthread_mutex_unlock(&procedure_mutex);
	
	if (wait_result == 0)
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
	pthread_mutex_lock(&procedure_mutex);
	int wait_result = pthread_cond_wait(&procedure_condition, &procedure_mutex);
	pthread_mutex_unlock(&procedure_mutex);
	
	if (wait_result == 0)
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
	struct timespec ts;
	
	ts.tv_sec = 0;
	ts.tv_nsec = 2;
	
	pthread_mutex_lock(&procedure_mutex);
	int wait_result = pthread_cond_timedwait_relative_np(&procedure_condition, &procedure_mutex, &ts);
	pthread_mutex_unlock(&procedure_mutex);

	if (wait_result == 0)
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
		pthread_cond_signal(&procedure_condition);
#else
	queue_guard.Lock();
	procedure_queue.push_front(apc);
	queue_guard.Unlock();
	pthread_cond_signal_thread_np(<#pthread_cond_t *#>, <#pthread_t#>)
#endif
}

void ThreadLoop::Kill()
{
	nx_atomic_write(1,&kill_switch);
	pthread_cond_signal(&procedure_condition);
}
