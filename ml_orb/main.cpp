#include "main.h"
#include "./navigation.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./external.h"

#include "../winamp/wa_ipc.h"

#include <strsafe.h>

static DWORD externalCookie = 0;

static INT Plugin_Init(void);
static void Plugin_Quit(void);
static INT_PTR Plugin_MessageProc(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3);

EXTERN_C static winampMediaLibraryPlugin plugin =
{
	MLHDR_VER,
	"nullsoft(ml_orb.dll)",
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

HRESULT Plugin_GetSessionId(LPWSTR pszBuffer, INT cchBufferMax)
{
	return E_NOTIMPL;
}

HRESULT Plugin_GetClientId(LPWSTR pszBuffer, INT cchBufferMax)
{
	return E_NOTIMPL;
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
		StringCchPrintf(szDescription, ARRAYSIZE(szDescription),
						WASABI_API_LNGSTRINGW(IDS_PLUGIN_NAME),
						PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
		plugin.description = (char*)szDescription;
	}
	
	ExternalDispatch *externalDispatch;
	if (SUCCEEDED(ExternalDispatch::CreateInstance(&externalDispatch)))
	{
		DispatchInfo dispatchInfo;
		dispatchInfo.id = 0;
		dispatchInfo.name =(LPWSTR)externalDispatch->GetName();
		dispatchInfo.dispatch = externalDispatch;

		if (0 == SENDWAIPC(Plugin_GetWinamp(), IPC_ADD_DISPATCH_OBJECT, (WPARAM)&dispatchInfo))
			externalCookie = dispatchInfo.id;

		externalDispatch->Release();
	}
	
	Navigation_Initialize();

	return ML_INIT_SUCCESS;
}

static void Plugin_Quit(void)
{	
	if (0 != externalCookie)
	{
		HWND hWinamp = Plugin_GetWinamp();
		SENDWAIPC(hWinamp, IPC_REMOVE_DISPATCH_OBJECT, (WPARAM)externalCookie);
		externalCookie = 0;
	}

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

EXTERN_C __declspec(dllexport) int winampUninstallPlugin(HINSTANCE hDllInst, HWND hwndDlg, int param) 
{	
	Navigation_RemoveService(SERVICE_MAIN);
	return ML_PLUGIN_UNINSTALL_NOW;
}