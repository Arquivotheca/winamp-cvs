#include "api.h"
#include "main.h"
#include "../Winamp/wa_ipc.h"
#include "config.h"
#include "resource.h"
#include <shlwapi.h>
#include <shellapi.h>
#include <strsafe.h>

prefsDlgRecW g_prefsItem = {0};
const char *inidir=0;
int winampVersion = 0;
char session_key[512]="";
char token_a[512]="";
__time64_t session_expiration=0;
HMENU ctrlmenu=0;
WORD enable_menu_id=0;
WORD options_menu_id=0;
WORD login_menu_id=0;

static void CreateMUDMenu()
{
	WCHAR szBuffer[256];
	ctrlmenu = CreatePopupMenu();

	WASABI_API_LNGSTRINGW_BUF(IDS_MENU_ENABLE, szBuffer, ARRAYSIZE(szBuffer));
	enable_menu_id = (WORD)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_REGISTER_LOWORD_COMMAND);
	InsertMenuW(ctrlmenu, -1, MF_BYPOSITION|MF_STRING, (UINT_PTR)enable_menu_id, szBuffer);
	
	WASABI_API_LNGSTRINGW_BUF(IDS_MENU_OPTIONS, szBuffer, ARRAYSIZE(szBuffer));
	options_menu_id = (WORD)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_REGISTER_LOWORD_COMMAND);
	InsertMenuW(ctrlmenu, -1, MF_BYPOSITION|MF_STRING, (UINT_PTR)options_menu_id, szBuffer);

	WASABI_API_LNGSTRINGW_BUF(IDS_MENU_LOGIN, szBuffer, ARRAYSIZE(szBuffer));
	login_menu_id = (WORD)SendMessage(plugin.hwndParent,WM_WA_IPC,0,IPC_REGISTER_LOWORD_COMMAND);
	InsertMenuW(ctrlmenu, -1, MF_BYPOSITION|MF_STRING, (UINT_PTR)login_menu_id, szBuffer);

	HMENU rclick_menu = (HMENU)SendMessage(plugin.hwndParent, WM_WA_IPC,(WPARAM)0,IPC_GET_HMENU);

#define ID_HELP_HELPTOPICS              40347
	if (NULL != rclick_menu)
	{
		WASABI_API_LNGSTRINGW_BUF(IDS_MENU_ORGLER, szBuffer, ARRAYSIZE(szBuffer));
		InsertMenuW(rclick_menu, ID_HELP_HELPTOPICS, MF_BYCOMMAND | MF_POPUP, (UINT_PTR)ctrlmenu, szBuffer);
		InsertMenuW(rclick_menu, ID_HELP_HELPTOPICS, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);
	}

	Config_SyncEnabled();
	Config_SyncLogin();
}

bool OpenUrl(const wchar_t *url, bool forceExternalBrowser)
{
	if (false != forceExternalBrowser)
	{
		int result = (int)(INT_PTR)ShellExecuteW(plugin.hwndParent, L"open", url, NULL, NULL, SW_SHOWNORMAL);
		return (result > 32);
	}

	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)url, IPC_OPEN_URL);
	return true;
}

static char szDescription[256];

int Init()
{
	winampVersion = (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETVERSION);
	inidir = (const char *)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINIDIRECTORY);

	api_service *service= (api_service *)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (!service || service == (api_service *)1)
		return 1;

	WasabiInit(service);

	if (!AGAVE_API_AUTH)
	{
		WasabiQuit();
		return 1;
	}

	WASABI_API_LNGSTRING_BUF(IDS_PLUGIN_NAME, szDescription, ARRAYSIZE(szDescription));
	INT cchDescription = lstrlenA(szDescription);
	StringCchPrintfExA(szDescription + cchDescription, ARRAYSIZE(szDescription) - cchDescription, 
		NULL, NULL, 0, " v%u.0%u", VERSION_MAJOR, VERSION_MINOR);
	plugin.description = szDescription;

	Config_Load();

	WCHAR szBuffer[256];

	

	//register prefs screen

	WASABI_API_LNGSTRINGW_BUF(IDS_PREFERENCE_NAME, szBuffer, ARRAYSIZE(szBuffer));
	g_prefsItem.name = _wcsdup(szBuffer);

	g_prefsItem.dlgID = IDD_PREFS;
	g_prefsItem.proc = PreferencesDialog;
	g_prefsItem.hInst = WASABI_API_LNG_HINST;
	// delay the adding of this
	// for some reason when changing a lang pack it can cause the WM_DESTROY of ConfigProc
	// to be called which completely messes up the ghk list and so can wipe it :(
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM) &g_prefsItem, IPC_ADD_PREFS_DLGW);

	CreateMUDMenu();

	/** we need to subclass the Winamp main window to get song change notifications
	** subclassing is the "classic" way to get notifications and do fancy stuff in plugins
	** it's ugly, and can cause potential problems, but it works
	**/	
	HookWinampProc(plugin.hwndParent);

	// if we closed in the midst of a song playback last time, we need to send the data now.
	if (config_awaken_on_load)
	{
		AwakenQueue();
		config_awaken_on_load=0;
	}

	if (config_first)
	{
		PostMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&g_prefsItem, IPC_OPENPREFSTOPAGE);
	}

	return GEN_INIT_SUCCESS;
}

void Config()
{
	SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)&g_prefsItem, IPC_OPENPREFSTOPAGE);

}

void Quit()
{
	OnStop(/*quick=*/true);
	ListenReset();
	CloseQueue();
	FlushAvQueue();
	CloseDatabase();
	Config_Save();
	WasabiQuit();
	CloseLog();

	if (NULL != g_prefsItem.name)
	{
		free(g_prefsItem.name);
		g_prefsItem.name = NULL;
	}
}

winampGeneralPurposePlugin plugin =
{
	GPPHDR_VER,
		"nullsoft(gen_orgler.dll)",
		Init,
		Config,
		Quit,
		0,
		0,
};

extern "C" __declspec(dllexport) winampGeneralPurposePlugin *winampGetGeneralPurposePlugin()
{
	return &plugin;
}
