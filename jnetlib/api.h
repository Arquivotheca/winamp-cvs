#ifndef NULLSOFT_APIH
#define NULLSOFT_APIH

#include "netinc.h"
#include "asyncdns.h"
JNL_AsyncDNS *GetGlobalDNS();
void DestroyGlobalDNS();

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#endif