#include "main.h"
#include "./plugin.h"
#include "./resource.h"
#include "./preferences.h"
#include "./wasabiApi.h"
#include "./winampHook.h"

#include "./groupedList.h"
#include "./guiObjects.h"
#include "./pluginShortcut.h"

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>


#define IDC_PROFILEVIEW		10000
#define NULL_ITEM			((INT)-1)

static prefsDlgRecW preferenceShorctut = {0,};

static INT_PTR CALLBACK PreferencesShortcut_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct __SHORTCUTPAGE
{
	GUID	*profileUidList;
	INT		profileUidCount;
} SHORTCUTPAGE;

#define SHORTCUTPAGE_PROP		TEXT("waDropboxShortcutPrefPage")
#define GetShortcut(__hwnd) ((SHORTCUTPAGE*)GetProp((__hwnd), SHORTCUTPAGE_PROP))

BOOL Plugin_RegisterShortcutPreferences()
{
	TCHAR szBuffer[128];
	
	if (0 != preferenceShorctut.dlgID)
		return FALSE;

	WASABI_API_LNGSTRINGW_BUF(IDS_PREFERENCES_SHORTCUT, szBuffer, ARRAYSIZE(szBuffer));
	if (TEXT('\0') == *szBuffer)
		StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), TEXT("Shortcut"));

	ZeroMemory(&preferenceShorctut, sizeof(preferenceShorctut));
	preferenceShorctut.hInst = WASABI_API_LNG_HINST;
	preferenceShorctut.dlgID = IDD_PREFERENCES_SHORTCUT;
	preferenceShorctut.name = lfh_strdup(szBuffer);
	preferenceShorctut.proc = (void*)PreferencesShortcut_DialogProc;
	preferenceShorctut.where = (intptr_t)Plugin_GetPreferences(); 
	SENDWAIPC(plugin.hwndParent, IPC_ADD_PREFS_DLGW, &preferenceShorctut);
	return TRUE;

}

BOOL Plugin_UnregisterShortcutPreferences()
{
	if (0 == preferenceShorctut.dlgID)
		return FALSE;

	SENDWAIPC(plugin.hwndParent, IPC_REMOVE_PREFS_DLG, &preferenceShorctut);

	if (NULL != preferenceShorctut.name)
		lfh_free(preferenceShorctut.name);

	ZeroMemory(&preferenceShorctut, sizeof(preferenceShorctut));
	return TRUE;
}

BOOL Plugin_ShowShortcutPreferences()
{
	if (0 == preferenceShorctut.dlgID)
		return FALSE;

	SENDWAIPC(plugin.hwndParent, IPC_OPENPREFSTOPAGE, &preferenceShorctut);
	return TRUE;

}

static BOOL PreferencesShortcut_GetBoxRect(HWND hwnd, RECT *prc)
{
	if (NULL == prc)
		return FALSE;

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent)
	{
		HWND hBox = GetDlgItem(hParent, 1186 /*IDC_RECT*/);
		if (NULL != hBox && GetWindowRect(hBox, prc))
		{
			MapWindowPoints(HWND_DESKTOP, hParent, (POINT*)prc, 2);
			return TRUE;
		}
	}
	SetRectEmpty(prc);
	return FALSE;
}



static void PreferencesShortcut_AdjustWindow(HWND hwnd)
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 == (WS_CHILD & windowStyle)) 
		return;
		
	RECT rcBox;
	if (!PreferencesShortcut_GetBoxRect(hwnd, &rcBox))
		return;
	
	SetWindowPos(hwnd, NULL, rcBox.left, rcBox.top, rcBox.right - rcBox.left, rcBox.bottom - rcBox.top, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);

}

static void PreferencesShortcut_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	HWND hControl;
	RECT controlRect;
	hControl = GetDlgItem(hwnd, IDC_PROFILEVIEW_RECT);
	if (NULL != hControl && GetWindowRect(hControl, &controlRect))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);

		hControl = GetDlgItem(hwnd, IDC_PROFILEVIEW);
		if (NULL != hControl)
		{
			SetWindowPos(hControl, NULL, controlRect.left, controlRect.top, 
					controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, 
					SWP_NOACTIVATE | SWP_NOZORDER);
		}
		
	}
}

