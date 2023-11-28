#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager


#include "../Agave/Config/api_config.h"
extern api_config *configApi;
#define AGAVE_API_CONFIG configApi

#include "../Agave/Language/api_language.h"

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#endif