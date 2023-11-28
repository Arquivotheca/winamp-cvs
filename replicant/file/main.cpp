#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "application/features.h"
#include "FilePlayback.h"
#include "FileDecoder.h"
#include "FileMetadata.h"
#include "FileRawReader.h"
#include "main.h"

int MetadataKey_GracenoteFileID=-1;
int MetadataKey_GracenoteExtData=-1;

DEFINE_EXTERNAL_SERVICE(api_service,          WASABI2_API_SVC);
DEFINE_EXTERNAL_SERVICE(api_application,      WASABI2_API_APP);
DEFINE_EXTERNAL_SERVICE(api_syscb,            WASABI2_API_SYSCB);
DEFINE_EXTERNAL_SERVICE(api_metadata,         REPLICANT_API_METADATA);
DEFINE_EXTERNAL_SERVICE(api_filelock,         REPLICANT_API_FILELOCK);

static SingletonService<FilePlaybackService, svc_playback> playback_factory;
static SingletonService<FileMetadataService, svc_metadata> metadata_factory;
static SingletonService<FileDecoderService, svc_decode> decoder_factory;
static SingletonService<FileRawReaderService, svc_raw_media_reader> raw_reader_factory;

// {226C9B92-B8C3-4EFD-B31E-1D1E090B0ACA}
static const GUID file_component_guid = 
{ 0x226c9b92, 0xb8c3, 0x4efd, { 0xb3, 0x1e, 0x1d, 0x1e, 0x9, 0xb, 0xa, 0xca } };

class FileComponent : public ifc_component, public Features::SystemCallback
{
public:
	FileComponent() : ifc_component(file_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service_manager);

	int WASABICALL FeaturesSystemCallback_OnPermissionsChanged();

};

static FileComponent file_component;

int FileComponent::Component_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;

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

	ID3v2Metadata::Initialize(REPLICANT_API_METADATA);
	ID3v1Metadata::Initialize(REPLICANT_API_METADATA);
	APEv2Metadata::Initialize(REPLICANT_API_METADATA);

	return NErr_Success;
}

int FileComponent::Component_RegisterServices(api_service *service)
{

	playback_factory.Register(WASABI2_API_SVC);
	metadata_factory.Register(WASABI2_API_SVC);
	decoder_factory.Register(WASABI2_API_SVC);
	raw_reader_factory.Register(WASABI2_API_SVC);

	return NErr_Success;
}

void FileComponent::Component_DeregisterServices(api_service *service)
{
	playback_factory.Deregister(WASABI2_API_SVC);
	metadata_factory.Deregister(WASABI2_API_SVC);
	decoder_factory.Deregister(WASABI2_API_SVC);
	raw_reader_factory.Deregister(WASABI2_API_SVC);
}

int FileComponent::FeaturesSystemCallback_OnPermissionsChanged()
{
	// TODO: rebuild extension list
	
	return NErr_Success;
}

int FileComponent::Component_Quit(api_service *service_manager)
{
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
	return &file_component;
}
