#include "api.h"
#include "main.h"
#include "JSAPI2_Creator.h"
#include "../nu/ServiceWatcher.h"
#include "./authCallback.h"
#include <api/service/waservicefactory.h>

static ServiceWatcher serviceWatcher;
static OpenAuthCallback *authCallback = NULL;

api_service *WASABI_API_SVC = 0;
api_syscb *WASABI_API_SYSCB = 0;
api_application *WASABI_API_APP = 0;
api_metadata *AGAVE_API_METADATA=0;
api_mldb *AGAVE_API_MLDB = 0;
api_config *AGAVE_API_CONFIG = 0;
api_auth *AGAVE_API_AUTH = 0;
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
JSAPI2::api_security *AGAVE_API_JSAPI2_SECURITY = 0;
JSAPI2Factory jsapi2Factory;

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
	ServiceBuild(AGAVE_API_METADATA, api_metadataGUID);
	ServiceBuild(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceBuild(WASABI_API_LNG, languageApiGUID);
	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(plugin.hDllInstance,GenMudLangGUID);
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(AGAVE_API_AUTH, AuthApiGUID);
	ServiceBuild(AGAVE_API_JSAPI2_SECURITY, JSAPI2::api_securityGUID);

	WASABI_API_SVC->service_register(&jsapi2Factory);

	serviceWatcher.WatchWith(WASABI_API_SVC);
	serviceWatcher.WatchFor(&AGAVE_API_MLDB, mldbApiGuid);
	WASABI_API_SYSCB->syscb_registerCallback(&serviceWatcher);

	if (NULL == authCallback && 
		NULL != WASABI_API_SYSCB &&
		SUCCEEDED(OpenAuthCallback::CreateInstance(&authCallback)))
	{
		INT r = WASABI_API_SYSCB->syscb_registerCallback(authCallback);
		if (0 == r)
		{
			authCallback->Release();
			authCallback = NULL;
		}
	}
}

void WasabiQuit()
{
	serviceWatcher.StopWatching();
	serviceWatcher.Clear();

	if (NULL != authCallback)
	{
		if (NULL != WASABI_API_SYSCB)
			WASABI_API_SYSCB->syscb_deregisterCallback(authCallback);

		authCallback->Release();
		authCallback = NULL;
	}

	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(AGAVE_API_METADATA, api_metadataGUID);
	ServiceRelease(WASABI_API_SYSCB, syscbApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(AGAVE_API_MLDB, mldbApiGuid);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(AGAVE_API_AUTH, AuthApiGUID);
	ServiceRelease(AGAVE_API_JSAPI2_SECURITY, JSAPI2::api_securityGUID);
	WASABI_API_SVC->service_deregister(&jsapi2Factory);
}