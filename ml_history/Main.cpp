#include "api.h"
#include "main.h"
#include "config.h"
#include "HistoryAPIFactory.h"
#include "JSAPI2_Creator.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "../nu/AutoCharFn.h"
#include <strsafe.h>

#define LOCAL_WRITE_VER L"1.95"

// wasabi based services for localisation support
api_service *WASABI_API_SVC=0;
api_language *WASABI_API_LNG = 0;
api_explorerfindfile *WASABI_API_EXPLORERFINDFILE = 0;
JSAPI2::api_security *AGAVE_API_JSAPI2_SECURITY = 0;
api_application *WASABI_API_APP=0;

HINSTANCE WASABI_API_LNG_HINST = 0, WASABI_API_ORIG_HINST = 0;

static HistoryAPIFactory historyAPIFactory;
static JSAPI2Factory jsapi2Factory;

int Init();
void Quit();
INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3);

prefsDlgRecW preferences = {0};
wchar_t g_tableDir[MAX_PATH] = {0};

winampMediaLibraryPlugin lMedia =
{
    MLHDR_VER,
    "nullsoft(ml_history.dll)",
    Init,
    Quit,
    MessageProc,
    0,
    0,
    0,
};

// Delay load library control << begin >>
#include <delayimp.h>
#pragma comment(lib, "delayimp")

bool nde_error = false;

FARPROC WINAPI FailHook(unsigned dliNotify, PDelayLoadInfo  pdli) 
{
	nde_error = true;
	return 0;
}

extern "C"
{
	PfnDliHook __pfnDliFailureHook2 = FailHook;
}
// Delay load library control << end >>

int Init()
{
	WASABI_API_SVC = (api_service*)SendMessage(lMedia.hwndWinampParent, WM_WA_IPC, 0, IPC_GET_API_SERVICE);
	if (WASABI_API_SVC == (api_service*)1) WASABI_API_SVC = NULL;

	InitializeCriticalSection(&g_db_cs);

	g_db = NULL;
	g_table = NULL;
	
	InitCommonControls();

	mediaLibrary.library = lMedia.hwndLibraryParent;
	mediaLibrary.winamp = lMedia.hwndWinampParent;
	mediaLibrary.instance = lMedia.hDllInstance;

	wchar_t configName[MAX_PATH] = {0};
	const wchar_t *dir = mediaLibrary.GetIniDirectoryW();
	if ((INT_PTR) (dir) < 65536) return ML_INIT_FAILURE;

	PathCombineW(g_tableDir, dir, L"Plugins");
	PathCombineW(configName, g_tableDir, L"gen_ml.ini");
	g_config = new C_Config(AutoCharFn(configName));

	CreateDirectoryW(g_tableDir, NULL);
	PathCombineW(g_tableDir, g_tableDir, L"ml");
	CreateDirectoryW(g_tableDir, NULL);

	// loader so that we can get the localisation service api for use

	waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) WASABI_API_LNG = reinterpret_cast<api_language*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(JSAPI2::api_securityGUID);
	if (sf) AGAVE_API_JSAPI2_SECURITY = reinterpret_cast<JSAPI2::api_security*>(sf->getInterface());
	
	sf = WASABI_API_SVC->service_getServiceByGuid(applicationApiServiceGuid);
	if (sf) WASABI_API_APP = reinterpret_cast<api_application*>(sf->getInterface());

	sf = WASABI_API_SVC->service_getServiceByGuid(ExplorerFindFileApiGUID);
	if (sf) WASABI_API_EXPLORERFINDFILE = reinterpret_cast<api_explorerfindfile*>(sf->getInterface());

	WASABI_API_SVC->service_register(&historyAPIFactory);
	WASABI_API_SVC->service_register(&jsapi2Factory);		
	// need to have this initialised before we try to do anything with localisation features
	WASABI_API_START_LANG(lMedia.hDllInstance,MlHistoryLangGUID);

	static wchar_t szDescription[256];
	StringCchPrintfW(szDescription, ARRAYSIZE(szDescription),
					 WASABI_API_LNGSTRINGW(IDS_NULLSOFT_HISTORY), LOCAL_WRITE_VER);
	lMedia.description = (char*)szDescription;

	static wchar_t preferencesName[64] = {0};
	preferences.hInst = WASABI_API_LNG_HINST;
	preferences.dlgID = IDD_PREFS;
	preferences.proc = (void *)PrefsProc;
	preferences.name = 	WASABI_API_LNGSTRINGW_BUF(IDS_HISTORY,preferencesName,64);
	SENDWAIPC(lMedia.hwndWinampParent, IPC_ADD_PREFS_DLGW, &preferences);
	
	if (!history_init()) return ML_INIT_FAILURE; 

	return ML_INIT_SUCCESS;
}

void Quit()
{
	WASABI_API_SVC->service_deregister(&historyAPIFactory);
	WASABI_API_SVC->service_deregister(&jsapi2Factory);
	history_quit();
	delete g_config;
	DeleteCriticalSection(&g_db_cs);
		waServiceFactory *sf = WASABI_API_SVC->service_getServiceByGuid(languageApiGUID);
	if (sf) sf->releaseInterface(WASABI_API_LNG);

	sf = WASABI_API_SVC->service_getServiceByGuid(JSAPI2::api_securityGUID);
	if (sf) sf->releaseInterface(AGAVE_API_JSAPI2_SECURITY);
}

static INT_PTR History_OnContextMenu(INT_PTR param1, HWND hHost, POINTS pts)
{
	HNAVITEM hItem = (HNAVITEM)param1;
	HNAVITEM myItem = MLNavCtrl_FindItemById(lMedia.hwndLibraryParent, ml_history_tree);
	if (hItem != myItem) 
		return FALSE;

	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(lMedia.hwndLibraryParent, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}


	HMENU hMenu = WASABI_API_LOADMENUW(IDR_CONTEXTMENUS);
	HMENU subMenu = (NULL != hMenu) ? GetSubMenu(hMenu, 1) : NULL;
	if (NULL != subMenu)
	{

		INT r = Menu_TrackPopup(subMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | 
				TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, 
				pt.x, pt.y, hHost, NULL);

		switch(r)
		{
			case ID_NAVIGATION_PREFERENCES:
				SENDWAIPC(lMedia.hwndWinampParent, IPC_OPENPREFSTOPAGE, &preferences);
				break;
			case ID_NAVIGATION_HELP:
				SENDWAIPC(lMedia.hwndWinampParent, IPC_OPEN_URL, L"http://www.winamp.com/help/Main_Page#The_Winamp_Media_Library");
				break;
		}
	}

	if (NULL != hMenu)
		DestroyMenu(hMenu);
	
	return TRUE;
}

INT_PTR MessageProc(int message_type, INT_PTR param1, INT_PTR param2, INT_PTR param3)
{
	switch (message_type)
	{
		case ML_MSG_TREE_ONCREATEVIEW:     // param1 = param of tree item, param2 is HWND of parent. return HWND if it is us
			return (param1 == ml_history_tree) ?  (INT_PTR)onTreeViewSelectChange((HWND)param2) : 0; 
			
		case ML_MSG_CONFIG:
			mediaLibrary.GoToPreferences(preferences._id);
			return TRUE;

		case ML_MSG_NAVIGATION_CONTEXTMENU:
			return History_OnContextMenu(param1, (HWND)param2, MAKEPOINTS(param3));
	}
	return 0;
}

extern "C" __declspec(dllexport) winampMediaLibraryPlugin *winampGetMediaLibraryPlugin()
{
	return &lMedia;
}