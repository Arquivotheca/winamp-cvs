#pragma once

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "../ml_local/api_mldb.h"
extern api_mldb *mldbApi;
#define AGAVE_API_MLDB mldbApi

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include "../Agave/Metadata/api_metadata.h"
extern api_metadata *metadataApi;
#define AGAVE_API_METADATA metadataApi

#include <api/syscb/api_syscb.h>
#define WASABI_API_SYSCB sysCallbackApi

#include "../Agave/Config/api_config.h"
extern api_config *config;
#define AGAVE_API_CONFIG config

#include "../Agave/Language/api_language.h"

#include "../auth/api_auth.h"
extern api_auth *authApi;
#define AGAVE_API_AUTH authApi

#include "../Winamp/JSAPI2_api_security.h"
extern JSAPI2::api_security *jsapi2_security;
#define AGAVE_API_JSAPI2_SECURITY jsapi2_security

void WasabiInit(api_service *service);
void WasabiQuit();
