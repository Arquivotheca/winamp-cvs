#include "main.h"
#include "./plugin.h"
#include "./resource.h"
#include "./preferences.h"
#include "./wasabiApi.h"
#include "./winampHook.h"
#include "../winamp/commandLink.h"
#include "./messageBoxTweak.h"

#include "./configIniSection.h"
#include "./configManager.h"

#include "../nu/ptrlist.h"
#include "../nu/vector.h"

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <strsafe.h>

typedef Vector<UUID> UidList;

#define PREVIOUSLIST_PROP		TEXT("PREVIOUSLIST_PROP")

typedef struct __PREFPAGE
{
	INT			id;
	INT			imageId;
	HINSTANCE	hInstance;
	LPTSTR		pszTitle;
	LPTSTR		pszDialog;
	DLGPROC		dialogProc;
} PREFPAGE;

typedef struct __LINKDATA
{
	INT_PTR controlId;
	LPCTSTR pszText;
} LINKDATA;

typedef nu::PtrList<PREFPAGE> PreferencePageList;

#define IDC_ACTIVEPAGE		10001
#define IDC_LINK_CREATE		10002
#define IDC_LINK_DUPLICATE	10003
#define IDC_LINK_DELETE		10004
#define IDC_VIEW_EMPTY		10005

#define FRAME_PROP			TEXT("waDropboxPrefFrame")


extern HWND PreferencesEmpty_CreateView(HWND hParent, INT_PTR controlId);

static prefsDlgRecW preferenceInstance = {0,};
static PreferencePageList preferencePages;
static INT preferenceLastPageId = PREFPAGE_INVALID;

static LINKDATA szProfileLinks[] = 
{
	{ IDC_LINK_CREATE, MAKEINTRESOURCE(IDS_LINK_CREATE), },
	{ IDC_LINK_DUPLICATE, MAKEINTRESOURCE(IDS_LINK_DUPLICATE), },
	{ IDC_LINK_DELETE, MAKEINTRESOURCE(IDS_LINK_DELETE), },
};

static INT_PTR CALLBACK PreferencesFrame_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static PREFPAGE *Preferences_CreatePage(INT pageId, HINSTANCE hInstance, LPCTSTR pszTitle, INT imageId, LPCTSTR pszDialog, DLGPROC dialogProc)
{

	if (NULL == pszTitle ||
		NULL == pszDialog ||
		NULL == dialogProc)
	{
		return NULL;
	}

	PREFPAGE *page = (PREFPAGE*)malloc(sizeof(PREFPAGE));
	if (NULL == page)
		return NULL;

	page->id = pageId;
	page->hInstance = hInstance;
	page->dialogProc = dialogProc;
	page->imageId = imageId;
	
	if (IS_INTRESOURCE(pszTitle))
		page->pszTitle = (LPTSTR)pszTitle;
	else
		page->pszTitle = lfh_strdup(pszTitle);

	if (IS_INTRESOURCE(pszDialog))
		page->pszDialog = (LPTSTR)pszDialog;
	else
		page->pszDialog = lfh_strdup(pszDialog);

	return page;
}

static BOOL Preferences_ReleasePage(PREFPAGE *page)
{
	if (NULL == page)
		return FALSE;

	if (NULL != page->pszTitle && !IS_INTRESOURCE(page->pszTitle))
			lfh_free(page->pszTitle);
	if (NULL != page->pszDialog && !IS_INTRESOURCE(page->pszDialog))
		lfh_free(page->pszDialog);

	free(page);
	return TRUE;
}

BOOL Plugin_RegisterPreferences()
{
	TCHAR szBuffer[128];
	
	if (0 != preferenceInstance.dlgID)
		return FALSE;

	WASABI_API_LNGSTRINGW_BUF(IDS_DROPBOX, szBuffer, ARRAYSIZE(szBuffer));
	if (TEXT('\0') == *szBuffer)
		StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), TEXT("Dropbox"));

	ZeroMemory(&preferenceInstance, sizeof(preferenceInstance));
	preferenceInstance.hInst = WASABI_API_LNG_HINST;
	preferenceInstance.dlgID = IDD_PREFERENCES_FRAME;
	preferenceInstance.name = lfh_strdup(szBuffer);
	preferenceInstance.proc = (void*)PreferencesFrame_DialogProc;;
	preferenceInstance.where = 0; 
	SENDWAIPC(plugin.hwndParent, IPC_ADD_PREFS_DLGW, &preferenceInstance);
	return TRUE;

}

BOOL Plugin_UnregisterPreferences()
{
	if (0 == preferenceInstance.dlgID)
		return FALSE;

	SENDWAIPC(plugin.hwndParent, IPC_REMOVE_PREFS_DLG, &preferenceInstance);

	if (NULL != preferenceInstance.name)
		lfh_free(preferenceInstance.name);

	ZeroMemory(&preferenceInstance, sizeof(preferenceInstance));
	
	size_t index = preferencePages.size();
	
	while(index--)
		Preferences_ReleasePage(preferencePages[index]);
	
	preferencePages.clear();

	return TRUE;
}

