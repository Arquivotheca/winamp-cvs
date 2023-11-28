#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <foundation/types.h>

#include "service/api_service.h"
extern api_service *serviceManager;
#define WASABI2_API_SVC serviceManager

#include "application/api_application.h"
extern api_application *applicationApi;
#define WASABI2_API_APP applicationApi

#include "cloud/api_cloud.h"
extern api_cloud *cloudApi;
#define REPLICANT_API_CLOUD cloudApi

#include "ssdp/api_ssdp.h"
extern api_ssdp *ssdpApi;
#define REPLICANT_API_SSDP ssdpApi

#include "metadata/api_metadata.h"
extern api_metadata *metadata_api;
#define REPLICANT_API_METADATA metadata_api

#include "media-server.h"
extern MediaServer media_server;
#define REPLICANT_API_MEDIASERVER (&media_server)