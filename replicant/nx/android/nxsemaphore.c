#include "nxsemaphore.h"
#include "foundation/error.h"
#include <stdlib.h>

int NXSemaphoreCreate(nx_semaphore_t *sem)
{
	*sem = malloc(sizeof(sem_t));
	sem_init(*sem, 0, 0);
	return NErr_Success;
}

int NXSemaphoreRelease(nx_semaphore_t sem)
{
	sem_post(sem);
	return NErr_Success;
}

int NXSemaphoreWait(nx_semaphore_t sem)
{
	sem_wait(sem);
	return NErr_Success;
}

void NXSemaphoreClose(nx_semaphore_t sem)
{
	if (sem)
	{
		sem_close(sem);
		free(sem);
	}
}