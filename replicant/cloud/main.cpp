#include "api.h"
#include "CloudAPI.h"
#include "foundation/export.h"
#include "component/ifc_component.h"
#include "nx/nxuri.h"
#include "CloudThread.h"
#include "nswasabi/ReferenceCounted.h"
#include "nswasabi/singleton.h"


int MetadataKey_CloudID = -1;//;
int MetadataKey_CloudIDHash = -1;//"cloud/idhash";
int MetadataKey_CloudMetaHash = -1;//"cloud/metahash";
int MetadataKey_CloudMediaHash = -1;//"cloud/mediahash";
int MetadataKey_CloudAlbumHash = -1;//"cloud/albumhash";
int MetadataKey_CloudDevice = -1;//"cloud/device";
int MetadataKey_CloudArtHashAlbum = -1;

api_service *WASABI2_API_SVC=0;
api_application *WASABI2_API_APP=0;
api_syscb *WASABI2_API_SYSCB=0;
api_ssdp *REPLICANT_API_SSDP=0;
api_metadata *REPLICANT_API_METADATA=0;
api_artwork *REPLICANT_API_ARTWORK=0;

#ifdef __ANDROID__
api_android *WASABI2_API_ANDROID=0;
#endif

CloudAPI cloud_api;
static SingletonServiceFactory<CloudAPI, api_cloud> cloud_factory;

// {3A63E900-EBCC-42F5-80FA-6C7999DEFD9D}
static const GUID cloud_component_guid = 
{ 0x3a63e900, 0xebcc, 0x42f5, { 0x80, 0xfa, 0x6c, 0x79, 0x99, 0xde, 0xfd, 0x9d } };

class CloudComponent : public ifc_component
{
public:
	CloudComponent() : ifc_component(cloud_component_guid) {}
	int WASABICALL Component_Initialize(api_service *service);
	int WASABICALL Component_RegisterServices(api_service *service);
	int WASABICALL Component_OnLoading(api_service *_service_manager);
	void WASABICALL Component_DeregisterServices(api_service *service);
	int WASABICALL Component_OnClosed(api_service *_service_manager);
	int WASABICALL Component_Quit(api_service *service);
};

int CloudComponent::Component_Initialize(api_service *service)
{
	WASABI2_API_SVC = service;
	int ret = jnl_init();
	if (ret != NErr_Success)
		return ret;

	// TODO: error check
	sqlite3_config(SQLITE_CONFIG_MULTITHREAD);
	sqlite3_config(SQLITE_CONFIG_MEMSTATUS, 0);
	sqlite3_initialize();

	WASABI2_API_SVC->GetService(&REPLICANT_API_METADATA);
	WASABI2_API_SVC->GetService(&REPLICANT_API_ARTWORK);

	if (REPLICANT_API_METADATA)
	{
		ReferenceCountedNXString metadata;
		if (NXStringCreateWithUTF8(&metadata, "cloud/id") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MetadataKey_CloudID);
		if (NXStringCreateWithUTF8(&metadata, "cloud/idhash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MetadataKey_CloudIDHash);
		if (NXStringCreateWithUTF8(&metadata, "cloud/metahash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MetadataKey_CloudMetaHash);
		if (NXStringCreateWithUTF8(&metadata, "cloud/mediahash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MetadataKey_CloudMediaHash);
		if (NXStringCreateWithUTF8(&metadata, "cloud/albumhash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MetadataKey_CloudAlbumHash);
		if (NXStringCreateWithUTF8(&metadata, "cloud/device") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MetadataKey_CloudDevice);
		if (NXStringCreateWithUTF8(&metadata, "cloud/art/album/hash") == NErr_Success)
			REPLICANT_API_METADATA->RegisterField(metadata, &MetadataKey_CloudArtHashAlbum);
	}
	return NErr_Success;
}
	
int CloudComponent::Component_RegisterServices(api_service *service)
{
	WASABI2_API_SVC = service;
	if (WASABI2_API_SVC)
	{
		WASABI2_API_SVC->GetService(&WASABI2_API_APP);
		WASABI2_API_SVC->GetService(&WASABI2_API_SYSCB);

#ifdef __ANDROID__	
		WASABI2_API_SVC->GetService(&WASABI2_API_ANDROID);
#endif
	}

	ReferenceCountedNXURI nx_filename;
	WASABI2_API_APP->GetDataPath(&nx_filename);
#ifdef _WIN32
	Logger::Init(nx_filename, "Cloud\\logs");
#else
	Logger::Init(nx_filename, "cloud-logs");
#endif
	cloud_factory.Register(WASABI2_API_SVC, REPLICANT_API_CLOUD);
	
	return NErr_Success;
}

int CloudComponent::Component_OnLoading(api_service *_service_manager)
{
	WASABI2_API_SVC->GetService(&REPLICANT_API_SSDP);
	return NErr_Success;
}

void CloudComponent::Component_DeregisterServices(api_service *service)
{
	cloud_factory.Deregister(WASABI2_API_SVC);

	if (WASABI2_API_SVC)
		WASABI2_API_SVC->Release();

	if (WASABI2_API_APP)
		WASABI2_API_APP->Release();

	if (WASABI2_API_SYSCB)
		WASABI2_API_SYSCB->Release();

#ifdef __ANDROID__
	if (WASABI2_API_ANDROID)
		WASABI2_API_ANDROID->Release();
#endif	
}

int CloudComponent::Component_OnClosed(api_service *_service_manager)
{
	if (REPLICANT_API_SSDP)
		REPLICANT_API_SSDP->Release();
	REPLICANT_API_SSDP=0;

	return NErr_Success;
}

int CloudComponent::Component_Quit(api_service *_service_manager)
{
	sqlite3_shutdown();
	jnl_quit();
	return NErr_Success;
}

static CloudComponent component;
extern "C" DLLEXPORT ifc_component *GetWasabi2Component()
{
	return &component;
}