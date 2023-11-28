#include "main.h"
#include "./documentFilter.h"
#include "./filterPolicy.h"
#include "./fileInfoInterface.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./plugin.h"
#include "./itemTypeInterface.h"

#include <shlwapi.h>
#include <strsafe.h>

#define CENTER_X		-20000
#define CENTER_Y			-20000

typedef struct __FILTERDIALOGPARAM
{
	IFileInfo		*item;
	FilterPolicy	*filterPolicy;
	HWND				hHost;
	POINT			position;
	BOOL			applyToAll;
} FILTERDIALOGPARAM;

#define FILTERDLG_PROP		TEXT("waDropboxFilterDialogParam")
#define SetFilterProp(__hwnd, __data) SetProp((__hwnd), FILTERDLG_PROP, (HANDLE)(__data))
#define GetFilterProp(__hwnd) ((FILTERDIALOGPARAM*)GetProp((__hwnd), FILTERDLG_PROP))
#define RemoveFilterProp(__hwnd) RemoveProp((__hwnd), FILTERDLG_PROP)

ReloadFilter::ReloadFilter() : ref(1) 
{
}

ReloadFilter::~ReloadFilter()
{
}

STDMETHODIMP_(ULONG) ReloadFilter::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) ReloadFilter::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP ReloadFilter::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IDocumentFilter))
		*ppvObject = (IFileInfo*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((ReloadFilter*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP_(BYTE) ReloadFilter::GetRule(IFileInfo *item)
{
	return FilterPolicy::entryRuleAdd;
}


InsertFilter::InsertFilter(FilterPolicy *pFilterPolicy, HWND hwndHost) : 
	ref(1), filterPolicy(NULL), hHost(hwndHost)
{
	if (NULL != pFilterPolicy)
		filterPolicy = pFilterPolicy->Clone();
	else
		filterPolicy = FilterPolicy::Create();
	lastPosition.x = CENTER_X;
	lastPosition.y = CENTER_Y;
}

InsertFilter::~InsertFilter()
{
	filterPolicy->Release();
}


InsertFilter* InsertFilter::CreateInstance(FilterPolicy *pFilterPolicy, HWND hwndHost)
{
	InsertFilter *instance = NULL;
	instance = new InsertFilter(pFilterPolicy, hwndHost);
	return instance;
}

STDMETHODIMP_(ULONG) InsertFilter::AddRef(void)
{
	return InterlockedIncrement((LONG*)&ref);
}

STDMETHODIMP_(ULONG) InsertFilter::Release(void)
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

STDMETHODIMP InsertFilter::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	if (!ppvObject)
		return E_POINTER;
	
	if (IsEqualIID(riid, IID_IDocumentFilter))
		*ppvObject = (IFileInfo*)this;
	else
		*ppvObject = NULL;
	
	if (NULL == *ppvObject)
		return E_NOINTERFACE;
	
	((ReloadFilter*)*ppvObject)->AddRef();
    return S_OK;
}

STDMETHODIMP_(BYTE) InsertFilter::GetRule(IFileInfo *item)
{
	DWORD typeId;
	if (NULL == item || FAILED(item->GetType(&typeId)))
		return FilterPolicy::entryRuleError;
		
	BYTE rule = filterPolicy->GetRule((BYTE)typeId);
	if (FilterPolicy::entryRuleAsk == rule)
		rule = ShowDialog(item);
	
	return rule;
}
static BOOL CALLBACK CloneFilter_Enumerator()
{
	return TRUE;
}


BYTE InsertFilter::ShowDialog(IFileInfo *item)
{
	DWORD typeId;
	if (NULL == item || FAILED(item->GetType(&typeId)))
		return FilterPolicy::entryRuleError;
		

	FILTERDIALOGPARAM dialogParam;
	dialogParam.filterPolicy = filterPolicy;
	dialogParam.hHost = hHost;
	dialogParam.item = item;
	dialogParam.position = lastPosition;
	dialogParam.applyToAll = FALSE;

	BYTE rule = (BYTE)WASABI_API_DIALOGBOXPARAMW(IDD_INSERTFILTER, hHost, InsertFilter_DialogProc, (LPARAM)&dialogParam);
	if (FilterPolicy::entryRuleError != rule)
	{
		if (dialogParam.applyToAll)
			filterPolicy->SetRule((BYTE)typeId, rule);
		lastPosition = dialogParam.position;
	}
	return rule;
}


static void InsertFilter_LoadIcon(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_MESSAGEICON);
	if (NULL == hControl) return;

	HICON hIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(OIC_QUES), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED);
	SendMessage(hControl, STM_SETICON, (WPARAM)hIcon, 0L);
}

