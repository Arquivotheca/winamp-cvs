#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "application/features.h"
#include "FLACPlaybackService.h"
#include "FLACMetadataService.h"
#include "FLACDecoder.h"
#include "FLACRawReader.h"
#include "main.h"
#include "FLACHTTP.h"

nx_string_t flac_extension=0;

int MetadataKey_GracenoteFileID=-1;
int MetadataKey_GracenoteExtData=-1;

static SingletonService<FLACPlaybackService, svc_fileplayback> playback_factory;
static SingletonService<FLACMetadataService, svc_filemetadata> metadata_factory;
static SingletonService<FLACDecoder, svc_filedecode> decoder_factory;
static SingletonService<FLACRawReaderService, svc_filerawreader> raw_reader_factory;
static SingletonService<FLACHTTPService, svc_http_demuxer> http_demuxer_factory;

// {8ED3898A-55E3-4E8E-825F-BF0CD255FBC4}
static const GUID flac_component_guid = 
{ 0x8ed3898a, 0x55e3, 0x4e8e, { 0x82, 0x5f, 0xbf, 0xc, 0xd2, 0x55, 0xfb, 0xc4 } };

class FLACComponent : public ifc_component, public Features::SystemCallback
{
public:
	FLACComponent() : ifc_component(flac_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service_manager);

	int WASABICALL FeaturesSystemCallback_OnPermissionsChanged();

};

static bool flac_registered=false;
static FLACComponent flac_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_syscb *WASABI2_API_SYSCB=0;
api_metadata *REPLICANT_API_METADATA=0;
api_filelock *REPLICANT_API_FILELOCK=0;

int FLACComponent::Component_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;
	int ret = NXStringCreateWithUTF8(&flac_extension, "flac");
	if (ret != NErr_Success)
		return ret;

	WASABI2_API_SVC->GetService(&WASABI2_API_APP);
	WASABI2_API_SVC->GetService(&WASABI2_API_SYSCB);
	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);
	WASABI2_API_SVC->GetService(&REPLICANT_API_FILELOCK);

	if (WASABI2_API_SYSCB)
		WASABI2_API_SYSCB->RegisterCallback((Features::SystemCallback *)this);

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
	}

	return NErr_Success;
}

int FLACComponent::Component_RegisterServices(api_service *service)
{
	if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::flac_playback) == NErr_True)
	{
		playback_factory.Register(WASABI2_API_SVC);
		metadata_factory.Register(WASABI2_API_SVC);
		decoder_factory.Register(WASABI2_API_SVC);
		raw_reader_factory.Register(WASABI2_API_SVC);
		http_demuxer_factory.Register(WASABI2_API_SVC);
		flac_registered=true;
	}
	return NErr_Success;
}

void FLACComponent::Component_DeregisterServices(api_service *service)
{

	if (flac_registered)
	{
		playback_factory.Deregister(WASABI2_API_SVC);
		metadata_factory.Deregister(WASABI2_API_SVC);
		decoder_factory.Deregister(WASABI2_API_SVC);
		raw_reader_factory.Deregister(WASABI2_API_SVC);
		http_demuxer_factory.Deregister(WASABI2_API_SVC);
		flac_registered=false;
	}
}

int FLACComponent::FeaturesSystemCallback_OnPermissionsChanged()
{
	if (flac_registered)
	{
		if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::flac_playback) != NErr_True)
		{
			playback_factory.Deregister(WASABI2_API_SVC);
			metadata_factory.Deregister(WASABI2_API_SVC);
			decoder_factory.Deregister(WASABI2_API_SVC);
			raw_reader_factory.Deregister(WASABI2_API_SVC);
			http_demuxer_factory.Deregister(WASABI2_API_SVC);
			flac_registered=false;
		}
	}
	else
	{
		if (WASABI2_API_APP && WASABI2_API_APP->GetPermission(Features::flac_playback) == NErr_True)
		{
			playback_factory.Register(WASABI2_API_SVC);
			metadata_factory.Register(WASABI2_API_SVC);
			decoder_factory.Register(WASABI2_API_SVC);
			raw_reader_factory.Register(WASABI2_API_SVC);
			http_demuxer_factory.Register(WASABI2_API_SVC);
			flac_registered=true;
		}
	}

	return NErr_Success;
}

int FLACComponent::Component_Quit(api_service *service_manager)
{
	NXStringRelease(flac_extension);
	flac_extension=0;
	if (WASABI2_API_SYSCB)
	{
		WASABI2_API_SYSCB->UnregisterCallback((Features::SystemCallback *)this);
		WASABI2_API_SYSCB->Release();
	}

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();
	WASABI2_API_APP=0;

	if (REPLICANT_API_METADATA)
		REPLICANT_API_METADATA->Release();
	REPLICANT_API_METADATA=0;

	if (REPLICANT_API_FILELOCK)
		REPLICANT_API_FILELOCK->Release();
	REPLICANT_API_FILELOCK=0;

	return NErr_Success;
}

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &flac_component;
}
