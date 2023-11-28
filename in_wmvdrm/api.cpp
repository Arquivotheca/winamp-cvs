#include "main.h"
#include "api.h"
#include <windows.h>
#include "../Winamp/wa_ipc.h"
#include "FactoryHelper.h"
#include "MetaTagFactory.h"
#include "factory_Handler.h"
#include "AlbumArt.h"
#include "RawReader.h"
#include "../nu/Singleton.h"
MetaTagFactory metaTagFactory;

api_service *serviceManager = 0;
api_playlistmanager *playlistManager=0;
api_config *AGAVE_API_CONFIG=0;
api_application *applicationApi=0;

// wasabi based services for localisation support
api_language *WASABI_API_LNG = 0;
HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;
api_memmgr *WASABI_API_MEMMGR = 0;
WPLHandlerFactory wplHandlerFactory;
ASXHandlerFactory asxHandlerFactory;
AlbumArtFactory albumArtFactory;
static RawMediaReaderService raw_media_reader_service;
static SingletonServiceFactory<svc_raw_media_reader, RawMediaReaderService> raw_factory;

void LoadWasabi(HWND winamp)
{
	if (winamp)
	{
		WASABI_API_SVC = (api_service *)SendMessage(winamp, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
		if (WASABI_API_SVC == (api_service *)1)
			WASABI_API_SVC = 0;
		ServiceBuild(AGAVE_API_CONFIG, AgaveConfigGUID);
		ServiceBuild(playlistManager, api_playlistmanagerGUID);
		ServiceBuild(WASABI_API_APP, applicationApiServiceGuid);
		ServiceBuild(WASABI_API_LNG, languageApiGUID);
		ServiceBuild(WASABI_API_MEMMGR, memMgrApiServiceGuid);
		WASABI_API_SVC->service_register(&metaTagFactory);
		WASABI_API_SVC->service_register(&wplHandlerFactory);
		WASABI_API_SVC->service_register(&asxHandlerFactory);
		WASABI_API_SVC->service_register(&albumArtFactory);
		raw_factory.Register(WASABI_API_SVC, &raw_media_reader_service);
	}
}

void UnloadWasabi()
{
	WASABI_API_SVC->service_deregister(&metaTagFactory);
	WASABI_API_SVC->service_deregister(&wplHandlerFactory);
	WASABI_API_SVC->service_deregister(&asxHandlerFactory);
	WASABI_API_SVC->service_deregister(&albumArtFactory);
	WASABI_API_SVC->service_deregister(&raw_factory);
	ServiceRelease(playlistManager, api_playlistmanagerGUID);
	ServiceRelease(AGAVE_API_CONFIG, AgaveConfigGUID);
	ServiceRelease(WASABI_API_APP, applicationApiServiceGuid);
	ServiceRelease(WASABI_API_LNG, languageApiGUID);
	ServiceRelease(WASABI_API_MEMMGR, memMgrApiServiceGuid);
}