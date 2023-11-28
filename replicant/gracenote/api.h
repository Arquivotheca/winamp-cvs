#pragma once
#include "service/api_service.h"
extern api_service *serviceManager;
#define WASABI2_API_SVC serviceManager

#include "application/api_application.h"
extern api_application *applicationApi;
#define WASABI2_API_APP applicationApi

#include "syscb/api_syscb.h"
extern api_syscb *syscbApi;
#define WASABI2_API_SYSCB syscbApi

#include "decode/api_decode.h"
extern api_decode *decode_api;
#define REPLICANT_API_DECODE decode_api

#include "metadata/api_metadata.h"
extern api_metadata *metadata_api;
#define REPLICANT_API_METADATA metadata_api

#include "Gracenote.h"
extern GracenoteAPI gracenote_api;
#define REPLICANT_API_GRACENOTE (&gracenote_api)