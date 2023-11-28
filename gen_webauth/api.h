#pragma once

#include <api/service/api_service.h>
extern api_service *serviceManager;
#define WASABI_API_SVC serviceManager

#include <api/application/api_application.h>
extern api_application *applicationApi;
#define WASABI_API_APP applicationApi

#include <../omBrowser/obj_ombrowser.h>
extern obj_ombrowser *browserManager;
#define OMBROWSERMNGR browserManager

void WasabiInit(api_service *service);
void WasabiQuit();
