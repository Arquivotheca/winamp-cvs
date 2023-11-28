#define GUID_EQUALS_DEFINED
#include "netinc.h"
#include "api.h"
#include "webserv_factory.h"
#include "webserver.h"
#include "api_webserv.h"
#include "util.h"
static const char serviceName[] = "JNetLib WebServ";

FOURCC JNL_WebServFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *JNL_WebServFactory::GetServiceName()
{
	return serviceName;
}

GUID JNL_WebServFactory::GetGUID()
{
	return webservGUID;
}

void *JNL_WebServFactory::GetInterface(int global_lock)
{
	if (JNL::open_socketlib())
		return NULL;
	api_webserv *ifc= new WebServerBaseClass;

	//if (global_lock)
	//	WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int JNL_WebServFactory::SupportNonLockingInterface()
{
	return 1;
}

int JNL_WebServFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	api_webserv *webserv = static_cast<api_webserv *>(ifc);
	WebServerBaseClass *jnl_webserv = static_cast<WebServerBaseClass *>(webserv);
	delete jnl_webserv;
	JNL::close_socketlib();
	return 1;
}

const char *JNL_WebServFactory::GetTestString()
{
	return NULL;
}

int JNL_WebServFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS JNL_WebServFactory
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
