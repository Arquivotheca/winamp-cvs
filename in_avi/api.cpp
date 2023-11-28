#include "api.h"
#include "main.h"
#include <api/service/waservicefactory.h>

api_service *WASABI_API_SVC = 0;
api_config *AGAVE_API_CONFIG = 0;
api_application *WASABI_API_APP = 0;
api_stats *AGAVE_API_STATS = 0;
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
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_STATS, AnonymousStatsGUID);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,InAviLangGUID);
}

void WasabiQuit()
{
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_STATS, AnonymousStatsGUID);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
}