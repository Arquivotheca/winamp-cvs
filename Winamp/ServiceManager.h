#ifndef NULLSOFT_SERVICEMANAGERH
#define NULLSOFT_SERVICEMANAGERH

#include <api/service/api_service.h>
#include "../nu/AutoLock.h"
#include "../nu/PtrList.h"
#include "../nu/Map.h"

struct Counter;

class ServiceManager : public api_service
{
public:
	ServiceManager();
	int service_register(waServiceFactory *svc);
	int service_deregister(waServiceFactory *svc);
	size_t service_getNumServices(FOURCC svc_type);
	waServiceFactory *service_enumService(FOURCC svc_type, size_t n);
	waServiceFactory *service_getServiceByGuid(GUID guid);
	int service_lock(waServiceFactory *owner, void *svcptr);
  int service_clientLock(void *svcptr);
  int service_release(void *svcptr);
  const char *service_getTypeName(FOURCC svc_type);
	int service_unlock(void *svcptr);
  int service_isvalid(FOURCC svctype, waServiceFactory *service);
	int service_compactDuplicates(waServiceFactory *me);
protected:
	RECVS_DISPATCH;
private:
	Nullsoft::Utility::LockGuard serviceGuard;
	typedef nu::PtrList<waServiceFactory> ServiceList;
	ServiceList services;
	nu::PtrList<Counter> locks;
	nu::Map<FOURCC, ServiceList *> services_by_type;
};

extern api_service *serviceManager;

#endif