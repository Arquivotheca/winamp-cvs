#include "api.h"
#include "component/ifc_component.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "foundation/error.h"
#include "nx/nx.h"
#include "M3UHandler.h"

static SingletonService<M3UHandler, svc_playlisthandler> m3u_factory;

// {6FF22BDA-277C-460B-8C74-58888B68C7FC}
static const GUID m3u_component_guid = 
{ 0x6ff22bda, 0x277c, 0x460b, { 0x8c, 0x74, 0x58, 0x88, 0x8b, 0x68, 0xc7, 0xfc } };

class M3UComponent : public ifc_component
{
public:
	M3UComponent() : ifc_component(m3u_component_guid) {}
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
};

static M3UComponent m3u_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
nx_string_t extension_m3u, extension_m3u8;


int M3UComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);

	m3u_factory.Register(WASABI2_API_SVC);
	extension_m3u = NXStringCreateFromUTF8("m3u");
	extension_m3u8 = NXStringCreateFromUTF8("m3u8");

	return NErr_Success;
}

void M3UComponent::Component_DeregisterServices(api_service *service)
{
	m3u_factory.Deregister(WASABI2_API_SVC);

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &m3u_component;
}
