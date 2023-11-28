#define GUID_EQUALS_DEFINED
#include "netinc.h"
#include "api.h"
#include "httpget_factory.h"
#include "httpget.h"
#include "api_httpget.h"
#include "util.h"

static const char serviceName[] = "JNetLib HTTPGet";

FOURCC JNL_HTTPGetFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *JNL_HTTPGetFactory::GetServiceName()
{
	return serviceName;
}

GUID JNL_HTTPGetFactory::GetGUID()
{
	return httpreceiverGUID;
}

void *JNL_HTTPGetFactory::GetInterface(int global_lock)
{
	if (JNL::open_socketlib())
		return NULL;
	api_httpreceiver *ifc= new JNL_HTTPGet(GetGlobalDNS());
	JNL::close_socketlib(); // new JNL_HTTPGet will call open_socketlib also, so we can release now

//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int JNL_HTTPGetFactory::SupportNonLockingInterface()
{
	return 1;
}

int JNL_HTTPGetFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	api_httpreceiver *httpget = static_cast<api_httpreceiver *>(ifc);
	if (httpget)
		httpget->Release();
	
	return 1;
}

const char *JNL_HTTPGetFactory::GetTestString()
{
	return NULL;
}

int JNL_HTTPGetFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#define CBCLASS JNL_HTTPGetFactory
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