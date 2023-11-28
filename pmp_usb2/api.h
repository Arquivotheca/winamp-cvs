#pragma once
#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "../Agave/Language/api_language.h"

#include "../Agave/Metadata/api_metadata.h"
extern api_metadata *metadataApi;
#define AGAVE_API_METADATA metadataApi

#include "../playlist/api_playlistmanager.h"
extern api_playlistmanager *playlistManagerApi;
#define WASABI_API_PLAYLISTMNGR playlistManagerApi

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../devices/api_devicemanager.h"
extern api_devicemanager *deviceManagerApi;
#define AGAVE_API_DEVICEMANAGER deviceManagerApi

void WasabiInit(api_service *service);
void WasabiQuit();
