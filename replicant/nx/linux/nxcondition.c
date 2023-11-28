#include "nxcondition.h"
#include "foundation/error.h"
#include <sys/time.h>
#include <time.h>

int NXConditionInitialize(nx_condition_t condition)
{
	if (condition == 0)
		return NErr_NullPointer;

	pthread_mutexattr_t wake_mutex_attributes;
	pthread_mutexattr_init(&wake_mutex_attributes);
	pthread_mutexattr_settype(&wake_mutex_attributes, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&condition->mutex, &wake_mutex_attributes);
	pthread_mutexattr_destroy(&wake_mutex_attributes);

	//pthread_condattr_t wake_condition_attributes;
	//pthread_condattr_init(&wake_condition_attributes);
	pthread_cond_init(&condition->condition, NULL);
	//pthread_condattr_destroy(&wake_condition_attributes);

	return NErr_Success;
}

int NXConditionDestroy(nx_condition_t condition)
{
	if (condition == 0)
		return NErr_NullPointer;

	pthread_cond_destroy(&condition->condition);
	pthread_mutex_destroy(&condition->mutex);
	return NErr_Success;
}

int NXConditionLock(nx_condition_t condition)
{
	if (condition == 0)
		return NErr_NullPointer;

	pthread_mutex_lock(&condition->mutex);
	return NErr_Success;
}

int NXConditionUnlock(nx_condition_t condition)
{
if (condition == 0)
		return NErr_NullPointer;

	pthread_mutex_unlock(&condition->mutex);
	return NErr_Success;
}

int NXConditionWait(nx_condition_t condition)
{
	if (condition == 0)
		return NErr_NullPointer;

	pthread_cond_wait(&condition->condition, &condition->mutex);
	return NErr_Success;
}

int NXConditionTimedWait(nx_condition_t condition, unsigned int milliseconds)
{
		          struct timeval now;
              struct timespec timeout;


	if (condition == 0)
		return NErr_NullPointer;

	            gettimeofday(&now, 0);
              timeout.tv_sec = now.tv_sec + milliseconds/1000;
              timeout.tv_nsec = now.tv_usec * 1000 + (milliseconds%1000)*1000000;
              pthread_cond_timedwait(&condition->condition, &condition->mutex, &timeout);

							return NErr_Success;
}

int NXConditionSignal(nx_condition_t condition)
{
		if (condition == 0)
		return NErr_NullPointer;

	pthread_cond_signal(&condition->condition);
	return NErr_Success;
}
