#define GUID_EQUALS_DEFINED
#include ".\api.h"
#include ".\factory_watcher.h"
#include ".\watcher.h"

const char WatcherFactory::serviceName[] = "Watcher Service";

FOURCC WatcherFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *WatcherFactory::GetServiceName()
{
	return serviceName;
}

GUID WatcherFactory::GetGUID()
{
	return watcherGUID;
}

void *WatcherFactory::GetInterface(int global_lock)
{
	api_watcher *ifc= new MLWatcher();
	return ifc;
}

int WatcherFactory::SupportNonLockingInterface()
{
	return 1;
}

int WatcherFactory::ReleaseInterface(void *ifc)
{
	api_watcher *watcher = static_cast<api_watcher *>(ifc);
	MLWatcher *mlWatcher = static_cast<MLWatcher *>(watcher);
	delete mlWatcher;
	return 1;
}

const char *WatcherFactory::GetTestString()
{
	return NULL;
}

int WatcherFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WatcherFactory
START_DISPATCH;
CB(WASERVICEFACTORY_GETSERVICETYPE, GetServiceType)
CB(WASERVICEFACTORY_GETSERVICENAME, GetServiceName)
CB(WASERVICEFACTORY_GETGUID, GetGUID)
CB(WASERVICEFACTORY_GETINTERFACE, GetInterface)
CB(WASERVICEFACTORY_SUPPORTNONLOCKINGGETINTERFACE, SupportNonLockingInterface) 
CB(WASERVICEFACTORY_RELEASEINTERFACE, ReleaseInterface)
CB(WASERVICEFACTORY_GETTESTSTRING, GetTestString)
CB(WASERVICEFACTORY_SERVICENOTIFY, ServiceNotify)
END_DISPATCH;
