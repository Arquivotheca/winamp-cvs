#pragma once
#include "nx/nxapi.h"
#include "foundation/types.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct nx_once_s 
	{
		volatile int32_t status;
		pthread_mutex_t mutex;
	} nx_once_value_t, *nx_once_t;
	
#define NX_ONCE_INITIALIZE {0, PTHREAD_MUTEX_INITIALIZER}
#define NX_ONCE_API

	NX_API void NXOnce(nx_once_t once, int (NX_ONCE_API *init_fn)(nx_once_t, void *, void **), void *); 
	NX_API void NXOnceInit(nx_once_t once);
	
#ifdef __cplusplus
}
#endif