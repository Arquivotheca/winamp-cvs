#include "main.h"
#include "./navigation.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./wasabiCallback.h"

#include "../winamp/wa_ipc.h"
#include "handler.h"
#include "../nu/Singleton.h"
#include <strsafe.h>

static AddonsURIHandler uri_handler;
static SingletonServiceFactory<svc_urihandler, AddonsURIHandler> uri_handler_factory;

static SysCallback *wasabiCallback = NULL;


static INT Plugin_Init(void);
static void Plugin_Quit(void);
static INT_PTR Plugin_MessageProc(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3);

EXTERN_C static winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_addons.dll)",
    Plugin_Init,
    Plugin_Quit,
    Plugin_MessageProc,
    0,
    0,
    0,
};

HINSTANCE Plugin_GetInstance(void)
{
	return plugin.hDllInstance;
}

HWND Plugin_GetWinamp(void)
{
	return plugin.hwndWinampParent;
}

HWND Plugin_GetLibrary(void)
{
	return plugin.hwndLibraryParent;
}

static INT Plugin_Init(void)
{
	api_service *serviceApi = (api_service*)SENDWAIPC(Plugin_GetWinamp(), IPC_GET_API_SERVICE,0);
	if (!WasabiApi_Initialize(Plugin_GetInstance(), serviceApi))
		return 1;

	if (NULL == OMBROWSERMNGR)
	{
		WasabiApi_Release();
		return 2;
	}

	if (NULL != WASABI_API_LNG)
	{
		static wchar_t szDescription[256];
		StringCchPrintf(szDescription, ARRAYSIZE(szDescription), WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME),
						PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
		plugin.description = (char*)szDescription;
	}

	if(NULL != WASABI_API_SYSCB && NULL == wasabiCallback && 
		SUCCEEDED(WasabiCallback::CreateInstance((WasabiCallback**)&wasabiCallback)))
	{
		WASABI_API_SYSCB->syscb_registerCallback(wasabiCallback, 0);
		for (;;)
		{
			SysCallback *callback = WASABI_API_SYSCB->syscb_enum(SysCallback::BROWSER, 0);
			if (NULL == callback || callback == wasabiCallback)
			{
				if (NULL != callback)
					callback->Release();
				break;
			}

			WASABI_API_SYSCB->syscb_deregisterCallback(callback);
			WASABI_API_SYSCB->syscb_registerCallback(callback, 0);
			callback->Release(); 
		}
		
	}
	uri_handler_factory.Register(WASABI_API_SVC, &uri_handler);
	Navigation_Initialize();

	return ML_INIT_SUCCESS;
}

static void Plugin_Quit(void)
{	
	if (NULL != wasabiCallback)
	{
		if (NULL != WASABI_API_SYSCB)
			WASABI_API_SYSCB->syscb_deregisterCallback(wasabiCallback);
		wasabiCallback->Release();
		wasabiCallback = NULL;
	}

	uri_handler_factory.Deregister(WASABI_API_SVC);
	WasabiApi_Release();
}

static INT_PTR Plugin_MessageProc(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	INT_PTR result = 0;
	if (FALSE != Navigation_ProcessMessage(msg, param1, param2, param3, &result))
		return result;
	
	return FALSE;
}

EXTERN_C __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}