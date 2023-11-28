#include "api.h"
#include <api/service/waservicefactory.h>
namespace Wasabi2
{
#include "service/api_service.h"
}
DEFINE_EXTERNAL_SERVICE(api_service,          WASABI_API_SVC);
DEFINE_EXTERNAL_SERVICE(api_language,         WASABI_API_LNG); 
DEFINE_EXTERNAL_SERVICE(api_application,      WASABI_API_APP);
DEFINE_EXTERNAL_SERVICE(api_mldb,             AGAVE_API_MLDB);
DEFINE_EXTERNAL_SERVICE(api_syscb,            WASABI_API_SYSCB);
DEFINE_EXTERNAL_SERVICE(api_metadata,         AGAVE_API_METADATA);
DEFINE_EXTERNAL_SERVICE(api_config,           AGAVE_API_CONFIG);
DEFINE_EXTERNAL_SERVICE(api_memmgr,           WASABI_API_MEMMGR);
DEFINE_EXTERNAL_SERVICE(api_albumart,         AGAVE_API_ALBUMART);
DEFINE_EXTERNAL_SERVICE(obj_ombrowser,        AGAVE_OBJ_BROWSER);
DEFINE_EXTERNAL_SERVICE(api_threadpool,       AGAVE_API_THREADPOOL);
DEFINE_EXTERNAL_SERVICE(api_devicemanager,    AGAVE_API_DEVICEMANAGER);
DEFINE_EXTERNAL_SERVICE(api_playlistmanager,  AGAVE_API_PLAYLISTMANAGER);
DEFINE_EXTERNAL_SERVICE(api_cloud,            REPLICANT_API_CLOUD);
DEFINE_EXTERNAL_SERVICE(Wasabi2::api_metadata,            REPLICANT_API_METADATA);
DEFINE_EXTERNAL_SERVICE(Wasabi2::api_artwork,            REPLICANT_API_ARTWORK);

static Wasabi2::api_service *WASABI2_API_SVC=0;

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
		api_t = NULL;
	}
}

int WasabiInit(api_service *service)
{
	WASABI_API_SVC = service;
	ServiceBuild(WASABI2_API_SVC, Wasabi2::api_service::GetServiceGUID());
	if (!WASABI2_API_SVC)
		return 0;
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	ServiceBuild(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
	ServiceBuild(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);
	WASABI2_API_SVC->GetService(&REPLICANT_API_CLOUD);
	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);
	WASABI2_API_SVC->GetService(&REPLICANT_API_ARTWORK);
	return 1;
}

void WasabiQuit()
{
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(AGAVE_API_PLAYLISTMANAGER, api_playlistmanagerGUID);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(AGAVE_API_ALBUMART, albumArtGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_DEVICEMANAGER, DeviceManagerGUID);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	
	if (REPLICANT_API_CLOUD)
		REPLICANT_API_CLOUD->Release();
	if (REPLICANT_API_METADATA)
		REPLICANT_API_METADATA->Release();
	if (REPLICANT_API_ARTWORK)
		REPLICANT_API_ARTWORK->Release();
	if (WASABI2_API_SVC)
		WASABI2_API_SVC->Release();
}