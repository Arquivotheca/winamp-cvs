#include "./main.h"
#include "./wasabiApi.h"
#include <api/service/waServiceFactory.h>

api_service *WASABI_API_SVC = NULL;
api_application *WASABI_API_APP = NULL;
api_mldb *mldbApi = NULL;
api_metadata *metaDataApi = NULL;
api_tagz *tagzApi = NULL;
api_memmgr *memManagerApi = NULL;
svc_imageLoader *pngLoaderApi = NULL;
api_playlistmanager *playlistManagerApi = NULL;
api_syscb *sysCallbackApi = NULL;



api_language *WASABI_API_LNG = NULL;
HINSTANCE WASABI_API_LNG_HINST = NULL, WASABI_API_ORIG_HINST = NULL;

// {E09866DC-0E6C-416c-8920-FB65A562628D}
static const GUID GenDropBoxLangGUID = 
{ 0xe09866dc, 0xe6c, 0x416c, { 0x89, 0x20, 0xfb, 0x65, 0xa5, 0x62, 0x62, 0x8d } };


EXTERN_C const GUID pngLoaderGUID = 
{ 0x5e04fb28, 0x53f5, 0x4032, { 0xbd, 0x29, 0x3, 0x2b, 0x87, 0xec, 0x37, 0x25 } };

void *Wasabi_QueryInterface(REFGUID interfaceGuid)
{
	waServiceFactory *serviceFactory = WASABI_API_SVC->service_getServiceByGuid(interfaceGuid);
	return (NULL != serviceFactory) ? serviceFactory->getInterface() : NULL;
}

void Wasabi_ReleaseInterface(REFGUID interfaceGuid, void *pInstance)
{
	waServiceFactory *serviceFactory = WASABI_API_SVC->service_getServiceByGuid(interfaceGuid);
	if (NULL != serviceFactory) serviceFactory->releaseInterface(pInstance);
}


BOOL InitializeWasabiApi()
{
	WASABI_API_SVC = (api_service*)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if ((api_service*)1 == WASABI_API_SVC ) WASABI_API_SVC = NULL;
	if (NULL == WASABI_API_SVC) return FALSE;
	
	WASABI_API_APP = QueryWasabiInterface(api_application, applicationApiServiceGuid);
	WASABI_API_LNG = QueryWasabiInterface(api_language, languageApiGUID);
	WASABI_API_SYSCB = QueryWasabiInterface(api_syscb, syscbApiServiceGuid);
	
	if (NULL != WASABI_API_LNG)
		WASABI_API_START_LANG(plugin.hDllInstance, GenDropBoxLangGUID);
	
	return TRUE;
}


void UninitializeWasabiApi()
{
	if (NULL != WASABI_API_SVC)
	{	
		ReleaseWasabiInterface(applicationApiServiceGuid, WASABI_API_APP);
		ReleaseWasabiInterface(languageApiGUID, WASABI_API_LNG);
		ReleaseWasabiInterface(api_metadataGUID, AGAVE_API_METADATA);
		ReleaseWasabiInterface(mldbApiGuid, AGAVE_API_MLDB);
		ReleaseWasabiInterface(tagzGUID, AGAVE_API_TAGZ);
		ReleaseWasabiInterface(memMgrApiServiceGuid, WASABI_API_MEMMNGR);
		ReleaseWasabiInterface(pngLoaderGUID, WASABI_API_PNGLOADER);
		ReleaseWasabiInterface(api_playlistmanagerGUID, WASABI_API_PLAYLISTMNGR);
		ReleaseWasabiInterface(syscbApiServiceGuid, WASABI_API_SYSCB);
	}

	WASABI_API_APP = NULL;
	WASABI_API_LNG = NULL;
	AGAVE_API_METADATA = NULL;
	AGAVE_API_MLDB = NULL;
	AGAVE_API_TAGZ = NULL;
	WASABI_API_MEMMNGR = NULL;
	WASABI_API_PNGLOADER = NULL;
	WASABI_API_PLAYLISTMNGR = NULL;
	WASABI_API_SYSCB = NULL;

}