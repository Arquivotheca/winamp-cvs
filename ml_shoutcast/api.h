#ifndef NULLSOFT_ML_SHOUTCAST_API_H
#define NULLSOFT_ML_SHOUTCAST_API_H

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/Language/api_language.h"

#endif