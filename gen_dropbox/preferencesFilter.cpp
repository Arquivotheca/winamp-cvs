#include "main.h"
#include "./plugin.h"
#include "./resource.h"
#include "./preferences.h"
#include "./wasabiApi.h"
#include "./filterPolicy.h"

#include "./groupedList.h"
#include "./groupHeader.h"

#include <windows.h>
#include <strsafe.h>

#define IDC_GROUPVIEW		10000
#define IDC_GROUPHEADER		10001

#define PROFILE_PROP	TEXT("PROFLE_PROP")

static INT_PTR CALLBACK PreferencesFilter_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


BOOL WINAPI PreferencesFilter_RegisterPage()
{
	return Preferences_InsertPage(-1, PREFPAGE_FILTER, WASABI_API_LNG_HINST, 
							MAKEINTRESOURCE(IDS_PREFPAGE_FILTER), 
							-1,
							MAKEINTRESOURCE(IDD_PREFPAGE_FILTER), 
							PreferencesFilter_DialogProc);
	
}

		
static void PreferencesFilter_UpdateLayout(HWND hwnd, BOOL bRedraw)
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


typedef struct __TYPEGROUPPARAM
{
	FilterPolicy *filterPolicy;
	GLRoot *root;
} TYPEGROUPPARAM;

static BOOL CALLBACK PreferenceFilter_InsertTypeGroup(IItemType *itemType, ULONG_PTR user)
{
	TYPEGROUPPARAM *param = (TYPEGROUPPARAM*)user;
	
	TCHAR szBuffer[256];
	
	if (SUCCEEDED(itemType->GetDisplayName(szBuffer, ARRAYSIZE(szBuffer))))
	{
		GLGroup *lg = GLGroup::CreateInstance(itemType->GetId(), itemType->GetIconId(), szBuffer, 0);
		
		param->root->InsertItem(-1, lg);
		
		UINT typeCaps = itemType->GetCapabilities();
		BYTE szRules[] = {	FilterPolicy::entryRuleAsk, 
							FilterPolicy::entryRuleAdd,
							FilterPolicy::entryRuleIgnore,
							FilterPolicy::entryRuleEnumerate,};
		
		BYTE ruleSelected  = (NULL != param->filterPolicy) ? 
								param->filterPolicy->GetRule(itemType->GetId()) : 
								FilterPolicy::entryRuleError;

		INT cchRules = ARRAYSIZE(szRules);
		if (0 == (IItemType::typeCapEnumerate & typeCaps))
			cchRules -= 1;

		for (INT i = 0; i < cchRules; i++)
		{
			if (SUCCEEDED(FilterPolicy::GetRuleDisplayName(szRules[i], szBuffer, ARRAYSIZE(szBuffer))))
			{
				GLItem *item = GLButton::CreateRadiobutton(szRules[i], szBuffer, (szRules[i] == ruleSelected));
				lg->InsertItem(-1, item);
				item->Release();
			}
		}
		
		lg->Release();
		
	}
	return TRUE;
}

static GLRoot *PreferencesFilter_CreateGroup(FilterPolicy  *filterPolicy)
{	
	GLRoot *root = GLRoot::Create();
	if (NULL == root ) return NULL;
	
	if (NULL != PLUGIN_REGTYPES)
	{
		TYPEGROUPPARAM param;
		param.filterPolicy = filterPolicy;
		param.root = root;
		PLUGIN_REGTYPES->Enumerate(PreferenceFilter_InsertTypeGroup, (ULONG_PTR)&param);
	}
	
	root->Sort();
	return root;
}

