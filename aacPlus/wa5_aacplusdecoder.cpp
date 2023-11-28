#include "api.h"
#include "wa5_aacplusdecoder.h"
#include "factory_aacplusdecoder.h"
#include "factory_ctdecoder.h"
#include "factory_nsvFactory.h"
#include "../nu/Singleton.h"
#include "mKVDecoder.h"
#include "flv_aac_decoder.h"
#include "avi_aac_decoder.h"
#include <bfc/platform/export.h>

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

#ifdef _WIN32 // TODO: make this some kind of #ifdef WINAMP
AacPlusDecoderFactory aacPlusDecoderFactory;
CTDecoderFactory ctDecoderFactory;
static SingletonServiceFactory<svc_mkvdecoder, MKVDecoder> mkv_factory;
static MKVDecoder mkv_decoder;
static SingletonServiceFactory<svc_avidecoder, AVIDecoder> avi_factory;
static AVIDecoder avi_decoder;
static FLVDecoderCreator flvCreator;
static SingletonServiceFactory<svc_flvdecoder, FLVDecoderCreator> flvFactory;
#endif
WA5_AacPlusDecoder wa5_aacplusdecoder;
NSVFactoryFactory nsvFactoryFactory;
 
void WA5_AacPlusDecoder::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
#ifdef _WIN32 // TODO: make this some kind of #ifdef WINAMP
	WASABI_API_SVC->service_register(&aacPlusDecoderFactory);  
	WASABI_API_SVC->service_register(&ctDecoderFactory);
	mkv_factory.Register(WASABI_API_SVC, &mkv_decoder);
	avi_factory.Register(WASABI_API_SVC, &avi_decoder);
	flvFactory.Register(WASABI_API_SVC, &flvCreator);
#endif
	WASABI_API_SVC->service_register(&nsvFactoryFactory);  	
	ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

void WA5_AacPlusDecoder::DeregisterServices(api_service *service)
{
#ifdef _WIN32 // TODO: make this some kind of #ifdef WINAMP
	service->service_deregister(&aacPlusDecoderFactory);
	service->service_deregister(&ctDecoderFactory);
	mkv_factory.Deregister(WASABI_API_SVC);
	avi_factory.Deregister(WASABI_API_SVC);
	flvFactory.Deregister(WASABI_API_SVC);
#endif
	service->service_deregister(&nsvFactoryFactory);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}

extern "C" DLLEXPORT api_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_aacplusdecoder;
}

#define CBCLASS WA5_AacPlusDecoder
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
