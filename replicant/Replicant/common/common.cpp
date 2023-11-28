#include "api.h"
#include "common.h"
#include "nswasabi/singleton.h"
#include "FileLock.h"

static FileLock file_lock;
api_service *WASABI2_API_SVC=0;

static SingletonServiceFactory<FileLock, api_filelock> filelock_factory;

int Replicant_Common_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;
	filelock_factory.Register(WASABI2_API_SVC, &file_lock);
	return NErr_Success;
}

void Replicant_Common_Shutdown()
{
	filelock_factory.Deregister(WASABI2_API_SVC);
}