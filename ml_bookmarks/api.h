#ifndef NULLSOFT_API_H
#define NULLSOFT_API_H

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/service/waServiceFactory.h>

#include "../Agave/Language/api_language.h"

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#endif