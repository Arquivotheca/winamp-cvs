#include "nxonce.h"

void NXOnceInit(nx_once_t once)
{
	once->status = 0;
	pthread_mutexattr_t mtxattr;
  pthread_mutexattr_init(&mtxattr);
  pthread_mutexattr_settype(&mtxattr, PTHREAD_MUTEX_RECURSIVE_NP);           
	pthread_mutex_init(&once->mutex, &mtxattr);
	pthread_mutexattr_destroy(&mtxattr);
}