BOOL Plugin_ShowPreferences()
{
	if (0 == preferenceInstance.dlgID)
		return FALSE;

	SENDWAIPC(plugin.hwndParent, IPC_OPENPREFSTOPAGE, &preferenceInstance);
	return TRUE;

}

const prefsDlgRecW *Plugin_GetPreferences()
{
	return &preferenceInstance;
}



class PreferencesFrame : public ProfileCallback
{
protected:
	PreferencesFrame(HWND hwndHost) 
		: ref(1), hwnd(hwndHost), profile(NULL)
	{
		winampHook = AttachWinampHook(plugin.hwndParent, WinampHookProc, (ULONG_PTR)this);
	}

	virtual ~PreferencesFrame() 
	{
		SelectProfile(NULL);
		if (NULL != winampHook)
			ReleaseWinampHook(plugin.hwndParent, winampHook);
        		
	}

public:
	static PreferencesFrame *CreateInstance(HWND hwnd)
	{
		PreferencesFrame *instance = new PreferencesFrame(hwnd);
		if (NULL != instance)
		{			
		}
		return instance;
	}

	ULONG AddRef(void) { return InterlockedIncrement((LONG*)&ref); }
	ULONG Release(void) 
	{
		if (0 == ref)
			return ref;
	
		LONG r = InterlockedDecrement((LONG*)&ref);
		if (0 == r)
			delete(this);
		return r;
	}
	
	void Notify(UINT eventId, const UUID *profileUid) 
	{
		SendMessage(hwnd, PREF_MSG_PROFILECHANGED, (WPARAM)eventId, (LPARAM)profileUid);
	}
	
	LRESULT WinampHook(HWAHOOK hWaHook, UINT whcbId, WPARAM param)
	{
		switch(whcbId)
		{
			case WHCB_SYSCOLORCHANGE:
				SendNotifyMessage(hwnd, WM_SYSCOLORCHANGE, 0, 0L);
				break;
		}
		return CallNextWinampHook(hWaHook, whcbId, param);
	}

	static LRESULT CALLBACK WinampHookProc(HWAHOOK hWaHook, UINT whcbId, WPARAM param, ULONG_PTR user)
	{
		PreferencesFrame *instance = (PreferencesFrame*)user;
		if (NULL != instance)
			return instance->WinampHook(hWaHook, whcbId, param);
		return CallNextWinampHook(hWaHook, whcbId, param);
		
	}

	void SelectProfile(Profile *selectProfile)
	{
		if (NULL != profile)
		{			
			profile->Release();
		}
		profile = selectProfile;

		if (NULL != profile)
		{
			profile->AddRef();
		}
	}

	Profile *GetProfile() { return profile; }

	void ActivateCallback(BOOL fActivate)
	{
		if (0 != fActivate)
			PLUGIN_PROFILEMNGR->RegisterCallback(GUID_NULL, this);
		else
			PLUGIN_PROFILEMNGR->UnregisterCallback(GUID_NULL, this);
	}

protected:
	ULONG ref;
	HWND hwnd;
	HWAHOOK winampHook;
	Profile *profile;
};

static UINT Preferences_PageToIndex(INT pageId)
{
	size_t index = preferencePages.size();
	while(index--)
	{
		if (NULL != preferencePages[index] && 
			preferencePages[index]->id == pageId)
		{
			return (UINT)index;
		}
	}
	return PREFPAGE_BADINDEX;
}

UINT Preferences_InsertPage(UINT index, INT pageId, HINSTANCE hInstance, LPCTSTR pszTitle, INT imageId, LPCTSTR pszDialog, DLGPROC dialogProc)
{
	INT existCheck = Preferences_PageToIndex(pageId);
	if (existCheck >= 0)
		return PREFPAGE_BADINDEX;

	if ((UINT)index > preferencePages.size())
		index = (INT)preferencePages.size();

	PREFPAGE *page = Preferences_CreatePage(pageId, hInstance, pszTitle, imageId, pszDialog, dialogProc);
	if (NULL == page)
		return PREFPAGE_BADINDEX;

	if ((UINT)index == preferencePages.size())
		preferencePages.push_back(page);
	else
		preferencePages.insertBefore(index, page);
	
	return index;
}

BOOL Preferences_RemovePage(INT pageId)
{
	INT index = Preferences_PageToIndex(pageId);
	if (index < 0)
		return FALSE;

	PREFPAGE *page = preferencePages[index];
	preferencePages.eraseindex(index);	

	return Preferences_ReleasePage(page);
	
}

UINT Preferences_GetPageCount()
{
	return (UINT)preferencePages.size();
}

Profile *PreferencesFrame_GetProfile(HWND hwnd)
{
	PreferencesFrame *frame = (PreferencesFrame*)GetProp(hwnd, FRAME_PROP);
	return (NULL != frame) ? frame->GetProfile() : NULL;
}

BOOL PreferencesFrame_SelectProfile(HWND hwnd,  Profile *profile)
{
	PreferencesFrame *frame = (PreferencesFrame*)GetProp(hwnd, FRAME_PROP);
	if (NULL == frame)
		return FALSE;
	frame->SelectProfile(profile);
	return TRUE;
}


