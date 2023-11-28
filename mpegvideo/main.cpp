#define WIN32_LEAN_AND_MEAN
#include "api.h"
#include <bfc/platform/export.h>
#include "../Winamp/api_wa5component.h"
#include "../nu/Singleton.h"
#include "../nu/factoryt.h"
#include "mkv_mpgv_decoder.h"

api_service *WASABI_API_SVC=0;
class MPEGVideoComponent : public api_wa5component
{
public:
	void RegisterServices(api_service *service);
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
static MKVDecoderCreator mkvCreator;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoderCreator> mkvFactory;
 
void MPEGVideoComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	mkvFactory.Register(WASABI_API_SVC, &mkvCreator);
}

void MPEGVideoComponent::DeregisterServices(api_service *service)
{
	mkvFactory.Deregister(WASABI_API_SVC);	
}

static MPEGVideoComponent component;
extern "C" DLLEXPORT api_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS MPEGVideoComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