static GLRoot *PreferencesShortcut_CreateGroup(SHORTCUTPAGE *page, const UUID &activeProfile)
{	
	GLRoot *root = GLRoot::Create();
	if (NULL == root ) return NULL;
	
	Profile *szProfiles[256];

	INT cchLoaded = PLUGIN_PROFILEMNGR->LoadProfiles(szProfiles, ARRAYSIZE(szProfiles));
	TCHAR szName[128];
	TCHAR szDescription[1024];

	if (cchLoaded > 0)
	{
		page->profileUidCount = 0;
		page->profileUidList = (UUID*)malloc(sizeof(UUID) * cchLoaded);
		if (NULL == page->profileUidList)
		{
			root->Release();
			return NULL;
		}
		
	}

	BOOL profileSelected  = FALSE;
	GLItem *item;
	for (INT i = 0; i < cchLoaded; i++)
	{
		if (SUCCEEDED(szProfiles[i]->GetName(szName, ARRAYSIZE(szName))))
		{
			if (FAILED(szProfiles[i]->GetDescription(szDescription, ARRAYSIZE(szDescription))))
				szDescription[0] = TEXT('\0');
		
			
			if (SUCCEEDED(szProfiles[i]->GetUID(&page->profileUidList[page->profileUidCount])))
			{
				BOOL checked = FALSE;
				if (!profileSelected && IsEqualGUID(activeProfile, page->profileUidList[page->profileUidCount]))
				{
					checked = TRUE;
					profileSelected = TRUE;
				}
				item = GLButtonEx::CreateRadiobutton(page->profileUidCount, szName, szDescription, checked);
				if (-1 != root->InsertItem(-1, item))
				{				
					page->profileUidCount++;
				}
				item->Release();
			}
			szProfiles[i]->Release();
		}
	}
	root->Sort();

	WASABI_API_LNGSTRINGW_BUF(IDS_PROFILES_ASK, szName, ARRAYSIZE(szName));
	WASABI_API_LNGSTRINGW_BUF(IDS_PROFILES_ASK_DESCRIPTION, szDescription, ARRAYSIZE(szDescription));

	item = GLButtonEx::CreateRadiobutton(NULL_ITEM, szName, szDescription, (FALSE == profileSelected));
	root->InsertItem(0, item);
	item->Release();

	return root;
}

static INT_PTR PreferencesShortcut_OnInit(HWND hwnd, HWND hFocus, LPARAM param)
{
	
	SHORTCUTPAGE *page = (SHORTCUTPAGE*)malloc(sizeof(SHORTCUTPAGE));
	if (NULL == page || !SetProp(hwnd, SHORTCUTPAGE_PROP, page))
	{
		if (NULL != page)
			free(page);
		return FALSE;
	}
	ZeroMemory(page, sizeof(SHORTCUTPAGE));

	DROPBOXCLASSINFO *classInfo = Plugin_FindRegisteredClass(defaultClassUid);
	if (NULL == classInfo)
	{
		return FALSE;
	}

	GLRoot *root = NULL;
	root = PreferencesShortcut_CreateGroup(page, classInfo->profileUid);
	

	HWND groupView = GroupedListView_CreateWindow(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, 
						WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_VSCROLL | WS_CLIPCHILDREN, 
						0, 0, 1, 1, 
						hwnd, IDC_PROFILEVIEW, plugin.hDllInstance, root, NULL);
	
	if (NULL != root)
		root->Release();

	HFONT windowFont =  (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
	
	if (NULL != groupView)
		SendMessage(groupView, WM_SETFONT, (WPARAM)windowFont, (LPARAM)0);

	HWND hControl;
	hControl = GetDlgItem(hwnd, IDC_CHECK_SHORTCUT);
	if (NULL != hControl)
	{
		SendMessage(hControl, BM_SETCHECK,  (WPARAM)((0 != classInfo->shortcut.key) ? BST_CHECKED : BST_UNCHECKED), 0L);
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_CHECK_SHORTCUT, BN_CLICKED), (LPARAM)hControl);
	}


	PostMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent) Preferences_EnableWarning(hParent, TRUE);

	return FALSE;
}

static void PreferencesShortcut_OnDestroy(HWND hwnd)
{
	SHORTCUTPAGE *page = GetShortcut(hwnd);
	RemoveProp(hwnd, SHORTCUTPAGE_PROP);
	if (NULL != page)
	{
		if (NULL != page->profileUidList)
			free(page->profileUidList);

		free(page);
	}

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent) Preferences_EnableWarning(hParent, FALSE);
}


static void PreferencesShortcut_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (0 == (SWP_NOMOVE & pwp->flags))
	{
		PreferencesShortcut_AdjustWindow(hwnd);
		return;
	}
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;	
	PreferencesShortcut_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)));

}