static void InsertFilter_FormatCheckbox(HWND hwnd, IFileInfo *item, BOOL bCheck)
{
	HWND hControl = GetDlgItem(hwnd, IDC_CHECK_APPLYTOALL);
	if (NULL == hControl) return;
		
	DWORD typeId;
	TCHAR szType[128];
	szType[0] = TEXT('\0');

	if (NULL != item && SUCCEEDED(item->GetType(&typeId)))
	{
		IItemType *type = PLUGIN_REGTYPES->FindById((BYTE)typeId);
		if (NULL != type && FAILED(type->GetDisplayName(szType, ARRAYSIZE(szType))))
			szType[0] = TEXT('\0');
	}

	if (TEXT('\0') != szType[0])
	{
		TCHAR szBuffer[256 + ARRAYSIZE(szType)];
		INT cchLen = GetWindowText(hControl, szBuffer, ARRAYSIZE(szBuffer));
		if (cchLen > 0 && 
			SUCCEEDED(StringCchPrintf(szBuffer + cchLen, (ARRAYSIZE(szBuffer) - cchLen), TEXT(" (%s)."), szType)))
		{
			SetWindowText(hControl, szBuffer);
		}
	}

	SendMessage(hControl, BM_SETCHECK, (WPARAM)((FALSE != bCheck) ? BST_CHECKED : BST_UNCHECKED), 0L);
}

static void InsertFilter_FormatPath(HWND hwnd, IFileInfo *item)
{
	
	LPCTSTR pszPath;
	RECT controlRect;
	TCHAR szPath[MAX_PATH * 2];
	HWND hControl = GetDlgItem(hwnd, IDC_EDIT_PATH);
	if (NULL == hControl || !GetClientRect(hControl, &controlRect)) return;

	if (NULL == item || FAILED(item->GetPath(&pszPath)))
		pszPath = NULL;

	if (NULL == pszPath || FAILED(StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath)))
	{
		StringCchCopy(szPath, ARRAYSIZE(szPath), TEXT("Unable to get item path"));
	}
	else
	{
		HDC hdc = GetDCEx(hControl, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdc)
		{
			HFONT hf = (HFONT)SendMessage(hControl, WM_GETFONT, 0, 0L);
			if (NULL == hf) (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
			if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			HFONT hfo = (HFONT)SelectObject(hdc, hf);
			
			LONG width = controlRect.right - controlRect.left;
			DWORD margins = (DWORD)SendMessage(hControl, EM_GETMARGINS, 0, 0L);
			width -= (LOWORD(margins) + HIWORD(margins));
			PathCompactPath(hdc, szPath, width);

			SelectObject(hdc, hfo);
			ReleaseDC(hControl, hdc);
		}
	}
	
	SetWindowText(hControl, szPath);

}

static void InsertFilter_SetPosition(HWND hwnd, HWND hHost, POINT pt)
{
	if (CENTER_X == pt.x || CENTER_Y == pt.y)
	{
		RECT windowRect, centerRect;
		if (!GetWindowRect(hwnd, &windowRect) || !GetWindowRect(hHost, &centerRect))
			return;
		
		if (CENTER_X == pt.x) pt.x = centerRect.left + ((centerRect.right - centerRect.left) - (windowRect.right - windowRect.left))/2;
		if (CENTER_Y == pt.y) pt.y = centerRect.top + ((centerRect.bottom - centerRect.top) - (windowRect.bottom - windowRect.top))/2;
	}

	SetWindowPos(hwnd, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	SendMessage(hwnd, DM_REPOSITION, 0, 0L);
}

static void InsertFilter_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	RECT clientRect, controlRect;
	HWND hControl;
	DWORD controlStyle, windowposFlags;
	FILTERDIALOGPARAM *dialogParam = GetFilterProp(hwnd);

	if (NULL == dialogParam || !GetClientRect(hwnd, &clientRect))
		return;

	windowposFlags = SWP_NOACTIVATE  | SWP_NOZORDER;
	if (FALSE == bRedraw) windowposFlags |= SWP_NOREDRAW;

	hControl = GetDlgItem(hwnd, IDC_MESSAGEICON);
	if (NULL != hControl)
	{
		controlStyle = GetWindowStyle(hControl);
		if (0 != (WS_VISIBLE & controlStyle) && GetWindowRect(hControl, &controlRect))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);
			clientRect.left = controlRect.right + (controlRect.left - clientRect.left);
		}
	}
	
	hControl = GetDlgItem(hwnd, IDC_MESSAGETITLE);
	if (NULL != hControl)
	{
		controlStyle = GetWindowStyle(hControl);
		if (0 != (WS_VISIBLE & controlStyle) && GetWindowRect(hControl, &controlRect))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);
			controlRect.left = clientRect.left + (clientRect.right - controlRect.right);
			SetWindowPos(hControl, NULL, controlRect.left, controlRect.top, 
						controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, windowposFlags);
		}
	}

	hControl = GetDlgItem(hwnd, IDC_EDIT_PATH);
	if (NULL != hControl)
	{
		controlStyle = GetWindowStyle(hControl);
		if (0 != (WS_VISIBLE & controlStyle) && GetWindowRect(hControl, &controlRect))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);
			controlRect.left = clientRect.left + (clientRect.right - controlRect.right);
			SetWindowPos(hControl, NULL, controlRect.left, controlRect.top, 
						controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, windowposFlags);
			
			InsertFilter_FormatPath(hwnd, dialogParam->item);
		}
	}
}