static BOOL PreferencesFrame_GetBoxRect(HWND hwnd, RECT *prc)
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

static void PreferencesFrame_AdjustWindow(HWND hwnd)
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 == (WS_CHILD & windowStyle)) 
		return;
	
	RECT rcBox;
	if (!PreferencesFrame_GetBoxRect(hwnd, &rcBox))
		return;
	
	SetWindowPos(hwnd, NULL, rcBox.left, rcBox.top, rcBox.right - rcBox.left, rcBox.bottom - rcBox.top, 
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);

}

static void PreferencesFrame_CreateLinks(HWND hwnd)
{
	HWND hControl, insertAfter;
	TCHAR szBuffer[256];
	RECT comboRect;
	SIZE linkSize;
	
	INT spacer = 6;
	SetRect(&comboRect, 0, 0, 3, 3);
	if (MapDialogRect(hwnd, &comboRect))
		spacer = comboRect.right - comboRect.left;

	HFONT windowFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);

	insertAfter = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);
	if (NULL == insertAfter || !GetWindowRect(insertAfter, &comboRect))
		SetRectEmpty(&comboRect);
	else
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&comboRect, 2);

	for (INT i = ARRAYSIZE(szProfileLinks) - 1; i >= 0; i--)
	{
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)szProfileLinks[i].pszText, szBuffer, ARRAYSIZE(szBuffer));
		hControl = CommandLink_CreateWindow(WS_EX_NOPARENTNOTIFY, szBuffer, 
						WS_CHILD | WS_TABSTOP | CLS_HOTTRACK | CLS_DEFAULTCOLORS, 
						0, 0, 0, 0, hwnd, szProfileLinks[i].controlId);
		
		if (NULL != hControl)
		{

			SendMessage(hControl, WM_SETFONT, (WPARAM)windowFont, 0L);
			if (0 == CommandLink_GetIdealSize(hControl, &linkSize))
			{
				linkSize.cx = 75;
				linkSize.cy = 17;
			}

			comboRect.right -= linkSize.cx;
			SetWindowPos(hControl, insertAfter, comboRect.right, comboRect.bottom + 1, linkSize.cx, linkSize.cy, SWP_NOACTIVATE | SWP_NOREDRAW);
			comboRect.right -= spacer;

			ShowWindow(hControl, SW_SHOWNA);
		}
	}
	
}

static BOOL PreferencesFrame_GetViewRect(HWND hwnd, RECT *prcView)
{
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame || NULL == prcView) return FALSE;
	
	RECT tabRect;
	if (!GetWindowRect(hFrame, prcView) || !GetClientRect(hFrame, &tabRect))
		return FALSE;
	
	MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)prcView, 2);
	MapWindowPoints(hFrame, hwnd, (POINT*)&tabRect, 2);
	TabCtrl_AdjustRect(hFrame, FALSE, &tabRect);

	prcView->top = tabRect.top - GetSystemMetrics(SM_CYEDGE);
	
	return TRUE;
}

static void PreferencesFrame_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	RECT rc;
	if (!GetClientRect(hwnd, &rc))
		return;

	RECT controlRect;
	INT profileBarBottom = rc.top;
		
	HWND hControl = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);
	if (NULL != hControl && 
		0 != (WS_VISIBLE & GetWindowLongPtr(hControl, GWL_STYLE)) &&
		FALSE != GetWindowRect(hControl, &controlRect))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);
		profileBarBottom = controlRect.bottom + 4;
		
	}
	

	if(profileBarBottom != rc.top)
			profileBarBottom += 10;

	UINT windowposFlags = SWP_NOACTIVATE | SWP_NOZORDER | ((FALSE == bRedraw) ? SWP_NOREDRAW : 0);


	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL != hFrame)
	{
		CopyRect(&controlRect, &rc);
		controlRect.top = profileBarBottom;
	
		SetWindowPos(hFrame, NULL, controlRect.left, controlRect.top,
				controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, 	windowposFlags);

		HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
		if (NULL != hPage)
		{						
			RECT rcTab;
			GetClientRect(hFrame, &rcTab);
			MapWindowPoints(hFrame, hwnd, (POINT*)&rcTab, 2);

			TabCtrl_AdjustRect(hFrame, FALSE, &rcTab);

			SetWindowPos(hPage, NULL, rcTab.left, rcTab.top, 
					rcTab.right - rcTab.left, rcTab.bottom - rcTab.top, 	windowposFlags);
		}

	}

	HWND hEmpty = GetDlgItem(hwnd, IDC_VIEW_EMPTY);
	if (NULL != hEmpty)
	{				
		PreferencesFrame_GetViewRect(hwnd, &controlRect);
		SetWindowPos(hEmpty, NULL, controlRect.left, controlRect.top,
				controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, 	windowposFlags);
	}
}


