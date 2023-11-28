#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "../Agave/Language/api_language.h"

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/ExplorerFindFile/api_explorerfindfile.h"

#endif