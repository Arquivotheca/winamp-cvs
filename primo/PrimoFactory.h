#ifndef NULLSOFT_PRIMO_PRIMOFACTORY_H
#define NULLSOFT_PRIMO_PRIMOFACTORY_H

#include <api/service/waservicefactory.h>
#include <api/service/services.h>

class PrimoFactory : public waServiceFactory
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

extern HMODULE primoDLL;
extern bool veritas_loaded;

#endif