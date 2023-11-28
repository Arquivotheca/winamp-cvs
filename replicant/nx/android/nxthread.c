#include "nxthread.h"
#include "foundation/error.h"
#include <sys/resource.h>
#include <sys/syscall.h>


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
	//pid_t thread_id = syscall(SYS_gettid);
	pid_t thread_id = gettid();
	int new_priority, old_priority = getpriority(PRIO_PROCESS, thread_id);
	setpriority(PRIO_PROCESS, thread_id, priority);
	new_priority = getpriority(PRIO_PROCESS, thread_id);

	return NErr_Success;
}

// Return current priority value
int NXThreadCurrentGetPriority(void)
{
	//pid_t thread_id = syscall(SYS_gettid);
	pid_t thread_id = gettid();
	int current_priority = getpriority(PRIO_PROCESS, thread_id);
	
	return current_priority;
}
