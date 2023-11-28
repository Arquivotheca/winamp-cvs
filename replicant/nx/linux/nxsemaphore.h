#pragma once
#include <semaphore.h>
#include "nx/nxapi.h"

#ifdef __cplusplus
extern "C" {
#endif
	typedef sem_t *nx_semaphore_t;

	NX_API int NXSemaphoreCreate(nx_semaphore_t *sem);
	NX_API int NXSemaphoreRelease(nx_semaphore_t sem);
	NX_API int NXSemaphoreWait(nx_semaphore_t sem);
	NX_API void NXSemaphoreClose(nx_semaphore_t sem);
#ifdef __cplusplus
}
#endif