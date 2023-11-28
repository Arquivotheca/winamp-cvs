#ifndef NULLSOFT_FACTORY_SSLCONNECTIONH
#define NULLSOFT_FACTORY_SSLCONNECTIONH

#include <api/service/waservicefactory.h>
#include <api/service/services.h>

class JNL_SSL_ConnectionFactory : public waServiceFactory
{
public:
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGUID();
	void *GetInterface(int global_lock);
	int SupportNonLockingInterface();
	int ReleaseInterface(void *ifc);
	const char *GetTestString();
	int ServiceNotify(int msg, int param1, int param2);
	
protected:
	RECVS_DISPATCH;
};

#endif