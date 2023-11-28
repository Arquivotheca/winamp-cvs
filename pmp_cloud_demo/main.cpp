#include "api.h"
#include "main.h"
#include "CloudDevice.h"
#include "../Winamp/wa_ipc.h"

int winampVersion;
static int Init();
static void Quit();
static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3);
PMPDevicePlugin plugin = {PMPHDR_VER,0,Init,Quit,MessageProc};
void Parse(itemRecordListW &record_list);

static const char *json_command_list_media = "{\"version\": 1,"
 "\"command\": {"
    "\"type\": \"list-media\","
    "\"device-id\": \"ole-reference-protocol\","
    "\"username\": \"demo-may1\","
    "\"auth-token\": \"TOKEN\","
    "\"per-page\": 4000000000,"
    "\"page\": 0"
	"}}";

static DWORD CALLBACK CloudThreadProcedure(LPVOID param)
{
	ItemParser *parser = Parse();
	if (!parser)
		return 0;


	if (PostJSON("http://o2d2.office.aol.com:8090/command", json_command_list_media, parser->parser) == 0)
	{
		CloudDevice *device = new CloudDevice(parser->record_list);
		SendMessage(plugin.hwndPortablesParent,WM_PMP_IPC,(intptr_t)device,PMP_IPC_DEVICECONNECTED);
	}
	delete parser;
	return 0;
}

static int Init() 
{
	winampVersion = (int)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVERSION);
	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	WasabiInit(WASABI_API_SVC);
	CreateThread(0, 0, CloudThreadProcedure, 0, 0, 0);
	return 0;
}

static void Quit() 
{
	WasabiQuit();
}

static intptr_t MessageProc(int msg, intptr_t param1, intptr_t param2, intptr_t param3) 
{
	return 0;
}

extern "C" 	__declspec(dllexport) PMPDevicePlugin *winampGetPMPDevicePlugin()
{
	return &plugin;
}

