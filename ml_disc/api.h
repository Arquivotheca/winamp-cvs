#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include <api/service/api_service.h>
#define WASABI_API_SVC serviceManager
extern api_service *serviceManager;

#include <api/service/waServiceFactory.h>

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/Language/api_language.h"

#include "../Winamp/api_stats.h"
extern api_stats *statsApi;
#define AGAVE_API_STATS statsApi

#endif