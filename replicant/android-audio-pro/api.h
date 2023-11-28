#pragma once
#include "service/api_service.h"
extern api_service *serviceManager;
#define WASABI2_API_SVC serviceManager

#include "application/api_application.h"
extern api_application *applicationApi;
#define WASABI2_API_APP applicationApi

#include "application/api_android.h"
extern api_android *androidApi;
#define WASABI2_API_ANDROID androidApi
