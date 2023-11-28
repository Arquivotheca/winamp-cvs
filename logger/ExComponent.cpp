#include "ExComponent.h"
#include "api.h"
#include <api/service/api_service.h> // Service Manager is central to Wasabi
#include "LoggerFactory.h" // the Service Factory we're going to regsister

api_service *serviceManager=0;
api_application *WASABI_API_APP=0;

// the service factory we're going to register
static LoggerFactory loggerFactory;

void ExComponent::RegisterServices(api_service *service)
{
    WASABI_API_SVC = service;
	// If we need any services, we can retrieve them here
	// however, you have no guarantee that a service you want will be active yet
	// so it's best to "lazy load" and get it the first time you need it

	// Register any services we provide here
	service->service_register(&loggerFactory);
}

void ExComponent::DeregisterServices(api_service *service)
{
	// Unregister our services
	service->service_deregister(&loggerFactory);

	// And release any services we retrieved
}

// Define the dispatch table
#define CBCLASS ExComponent
START_DISPATCH;
VCB(API_WA5COMPONENT_REGISTERSERVICES, RegisterServices)
VCB(API_WA5COMPONENT_DEREEGISTERSERVICES, DeregisterServices)
END_DISPATCH;
#undef CBCLASS