static void PreferencesFrame_NotifyTabSelected(HWND hwnd)
{
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame) return;

	NMHDR nmh;
	nmh.code = TCN_SELCHANGE;
	nmh.hwndFrom = hFrame;
	nmh.idFrom = GetDlgCtrlID(hFrame);
	SendNotifyMessage(hwnd, WM_NOTIFY, (WPARAM)nmh.idFrom, (LPARAM)&nmh);
}

static void PreferencesFrame_SelectComboProfile(HWND hwnd)
{	
	Profile *profile = NULL;
	HWND hControl = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);

	if (NULL != hControl)
	{
		INT index = (INT)SendMessage(hControl, CB_GETCURSEL, 0, 0L);
		if (CB_ERR != index)
			profile = (Profile*)SendMessage(hControl, CB_GETITEMDATA, index, 0L);
	}
	
		
	if (profile != PreferencesFrame_GetProfile(hwnd))
	{		
		PreferencesFrame_SelectProfile(hwnd, profile);
		HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
		if (NULL != hPage)
			PreferencesFrame_NotifyTabSelected(hwnd);
	}
}

static BOOL PreferencesFrame_ShowEmptyView(HWND hwnd, BOOL fShow)
{	

	HWND hEmpty = GetDlgItem(hwnd, IDC_VIEW_EMPTY);
	if (fShow)
	{
		if (NULL != hEmpty) 
			return TRUE;
		
		hEmpty = PreferencesEmpty_CreateView(hwnd, IDC_VIEW_EMPTY);
		if (NULL == hEmpty) return FALSE;
		
		RECT controlRect;
		PreferencesFrame_GetViewRect(hwnd, &controlRect);
				
		
		SetWindowPos(hEmpty, GetDlgItem(hwnd, IDC_ACTIVEPAGE), controlRect.left, controlRect.top, 
				controlRect.right - controlRect.left, controlRect.bottom - controlRect.top,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);

	
		HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
		ShowWindow(hEmpty, SW_SHOW);
		
		if (NULL != hFrame)	ShowWindow(hFrame, SW_HIDE);
		
	}
	else
	{
		if (NULL != hEmpty)
		{
			HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
			if (NULL != hFrame)	ShowWindow(hFrame, SW_SHOWNA);

			ShowWindow(hEmpty, SW_HIDE);
			DestroyWindow(hEmpty);
		}
	}
	return TRUE;

}
static INT PreferencesFrame_LoadProfiles(HWND hwnd, const GUID *selectUid)
{
	HWND hCombo = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);
	if (NULL == hCombo) 	return 0;

	INT previousCount = (INT)SendMessage(hCombo, CB_GETCOUNT, 0, 0L);
	if (previousCount > 0)
	{
		UidList *previousList = new UidList();
		if (NULL != previousList)
		{				
			Profile *profile;
			UUID profileUid;
			for (INT i = 0; i < previousCount; i++)
			{
				profile = (Profile*)SendMessage(hCombo, CB_GETITEMDATA, (WPARAM)i, 0L);
				if (NULL != profile && ((Profile*)CB_ERR) != profile && 
					SUCCEEDED(profile->GetUID(&profileUid)))
				{
					previousList->push_back(profileUid);
				}
			}

			if (0 == previousList->size() || !SetProp(hwnd, PREVIOUSLIST_PROP, previousList))
			{
				delete(previousList);
				previousList = NULL;
			}
		}
	}

	SendMessage(hCombo, WM_SETREDRAW, FALSE, 0L);
	SendMessage(hCombo, CB_RESETCONTENT, 0, 0L);
	
	Profile *szProfiles[256];
	
	INT cchLoaded = PLUGIN_PROFILEMNGR->LoadProfiles(szProfiles, ARRAYSIZE(szProfiles));
		
	INT profilesCount = 0;
	INT selectDone = FALSE;
	UUID profileUid;
	for (INT i = 0; i < cchLoaded; i++)
	{
		INT index = (INT)SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)szProfiles[i]);

		if (CB_ERR == index)
			szProfiles[i]->Release();
		else
		{
			if (NULL != selectUid && FALSE == selectDone &&
				SUCCEEDED(szProfiles[i]->GetUID(&profileUid)) &&
				IsEqualGUID(profileUid, *selectUid))
			{

				SendMessage(hCombo, CB_SETCURSEL, index, 0L);
				selectDone = TRUE;
			}

			profilesCount++;
		}
	}

	if (FALSE == selectDone && profilesCount > 0)
		SendMessage(hCombo, CB_SETCURSEL, 0, 0L);

	PreferencesFrame_ShowEmptyView(hwnd, 0 == profilesCount);

	INT szLinks[] = { IDC_LINK_DELETE, IDC_LINK_DUPLICATE, IDC_TABFRAME, IDC_COMBO_SELECTPROFILE};

	HWND hFocus = GetFocus();

	for (INT i = 0; i < ARRAYSIZE(szLinks); i++)
	{
		HWND hLink = GetDlgItem(hwnd, szLinks[i]);
		if (NULL != hLink) 
		{
			if (0 == profilesCount && NULL != hFocus)
			{
				HWND hStart = hFocus;
				while (hFocus == hLink || IsChild(hLink, hFocus))
				{
					SendMessage(hwnd, WM_NEXTDLGCTL, 0, 0L);
					hFocus = GetFocus();
					if (NULL == hFocus || hStart == hFocus) break;
				}
			}
			EnableWindow(hLink, (0 != profilesCount));
		}
	}

	SendMessage(hCombo, WM_SETREDRAW, TRUE, 0L);
	InvalidateRect(hCombo, NULL, TRUE);

	UidList *previousList = (UidList*)GetProp(hwnd, PREVIOUSLIST_PROP);
    RemoveProp(hwnd, PREVIOUSLIST_PROP);
	if (NULL != previousList)
		delete(previousList);

	return profilesCount;
}

