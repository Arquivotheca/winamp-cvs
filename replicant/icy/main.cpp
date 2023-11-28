#include "api.h"
#include "component/ifc_component.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "ICYDemuxerService.h"

int MetadataKey_streamname=-1;
int MetadataKey_streamtitle=-1;
int MetadataKey_streamurl=-1;

static SingletonService<ICYDemuxerService, svc_http_demuxer> demuxer_factory;

// {F8462102-3C49-4D7E-BD2B-222B7D200B16}
static const GUID icy_component_guid = 
{ 0xf8462102, 0x3c49, 0x4d7e, { 0xbd, 0x2b, 0x22, 0x2b, 0x7d, 0x20, 0xb, 0x16 } };

class ICYComponent : public ifc_component
{
public:
	ICYComponent() : ifc_component(icy_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service_manager);
};

static ICYComponent icy_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_metadata *REPLICANT_API_METADATA = 0;

int ICYComponent::Component_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;

	WASABI2_API_SVC->GetService(&WASABI2_API_APP);
	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);

	if (REPLICANT_API_METADATA)
	{
		nx_string_t metadata_key;
		if (NXStringCreateWithUTF8(&metadata_key, "STREAMTITLE") == NErr_Success)
		{ 
			REPLICANT_API_METADATA->RegisterField(metadata_key, &MetadataKey_streamtitle);
			NXStringRelease(metadata_key);
		}

		if (NXStringCreateWithUTF8(&metadata_key, "STREAMNAME") == NErr_Success)
		{ 
			REPLICANT_API_METADATA->RegisterField(metadata_key, &MetadataKey_streamname);
			NXStringRelease(metadata_key);
		}

		if (NXStringCreateWithUTF8(&metadata_key, "STREAMURL") == NErr_Success)
		{ 
			REPLICANT_API_METADATA->RegisterField(metadata_key, &MetadataKey_streamurl);
			NXStringRelease(metadata_key);
		}
	}
	return NErr_Success;
}

int ICYComponent::Component_RegisterServices(api_service *service)
{
	demuxer_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void ICYComponent::Component_DeregisterServices(api_service *service)
{
	demuxer_factory.Deregister(WASABI2_API_SVC);
}

int ICYComponent::Component_Quit(api_service *service_manager)
{
	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
	WASABI2_API_APP=0;

	if (REPLICANT_API_METADATA)
		REPLICANT_API_METADATA->Release();
	REPLICANT_API_METADATA=0;
	return NErr_Success;
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &icy_component;
}
