#include "api.h"
#include <api/service/waservicefactory.h>

api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

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
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	// need to have this initialised before we try to do anything with localisation features
	// TODO: WASABI_API_START_LANG(plugin.hDllInstance,InAviLangGUID);
}

void WasabiQuit()
{
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
}