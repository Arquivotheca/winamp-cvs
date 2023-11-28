#ifndef NULLOSFT_API_DROPBOX_FACTORY_HEADER
#define NULLOSFT_API_DROPBOX_FACTORY_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <api/service/waservicefactory.h>
#include <api/service/services.h>

class DropBoxFactory : public waServiceFactory
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

#endif // NULLOSFT_API_DROPBOX_FACTORY_HEADER
