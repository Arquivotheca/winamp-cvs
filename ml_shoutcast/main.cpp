#include "main.h"
#include "api.h"
#include <api/service/waservicefactory.h>
#include "resource.h"
#include <shlwapi.h>

#define WINAMP_VIDEO_TVBUTTON           40338 // we hook this =)

static WNDPROC wa_oldWndProc;
static LRESULT WINAPI wa_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
wchar_t ini_file[MAX_PATH];
int Init();
void Quit();
INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);
int radioTreeId, tvTreeId;
Nullsoft::Utility::LockGuard radio_guard;
winampMediaLibraryPlugin plugin =
    {
        MLHDR_VER,
        "Nullsoft SHOUTcast Radio Listings v1.0",

        Init,
        Quit,
        PluginMessageProc,

        0,
        0,
        0,
    };


api_service *WASABI_API_SVC = 0;
int winampVersion;
api_application *WASABI_API_APP = 0;
nde_database_t db=0;
api_language * languageManager = 0;
int Init()
{
	mediaLibrary.library = plugin.hwndLibraryParent;
	mediaLibrary.winamp = plugin.hwndWinampParent;
	mediaLibrary.instance = plugin.hDllInstance;

	WASABI_API_SVC = (api_service *)SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	winampVersion = SendMessage(plugin.hwndWinampParent, WM_WA_IPC, 0, IPC_GETVERSION);

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf)
		WASABI_API_APP = (api_application *)sf->getInterface();

	PathCombine(ini_file, WASABI_API_APP->path_getUserSettingsPath(), L"Plugins\\ml\\ml_shoutcast.ini");
	mediaLibrary.AddTreeImage(IDB_SC_RADIO, 12001, (BMPFILTERPROC)FILTER_DEFAULT1);
	mediaLibrary.AddTreeImage(IDB_SC_TV, 12002, (BMPFILTERPROC)FILTER_DEFAULT1);

	MLTREEITEM newTree;
	newTree.size = sizeof(MLTREEITEM);
	newTree.titleLen = -1;

	newTree.parentId = 0;
	newTree.title = "SHOUTcast Radio";
	newTree.hasChildren = 0;
	newTree.id = 0;
	newTree.imageIndex = 12001;
	mediaLibrary.AddTreeItem(newTree);
	radioTreeId = newTree.id;
/*
	newTree.parentId = 0;
	newTree.title = "SHOUTcast TV";
	newTree.hasChildren = 0;
	newTree.id = 0;
	newTree.imageIndex = 12002;
	mediaLibrary.AddTreeItem(newTree);
	tvTreeId = newTree.id;
*/
	if (IsWindowUnicode(plugin.hwndWinampParent))
		wa_oldWndProc = (WNDPROC) SetWindowLongPtrW(plugin.hwndWinampParent, GWLP_WNDPROC, (LONG_PTR)wa_newWndProc);
	else
		wa_oldWndProc = (WNDPROC) SetWindowLongPtrA(plugin.hwndWinampParent, GWLP_WNDPROC, (LONG_PTR)wa_newWndProc);

	return 0;
}

void Quit()
{
	CloseDatabase();

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf)
		sf->releaseInterface(WASABI_API_APP);

}

INT_PTR PluginMessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch (message_type)
	{
	case ML_MSG_CONFIG:
		break;
	case ML_MSG_TREE_ONCREATEVIEW:      // param1 = param of tree item, param2 is HWND of parent. return HWND if it is us
		if (param1 == radioTreeId)
		{
			return (INT_PTR)CreateDialog(plugin.hDllInstance, MAKEINTRESOURCE(IDD_RADIO_VIEW), (HWND)param2, StationProc);
		}
		else if (param1 == tvTreeId)
		{
			//return (INT_PTR)CreateDialog(plugin.hDllInstance, MAKEINTRESOURCE(IDD_RADIO_VIEW), (HWND)param2, TVProc);
		}
		break;
	}

	return 0;
}


static LRESULT WINAPI wa_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_COMMAND && LOWORD(wParam) == WINAMP_VIDEO_TVBUTTON)
	{
		mediaLibrary.SelectTreeItem(tvTreeId);
		mediaLibrary.ShowMediaLibrary();
		SetFocus(plugin.hwndLibraryParent);
	}

	return CallWindowProc(wa_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &plugin;
}
