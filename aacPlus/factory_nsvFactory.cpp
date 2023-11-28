#include "factory_nsvFactory.h"
#include "api.h"
#include "NSVFactory.h"

static const char serviceName[] = "aacPlus NSV Decoder";

// {55632E28-8171-4670-AE5D-CF714900C62E}
static const GUID NSV_AACP_GUID = 
{ 0x55632e28, 0x8171, 0x4670, { 0xae, 0x5d, 0xcf, 0x71, 0x49, 0x0, 0xc6, 0x2e } };

static NSVFactory nsvFactory;
FOURCC NSVFactoryFactory::GetServiceType()
{
	return WaSvc::NSVFACTORY; 
}

const char *NSVFactoryFactory::GetServiceName()
{
	return serviceName;
}

GUID NSVFactoryFactory::GetGUID()
{
	return NSV_AACP_GUID;
}

void *NSVFactoryFactory::GetInterface(int global_lock)
{
	svc_nsvFactory *ifc = &nsvFactory;
	return ifc;
}

int NSVFactoryFactory::SupportNonLockingInterface()
{
	return 1;
}

int NSVFactoryFactory::ReleaseInterface(void *ifc)
{
	return 1;
}

const char *NSVFactoryFactory::GetTestString()
{
	return 0;
}

int NSVFactoryFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}


#define CBCLASS NSVFactoryFactory
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