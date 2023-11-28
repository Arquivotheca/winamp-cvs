#define GUID_EQUALS_DEFINED
#include "netinc.h"
#include "api.h"
#include "factory_connection.h"
#include "connection.h"
#include "util.h"

static const char serviceName[] = "JNetLib Connection";

FOURCC JNL_ConnectionFactory::GetServiceType()
{
	return WaSvc::OBJECT; 
}

const char *JNL_ConnectionFactory::GetServiceName()
{
	return serviceName;
}

GUID JNL_ConnectionFactory::GetGUID()
{
	return connectionFactoryGUID;
}

void *JNL_ConnectionFactory::GetInterface(int global_lock)
{
	if (JNL::open_socketlib())
		return NULL;
	JNL_Connection *conn = new JNL_Connection;
	conn->set_dns(GetGlobalDNS());
	api_connection *ifc= static_cast<api_connection *>(conn);
//	if (global_lock)
//		WASABI_API_SVC->service_lock(this, (void *)ifc);
	return ifc;
}

int JNL_ConnectionFactory::SupportNonLockingInterface()
{
	return 1;
}

int JNL_ConnectionFactory::ReleaseInterface(void *ifc)
{
	//WASABI_API_SVC->service_unlock(ifc);
	api_connection *connection = static_cast<api_connection *>(ifc);
	JNL_Connection *jnl_connection = static_cast<JNL_Connection *>(connection);
	delete jnl_connection;
	JNL::close_socketlib();
	return 1;
}

const char *JNL_ConnectionFactory::GetTestString()
{
	return NULL;
}

int JNL_ConnectionFactory::ServiceNotify(int msg, int param1, int param2)
{
	return 1;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS JNL_ConnectionFactory
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
