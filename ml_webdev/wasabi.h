#ifndef NULLSOFT_WEBDEV_PLUGIN_WASABI_HEADER
#define NULLSOFT_WEBDEV_PLUGIN_WASABI_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>

#include <api/service/api_service.h>
extern api_service *wasabiManager;
#define WASABI_API_SVC wasabiManager

#include <api/application/api_application.h>
#define WASABI_API_APP applicationApi

#include "../Agave/Language/api_language.h"

#include "../Winamp/JSAPI2_api_security.h"
extern JSAPI2::api_security *jsapi2_securityApi;
#define AGAVE_API_JSAPI2_SECURITY jsapi2_securityApi

#include <obj_ombrowser.h>
extern obj_ombrowser *browserManager;
#define OMBROWSERMNGR browserManager

#include <ifc_omservicemanager.h>
extern ifc_omservicemanager *serviceManager;
#define OMSERVICEMNGR serviceManager

#include <ifc_omutility.h>
extern ifc_omutility *omUtility;
#define OMUTILITY omUtility

BOOL WasabiApi_Initialize(HINSTANCE hInstance, api_service *serviceApi);
ULONG WasabiApi_AddRef(void);
ULONG WasabiApi_Release(void);

void *Wasabi_QueryInterface(REFGUID interfaceGuid);
void Wasabi_ReleaseInterface(REFGUID interfaceGuid, void *pInstance);

#define QueryWasabiInterface(__interfaceType, __interfaceGuid) ((##__interfaceType*)Wasabi_QueryInterface(__interfaceGuid))
#define ReleaseWasabiInterface(__interfaceGuid, __interfaceInstance) (Wasabi_ReleaseInterface((__interfaceGuid), (__interfaceInstance)))

void Wasabi_SafeRelease(Dispatchable *pDisp);

#endif // NULLSOFT_WEBDEV_PLUGIN_WASABI_HEADER
