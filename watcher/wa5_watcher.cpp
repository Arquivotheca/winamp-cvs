#include <windows.h>
#include "api.h"
#include "wa5_watcher.h"
#include "factory_watcher.h"
#include "factory_watchfilter.h"
#include "../nu/nonewthrow.c"
WA5_WATCHER wa5_watcher;
WatcherFactory watcherFactory;
WatchFilterFactory watchfilterFactory;

api_service *serviceManager=0;


void WA5_WATCHER::RegisterServices(api_service *service)
{
	WASABI_API_SVC = service;
	WASABI_API_SVC->service_register(&watchfilterFactory);
	WASABI_API_SVC->service_register(&watcherFactory);
}

void WA5_WATCHER::DeregisterServices(api_service *service)
{
	service->service_deregister(&watchfilterFactory);
	service->service_deregister(&watcherFactory);
}

extern "C" __declspec(dllexport) api_wa5component *GetWinamp5SystemComponent()
{
	return &wa5_watcher;
}

#ifdef CBCLASS
#undef CBCLASS
#endif

#define CBCLASS WA5_WATCHER
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
