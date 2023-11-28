#pragma once

#include "../Agave/Agave.h"
#include "../playlist/api_playlists.h"
#include "../replicant/cloud/api_cloud.h"

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

DECLARE_EXTERNAL_SERVICE(api_service,          WASABI_API_SVC);
DECLARE_EXTERNAL_SERVICE(api_language,         WASABI_API_LNG);
//DECLARE_EXTERNAL_SERVICE(api_application,      WASABI_API_APP);
DECLARE_EXTERNAL_SERVICE(api_mldb,             AGAVE_API_MLDB);
DECLARE_EXTERNAL_SERVICE(api_syscb,            WASABI_API_SYSCB);
DECLARE_EXTERNAL_SERVICE(api_metadata,         AGAVE_API_METADATA);
DECLARE_EXTERNAL_SERVICE(api_config,           AGAVE_API_CONFIG);
DECLARE_EXTERNAL_SERVICE(api_memmgr,           WASABI_API_MEMMGR);
DECLARE_EXTERNAL_SERVICE(api_albumart,         AGAVE_API_ALBUMART);
DECLARE_EXTERNAL_SERVICE(JSAPI2::api_security, AGAVE_API_JSAPI2_SECURITY);
DECLARE_EXTERNAL_SERVICE(obj_ombrowser,        AGAVE_OBJ_BROWSER);
DECLARE_EXTERNAL_SERVICE(api_threadpool,       AGAVE_API_THREADPOOL);
DECLARE_EXTERNAL_SERVICE(api_cloud,			   REPLICANT_API_CLOUD);
DECLARE_EXTERNAL_SERVICE(api_playlists,		   AGAVE_API_PLAYLISTS);
DECLARE_EXTERNAL_SERVICE(api_playlistmanager,  AGAVE_API_PLAYLISTMANAGER);

namespace Wasabi2
{
	#include "service/api_service.h"
	#include "application/api_application.h"
	#include "media-server/api_mediaserver.h"
	#include "metadata/api_metadata.h"
	#include "metadata/api_artwork.h"
}

extern Wasabi2::api_service *WASABI2_API_SVC;
extern Wasabi2::api_application *WASABI2_API_APP;
extern Wasabi2::api_mediaserver *REPLICANT_API_MEDIASERVER;
extern Wasabi2::api_metadata *REPLICANT_API_METADATA;
extern Wasabi2::api_artwork *REPLICANT_API_ARTWORK;