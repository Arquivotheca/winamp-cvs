#define GUID_EQUALS_DEFINED
#include ".\api.h"
#include ".\factory_watchfilter.h"
#include ".\watchFilter.h"


const char WatchFilterFactory::serviceName[] = "Watcher Filter Service";

FOURCC WatchFilterFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *WatchFilterFactory::GetServiceName()
{
	return serviceName;
}

GUID WatchFilterFactory::GetGUID()
{
	return watchfilterGUID;
}

void *WatchFilterFactory::GetInterface(int global_lock)
{
	api_watchfilter *ifc= new MLWatchFilter();
	return ifc;
}

int WatchFilterFactory::SupportNonLockingInterface()
{
	return 1;
}

int WatchFilterFactory::ReleaseInterface(void *ifc)
{
	api_watchfilter *watchfilter = static_cast<api_watchfilter *>(ifc);
	MLWatchFilter *mlWatchFilter = static_cast<MLWatchFilter*>(watchfilter);
	delete mlWatchFilter;
	return 1;
}

const char *WatchFilterFactory::GetTestString()
{
	return NULL;
}

int WatchFilterFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WatchFilterFactory
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