#define WIN32_LEAN_AND_MEAN
#include "api.h"
#include <bfc/platform/export.h>
#include "../Agave/Component/ifc_wa5component.h"
#include "../nu/Singleton.h"
#include "ogg_vorbis_decoder.h"
#include "mkv_vorbis_decoder.h"

api_service *WASABI_API_SVC=0;
class VorbisComponent : public ifc_wa5component
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

static OggDecoderFactory ogg_decoder;
static SingletonServiceFactory<svc_oggdecoder, OggDecoderFactory> ogg_factory;

static MKVDecoderCreator mkv_decoder;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoderCreator> mkv_factory;


void VorbisComponent::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	ogg_factory.Register(WASABI_API_SVC, &ogg_decoder);
	mkv_factory.Register(WASABI_API_SVC, &mkv_decoder);
}

void VorbisComponent::DeregisterServices(api_service *service)
{
	ogg_factory.Deregister(WASABI_API_SVC);
	mkv_factory.Deregister(WASABI_API_SVC);
}

static VorbisComponent component;
extern "C" DLLEXPORT ifc_wa5component *GetWinamp5SystemComponent()
{
	return &component;
}

#define CBCLASS VorbisComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
