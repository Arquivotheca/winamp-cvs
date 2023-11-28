#include "main.h"
#include "./plugin.h"
#include "./resource.h"
#include "./preferences.h"
#include "./wasabiApi.h"
#include "./dropWindow.h"

#include "./itemViewManager.h"
#include "./itemViewMeta.h"

#include "./groupedList.h"
#include "./guiObjects.h"
#include "./groupHeader.h"

#include <windows.h>
#include <strsafe.h>


#define IDC_GROUPVIEW		10000
#define IDC_GROUPHEADER		10001

#define PROFILE_PROP	TEXT("PROFLE_PROP")

static INT_PTR CALLBACK PreferencesView_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern GLRoot *PreferencesView_CreateGroup(INT activeViewId);

BOOL WINAPI PreferencesView_RegisterPage()
{
	return Preferences_InsertPage(-1, PREFPAGE_VIEW, WASABI_API_LNG_HINST, 
							MAKEINTRESOURCE(IDS_PREFPAGE_VIEW), 
							-1,
							MAKEINTRESOURCE(IDD_PREFPAGE_VIEW), 
							PreferencesView_DialogProc);
}
			
static void PreferencesView_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	HWND hctrl;
	RECT rc, rcControl;

	GetClientRect(hwnd, &rc);
	
	LONG top = rc.top;
	UINT windowposFlags = SWP_NOACTIVATE | SWP_NOZORDER | ((FALSE == bRedraw) ? SWP_NOREDRAW : 0);

	if (NULL != (hctrl = GetDlgItem(hwnd, IDC_GROUPHEADER)) &&
		0 != (WS_VISIBLE & GetWindowLongPtr(hctrl, GWL_STYLE)))
	{
		CopyRect(&rcControl, &rc);
		if (GroupHeader_AdjustRect(hctrl, &rcControl))
		{
			SetWindowPos(hctrl, NULL, rcControl.left, rcControl.top, 
							rcControl.right - rcControl.left, rcControl.bottom - rcControl.top, 
							windowposFlags);
			
			top = rcControl.bottom;
		}
	}

	if (NULL != (hctrl = GetDlgItem(hwnd, IDC_GROUPVIEW)))
	{		
		SetRect(&rcControl, rc.left, top, rc.right, rc.bottom);
		if (rcControl.top > rcControl.bottom) rcControl.top = rcControl.bottom;
		SetWindowPos(hctrl, NULL, rcControl.left, rcControl.top, 
						rcControl.right - rcControl.left, rcControl.bottom - rcControl.top, 
						windowposFlags);		
	}
}

typedef struct __PREFVIEW
{
	INT		itemId;
	LPCTSTR pszImage;
} PREFVIEW;


static INT_PTR PreferencesView_OnInit(HWND hwnd, HWND hFocus, LPARAM param)
{	
	GLRoot *root = NULL;
	
	Profile *profile = (Profile*)param;
	if (NULL != profile &&
		FALSE != SetProp(hwnd, PROFILE_PROP, (HANDLE)profile))
	{
		profile->AddRef();
	}

	DropboxViewMeta *viewMeta = NULL;
	IConfiguration *pConfig;
	if (NULL != profile && SUCCEEDED(profile->QueryConfiguration(windowSettingsGuid, &pConfig)))
	{
		TCHAR szBuffer[512];
		if (SUCCEEDED(pConfig->ReadString(CFG_ACTIVEVIEW, szBuffer, ARRAYSIZE(szBuffer))))
			viewMeta = PLUGIN_VIEWMNGR->FindByName(szBuffer);
		pConfig->Release();
	}

	if (NULL == viewMeta)
		viewMeta = PLUGIN_VIEWMNGR->First();
	

	TCHAR szBuffer[1024];
	WASABI_API_LNGSTRINGW_BUF(IDS_PREFPAGE_VIEW_DESC, szBuffer, ARRAYSIZE(szBuffer));
	GroupHeader_RegisterClass(plugin.hDllInstance);
	HWND groupHeader = GroupHeader_CreateWindow(WS_EX_NOPARENTNOTIFY, 
							szBuffer, 
							WS_CHILD | WS_VISIBLE | GHS_DEFAULTCOLORS,
							0, 0, 1, 1, hwnd, IDC_GROUPHEADER, plugin.hDllInstance); 

	root = PreferencesView_CreateGroup((NULL != viewMeta) ? viewMeta->GetId() : 0);
	HWND groupView = GroupedListView_CreateWindow(WS_EX_NOPARENTNOTIFY, 
						WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_VISIBLE, 
						0, 0, 1, 1, 
						hwnd, IDC_GROUPVIEW, plugin.hDllInstance, root,	NULL);
	
	if (NULL != root)
		root->Release();

	HFONT windowFont =  (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
	
	if (NULL != groupView)
		SendMessage(groupView, WM_SETFONT, (WPARAM)windowFont, (LPARAM)0);
	
	return FALSE;
}

static void PreferencesView_OnDestroy(HWND hwnd)
{
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	RemoveProp(hwnd, PROFILE_PROP);

	if (NULL != profile)
	{		
		profile->Release();
	}
	
}

static void PreferencesView_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;	
	PreferencesView_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)));
}

