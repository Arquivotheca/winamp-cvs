/* copyright 2006 Ben Allison */
#include "api.h"
#include "wa5_alac.h"
#include "factory_alac.h"
#include <bfc/platform/export.h>
#include <api/service/waservicefactory.h>

static ALACFactory alacFactory;
static WA5_ALAC wa5_alac;

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

void WA5_ALAC::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	WASABI_API_SVC->service_register(&alacFactory);  
}

int WA5_ALAC::RegisterServicesSafeModeOk()
{
	return 1;
}

void WA5_ALAC::DeregisterServices(api_service *service)
{
	service->service_deregister(&alacFactory);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
}

extern "C" DLLEXPORT api_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_alac;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WA5_ALAC
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;