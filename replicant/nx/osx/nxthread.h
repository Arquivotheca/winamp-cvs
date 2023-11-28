#pragma once
#include "nx/nxapi.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef pthread_t nx_thread_t;
	typedef void *nx_thread_return_t;
	typedef void *nx_thread_parameter_t;
#define NXTHREADCALL
	typedef nx_thread_return_t (NXTHREADCALL *nx_thread_func_t)(nx_thread_parameter_t parameter);

	// TODO: add parameters for things like stack size
NX_API int NXThreadCreate(nx_thread_t *thread, nx_thread_func_t thread_function, nx_thread_parameter_t parameter);
NX_API int NXThreadJoin(nx_thread_t t, nx_thread_return_t *retval);

enum
{
	NX_THREAD_PRIORITY_PLAYBACK = -16,
	NX_THREAD_PRIORITY_AUDIO_OUTPUT = -19,
};

// sets priority of current thread
NX_API int NXThreadCurrentSetPriority(int priority);
#ifdef __cplusplus
}
#endif