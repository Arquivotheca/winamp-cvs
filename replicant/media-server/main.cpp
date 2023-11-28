#include "api.h"
#include "foundation/export.h"
#include "component/ifc_component.h"
#include "nx/nxuri.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/singleton.h"
#include "jnetlib/jnetlib.h"
#include "media-server.h"

api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_cloud *REPLICANT_API_CLOUD=0;
api_ssdp *REPLICANT_API_SSDP=0;
api_metadata *REPLICANT_API_METADATA=0;
MediaServer media_server;
static SingletonServiceFactory<MediaServer, api_mediaserver> media_server_factory;

// {38E630BB-114B-41D7-A7EB-ED37359CC2EC}
static const GUID media_server_component_guid = 
{ 0x38e630bb, 0x114b, 0x41d7, { 0xa7, 0xeb, 0xed, 0x37, 0x35, 0x9c, 0xc2, 0xec } };

class MediaServerComponent : public ifc_component
{
public:
	MediaServerComponent() : ifc_component(media_server_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service);
	int WASABICALL Component_RegisterServices(api_service *service);
	int WASABICALL Component_OnLoading(api_service *_service_manager);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_OnClosed(api_service *_service_manager);
	int WASABICALL Component_Quit(api_service *service);
};

	
int MediaServerComponent::Component_Initialize(api_service *service)
{
	WASABI2_API_SVC = service;
	int ret = jnl_init();
	if (ret != NErr_Success)
		return ret;

	WASABI2_API_SVC->GetService(&WASABI2_API_APP);
	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);

	return NErr_Success;
}

int MediaServerComponent::Component_RegisterServices(api_service *service)
{

	media_server_factory.Register(WASABI2_API_SVC, REPLICANT_API_MEDIASERVER);
	return NErr_Success;
}

int MediaServerComponent::Component_OnLoading(api_service *_service_manager)
{
	WASABI2_API_SVC->GetService(&REPLICANT_API_CLOUD);
	WASABI2_API_SVC->GetService(&REPLICANT_API_SSDP);
	//MediaServer_Initialize();
	return NErr_Success;
}

void MediaServerComponent::Component_DeregisterServices(api_service *service)
{
	media_server_factory.Deregister(WASABI2_API_SVC);
}

int MediaServerComponent::Component_OnClosed(api_service *_service_manager)
{
	if (REPLICANT_API_CLOUD)
		REPLICANT_API_CLOUD->Release();
	REPLICANT_API_CLOUD=0;

	if (REPLICANT_API_SSDP)
		REPLICANT_API_SSDP->Release();
	REPLICANT_API_SSDP=0;

	return NErr_Success;
}

int MediaServerComponent::Component_Quit(api_service *_service_manager)
{
	if (WASABI2_API_SVC)
		WASABI2_API_SVC->Release();

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();

	if (REPLICANT_API_METADATA)
		REPLICANT_API_METADATA->Release();

	jnl_quit();
	return NErr_Success;
}

static MediaServerComponent component;
extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &component;
}
