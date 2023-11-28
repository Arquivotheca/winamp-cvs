#include "netinc.h"
#include "api.h"
#include "factory_dns.h"
#include "asyncdns.h"
#include "util.h"
static const char serviceName[] = "JNetLib Asynchronous DNS";

FOURCC JNL_AsyncDNSFactory::GetServiceType()
{
	return WaSvc::UNIQUE; 
}

const char *JNL_AsyncDNSFactory::GetServiceName()
{
	return serviceName;
}

GUID JNL_AsyncDNSFactory::GetGUID()
{
	return dnsFactoryGUID;
}

void *JNL_AsyncDNSFactory::GetInterface(int global_lock)
{
	if (JNL::open_socketlib())
		return NULL;
	api_dns *ifc= GetGlobalDNS();
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int JNL_AsyncDNSFactory::SupportNonLockingInterface()
{
	return 1;
}

int JNL_AsyncDNSFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	JNL::close_socketlib();
	return 1;
}

const char *JNL_AsyncDNSFactory::GetTestString()
{
	return NULL;
}

int JNL_AsyncDNSFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS JNL_AsyncDNSFactory
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
