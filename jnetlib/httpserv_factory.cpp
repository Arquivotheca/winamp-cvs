#define GUID_EQUALS_DEFINED
#include "netinc.h"
#include "api.h"
#include "httpserv_factory.h"
#include "httpserv.h"
#include "api_httpserv.h"
#include "util.h"
static const char serviceName[] = "JNetLib HTTPServ";

FOURCC JNL_HttpServFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *JNL_HttpServFactory::GetServiceName()
{
	return serviceName;
}

GUID JNL_HttpServFactory::GetGUID()
{
	return httpservGUID;
}

void *JNL_HttpServFactory::GetInterface(int global_lock)
{
	if (JNL::open_socketlib())
		return NULL;
	api_httpserv *ifc= new JNL_HTTPServ;

	//if (global_lock)
	//	WASABI_API_SVC->service_lock(this, (void *)ifc);

	
	return ifc;
	
}

int JNL_HttpServFactory::SupportNonLockingInterface()
{
	return 1;
}

int JNL_HttpServFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	api_httpserv *httpserv = static_cast<api_httpserv *>(ifc);
	JNL_HTTPServ *jnl_httpserv = static_cast<JNL_HTTPServ *>(httpserv);
	delete jnl_httpserv;
	JNL::close_socketlib();
	return 1;
}

const char *JNL_HttpServFactory::GetTestString()
{
	return NULL;
}

int JNL_HttpServFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}


#ifdef CBCLASS
#undef CBCLASS
#endif
#define CBCLASS JNL_HttpServFactory
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
