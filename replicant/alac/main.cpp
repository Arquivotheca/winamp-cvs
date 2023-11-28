#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "MP4DecoderService.h"
#include "application/features.h"
#include "foundation/error.h"
#include <new>

/* Services that we are going to register */


SingletonService<MP4DecoderService, svc_mp4decoder> mp4_factory;

static bool alac_registered=false;
// {14AF8A9D-F1F6-4406-85D2-5FF8BC75B91A}
static const GUID alac_component_guid = 
{ 0x14af8a9d, 0xf1f6, 0x4406, { 0x85, 0xd2, 0x5f, 0xf8, 0xbc, 0x75, 0xb9, 0x1a } };

class ALACComponent : public ifc_component, public Features::SystemCallback
{
public:
	ALACComponent() : ifc_component(alac_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service_manager);

	int WASABICALL FeaturesSystemCallback_OnPermissionsChanged();
};

static ALACComponent alac_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_syscb *WASABI2_API_SYSCB=0;

int ALACComponent::Component_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);
	WASABI2_API_SVC->GetService(&WASABI2_API_SYSCB);

	if (WASABI2_API_SYSCB)
		WASABI2_API_SYSCB->RegisterCallback((Features::SystemCallback *)this);

	return NErr_Success;
}

int ALACComponent::Component_RegisterServices(api_service *service)
{
	if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::alac_playback) == NErr_True)
	{
		mp4_factory.Register(WASABI2_API_SVC);
		alac_registered=true;
	}

	return NErr_Success;
}

void ALACComponent::Component_DeregisterServices(api_service *service)
{
	if (alac_registered)
	{
		mp4_factory.Deregister(WASABI2_API_SVC);
		alac_registered=false;
	}
}

int ALACComponent::Component_Quit(api_service *service_manager)
{
	if (WASABI2_API_SYSCB)
	{
		WASABI2_API_SYSCB->UnregisterCallback((Features::SystemCallback *)this);
		WASABI2_API_SYSCB->Release();
	}

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
	WASABI2_API_APP=0;

	return NErr_Success;
}

int ALACComponent::FeaturesSystemCallback_OnPermissionsChanged()
{
	if (alac_registered)
	{
		if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::alac_playback) != NErr_True)
		{
			mp4_factory.Deregister(WASABI2_API_SVC);
			alac_registered=false;
		}
	}
	else
	{
		if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::alac_playback) == NErr_True)
		{
			mp4_factory.Register(WASABI2_API_SVC);			
			alac_registered=true;
		}
	}

	return NErr_Success;
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &alac_component;
}