static void PreferencesFrame_DrawProfileComboItem(Profile *profile, HDC hdc, const RECT *prcItem, UINT itemState)
{
	TCHAR szText[256];
	INT cchText;
	
	if (((Profile*)-1) == profile || 
		NULL == profile || 
		FAILED(profile->GetName(szText, ARRAYSIZE(szText))))
	{
		if (((Profile*)-1) == profile)
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_PROFILE_COMBOBOX_EMPTY, szText, ARRAYSIZE(szText));
		}
		else
		{
			szText[0] = TEXT('\0');
		}
	}

	cchText = lstrlen(szText);	

	COLORREF rgbBk, rgbFg; 
	if (0 != (ODS_DISABLED & itemState))
	{
		rgbBk = GetSysColor(COLOR_BTNFACE);
		rgbFg = GetSysColor(COLOR_GRAYTEXT);
	}
	else
	{
		if (0 != (ODS_SELECTED & itemState))
		{
			rgbBk = GetSysColor(COLOR_HIGHLIGHT);
			rgbFg = GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else
		{
			rgbBk = GetSysColor(COLOR_WINDOW);
			rgbFg = GetSysColor(COLOR_WINDOWTEXT);
		}
	}
	
	COLORREF rgbBkOld = SetBkColor(hdc, rgbBk);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcItem, NULL, 0, NULL);
	if (0 != cchText)
	{				
		COLORREF rgbFgOld = SetTextColor(hdc, rgbFg);
		RECT textRect;
		SetRect(&textRect, prcItem->left + 2, prcItem->top + 2, prcItem->right - 1, prcItem->bottom - 1);
		
		UINT drawtextFlags = DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS;
		if (((Profile*)-1) == profile) drawtextFlags |= DT_CENTER;
		
		DrawText(hdc, szText, cchText, &textRect, drawtextFlags);
		
		if (rgbFgOld != rgbFg)
			SetBkColor(hdc, rgbFgOld);
	}

	if (rgbBkOld != rgbBk)
		SetBkColor(hdc, rgbBkOld);

	if (ODS_FOCUS == ((ODS_NOFOCUSRECT | ODS_FOCUS) & itemState))
	{
		DrawFocusRect(hdc, prcItem);
	}

}


static void PreferencesFrame_CreateProfile(HWND hwnd)
{
	Profile *profile = Profile::Create(hwnd);
	if (NULL != profile) 
		profile->Release();
}

static void PreferencesFrame_DeleteProfile(HWND hwnd)
{
	Profile *profile = NULL;
	HWND hControl = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);

	if (NULL != hControl)
	{
		INT index = (INT)SendMessage(hControl, CB_GETCURSEL, 0, 0L);
		if (CB_ERR != index)
			profile = (Profile*)SendMessage(hControl, CB_GETITEMDATA, index, 0L);
	}

	if (NULL != profile)
	{
		MessageBoxTweak::CTRLOVERRIDE szButtons[] = 
		{
			{IDYES, MAKEINTRESOURCEW(IDS_BUTTON_DELETE)},
			{IDNO, MAKEINTRESOURCEW(IDS_BUTTON_CANCEL)},
		};

		DWORD tweakFlags = MessageBoxTweak::TWEAK_CENTERPARENT;

		TCHAR szName[256], szTemplate[512], szMessage[1024];

		WASABI_API_LNGSTRINGW_BUF(IDS_PROFILE_DELETE, szTemplate, ARRAYSIZE(szTemplate));
		
		if (FAILED(profile->GetName(szName, ARRAYSIZE(szName))) ||
			FAILED(StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szTemplate, szName)) || 
			0 == lstrlen(szMessage))
		{
			StringCchPrintf(szMessage, ARRAYSIZE(szMessage), TEXT("Are you sure you want to delete profile"));
		}
	
		INT result = MessageBoxTweak::ShowEx(hwnd, szMessage,
							MAKEINTRESOURCE(IDS_PROFILE_DELETE_TITLE), 
							MB_YESNO | MB_ICONQUESTION, tweakFlags, szButtons, ARRAYSIZE(szButtons));
		if (IDYES != result)
			return;
	}

	if (NULL == profile || FAILED(profile->Delete()))
	{
		MessageBox(hwnd, TEXT("Unable to delete profile."), TEXT("Error"), MB_OK | MB_ICONERROR);
	}
}

