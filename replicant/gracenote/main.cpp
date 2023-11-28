#include "api.h"
#include "main.h"
#include "MetadataGDO.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "application/features.h"
#include "Gracenote.h"

int MetadataKey_GracenoteFileID=-1;
int MetadataKey_GracenoteExtData=-1;
int MetadataKey_MatchedTrack=-1;
int MetadataKey_MatchScore=-1;

GracenoteAPI gracenote_api;
static SingletonServiceFactory<GracenoteAPI, api_gracenote> gracenote_factory;
static SingletonService<GracenoteMetadataService, svc_metadata> metadata_factory;
//static SingletonService<FLACDecoder, svc_decode> decoder_factory;

// {69EA467B-DF53-4E73-AA83-A96DD7CD720E}
static const GUID gracenote_component_guid = 
{ 0x69ea467b, 0xdf53, 0x4e73, { 0xaa, 0x83, 0xa9, 0x6d, 0xd7, 0xcd, 0x72, 0xe } };

class GracenoteComponent : public ifc_component, public Features::SystemCallback
{
public:
	GracenoteComponent() : ifc_component(gracenote_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service);

	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service_manager);
	
	int WASABICALL FeaturesSystemCallback_OnPermissionsChanged();
};

static bool gracenote_registered=false;
static GracenoteComponent gracenote_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_syscb *WASABI2_API_SYSCB=0;
api_metadata *REPLICANT_API_METADATA=0;
api_decode *REPLICANT_API_DECODE=0;

int GracenoteComponent::Component_Initialize(api_service *service)
{
	WASABI2_API_SVC = service;

	WASABI2_API_SVC->GetService(&WASABI2_API_APP);
	WASABI2_API_SVC->GetService(&WASABI2_API_SYSCB);
	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);

	if (REPLICANT_API_METADATA)
	{
		nx_string_t metadata_key;
		if (NXStringCreateWithUTF8(&metadata_key, "GracenoteFileID") == NErr_Success)
		{ 
			REPLICANT_API_METADATA->RegisterField(metadata_key, &MetadataKey_GracenoteFileID);
			NXStringRelease(metadata_key);
		}

		if (NXStringCreateWithUTF8(&metadata_key, "GracenoteExtData") == NErr_Success)
		{ 
			REPLICANT_API_METADATA->RegisterField(metadata_key, &MetadataKey_GracenoteExtData);
			NXStringRelease(metadata_key);
		}

		if (NXStringCreateWithUTF8(&metadata_key, "MatchedTrack") == NErr_Success)
		{ 
			REPLICANT_API_METADATA->RegisterField(metadata_key, &MetadataKey_MatchedTrack);
			NXStringRelease(metadata_key);
		}

		if (NXStringCreateWithUTF8(&metadata_key, "MatchScore") == NErr_Success)
		{ 
			REPLICANT_API_METADATA->RegisterField(metadata_key, &MetadataKey_MatchScore);
			NXStringRelease(metadata_key);
		}
	}

	return NErr_Success;
}

int GracenoteComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	
	WASABI2_API_SVC->GetService(&REPLICANT_API_DECODE);
	
	if (WASABI2_API_SYSCB)
		WASABI2_API_SYSCB->RegisterCallback((Features::SystemCallback *)this);
	
	if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::gracenote_autotag) == NErr_True)
	{
		gracenote_factory.Register(WASABI2_API_SVC, REPLICANT_API_GRACENOTE);
		metadata_factory.Register(WASABI2_API_SVC);
		gracenote_registered=true;
	}



	return NErr_Success;
}

void GracenoteComponent::Component_DeregisterServices(api_service *service)
{
	if (WASABI2_API_SYSCB)
		WASABI2_API_SYSCB->UnregisterCallback((Features::SystemCallback *)this);
	
	if (gracenote_registered)
	{
		gracenote_factory.Deregister(WASABI2_API_SVC);
		//playback_factory.Deregister(WASABI2_API_SVC);
		metadata_factory.Deregister(WASABI2_API_SVC);
//		decoder_factory.Deregister(WASABI2_API_SVC);
		gracenote_registered=false;
	}
	

	if (REPLICANT_API_DECODE)
		REPLICANT_API_DECODE->Release();
	REPLICANT_API_DECODE=0;
}

int GracenoteComponent::Component_Quit(api_service *service_manager)
{
	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
	WASABI2_API_APP=0;
	if (WASABI2_API_SYSCB)
		WASABI2_API_SYSCB->Release();
	WASABI2_API_SYSCB=0;
	if (REPLICANT_API_METADATA)
		REPLICANT_API_METADATA->Release();
	REPLICANT_API_METADATA=0;
	return NErr_Success;
}

int GracenoteComponent::FeaturesSystemCallback_OnPermissionsChanged()
{
	if (gracenote_registered)
	{
		if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::gracenote_autotag) != NErr_True)
		{
			gracenote_factory.Deregister(WASABI2_API_SVC);
			//playback_factory.Deregister(WASABI2_API_SVC);
			metadata_factory.Deregister(WASABI2_API_SVC);
			//decoder_factory.Deregister(WASABI2_API_SVC);
			gracenote_registered=false;
		}
	}
	else
	{
		if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::gracenote_autotag) == NErr_True)
		{
			gracenote_factory.Register(WASABI2_API_SVC, REPLICANT_API_GRACENOTE);
			//playback_factory.Register(WASABI2_API_SVC);
			metadata_factory.Register(WASABI2_API_SVC);
			//decoder_factory.Register(WASABI2_API_SVC);
			gracenote_registered=true;
		}
	}
	
	return NErr_Success;
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &gracenote_component;
}
