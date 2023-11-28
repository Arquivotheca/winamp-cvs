#include "api.h"
#include <api/service/waservicefactory.h>

api_service *WASABI_API_SVC = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

// Metadata service
api_metadata *AGAVE_API_METADATA=0;
api_playlistmanager *WASABI_API_PLAYLISTMNGR=0;
api_application *WASABI_API_APP=0;
api_threadpool *WASABI_API_THREADPOOL=0;
api_devicemanager *AGAVE_API_DEVICEMANAGER = 0;


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
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(WASABI_API_PLAYLISTMNGR, api_playlistmanagerGUID);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceBuild(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);
}

void WasabiQuit()
{
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_PLAYLISTMNGR, api_playlistmanagerGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(WASABI_API_THREADPOOL, ThreadPoolGUID);
	ServiceRelease(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);
}