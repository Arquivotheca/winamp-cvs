#ifndef NULLOSFT_DROPBOX_PLUGIN_WASABI_API_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_WASABI_API_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../Wasabi/api/service/api_service.h"
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "../Wasabi/api/application/api_application.h"
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Agave/Language/api_language.h"

#include "../ml_local/api_mldb.h"
extern api_mldb *mldbApi;
#define AGAVE_API_MLDB mldbApi

#ifdef _DEBUG
#pragma warning( push )
#pragma warning( disable : 4267 )
#endif

#include "../Agave/Metadata/api_metadata.h"
extern api_metadata *metaDataApi;
#define AGAVE_API_METADATA metaDataApi

#ifdef _DEBUG
#pragma warning( pop )
#endif

#include "../Tagz/api_tagz.h"
extern api_tagz *tagzApi;
#define AGAVE_API_TAGZ tagzApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memManagerApi;
#define WASABI_API_MEMMNGR memManagerApi

#include <api/service/svcs/svc_imgload.h>
extern svc_imageLoader *pngLoaderApi;
#define WASABI_API_PNGLOADER pngLoaderApi
EXTERN_C const GUID pngLoaderGUID;

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManagerApi;
#define WASABI_API_PLAYLISTMNGR playlistManagerApi

#include <api/syscb/api_syscb.h>
extern api_syscb *sysCallbackApi;
#define WASABI_API_SYSCB sysCallbackApi


BOOL InitializeWasabiApi(void);
void UninitializeWasabiApi(void);

void *Wasabi_QueryInterface(REFGUID interfaceGuid);
void Wasabi_ReleaseInterface(REFGUID interfaceGuid, void *pInstance);

#define QueryWasabiInterface(__interfaceType, __interfaceGuid) ((##__interfaceType*)Wasabi_QueryInterface(__interfaceGuid))
#define ReleaseWasabiInterface(__interfaceGuid, __interfaceInstance) (Wasabi_ReleaseInterface((__interfaceGuid), (__interfaceInstance)))

#endif // NULLOSFT_DROPBOX_PLUGIN_WASABI_API_HEADER
