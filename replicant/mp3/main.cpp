#include "api.h"
#include "main.h"
#include "component/ifc_component.h"
#include "service/ifc_servicefactory.h"
#include "foundation/export.h"
#include "nswasabi/singleton.h"
#include "MP3PlaybackService.h"
#include "ICYMP3Service.h"
#include "UltravoxMP3Service.h"
#include "MP3HTTPService.h"
#include "MP3RawReader.h"
#include "MP3Decoder.h"
#include "MP3MetadataService.h"
#include "MP4DecoderService.h"
#include "nx/nxpath.h"
#include <new>

static nx_string_t mp3_extension=0;
static nx_string_t mp2_extension=0;
static nx_string_t mp1_extension=0;

bool IsMyExtension(nx_uri_t filename)
{
	if (NXPathIsURL(filename) == NErr_True)
		return false;

	if (NXPathMatchExtension(filename, mp3_extension) == NErr_Success
		|| NXPathMatchExtension(filename, mp2_extension) == NErr_Success
		|| NXPathMatchExtension(filename, mp1_extension) == NErr_Success)
	{
		return true;
	}
	return false;
}

int EnumerateExtensions(unsigned int index, nx_string_t *value)
{
	switch(index)
	{
	case 0:
		*value = NXStringRetain(mp3_extension);
		return NErr_Success;
	case 1:
		*value = NXStringRetain(mp2_extension);
		return NErr_Success;
	case 2:
		*value = NXStringRetain(mp1_extension);
		return NErr_Success;
	}
	return NErr_False;
}

int GetGaps(ifc_metadata *metadata, size_t *pregap, size_t *postgap)
{
	if (!metadata)
		return NErr_Empty;

	int64_t pregap64, postgap64;
	int ret = metadata->GetInteger(MetadataKeys::PREGAP, 0, &pregap64);
	if (ret != NErr_Success)
	{
		*pregap = 529;
		return ret;
	}

	ret = metadata->GetInteger(MetadataKeys::POSTGAP, 0, &postgap64);
	if (ret != NErr_Success)
		return ret;

	*pregap = pregap64 + 529;
	if (postgap64 >= 529)
		*postgap = postgap64-529;
	else
		*postgap=0;

	return NErr_Success;
}

/* Services that we are going to register */
static WasabiServiceFactory *service_factories[] = 
{
#ifndef REPLICANT_NO_ULTRAVOX
	new (std::nothrow) SingletonService<UltravoxMP3Service, svc_ultravox_playback>,
#endif

#ifndef REPLICANT_NO_LOCAL
	new (std::nothrow) SingletonService<MP3PlaybackService, svc_fileplayback>,
	new (std::nothrow) SingletonService<MP3RawReaderService, svc_filerawreader>,
	new (std::nothrow) SingletonService<MP3Decoder, svc_filedecode>,
	new (std::nothrow) SingletonService<MP3MetadataService, svc_filemetadata>,
#endif

#ifndef REPLICANT_NO_ICY
	new (std::nothrow) SingletonService<ICYMP3Service, svc_icy_playback>,
#endif

#ifndef REPLICANT_NO_HTTP
	new (std::nothrow) SingletonService<MP3HTTPService, svc_http_demuxer>,
#endif

#ifndef REPLICANT_NO_MP4
	new (std::nothrow) SingletonService<MP4DecoderService, svc_mp4decoder>,
#endif
};

static const size_t num_factories = sizeof(service_factories)/sizeof(*service_factories);

/* services we need from Wasabi */
api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_metadata *REPLICANT_API_METADATA=0;
api_filelock *REPLICANT_API_FILELOCK=0;

// {40F573F1-6790-4FBF-A3DD-197B8C97AB86}
static const GUID mp3_component_guid = 
{ 0x40f573f1, 0x6790, 0x4fbf, { 0xa3, 0xdd, 0x19, 0x7b, 0x8c, 0x97, 0xab, 0x86 } };

/* Component implementation */
class MP3Component : public ifc_component
{
public:
	MP3Component() : ifc_component(mp3_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service_manager);
	int WASABICALL Component_RegisterServices(api_service *service_manager);
	void WASABICALL Component_DeregisterServices(api_service *service_manager);
	int WASABICALL Component_Quit(api_service *service_manager);
};

static int InitializeExtensions()
{
	int ret = NXStringCreateWithUTF8(&mp3_extension, "mp3");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&mp2_extension, "mp2");
	if (ret != NErr_Success)
		return ret;

	ret = NXStringCreateWithUTF8(&mp1_extension, "mp1");
	if (ret != NErr_Success)
		return ret;

	return NErr_Success;

}
int MP3Component::Component_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;

	int ret = InitializeExtensions();
	if (ret != NErr_Success)
	{
		NXStringRelease(mp3_extension);
		mp3_extension=0;
		NXStringRelease(mp2_extension);
		mp2_extension=0;
		NXStringRelease(mp1_extension);
		mp1_extension=0;
		return ret;
	}

	// get application API
	WASABI2_API_SVC->GetService(&WASABI2_API_APP);
	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);
	WASABI2_API_SVC->GetService(&REPLICANT_API_FILELOCK);

	ID3v2Metadata::Initialize(REPLICANT_API_METADATA);
	

	return NErr_Success;
}

int MP3Component::Component_RegisterServices(api_service *service_manager)
{
	for (size_t i=0;i<num_factories;i++)
	{
		service_factories[i]->Register(WASABI2_API_SVC);
	}

	return NErr_Success;
}

void MP3Component::Component_DeregisterServices(api_service *service)
{
	for (size_t i=0;i<num_factories;i++)
	{
		service_factories[i]->Deregister(WASABI2_API_SVC);
		delete service_factories[i];
	}
}

int MP3Component::Component_Quit(api_service *service_manager)
{
	NXStringRelease(mp3_extension);
	mp3_extension=0;
	NXStringRelease(mp2_extension);
	mp2_extension=0;
	NXStringRelease(mp1_extension);
	mp1_extension=0;

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

static MP3Component mp3_component;

extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &mp3_component;
}
