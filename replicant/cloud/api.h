#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "foundation/types.h"


#include "service/api_service.h"
extern api_service *serviceManager;
#define WASABI2_API_SVC serviceManager

#include "application/api_application.h"
extern api_application *applicationApi;
#define WASABI2_API_APP applicationApi

#include "syscb/api_syscb.h"
extern api_syscb *syscbApi;
#define WASABI2_API_SYSCB syscbApi

#ifdef __ANDROID__
#include "application/api_android.h"
extern api_android *androidApi;
#define WASABI2_API_ANDROID androidApi
#endif // #__ANDROID__

#include "ssdp/api_ssdp.h"
extern api_ssdp *ssdpApi;
#define REPLICANT_API_SSDP ssdpApi

#include "metadata/api_metadata.h"
extern api_metadata *metadataApi;
#define REPLICANT_API_METADATA metadataApi

#include "metadata/api_artwork.h"
extern api_artwork *artworkApi;
#define REPLICANT_API_ARTWORK artworkApi

//extern CloudAPI cloud_api;
#define REPLICANT_API_CLOUD (&cloud_api)