static void InsertFilter_UpdateButtons(HWND hwnd, IFileInfo *item)
{
	DWORD typeId;
	BOOL enableEnum = FALSE;
	if (NULL != item && SUCCEEDED(item->GetType(&typeId)))
	{
		IItemType *type = PLUGIN_REGTYPES->FindById((BYTE)typeId);
		if (NULL != type && 0 != (IItemType::typeCapEnumerate & type->GetCapabilities()))
			enableEnum = TRUE;
	}

	HWND hControl = GetDlgItem(hwnd, IDC_BUTTON_ENUMERATE);
	if (NULL != hControl)
	{
		DWORD controlStyle = GetWindowStyle(hControl);
		controlStyle =  (enableEnum) ? (	(controlStyle & ~WS_DISABLED) | WS_VISIBLE) : (	(controlStyle & ~WS_VISIBLE) | WS_DISABLED);
		SetWindowLongPtr(hControl, GWL_STYLE, controlStyle);
	}
}

static INT_PTR InsertFiler_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM param)
{
	FILTERDIALOGPARAM *dialogParam = (FILTERDIALOGPARAM*)param;
	SetFilterProp(hwnd, dialogParam);
	
	HMENU hMenu = GetSystemMenu(hwnd, FALSE);
	if (NULL != hMenu)
	 EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
	
	InsertFilter_LoadIcon(hwnd);
	InsertFilter_FormatCheckbox(hwnd, dialogParam->item, dialogParam->applyToAll);
	InsertFilter_UpdateButtons(hwnd, dialogParam->item);
	InsertFilter_UpdateLayout(hwnd, FALSE);
	InsertFilter_SetPosition(hwnd, dialogParam->hHost, dialogParam->position);
	
	HWND hIgnore = GetDlgItem(hwnd, IDC_BUTTON_IGNORE);
	return (NULL == hIgnore || !SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hIgnore, TRUE));
}

static void InsertFilter_OnDestroy(HWND hwnd)
{
	FILTERDIALOGPARAM *dialogParam = GetFilterProp(hwnd);
	RemoveFilterProp(hwnd);
	if (NULL != dialogParam)
	{
		RECT windowRect;
		if (GetWindowRect(hwnd, &windowRect))
			dialogParam->position = *((POINT*)&windowRect);
	
		dialogParam->applyToAll = (BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CHECK_APPLYTOALL));
	}
}

static void InsertFilter_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(controlId)
	{
		case IDC_BUTTON_ADD:		EndDialog(hwnd, FilterPolicy::entryRuleAdd); break;
		case IDC_BUTTON_IGNORE:		EndDialog(hwnd, FilterPolicy::entryRuleIgnore); break;
		case IDC_BUTTON_ENUMERATE:	EndDialog(hwnd, FilterPolicy::entryRuleEnumerate); break;
	}
}

static INT_PTR CALLBACK InsertFilter_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return InsertFiler_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:	InsertFilter_OnDestroy(hwnd); break;
		case WM_COMMAND:	InsertFilter_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
	}
	return 0;
}
