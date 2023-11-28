#include "./dropBoxFactory.h"
#include "./dropBoxApi.h"

static const char serviceName[] = "DropBox API";

static DropBoxApi dropBoxInstance;

FOURCC DropBoxFactory::GetServiceType()
{
	return WaSvc::UNIQUE;
}

const char *DropBoxFactory::GetServiceName()
{
	return serviceName;
}

GUID DropBoxFactory::GetGUID()
{
	return dropBoxApiGuid;
}

void *DropBoxFactory::GetInterface(int global_lock)
{
	return &dropBoxInstance;
}

int DropBoxFactory::SupportNonLockingInterface()
{
	return 1;
}

int DropBoxFactory::ReleaseInterface(void *ifc)
{
	return 1;
}

const char *DropBoxFactory::GetTestString()
{
	return NULL;
}

int DropBoxFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS DropBoxFactory
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
#undef CBCLASS