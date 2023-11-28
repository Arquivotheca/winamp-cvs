#pragma once
#include <api/service/api_service.h>

extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "../Agave/Config/api_config.h"
extern api_config *AGAVE_API_CONFIG;

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/Language/api_language.h"
