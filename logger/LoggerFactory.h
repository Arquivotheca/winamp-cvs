#pragma once
#include <api/service/waservicefactory.h>

class LoggerFactory :
    public waServiceFactory
{
public:
	FOURCC GetServiceType();
	const char *GetServiceName();
	GUID GetGuid();
	void *GetInterface(int global_lock = TRUE);
	int ReleaseInterface(void *ifc);

protected:
	RECVS_DISPATCH; // all Wasabi objects implementing a Dispatchable interface require this
};
