#pragma once
#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "../Agave/Language/api_language.h"

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memoryManager;
#define WASABI_API_MEMMGR memoryManager

#include "../Agave/AlbumArt/api_albumart.h"
extern api_albumart *albumArtApi;
#define AGAVE_API_ALBUMART albumArtApi

#include "../Agave/Config/api_config.h"
extern api_config *agaveConfigApi;
#define AGAVE_API_CONFIG agaveConfigApi

void WasabiInit(api_service *service);
void WasabiQuit();