static void PreferencesFrame_DuplicateProfile(HWND hwnd)
{
	Profile *profile = NULL;
	HWND hControl = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);

	if (NULL != hControl)
	{
		INT index = (INT)SendMessage(hControl, CB_GETCURSEL, 0, 0L);
		if (CB_ERR != index)
			profile = (Profile*)SendMessage(hControl, CB_GETITEMDATA, index, 0L);
	}
	
	Profile *duplicate = (NULL != profile) ? Profile::CreateCopy(profile) : NULL;
	if (NULL == duplicate)
	{
		MessageBox(hwnd, TEXT("Unable to duplicate profile."), TEXT("Error"), MB_OK | MB_ICONERROR);
	}
	else
	{
		duplicate->Release();
	}
}

static INT_PTR PreferencesFrame_OnInit(HWND hwnd, HWND hFocus, LPARAM param)
{
	HWND hFrame = GetDlgItem(hwnd, IDC_TABFRAME);
	if (NULL == hFrame)
		return FALSE;

	PreferencesFrame *frame = PreferencesFrame::CreateInstance(hwnd);
	if (NULL != frame && !SetProp(hwnd, FRAME_PROP, frame))
	{
		frame->Release();
		frame = NULL;
	}
	else
	{
		frame->ActivateCallback(TRUE);
	}
	
	HWND hCombo = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);
	if (NULL != hCombo)
	{
		SendMessage(hCombo,  0x1701/*CB_SETMINVISIBLE*/, 6, 0L);
		SendMessage(hCombo, CB_SETEXTENDEDUI, TRUE, 0L);
	}

	PreferencesFrame_CreateLinks(hwnd);
	PreferencesFrame_LoadProfiles(hwnd, NULL);
	PreferencesFrame_SelectComboProfile(hwnd);
		
	if (!SENDWAIPC(plugin.hwndParent, IPC_USE_UXTHEME_FUNC, IPC_ISWINTHEMEPRESENT))
		SENDWAIPC(plugin.hwndParent, IPC_USE_UXTHEME_FUNC, hFrame);

	TabCtrl_SetMinTabWidth(hFrame, 60);

	INT pageCount = 0;
	TCHAR szBuffer[64];
	LPCTSTR pszText;
	TCITEM tabItem;
	tabItem.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;

	INT activateIndex = 0;

	for (size_t i = 0; i < preferencePages.size(); i++)
	{
		PREFPAGE *page = preferencePages[i];
		
		tabItem.lParam = page->id;
		tabItem.iImage = page->imageId;
		
		if (IS_INTRESOURCE(page->pszTitle))
		{
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)page->pszTitle, szBuffer, ARRAYSIZE(szBuffer));
			if (TEXT('\0') == *szBuffer)
				StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), TEXT("Unknown"));
			pszText = szBuffer;
		}
		else
			pszText = page->pszTitle;

		tabItem.pszText = (LPTSTR)pszText;
		if (-1 != TabCtrl_InsertItem(hFrame, pageCount, &tabItem))
		{
			if (page->id == preferenceLastPageId)
				activateIndex = pageCount;

			pageCount++;
		}
	}

	TabCtrl_SetCurSel(hFrame, activateIndex);
	PreferencesFrame_NotifyTabSelected(hwnd);
	
	PostMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent) Preferences_EnableWarning(hParent, TRUE);
	return FALSE;
}

static void PreferencesFrame_OnDestroy(HWND hwnd)
{
	PreferencesFrame *frame = (PreferencesFrame*)GetProp(hwnd, FRAME_PROP);
	RemoveProp(hwnd, FRAME_PROP);
	if (NULL != frame)
	{
		frame->ActivateCallback(FALSE);
		frame->SelectProfile(NULL);
		frame->Release();
	}


	HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
	if (NULL != hPage)
		DestroyWindow(hPage);
	
	PreferencesFrame_SelectProfile(hwnd, NULL);

	HWND hCombo = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);
	if (NULL != hCombo)
		SendMessage(hCombo, CB_RESETCONTENT, 0, 0L);

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent) Preferences_EnableWarning(hParent, FALSE);
}


static void PreferencesFrame_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (0 == (SWP_NOMOVE & pwp->flags))
	{
		PreferencesFrame_AdjustWindow(hwnd);
		return;
	}
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;	
	
	PreferencesFrame_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)));

}

