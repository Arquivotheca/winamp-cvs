#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "PlaylistManager.h"
#include <new>
/* Services that we are going to register */
static SingletonService<PlaylistManager, api_playlistmanager> playlist_manager_factory;

/* services we need from Wasabi */
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;

// {FC25F660-22B4-4850-9D75-508AE60C6A8B}
static const GUID playlist_component_guid = 
{ 0xfc25f660, 0x22b4, 0x4850, { 0x9d, 0x75, 0x50, 0x8a, 0xe6, 0xc, 0x6a, 0x8b } };


/* Component implementation */
class PlaylistComponent : public ifc_component
{
public:
	PlaylistComponent() : ifc_component(playlist_component_guid) {}
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
};

int	PlaylistComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

	playlist_manager_factory.Register(WASABI2_API_SVC);

	return NErr_Success;
}

void PlaylistComponent::Component_DeregisterServices(api_service *service)
{
	playlist_manager_factory.Deregister(WASABI2_API_SVC);

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
}

static PlaylistComponent playlist_component;

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &playlist_component;
}