static INT_PTR PreferencesFilter_OnInit(HWND hwnd, HWND hFocus, LPARAM param)
{	
	GLRoot *root = NULL;
	
	Profile *profile = (Profile*)param;
	if (NULL != profile &&
		FALSE != SetProp(hwnd, PROFILE_PROP, (HANDLE)profile))
	{
		profile->AddRef();
	}

	FilterPolicy *filterPolicy;
	if (NULL == profile || FAILED(profile->GetFilterPolicy(&filterPolicy, FALSE)))
		filterPolicy = NULL;
	
	if (NULL != filterPolicy)
	{
		root = PreferencesFilter_CreateGroup(filterPolicy);
		filterPolicy->Release();
	}
	TCHAR szBuffer[1024];
	WASABI_API_LNGSTRINGW_BUF(IDS_PREFPAGE_FILTER_DESC, szBuffer, ARRAYSIZE(szBuffer));
	GroupHeader_RegisterClass(plugin.hDllInstance);
	HWND groupHeader = GroupHeader_CreateWindow(WS_EX_NOPARENTNOTIFY, 
							szBuffer, 
							WS_CHILD | WS_VISIBLE | GHS_DEFAULTCOLORS,
							0, 0, 1, 1, hwnd, IDC_GROUPHEADER, plugin.hDllInstance); 

	HWND groupView = GroupedListView_CreateWindow(WS_EX_NOPARENTNOTIFY, 
						WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_VISIBLE, 
						0, 0, 1, 1, 
						hwnd, IDC_GROUPVIEW, plugin.hDllInstance, root,
						MAKEINTRESOURCE(IDR_SMALLFILETYPES_IMAGE));
	
	if (NULL != root)
		root->Release();

	HFONT windowFont =  (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
	
	if (NULL != groupView)
		SendMessage(groupView, WM_SETFONT, (WPARAM)windowFont, (LPARAM)0);
	
	return FALSE;
}

static void PreferencesFilter_OnDestroy(HWND hwnd)
{
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	RemoveProp(hwnd, PROFILE_PROP);

	if (NULL != profile)
	{
		FilterPolicy *filterPolicy;
		if (SUCCEEDED(profile->GetFilterPolicy(&filterPolicy, FALSE)))
		{
			filterPolicy->Save(profile);
			filterPolicy->Release();
		}
		profile->Release();
	}
	
}

static void PreferencesFilter_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;	
	PreferencesFilter_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)));
}

static void PreferencesFilter_OnSysColorChange(HWND hwnd)
{
	HWND hList = GetDlgItem(hwnd, IDC_GROUPVIEW);
	if (NULL != hList)
		SendNotifyMessage(hList, WM_SYSCOLORCHANGE, 0, 0L);
}

static void PreferencesFilter_OnItemStyleChanged(HWND hwnd, NMGLITEM *pnmi)
{
	UINT oldVal, newVal;
	oldVal = (GLItem::FlagValueMask & pnmi->styleOld);
	newVal = (GLItem::FlagValueMask & pnmi->styleNew);
	if (oldVal == newVal || GLButton::FlagButtonChecked != newVal)
		return;
	
	GLItem *group = pnmi->item->GetParent();
	if (NULL == group)
		return;

	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	FilterPolicy *filterPolicy;
	if (NULL != profile && SUCCEEDED(profile->GetFilterPolicy(&filterPolicy, FALSE)))
	{
		filterPolicy->SetRule(group->GetId(), pnmi->item->GetId());
		filterPolicy->Save(profile);
		filterPolicy->Release();
	}
}

static LRESULT PreferencesFilter_OnNotify(HWND hwnd, NMHDR *pnmh)
{
	switch(pnmh->idFrom)
	{
		case IDC_GROUPVIEW:
			switch(pnmh->code)
			{
				case GLVN_ITEMSTYLECHANGED: PreferencesFilter_OnItemStyleChanged(hwnd, (NMGLITEM*)pnmh); break;
			}
			break;
	}
	return 0;
}
static INT_PTR CALLBACK PreferencesFilter_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return PreferencesFilter_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:				PreferencesFilter_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	PreferencesFilter_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_SYSCOLORCHANGE:		PreferencesFilter_OnSysColorChange(hwnd); break;
		case WM_NOTIFY:				MSGRESULT(hwnd, PreferencesFilter_OnNotify(hwnd, (NMHDR*)lParam)); return TRUE;
	}
	return 0;
}