static void PreferencesFrame_OnTabSelected(HWND hwnd, HWND hFrame)
{
	HWND hPageOld = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
	if (NULL != hPageOld)
		SetWindowLongPtr(hPageOld, GWLP_ID, 0);
	
	preferenceLastPageId = PREFPAGE_INVALID;

	if (NULL == hFrame)
	{
		if (NULL != hPageOld)
			DestroyWindow(hPageOld);
		return;
	}

	INT iSelected = TabCtrl_GetCurSel(hFrame);
	TCITEM tabItem;
	tabItem.mask = TCIF_PARAM;
	INT pageId = (TRUE == TabCtrl_GetItem(hFrame, iSelected, &tabItem)) ? (INT)tabItem.lParam : PREFPAGE_INVALID;
	INT pageIndex = (PREFPAGE_INVALID != pageId) ? Preferences_PageToIndex(pageId) : PREFPAGE_BADINDEX;
	
	HWND hPage = NULL;
	if (pageIndex >= 0)
	{
		PREFPAGE *page = preferencePages[pageIndex];
		Profile *activeProfile = PreferencesFrame_GetProfile(hwnd);
		hPage = WASABI_API_CREATEDIALOGPARAMW((INT)(INT_PTR)page->pszDialog, hwnd, page->dialogProc, (LPARAM)activeProfile);
		if (NULL != hPage)
		{
			RECT rcTab;
			GetClientRect(hFrame, &rcTab);
			MapWindowPoints(hFrame, hwnd, (POINT*)&rcTab, 2);

			SetWindowLongPtr(hPage, GWLP_ID, IDC_ACTIVEPAGE);
			
			TabCtrl_AdjustRect(hFrame, FALSE, &rcTab);
			
			SetWindowPos(hPage, hFrame, rcTab.left, rcTab.top, rcTab.right - rcTab.left, rcTab.bottom - rcTab.top,
						SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOREDRAW);
			
			SetWindowPos(hFrame, hPage, 0, 0, 0, 0, 	SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW);

			if (!SENDWAIPC(plugin.hwndParent, IPC_USE_UXTHEME_FUNC, IPC_ISWINTHEMEPRESENT))
				SENDWAIPC(plugin.hwndParent, IPC_USE_UXTHEME_FUNC, hPage);

			preferenceLastPageId = pageId;
		}
	}

	if (NULL != hPage)
	{
		ShowWindow(hPage, SW_SHOWNA);
	}
	
	if (NULL != hPageOld)
	{
		ShowWindow(hPageOld, SW_HIDE);
		if (NULL != hPage)
			RedrawWindow(hPage, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_NOINTERNALPAINT | RDW_NOERASE);

		DestroyWindow(hPageOld);
	}


}


static void PreferencesFrame_OnLinkClick(HWND hwnd, INT linkId)
{
	switch(linkId)
	{
		case IDC_LINK_CREATE:	PreferencesFrame_CreateProfile(hwnd); break;
		case IDC_LINK_DELETE:	PreferencesFrame_DeleteProfile(hwnd); break;
		case IDC_LINK_DUPLICATE:PreferencesFrame_DuplicateProfile(hwnd); break;
	}
}

static LRESULT PreferencesFrame_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	switch(controlId)
	{
		case IDC_TABFRAME:
			switch(pnmh->code)
			{
				case TCN_SELCHANGE:
					PreferencesFrame_OnTabSelected(hwnd, pnmh->hwndFrom);
					break;
			}
			break;

		case IDC_LINK_CREATE:
		case IDC_LINK_DELETE:
		case IDC_LINK_DUPLICATE:
			if (NM_CLICK == pnmh->code)
				PreferencesFrame_OnLinkClick(hwnd, controlId);
			return TRUE;
	}
	return 0;
}

static void PreferencesFrame_OnProfileComboCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(eventId)
	{
		case CBN_SELCHANGE:
			PreferencesFrame_SelectComboProfile(hwnd);
			break;
	}
}
static void PreferencesFrame_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(controlId)
	{
		case IDC_COMBO_SELECTPROFILE:
			PreferencesFrame_OnProfileComboCommand(hwnd, controlId, eventId, hControl);
			break;
	}

}
static LRESULT PreferencesFrame_OnDeleteItem(HWND hwnd, INT controlId, DELETEITEMSTRUCT *pdis)
{
	switch(controlId)
	{
		case IDC_COMBO_SELECTPROFILE:
			if (NULL != pdis->itemData)
				((Profile*)pdis->itemData)->Release();
			return TRUE;
	}
	return FALSE;
}

static LRESULT PreferencesFrame_OnMeasureItem(HWND hwnd, INT controlId, MEASUREITEMSTRUCT *pmis)
{
	HWND hControl = GetDlgItem(hwnd, controlId);
	switch(controlId)
	{
		case IDC_COMBO_SELECTPROFILE:
			if (NULL != hControl)
			{
				HDC hdc = GetDCEx(hControl, NULL, DCX_CACHE | DCX_NORESETATTRS);
				if (NULL != hdc)
				{
					HFONT hf = (HFONT)SendMessage(hControl, WM_GETFONT, 0, 0L);
					if (NULL == hf) hf = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
					if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

					HFONT hfo = (HFONT)SelectObject(hdc, hf);
					TEXTMETRIC tm;
					
					if (GetTextMetrics(hdc, &tm))
						pmis->itemHeight = tm.tmHeight + 3;

					SelectObject(hdc, hfo);
					ReleaseDC(hControl, hdc);
				}
			}
			return TRUE;
	}
	return FALSE;
}


static LRESULT PreferencesFrame_OnDrawItem(HWND hwnd, INT controlId, DRAWITEMSTRUCT *pdis)
{
	switch(controlId)
	{
		case IDC_COMBO_SELECTPROFILE:
			PreferencesFrame_DrawProfileComboItem((Profile*)pdis->itemData, pdis->hDC, &pdis->rcItem, pdis->itemState);
			return TRUE;
	}
	return FALSE;
}