static void PreferencesView_OnSysColorChange(HWND hwnd)
{
	HWND hList = GetDlgItem(hwnd, IDC_GROUPVIEW);
	if (NULL != hList)
		SendNotifyMessage(hList, WM_SYSCOLORCHANGE, 0, 0L);
}

static void PreferencesView_OnItemStyleChanged(HWND hwnd, NMGLITEM *pnmi)
{
	UINT oldVal, newVal;
	oldVal = (GLItem::FlagValueMask & pnmi->styleOld);
	newVal = (GLItem::FlagValueMask & pnmi->styleNew);
	if (oldVal == newVal || GLButton::FlagButtonChecked != newVal)
		return;
	
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	if (NULL != profile)
	{
		DropboxViewMeta *viewMeta = PLUGIN_VIEWMNGR->FindById(pnmi->item->GetId());
		if (NULL != viewMeta)
		{
			IConfiguration *pConfig;
			if (NULL != profile && 
				SUCCEEDED(profile->QueryConfiguration(windowSettingsGuid, &pConfig)))
			{
				HRESULT hr;
				hr = pConfig->WriteString(CFG_ACTIVEVIEW, viewMeta->GetName());
				pConfig->Release();

				if (SUCCEEDED(hr))
				{
					profile->Notify(ProfileCallback::eventViewChanged);
					HWND hParent = GetParent(hwnd);
					if (NULL != hParent) hParent = GetParent(hParent);
					if (NULL != hParent) Preferences_ShowWarning(hParent, TRUE);
				}
			}
		}
	}

}

static void PreferencesView_OnItemClicked(HWND hwnd, NMGLITEM *pnmi)
{
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	if (NULL != profile)
	{
		DropboxViewMeta *viewMeta = PLUGIN_VIEWMNGR->FindById(pnmi->item->GetId());
		if (NULL != viewMeta)
			viewMeta->ShowEditor(hwnd, profile);
	}

}
static LRESULT PreferencesView_OnNotify(HWND hwnd, NMHDR *pnmh)
{
	switch(pnmh->idFrom)
	{
		case IDC_GROUPVIEW:
			switch(pnmh->code)
			{
				case GLVN_ITEMSTYLECHANGED: PreferencesView_OnItemStyleChanged(hwnd, (NMGLITEM*)pnmh); break;
				case NM_CLICK:				PreferencesView_OnItemClicked(hwnd, (NMGLITEM*)pnmh); break;
			}
			break;
	}
	return 0;
}



static INT_PTR CALLBACK PreferencesView_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return PreferencesView_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:				PreferencesView_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	PreferencesView_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_SYSCOLORCHANGE:		PreferencesView_OnSysColorChange(hwnd); break;
		case WM_NOTIFY:				MSGRESULT(hwnd, PreferencesView_OnNotify(hwnd, (NMHDR*)lParam)); return TRUE;

	}
	return 0;
}