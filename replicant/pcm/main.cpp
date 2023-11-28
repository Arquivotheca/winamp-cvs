#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "main.h"
#include "DirectWAVDecoder.h"
#include "nx/nxpath.h"


static SingletonService<PCMWAVDecoderService, svc_wavdecoder> wav_factory;

// {1D54506F-1D94-4AAD-A0DE-C35D1C9A93FC}
static const GUID pcm_component_guid  = 
{ 0x1d54506f, 0x1d94, 0x4aad, { 0xa0, 0xde, 0xc3, 0x5d, 0x1c, 0x9a, 0x93, 0xfc } };


class PCMComponent : public ifc_component
{
public:
	PCMComponent() : ifc_component(pcm_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service_manager);
};

static PCMComponent pcm_component;
api_service *WASABI2_API_SVC=0;



int PCMComponent::Component_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;

	return NErr_Success;
}

int PCMComponent::Component_RegisterServices(api_service *service)
{
		wav_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void PCMComponent::Component_DeregisterServices(api_service *service)
{
		wav_factory.Deregister(WASABI2_API_SVC);
}

int PCMComponent::Component_Quit(api_service *service_manager)
{
	return NErr_Success;
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &pcm_component;
}