static INT PreferencesFrame_GetProfilePreviousIndex(UidList *list, Profile *profile)
{
	UUID profileUid;
	if (NULL != list && SUCCEEDED(profile->GetUID(&profileUid))) 
	{
		for(size_t i = 0; i < list->size(); i++)
		{
			if (IsEqualGUID(list->at(i), profileUid))
			{
				return	(INT)i; 
			}
		}
	}
	return 0x0FFFFFFF;

}
static INT PreferencesFrame_CompareProfiles(HWND hwnd, LCID lcid, Profile *p1, Profile *p2)
{	
	if (NULL == p1 || NULL == p2) return (INT)(INT_PTR)(p1 - p2);
	
	TCHAR szText1[256], szText2[256];
	UidList *previousList = (UidList*)GetProp(hwnd, PREVIOUSLIST_PROP);
	
	INT index1 = PreferencesFrame_GetProfilePreviousIndex(previousList, p1);
	INT index2 = PreferencesFrame_GetProfilePreviousIndex(previousList, p2);
	
	if (index1 != index2) 
		return (index1 - index2);

	if (FAILED(p1->GetName(szText1, ARRAYSIZE(szText1))))
	{
		szText1[0] = TEXT('\0');
	}

	if (FAILED(p2->GetName(szText2, ARRAYSIZE(szText2))))
	{
		szText2[0] = TEXT('\0');
	}

	return (CompareString(lcid, 0, szText1, -1, szText2, -1) - 2);
}

static LRESULT PreferencesFrame_OnCompareItem(HWND hwnd, INT controlId, COMPAREITEMSTRUCT *pcis)
{
	
	switch(controlId)
	{
		case IDC_COMBO_SELECTPROFILE:
			return PreferencesFrame_CompareProfiles(hwnd, pcis->dwLocaleId, 
						(Profile*)pcis->itemData1, (Profile*)pcis->itemData2);

	}
	return 0;
}

static void PreferencesFrame_OnSysColorChange(HWND hwnd)
{
	HWND hPage = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
	if (NULL != hPage)
		SendNotifyMessage(hPage, WM_SYSCOLORCHANGE, 0, 0L);
}

static void PreferencesFrame_OnProfileChanged(HWND hwnd, UINT eventId, const GUID *profileUid)
{
	HWND hControl;
	switch(eventId)
	{
		case ProfileCallback::eventNameChanged:
			hControl = GetDlgItem(hwnd, IDC_COMBO_SELECTPROFILE);
			if (NULL != hControl)
				InvalidateRect(hControl, NULL, TRUE);
			break;

		case ProfileCallback::eventProfileCreated:
			PreferencesFrame_LoadProfiles(hwnd, profileUid);
			PreferencesFrame_SelectComboProfile(hwnd);
			break;

		case ProfileCallback::eventProfileDeleted:
			PreferencesFrame_LoadProfiles(hwnd, NULL);
			PreferencesFrame_SelectComboProfile(hwnd);
			break;
	}

	hControl = GetDlgItem(hwnd, IDC_ACTIVEPAGE);
	if (NULL != hControl)
	{
		Profile *selecteProfile = PreferencesFrame_GetProfile(hwnd);
		UUID selectedUid;

		if (NULL != selecteProfile && 
			SUCCEEDED(selecteProfile->GetUID(&selectedUid)) &&
			IsEqualGUID(*profileUid, selectedUid))
		{
			SendMessage(hControl, PREF_MSG_PROFILECHANGED, (WPARAM)eventId, (LPARAM)profileUid);
		}
	}
}



static INT_PTR CALLBACK PreferencesFrame_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:	return PreferencesFrame_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		PreferencesFrame_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED: PreferencesFrame_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_NOTIFY:		MSGRESULT(hwnd, PreferencesFrame_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam));
		case WM_COMMAND:		PreferencesFrame_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_DELETEITEM:	MSGRESULT(hwnd, PreferencesFrame_OnDeleteItem(hwnd, (INT)wParam, (DELETEITEMSTRUCT*)lParam));	
		case WM_MEASUREITEM:	MSGRESULT(hwnd, PreferencesFrame_OnMeasureItem(hwnd, (INT)wParam, (MEASUREITEMSTRUCT*)lParam));	
		case WM_DRAWITEM:	MSGRESULT(hwnd, PreferencesFrame_OnDrawItem(hwnd, (INT)wParam, (DRAWITEMSTRUCT*)lParam));	
		case WM_COMPAREITEM:	return PreferencesFrame_OnCompareItem(hwnd, (INT)wParam, (COMPAREITEMSTRUCT*)lParam);
		case WM_SYSCOLORCHANGE:	PreferencesFrame_OnSysColorChange(hwnd); break;

		case PREF_MSG_PROFILECHANGED:	PreferencesFrame_OnProfileChanged(hwnd, (UINT)wParam, (const GUID*)lParam); return TRUE; 
	}
	return 0;
}