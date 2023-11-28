#define WIN32_LEAN_AND_MEAN
#include "api.h"
#include <bfc/platform/export.h>
#include "../Winamp/api_wa5component.h"
#include "../nu/Singleton.h"
#include "../nu/factoryt.h"
#include "mkv_a52_decoder.h"
#include "mp4_a52_decoder.h"
#include "avi_a52_decoder.h"


api_service *WASABI_API_SVC=0;
class A52Component : public api_wa5component
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
static SingletonServiceFactory<svc_avidecoder, AVIDecoder> avi_factory;
static AVIDecoder avi_decoder;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoder> mkv_factory;
static MKVDecoder mkv_decoder;
static ServiceFactoryT<MP4AudioDecoder, MP4A52Decoder> mp4Factory;

void A52Component::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	mkv_factory.Register(WASABI_API_SVC, &mkv_decoder);
	avi_factory.Register(WASABI_API_SVC, &avi_decoder);
	mp4Factory.Register(WASABI_API_SVC);
}

void A52Component::DeregisterServices(api_service *service)
{
	mp4Factory.Deregister(WASABI_API_SVC);
	mkv_factory.Deregister(WASABI_API_SVC);
	avi_factory.Deregister(WASABI_API_SVC);
}

static A52Component component;
extern "C" DLLEXPORT api_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS A52Component
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
