#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "UltravoxAACService.h"
#include "MP4DecoderService.h"
#include "ICYAACService.h"
#ifdef __ANDROID__
#include "android/PVMP4.h"
#endif
#include "application/features.h"
#include "foundation/error.h"
#include <new>

/* Services that we are going to register */

static WasabiServiceFactory *service_factories[] = 
{
#ifndef REPLICANT_NO_ULTRAVOX
new (std::nothrow) SingletonService<UltravoxAACService, svc_ultravox_playback>,
#endif

#if !defined(REPLICANT_NO_LOCAL) && !defined(REPLICANT_NO_MP4)
new (std::nothrow) SingletonService<MP4DecoderService, svc_mp4decoder>,
// TODO: new (std::nothrow) SingletonService<MP4RawReaderService, svc_raw_media_reader>,
#endif

#ifndef REPLICANT_NO_ICY
new (std::nothrow) SingletonService<ICYAACService, svc_icy_playback>,
#endif
};

static const size_t num_factories = sizeof(service_factories)/sizeof(*service_factories);

// {180B9018-FD31-4DF2-9E03-2FFCB18BEE84}
static const GUID aac_component_guid = 
{ 0x180b9018, 0xfd31, 0x4df2, { 0x9e, 0x3, 0x2f, 0xfc, 0xb1, 0x8b, 0xee, 0x84 } };

class AACComponent : public ifc_component
{
public:
	AACComponent() : ifc_component(aac_component_guid) {}
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
};

static AACComponent aac_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
#ifdef __ANDROID__
api_android *WASABI2_API_ANDROID=0;
#endif


int AACComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;

	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

#ifdef __ANDROID__
	WASABI2_API_SVC->GetService(&WASABI2_API_ANDROID);
#endif

#ifdef __ANDROID__
	if (AACDecoderLibraryInit() != NErr_Success)
			return NErr_Success; // skip registering components
#endif
	for (size_t i=0;i<num_factories;i++)
	{
		service_factories[i]->Register(WASABI2_API_SVC);
	}

	if (WASABI2_API_APP)
		WASABI2_API_APP->SetFeature(Features::aac_playback);

	return NErr_Success;
}

void AACComponent::Component_DeregisterServices(api_service *service)
{
	for (size_t i=0;i<num_factories;i++)
	{
		service_factories[i]->Deregister(WASABI2_API_SVC);
		delete service_factories[i];
	}

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
#ifdef __ANDROID__
	if (WASABI2_API_ANDROID)
		WASABI2_API_ANDROID->Release();
#endif
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &aac_component;
}
