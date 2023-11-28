#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "main.h"
#include "WAVPlayback.h"
#include "nx/nxpath.h"
#include "WAVRawReader.h"

static nx_string_t wav_extension=0;
static nx_string_t xa_extension=0;
int MetadataKey_GracenoteFileID=-1;
int MetadataKey_GracenoteExtData=-1;
static SingletonService<WAVPlaybackService, svc_fileplayback> playback_factory;
static SingletonService<WAVRawReaderService, svc_filerawreader> rawreader_factory;

bool IsMyExtension(nx_uri_t filename, int search_style)
{
	if (NXPathIsURL(filename) == NErr_True)
		return false;

	if (NXPathMatchExtension(filename, wav_extension) == NErr_Success
		|| NXPathMatchExtension(filename, xa_extension) == NErr_Success)
	{
		return true;
	}

	return false;
}

int EnumerateExtensions(unsigned int index, nx_string_t *extension, int search_style)
{
	switch(index)
	{
	case 0:
		*extension = NXStringRetain(wav_extension);
		return NErr_Success;
	case 1:
		*extension = NXStringRetain(xa_extension);
		return NErr_Success;
	}
	return NErr_False;
}

// {8537F867-10FC-486C-8577-70E09182DD57}
static const GUID wav_component_guid = 
{ 0x8537f867, 0x10fc, 0x486c, { 0x85, 0x77, 0x70, 0xe0, 0x91, 0x82, 0xdd, 0x57 } };

class WAVComponent : public ifc_component
{
public:
	WAVComponent() : ifc_component(wav_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service_manager);
};

static WAVComponent wav_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_syscb *WASABI2_API_SYSCB=0;
api_metadata *REPLICANT_API_METADATA=0;
api_filelock *REPLICANT_API_FILELOCK=0;

static int InitializeExtensions()
{
	int ret = NXStringCreateWithUTF8(&wav_extension, "wav");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&xa_extension, "xa");
	if (ret != NErr_Success)
		return ret;

	return NErr_Success;
}


int WAVComponent::Component_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;
	int ret = InitializeExtensions();
	if (ret != NErr_Success)
	{
		NXStringRelease(wav_extension);
		wav_extension=0;
		NXStringRelease(xa_extension);
		xa_extension=0;
		return ret;
	}

	WASABI2_API_SVC->GetService(&WASABI2_API_APP);
	WASABI2_API_SVC->GetService(&WASABI2_API_SYSCB);
	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);
	WASABI2_API_SVC->GetService(&REPLICANT_API_FILELOCK);

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

int WAVComponent::Component_RegisterServices(api_service *service)
{
		playback_factory.Register(WASABI2_API_SVC);
		rawreader_factory.Register(WASABI2_API_SVC);
//		metadata_factory.Register(WASABI2_API_SVC);
//		decoder_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void WAVComponent::Component_DeregisterServices(api_service *service)
{
		playback_factory.Deregister(WASABI2_API_SVC);
		rawreader_factory.Deregister(WASABI2_API_SVC);
//		metadata_factory.Deregister(WASABI2_API_SVC);
//		decoder_factory.Deregister(WASABI2_API_SVC);
}

int WAVComponent::Component_Quit(api_service *service_manager)
{
	if (WASABI2_API_SYSCB)
		WASABI2_API_SYSCB->Release();
	WASABI2_API_SYSCB=0;

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
	return &wav_component;
}
