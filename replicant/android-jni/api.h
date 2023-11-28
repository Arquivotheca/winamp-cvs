#pragma once
#include "Wasabi/api.h"

#include "Application.h"
#define WASABI2_API_APP (&application)
#define WASABI2_API_ANDROID (&android_api)

#include "metadata/api_metadata.h"
extern api_metadata *metadata_api;
#define REPLICANT_API_METADATA metadata_api

#include "metadata/api_artwork.h"
extern api_artwork *artwork_api;
#define REPLICANT_API_ARTWORK artwork_api

#include "gracenote/api_gracenote.h"
extern api_gracenote *gracenote_api;
#define REPLICANT_API_GRACENOTE gracenote_api

#include "cloud/api_cloud.h"
extern api_cloud *cloud_api;
#define REPLICANT_API_CLOUD cloud_api

#include "media-server/api_mediaserver.h"
extern api_mediaserver *media_serverApi;
#define REPLICANT_API_MEDIASERVER media_serverApi
