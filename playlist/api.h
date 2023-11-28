#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include "../Agave/Config/api_config.h"
extern api_config *config;
#define AGAVE_API_CONFIG config

#include <api/script/api_maki.h>
#define WASABI_API_MAKI makiApi

#include "../Winamp/JSAPI2_api_security.h"
extern JSAPI2::api_security *jsapi2_security;
#define AGAVE_API_JSAPI2_SECURITY jsapi2_security

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#include <api/service/waservicefactory.h>

template <class api_t>
class QuickService
{
public:
	QuickService(GUID serviceGUID) : sf(0), api(0)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(serviceGUID);
	if (sf)
		api = (api_t *)sf->getInterface();
	}
	~QuickService()
	{
		sf->releaseInterface(api);
		api=0;
	}
	bool OK()
	{
		return api!=0;
	}
	api_t *operator ->()
	{
		return api;
	}
	waServiceFactory *sf;
	api_t *api;
};

#include "../Agave/Language/api_language.h"

#endif