#include "../ml_pmp/pmp.h"
#include "../Winamp/wa_ipc.h"
#include "api.h"
#define PLUGIN_VERSION "v1.0"
static int Init();
static void Quit();
static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);
extern PMPDevicePlugin plugin = {PMPHDR_VER,0,Init,Quit,MessageProc};

static int Init() 
{
	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	WasabiInit(WASABI_API_SVC);

	static char szDescription[256];
	wsprintfA(szDescription,"%s "PLUGIN_VERSION, "PMP Template Plugin"); // TODO: pull from langpack, e.g. WASABI_API_LNGSTRING(IDS_NULLSOFT_USB_DEVICE_PLUGIN));
	plugin.description = szDescription;

	/* TODO: Use this if your device shows up as a normal drive
	SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(WPARAM)autoDetectCallback,PMP_IPC_ENUM_ACTIVE_DRIVES);
	*/
	return 0;
}

static void Quit() 
{
	WasabiQuit();
}



static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) 
{
	switch(msg) {
		case PMP_DEVICECHANGE:
			// TODO: Implement
			return 0;
		case PMP_CONFIG:
			// TODO: Implement
			return 1;
	}
	return 0;
}

extern "C" 	__declspec(dllexport) PMPDevicePlugin *winampGetPMPDevicePlugin()
{
	return &plugin;
}