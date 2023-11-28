#include "main.h"
#include "../../winamp/gen.h"
#include <strsafe.h>

static INT 
Plugin_Init(void);

static void 
Plugin_Quit(void);

static void
Plugin_Config(void);

EXTERN_C static 
winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
	NULL,
    Plugin_Init,
	Plugin_Config,
    Plugin_Quit,
	0,
    0,
};

static DeviceProvider *deviceProvider = NULL;

HINSTANCE 
Plugin_GetInstance(void)
{
	return plugin.hDllInstance;
}

HWND 
Plugin_GetWinampWindow(void)
{
	return plugin.hwndParent;
}

static void
Plugin_SetDescription()
{
	WCHAR szBuffer[256], szTemplate[256];
	
	if (NULL != plugin.description)
		AnsiString_Free(plugin.description);

	if (NULL != WASABI_API_LNG)
		WASABI_API_LNGSTRINGW_BUF(IDS_PLUGIN_NAME, szTemplate, ARRAYSIZE(szTemplate));
	else
		szTemplate[0] = L'\0';
	
	StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), 
					((L'\0' != szTemplate[0]) ? szTemplate : L"Nullsoft Test Device Provider v%d.%d"),
					PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);

	plugin.description = String_ToAnsi(CP_ACP, 0, szBuffer, -1, NULL, NULL);
}

static INT 
Plugin_Init(void)
{	
	
	if (FALSE == Wasabi_InitializeFromWinamp(plugin.hDllInstance, plugin.hwndParent))
		return 1;

	Wasabi_LoadDefaultServices();

	Plugin_SetDescription();

	if (NULL == deviceProvider)
	{
		if (FAILED(DeviceProvider::CreateInstance(&deviceProvider)))
		{
			Wasabi_Release();
			return 2;
		}
		
		deviceProvider->Register(WASABI_API_DEVICES);
	}

	return 0;
}

static void 
Plugin_Quit(void)
{	
	if (NULL != plugin.description)
	{
		AnsiString_Free(plugin.description);
		plugin.description = NULL;
	}

	if (NULL != deviceProvider)
	{
		deviceProvider->Unregister(WASABI_API_DEVICES);
		deviceProvider->Release();
	}

	Wasabi_Release();
}

static void 
Plugin_Config(void)
{	
}

EXTERN_C __declspec(dllexport) winampGeneralPurposePlugin *
winampGetGeneralPurposePlugin()
{
	if (NULL == plugin.description)
	{
		Plugin_SetDescription();
	}
	return &plugin;
}