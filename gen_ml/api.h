#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include "../Agave/Agave.h"
#include <api/wnd/api_wnd.h>
#include <api/skin/api_skin.h>
#include <api/skin/api_palette.h>
#include "../Winamp/api_decodefile.h"
#include "../gracenote/api_gracenote.h"

DECLARE_EXTERNAL_SERVICE(api_service,          WASABI_API_SVC);
DECLARE_EXTERNAL_SERVICE(api_syscb,            WASABI_API_SYSCB);
DECLARE_EXTERNAL_SERVICE(api_language,         WASABI_API_LNG);
DECLARE_EXTERNAL_SERVICE(api_application,      WASABI_API_APP);
DECLARE_EXTERNAL_SERVICE(api_mldb,             AGAVE_API_MLDB);
DECLARE_EXTERNAL_SERVICE(api_threadpool,       AGAVE_API_THREADPOOL);
DECLARE_EXTERNAL_SERVICE(obj_ombrowser,        AGAVE_OBJ_BROWSER);
DECLARE_EXTERNAL_SERVICE(api_decodefile,       AGAVE_API_DECODE);
DECLARE_EXTERNAL_SERVICE(wnd_api,              WASABI_API_WND);
DECLARE_EXTERNAL_SERVICE(api_skin,             WASABI_API_SKIN);
DECLARE_EXTERNAL_SERVICE(api_config,           AGAVE_API_CONFIG);
DECLARE_EXTERNAL_SERVICE(api_palette,          WASABI_API_PALETTE);
DECLARE_EXTERNAL_SERVICE(api_gracenote,        AGAVE_API_GRACENOTE);
DECLARE_EXTERNAL_SERVICE(JSAPI2::api_security, AGAVE_API_JSAPI2_SECURITY);

#endif