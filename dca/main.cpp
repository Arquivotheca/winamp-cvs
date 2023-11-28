#define WIN32_LEAN_AND_MEAN
#include "api.h"
#include <bfc/platform/export.h>
#include "../Winamp/api_wa5component.h"
#include "../nu/Singleton.h"
#include "mkv_dca_decoder.h"


api_service *WASABI_API_SVC=0;
class DCAComponent : public api_wa5component
{
public:
	void RegisterServices(api_service *service);
	int RegisterServicesSafeModeOk();
	void DeregisterServices(api_service *service);
protected:
	RECVS_DISPATCH;
};

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

static SingletonServiceFactory<svc_mkvdecoder, MKVDecoder> mkv_factory;
static MKVDecoder mkv_decoder;

void DCAComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	mkv_factory.Register(WASABI_API_SVC, &mkv_decoder);
}

int DCAComponent::RegisterServicesSafeModeOk()
{
	return 1;
}

void DCAComponent::DeregisterServices(api_service *service)
{
	mkv_factory.Deregister(WASABI_API_SVC);
}

static DCAComponent component;
extern "C" DLLEXPORT api_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS DCAComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
CB(15, RegisterServicesSafeModeOk)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
