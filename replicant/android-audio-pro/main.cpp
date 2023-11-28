#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "AudioThread.h"
#include "foundation/error.h"

static SingletonService<AudioThread, svc_output> output_factory;

// {BB8BE8CB-9383-4898-8AB9-F8E65EA5E517}
static const GUID audio_track_guid = 
{ 0xbb8be8cb, 0x9383, 0x4898, { 0x8a, 0xb9, 0xf8, 0xe6, 0x5e, 0xa5, 0xe5, 0x17 } };


class AudioTrackComponent : public ifc_component
{
public:
	AudioTrackComponent() : ifc_component(audio_track_guid) {}
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
};

static AudioTrackComponent audiotrack_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_android *WASABI2_API_ANDROID=0;

int AudioTrackComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	// get android API and see if we should even load ourselves
	WASABI2_API_SVC->GetService(&WASABI2_API_ANDROID);

	if (!WASABI2_API_ANDROID)
		return NErr_Error;

#ifdef REPLICANT_OPENSL
	if (WASABI2_API_ANDROID->GetSDKVersion() < 16)
	{
		WASABI2_API_ANDROID->Release();
		WASABI2_API_ANDROID=0;
		return NErr_OSNotSupported;	
	}
#elif defined(REPLICANT_AUDIOTRACK9)
	if (WASABI2_API_ANDROID->GetSDKVersion() < 9 || WASABI2_API_ANDROID->GetSDKVersion() >= 16)
	{
		WASABI2_API_ANDROID->Release();
		WASABI2_API_ANDROID=0;
		return NErr_OSNotSupported;	
	}
#else
	if (WASABI2_API_ANDROID->GetSDKVersion() >= 9)
	{
		WASABI2_API_ANDROID->Release();
		WASABI2_API_ANDROID=0;
		return NErr_OSNotSupported;	
	}
#endif	
	
	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

	output_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void AudioTrackComponent::Component_DeregisterServices(api_service *service)
{
	output_factory.Deregister(WASABI2_API_SVC);
	 
	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
	
	if (WASABI2_API_ANDROID)
		WASABI2_API_ANDROID->Release();
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &audiotrack_component;
}
