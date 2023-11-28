#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH
#include <api/service/api_service.h>

extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include "../Agave/Config/api_config.h"
extern api_config *AGAVE_API_CONFIG;

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include <api/memmgr/api_memmgr.h>
extern api_memmgr *memmgrApi;
#define WASABI_API_MEMMGR memmgrApi

//#include <api/service/waServiceFactory.h>

#include "../Agave/Language/api_language.h"

#include "../dlmgr/api_downloadmanager.h"
extern api_downloadManager *downloadManagerApi;
#define WASABI_API_DLMGR downloadManagerApi

#include "../nu/threadpool/api_threadpool.h"
extern api_threadpool *threadPoolApi;
#define WASABI_API_THREADPOOL threadPoolApi

#endif