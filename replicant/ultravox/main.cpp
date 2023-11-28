#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "UltravoxDemuxerService.h"

static SingletonService<UltravoxDemuxerService, svc_http_demuxer> demuxer_factory;

// {90A5F6BD-8AE1-447E-8FB1-BFC0C841F99B}
static const GUID ultravox_component_guid = 
{ 0x90a5f6bd, 0x8ae1, 0x447e, { 0x8f, 0xb1, 0xbf, 0xc0, 0xc8, 0x41, 0xf9, 0x9b } };

class UltravoxComponent : public ifc_component
{
public:
	UltravoxComponent() : ifc_component(ultravox_component_guid) {}
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
};

static UltravoxComponent ultravox_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;

int UltravoxComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

	demuxer_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void UltravoxComponent::Component_DeregisterServices(api_service *service)
{
	demuxer_factory.Deregister(WASABI2_API_SVC);

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &ultravox_component;
}
