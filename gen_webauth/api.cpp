#include "api.h"
#include <api/service/waservicefactory.h>

api_service *WASABI_API_SVC = 0;
api_application *WASABI_API_APP = 0;
obj_ombrowser *browserManager = NULL;

template <class api_T>
void ServiceBuild(api_T *&api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			api_t = (api_T *)factory->getInterface();
	}
}

template <class api_T>
void ServiceRelease(api_T *api_t, GUID factoryGUID_t)
{
	if (WASABI_API_SVC)
	{
		waServiceFactory *factory = WASABI_API_SVC->service_getServiceByGuid(factoryGUID_t);
		if (factory)
			factory->releaseInterface(api_t);
	}
}

void WasabiInit(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(OMBROWSERMNGR, OBJ_OmBrowser);
}

void WasabiQuit()
{
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(OMBROWSERMNGR, OBJ_OmBrowser);
}
