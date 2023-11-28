#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "OggPlaybackService.h"

static SingletonService<OggPlaybackService, svc_fileplayback> playback_factory;

// {BEC8E116-40FF-462A-BD87-A92EA5FCC754}
static const GUID ogg_component_guid = 
{ 0xbec8e116, 0x40ff, 0x462a, { 0xbd, 0x87, 0xa9, 0x2e, 0xa5, 0xfc, 0xc7, 0x54 } };


class OggComponent : public ifc_component
{
public:
	OggComponent() : ifc_component(ogg_component_guid) {}
		int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
};

static OggComponent ogg_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;


int OggComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

	playback_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void OggComponent::Component_DeregisterServices(api_service *service)
{
	playback_factory.Deregister(WASABI2_API_SVC);

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &ogg_component;
}
