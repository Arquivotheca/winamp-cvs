#include "api.h"
#include <bfc/platform/export.h>
#include "../Winamp/api_wa5component.h"
#include "../Winamp/Singleton.h"
#include "DownloadManager.h"
#include <api/service/waservicefactory.h>

static Singleton2<api_downloadManager, DownloadManager> dlMgrFactory;
static DownloadManager dlMgr;

class DownloadManager_Component: public api_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

DownloadManager_Component downloadManager_component;;
api_service *WASABI_API_SVC=0;
api_application *WASABI_API_APP=0;
api_config *AGAVE_API_CONFIG=0;
api_threadpool *WASABI_API_THREADPOOL = 0;

void DownloadManager_Component::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf)
		WASABI_API_APP = (api_application *)sf->getInterface();
	sf =  WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
	if (sf)
		AGAVE_API_CONFIG = (api_config *)sf->getInterface();

	sf =  WASABI_API_SVC->service_getServiceByGuid(ThreadPoolGUID);
	if (sf)
		WASABI_API_THREADPOOL = (api_threadpool*)sf->getInterface();

	dlMgrFactory.Register(&dlMgr);
}

int DownloadManager_Component::RegisterServicesSafeModeOk()
{
	return 1;
}

void DownloadManager_Component::DeregisterServices(api_service *service)
{
	waServiceFactory *sf=0;
	if (WASABI_API_APP)
	{
		sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
		if (sf)
			sf->releaseInterface(WASABI_API_APP);
	}

	if (AGAVE_API_CONFIG)
	{
		sf =  WASABI_API_SVC->service_getServiceByGuid(AgaveConfigGUID);
		if (sf)
			sf->releaseInterface(AGAVE_API_CONFIG);
	}

	dlMgr.Kill();
	dlMgrFactory.Deregister();

	if (WASABI_API_THREADPOOL)
	{
		sf =  WASABI_API_SVC->service_getServiceByGuid(ThreadPoolGUID);
		if (sf)
			sf->releaseInterface(WASABI_API_THREADPOOL);
	}
}

extern "C" DLLEXPORT api_wa5component *GetWinamp5SystemComponent()
{
	return &downloadManager_component;
}

#define CBCLASS DownloadManager_Component
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