static void PreferencesSHortcut_OnShortcutEnabledChanged(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_CHECK_SHORTCUT);
	if (NULL == hControl)
		return;
	BOOL bEnabled = (BST_CHECKED == SendMessage(hControl, BM_GETCHECK, 0, 0L));

	INT szControls[] = { IDC_LABEL_DEFAULT_PROFILE, IDC_PROFILEVIEW, };

	for (INT i =0; i < ARRAYSIZE(szControls); i++)
	{
		hControl = GetDlgItem(hwnd, szControls[i]);
		if (NULL != hControl) EnableWindow(hControl, bEnabled);
	}

	DROPBOXCLASSINFO *classInfo = Plugin_FindRegisteredClass(defaultClassUid);
	if (NULL != classInfo)
	{
		if (!bEnabled)
		{
			ZeroMemory(&classInfo->shortcut, sizeof(ACCEL));
			PluginShortcut_Unregister(classInfo->classUid);
		}
		else
		{
			classInfo->shortcut.fVirt = FVIRTKEY | FCONTROL | FSHIFT; 
			classInfo->shortcut.key = (WORD)'D';
			PluginShortcut_Register(classInfo->classUid, &classInfo->shortcut);
		}
		DropboxClass_Save(classInfo);
	}
}
static void PreferencesShortcut_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(controlId)
	{
		case IDC_CHECK_SHORTCUT:
			if (BN_CLICKED == eventId)
				PreferencesSHortcut_OnShortcutEnabledChanged(hwnd);
			break;
	}
}

static void PreferencesShortcut_OnColorChanging(HWND hwnd, NMGLCOLOR *pglc)
{
	switch(pglc->colorId)
	{
		case GLStyle::uiColorWindow:
//			pglc->rgb = ColorAdjustLuma(pglc->rgb, 0, TRUE);
			break;
		case GLStyle::uiColorItem:
//			pglc->rgb = ColorAdjustLuma(pglc->rgb, -20, TRUE);
			break;
	}
}

static void PreferencesShortcut_OnItemStyleChanged(HWND hwnd, NMGLITEM *pnmi)
{
	UINT oldVal, newVal;
	oldVal = (GLItem::FlagValueMask & pnmi->styleOld);
	newVal = (GLItem::FlagValueMask & pnmi->styleNew);
	if (oldVal == newVal || GLButton::FlagButtonChecked != newVal)
		return;
	
	DROPBOXCLASSINFO *classInfo = Plugin_FindRegisteredClass(defaultClassUid);
	if (NULL != classInfo)
	{
		SHORTCUTPAGE *page = GetShortcut(hwnd);

		INT itemId = pnmi->item->GetId();
		if (NULL_ITEM == itemId)
		{
			classInfo->profileUid = GUID_NULL;
			classInfo->style &= ~DBCS_REMEMBERPROFILE;
		}
		else
		{
			
			if (NULL != page && itemId >= 0 && itemId < page->profileUidCount)
			{
				classInfo->style &= ~DBCS_REMEMBERPROFILE;
				classInfo->profileUid = page->profileUidList[itemId];
			}
		}
		DropboxClass_Save(classInfo);
		if (NULL != page)
		{
			HWND hParent = GetParent(hwnd);
			if (NULL != hParent) Preferences_ShowWarning(hParent, TRUE);
		}
	}
}

static LRESULT PreferencesShortcut_OnNotify(HWND hwnd, NMHDR *pnmh)
{
	switch(pnmh->idFrom)
	{
		case IDC_PROFILEVIEW:
			switch(pnmh->code)
			{
				case GLVN_COLORCHANGING: PreferencesShortcut_OnColorChanging(hwnd, (NMGLCOLOR*)pnmh); break;
				case GLVN_ITEMSTYLECHANGED: PreferencesShortcut_OnItemStyleChanged(hwnd, (NMGLITEM*)pnmh); break;
			}
			break;
	}
	return 0;
}

static LRESULT PreferencesShortcut_OnStaticColor(HWND hwnd, HDC hdc, HWND hStatic)
{
	INT controlId = GetDlgCtrlID(hStatic);
	COLORREF rgbFg = 0xFF00FF;
	BOOL controlProcessed = FALSE;

	switch(controlId)
	{
		case IDC_LABEL_SHORTCUT_HELP:
			controlProcessed = TRUE;
			rgbFg = BlendColors(GetSysColor(COLOR_WINDOWTEXT), GetSysColor(COLOR_3DFACE), 150);
			break;
	}

	if (controlProcessed)
	{
		SetTextColor(hdc, rgbFg);
		SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
		return (LRESULT)GetSysColorBrush(COLOR_3DFACE);
	}
	return 0;
}

static INT_PTR CALLBACK PreferencesShortcut_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:	return PreferencesShortcut_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		PreferencesShortcut_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED: PreferencesShortcut_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:		PreferencesShortcut_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_NOTIFY:		MSGRESULT(hwnd, PreferencesShortcut_OnNotify(hwnd, (NMHDR*)lParam)); return TRUE;
		case WM_CTLCOLORSTATIC:	return PreferencesShortcut_OnStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
	}
	return 0;
}