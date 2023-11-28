#include "./main.h"
#include "./plugin.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include "./lfHeap.h"

#include <strsafe.h>

#ifdef __cplusplus
extern "C" {
#endif 


winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
	NULL,
	Plugin_OnInit,
	Plugin_OnConfig,
	Plugin_OnQuit,
};

_declspec(dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin()
{
	return &plugin;
}

INT_PTR GetPluginUID(void) 
{
	return (INT_PTR)&plugin;
}

static void LoadPluginDescription(HINSTANCE hInst)
{
	static char szDescription[256];
	char szTemplate[256];
	int cLen;
	HRESULT hr;

	cLen = LoadStringA(hInst, IDS_PLUGIN_DESCRIPTION_TEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
	if (!cLen) 
		StringCchCopyA(szTemplate, ARRAYSIZE(szTemplate), "Nullsoft DropBox v%d.%d");
	hr = StringCchPrintfA(szDescription, ARRAYSIZE(szDescription), szTemplate, PLUGIN_VERSION_MAJOR, PLUGIN_VERSION_MINOR);
	if (S_OK != hr)
		StringCchCopyA(szDescription, ARRAYSIZE(szDescription), "Nullsoft DropBox");
	
	plugin.description = szDescription;
}	


BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			lfh_init();
			LoadPluginDescription((HINSTANCE)hInst);
			break;
		case DLL_PROCESS_DETACH:
			lfh_shutdown();
			break;
	}
	
	return TRUE;
}

#ifdef __cplusplus
}
#endif 