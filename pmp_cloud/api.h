#pragma once

#include "../Agave/Agave.h"
#include "cloud/api_cloud.h"

namespace Wasabi2
{
	
	#include "metadata/api_metadata.h"
	#include "metadata/api_artwork.h"
};
DECLARE_EXTERNAL_SERVICE(api_service,         WASABI_API_SVC);
DECLARE_EXTERNAL_SERVICE(api_language,        WASABI_API_LNG);
DECLARE_EXTERNAL_SERVICE(api_application,     WASABI_API_APP);
DECLARE_EXTERNAL_SERVICE(api_mldb,            AGAVE_API_MLDB);
DECLARE_EXTERNAL_SERVICE(api_syscb,           WASABI_API_SYSCB);
DECLARE_EXTERNAL_SERVICE(api_metadata,        AGAVE_API_METADATA);
DECLARE_EXTERNAL_SERVICE(api_config,          AGAVE_API_CONFIG);
DECLARE_EXTERNAL_SERVICE(api_memmgr,          WASABI_API_MEMMGR);
DECLARE_EXTERNAL_SERVICE(api_albumart,        AGAVE_API_ALBUMART);
DECLARE_EXTERNAL_SERVICE(obj_ombrowser,       AGAVE_OBJ_BROWSER);
DECLARE_EXTERNAL_SERVICE(api_threadpool,      AGAVE_API_THREADPOOL);
DECLARE_EXTERNAL_SERVICE(api_cloud,           REPLICANT_API_CLOUD);
DECLARE_EXTERNAL_SERVICE(api_playlistmanager, AGAVE_API_PLAYLISTMANAGER);
DECLARE_EXTERNAL_SERVICE(api_devicemanager,   AGAVE_API_DEVICEMANAGER);
DECLARE_EXTERNAL_SERVICE(Wasabi2::api_metadata,   REPLICANT_API_METADATA);
DECLARE_EXTERNAL_SERVICE(Wasabi2::api_artwork,   REPLICANT_API_ARTWORK);

		

int WasabiInit(api_service *service);
void WasabiQuit();
