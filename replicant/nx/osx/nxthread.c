#include "nxthread.h"
#include "foundation/error.h"
#include <sys/resource.h>


int NXThreadCreate(nx_thread_t *thread, nx_thread_func_t thread_function, nx_thread_parameter_t parameter)
{
  pthread_create(thread, 0, thread_function, parameter);
	return NErr_Success;
}

int NXThreadJoin(nx_thread_t thread, nx_thread_return_t *retval)
{
	pthread_join(thread, retval);
	return NErr_Success;
}

int NXThreadCurrentSetPriority(int priority)
{
	/* TODO */
	return NErr_Success;
}