#include "main.h"
#include "api.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "MP4PlaybackService.h"
#include "MP4MetadataService.h"
#include "MP4Decoder.h"
#include "MP4RawReader.h"
#include "MP4HTTP.h"
#include "nx/nxpath.h"

static nx_string_t mp4_extension=0;
static nx_string_t m4a_extension=0;
static nx_string_t m4v_extension=0;
static nx_string_t mov_extension=0;

int MetadataKey_GracenoteFileID=-1;
int MetadataKey_GracenoteExtData=-1;

static SingletonService<MP4PlaybackService, svc_fileplayback> playback_factory;
static SingletonService<MP4MetadataService, svc_metadata> metadata_factory;
static SingletonService<MP4FileMetadataService, svc_filemetadata> file_metadata_factory;
static SingletonService<MP4Decoder, svc_filedecode> decode_factory;
static SingletonService<MP4RawReaderService, svc_filerawreader> raw_reader_factory;
static SingletonService<MP4HTTPService, svc_http_demuxer> http_demuxer_factory;


// {98BB966B-9C0D-4900-BE89-CAD7C3B26664}
static const GUID mp4_component_guid = 
{ 0x98bb966b, 0x9c0d, 0x4900, { 0xbe, 0x89, 0xca, 0xd7, 0xc3, 0xb2, 0x66, 0x64 } };

class MP4Component : public ifc_component
{
public:
	MP4Component() : ifc_component(mp4_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service);

	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_Quit(api_service *service_manager);
};

static MP4Component mp4_component;
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_metadata *REPLICANT_API_METADATA=0;
api_filelock *REPLICANT_API_FILELOCK=0;

bool IsMyExtension(nx_uri_t filename, int search_style)
{
	if (NXPathIsURL(filename) == NErr_True)
		return false;

	if (NXPathMatchExtension(filename, mp4_extension) == NErr_Success
		|| NXPathMatchExtension(filename, m4a_extension) == NErr_Success)
	{
		return true;
	}

	if (search_style == EXTENSION_FOR_METADATA || search_style == EXTENSION_FOR_AUDIO_DECODE)
	{
		if (NXPathMatchExtension(filename, m4v_extension) == NErr_Success
			|| NXPathMatchExtension(filename, mov_extension) == NErr_Success)
		{
			return true;
		}
	}
	return false;
}

int EnumerateExtensions(unsigned int index, nx_string_t *extension, int search_style)
{
	switch(index)
	{
	case 0:
		*extension = NXStringRetain(mp4_extension);
		return NErr_Success;
	case 1:
		*extension = NXStringRetain(m4a_extension);
		return NErr_Success;
	case 2:
		if (search_style == EXTENSION_FOR_METADATA || search_style == EXTENSION_FOR_AUDIO_DECODE)
		{
			*extension = NXStringRetain(m4v_extension);
			return NErr_Success;
		}
		else
			return NErr_False;
	case 3:
		if (search_style == EXTENSION_FOR_METADATA || search_style == EXTENSION_FOR_AUDIO_DECODE)
		{
			*extension = NXStringRetain(mov_extension);
			return NErr_Success;
		}
		else
			return NErr_False;
	}

	return NErr_False;
}

static int InitializeExtensions()
{
	int ret = NXStringCreateWithUTF8(&mp4_extension, "mp4");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&m4a_extension, "m4a");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&m4v_extension, "m4v");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&mov_extension, "mov");
	if (ret != NErr_Success)
		return ret;

	return NErr_Success;
}

int MP4Component::Component_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;

	int ret = InitializeExtensions();
	if (ret != NErr_Success)
	{
		NXStringRelease(mp4_extension);
		mp4_extension=0;
		NXStringRelease(m4a_extension);
		m4a_extension=0;
		NXStringRelease(m4v_extension);
		m4v_extension=0;
		NXStringRelease(mov_extension);
		mov_extension=0;
		return ret;
	}

	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);
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

int MP4Component::Component_RegisterServices(api_service *service)
{
	playback_factory.Register(WASABI2_API_SVC);
	metadata_factory.Register(WASABI2_API_SVC);
	file_metadata_factory.Register(WASABI2_API_SVC);
	decode_factory.Register(WASABI2_API_SVC);
	raw_reader_factory.Register(WASABI2_API_SVC);
	http_demuxer_factory.Register(WASABI2_API_SVC);
	return NErr_Success;
}

void MP4Component::Component_DeregisterServices(api_service *service)
{
	playback_factory.Deregister(WASABI2_API_SVC);
	metadata_factory.Deregister(WASABI2_API_SVC);
	file_metadata_factory.Deregister(WASABI2_API_SVC);
	decode_factory.Deregister(WASABI2_API_SVC);
	raw_reader_factory.Deregister(WASABI2_API_SVC);
	http_demuxer_factory.Deregister(WASABI2_API_SVC);
}

int MP4Component::Component_Quit(api_service *service_manager)
{
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
	return &mp4_component;
}
