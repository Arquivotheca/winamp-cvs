#include "./main.h"
#include "./plugin.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./messageBoxTweak.h"
#include "./modalSubclass.h"

static ATOM  MSGBOXTWEAKATOM = 0;

#define GetTweaker(__hwnd) ((MessageBoxTweak*)GetProp((__hwnd), MAKEINTATOM(MSGBOXTWEAKATOM)))

static MessageBoxTweak::CTRLOVERRIDE szButtonOverrides[] = 
{
	{IDABORT, MAKEINTRESOURCEW(IDS_BUTTON_ABORT)},
	{IDCANCEL, MAKEINTRESOURCEW(IDS_BUTTON_CANCEL)},
	{IDCONTINUE, MAKEINTRESOURCEW(IDS_BUTTON_CONTINUE)},
	{IDIGNORE, MAKEINTRESOURCEW(IDS_BUTTON_IGNORE)},
	{IDNO, MAKEINTRESOURCEW(IDS_BUTTON_NO)},
	{IDOK, MAKEINTRESOURCEW(IDS_BUTTON_OK)},
	{IDRETRY, MAKEINTRESOURCEW(IDS_BUTTON_RETRY)},
	{IDTRYAGAIN, MAKEINTRESOURCEW(IDS_BUTTON_TRYAGAIN)},
	{IDYES, MAKEINTRESOURCEW(IDS_BUTTON_YES)},
	{IDHELP, MAKEINTRESOURCEW(IDS_BUTTON_HELP)},
};

typedef struct ENUMOWNEDWND
{
	HWND hOwner;
	BOOL bEnable;
} ENUMOWNEDWND;

static BOOL CALLBACK EnumOwnedWindowsProc(HWND hwnd, LPARAM param)
{
	if (0 == param)
		return FALSE;
	if (0 == (WS_CHILD & GetWindowLongPtr(hwnd, GWL_STYLE)))
	{
		ENUMOWNEDWND *p = (ENUMOWNEDWND*)param;
		if (p->hOwner == hwnd || p->hOwner == GetWindow(hwnd, GW_OWNER))
		{
			EnableWindow(hwnd, p->bEnable);
		}
		
	}
	return TRUE;
}

static BOOL EnableOwnedWindows(HWND hOwner, BOOL bEnable)
{
	ENUMOWNEDWND param;
	param.hOwner = hOwner;
	param.bEnable = bEnable;

	if (NULL == hOwner)
		return FALSE;

	return EnumWindows(EnumOwnedWindowsProc, (LPARAM)&param);

}


MessageBoxTweak::MessageBoxTweak() 
	: hwnd(NULL), bUnicode(FALSE), originalProc(NULL), flags(TWEAK_NORMAL), hwndCenter(NULL), 
		firstActivate(FALSE)
{
}

MessageBoxTweak::~MessageBoxTweak()
{	
	Detach();
	
	size_t index = overrideList.size();
	while(index--)
	{
		LPWSTR p = overrideList[index].pszText;
		if (NULL != p && !IS_INTRESOURCE(p))
			free(p);
	}
	overrideList.clear();
}


INT MessageBoxTweak::Show(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	return ShowEx(hWnd, lpText, lpCaption, uType, TWEAK_CENTERPARENT | TWEAK_OVERRIDEBUTTONTEXT, NULL, 0);
}

INT MessageBoxTweak::ShowEx(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, 
						DWORD tweakFlags, CTRLOVERRIDE* pOverrides, size_t overridesCount)
{
	HWND hCenter = hWnd;

	if (NULL != hWnd)
		hWnd = GetAncestor(hWnd, GA_ROOT);
	
	if (NULL == hWnd || plugin.hwndParent == hWnd)
	{
		HWND hParent = (HWND)SENDWAIPC(plugin.hwndParent, IPC_GETDIALOGBOXPARENT, 0);
		if (NULL != hParent)
			hWnd = hParent;
	}

	
	TCHAR szText[1024], szCaption[128];
	
	if (IS_INTRESOURCE(lpText))
	{
		if (NULL == WASABI_API_LNG) 	return 0;
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)lpText, szText, ARRAYSIZE(szText));
		lpText = szText;
	}

	if (IS_INTRESOURCE(lpCaption))
	{
		if (NULL == WASABI_API_LNG) 	return 0;
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)lpCaption, szCaption, ARRAYSIZE(szCaption));
		lpCaption = szCaption;
	}

	MessageBoxTweak tweak;
	tweak.SetFlags(tweakFlags, tweakFlags);
	if (NULL != pOverrides && overridesCount > 0)
	{
		tweak.overrideList.reserve(overridesCount);
		for (size_t index = 0; index < overridesCount; index++)
		{
			if (NULL != pOverrides[index].pszText)
				tweak.overrideList.push_back(pOverrides[index]);
		}
	}		
	HWND hActive = NULL;
	if (0 != (TWEAK_APPLICATIONMODAL & tweakFlags))
	{
		hActive = GetActiveWindow();
		if (hActive != plugin.hwndParent && plugin.hwndParent != GetParent(hActive))
			hActive = NULL;

		EnableOwnedWindows(plugin.hwndParent, FALSE);
	}
	if (0 != (TWEAK_CENTERPARENT & tweakFlags) && NULL != hCenter)
	{
		tweak.SetCenterOn(hCenter);
	}

	tweak.BeginSubclass();
	INT result =  MessageBox(hWnd, lpText, lpCaption, uType);
	tweak.EndSubclass();

	if (TWEAK_APPLICATIONMODAL & tweakFlags)
	{
		EnableOwnedWindows(plugin.hwndParent, TRUE);
		if (NULL != hActive) SetActiveWindow(hActive);
	}
	
	return result;
}


BOOL MessageBoxTweak::BeginSubclass()
{
	BeginModalSubclass(MessageBoxTweak_SubclassProc, (ULONG_PTR)this);
	return TRUE;
}

void MessageBoxTweak::EndSubclass()
{
	EndModalSubclass();
}

BOOL MessageBoxTweak::Attach(HWND hTarget)
{
	hwnd = hTarget;
	bUnicode = IsWindowUnicode(hTarget);
	firstActivate = TRUE;
	originalProc =  (WNDPROC)(LONG_PTR)SetWindowLongPtr(hTarget, GWLP_WNDPROC, (LONGX86)(LONG_PTR)MessageBoxTweak_WindowProc);
	if (NULL == originalProc || !SetProp(hwnd, MAKEINTATOM(MSGBOXTWEAKATOM), this))
	{
		if (NULL != originalProc)
		{
			SetWindowLongPtr(hTarget, GWLP_WNDPROC, (LONGX86)(LONG_PTR)originalProc);
			originalProc = NULL;
		}
		hwnd = NULL;
		bUnicode = TRUE;
		return FALSE;
	}
	return TRUE;
}

void MessageBoxTweak::Detach()
{
	if (NULL != hwnd && NULL != originalProc && IsWindow(hwnd))
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)originalProc);
		RemoveProp(hwnd, MAKEINTATOM(MSGBOXTWEAKATOM));
	}

	hwnd = NULL;
	originalProc = NULL;
	firstActivate = FALSE;
}

void MessageBoxTweak::SetFlags(DWORD newFlags, DWORD flagsMask)
{
	flags = ((flags & ~flagsMask) | (newFlags & flagsMask));
}

DWORD MessageBoxTweak::GetFlags(DWORD flagsMask)
{
	return (flags & flagsMask);
}

void MessageBoxTweak::SetCenterOn(HWND hwndToCenterOn)
{
	hwndCenter = hwndToCenterOn;
}

BOOL MessageBoxTweak::CenterParent()
{
	if (NULL == hwndCenter)
	{
		hwndCenter = GetParent(hwnd);
		if (NULL == hwndCenter)
			return FALSE;
	}

	RECT rcCenter, rcMe;
	POINT center;
	if (!GetClientRect(hwndCenter, &rcCenter) ||
		!GetWindowRect(hwnd, &rcMe))
		return FALSE;
	
	MapWindowPoints(hwndCenter, HWND_DESKTOP, (POINT*)&rcCenter, 2);
	
	center.x = rcCenter.left + ((rcCenter.right - rcCenter.left) - (rcMe.right - rcMe.left))/2;
	center.y = rcCenter.top + ((rcCenter.bottom - rcCenter.top) - (rcMe.bottom - rcMe.top))/2;

	if (center.x != rcMe.left || center.y != rcMe.top)
	{
		SetWindowPos(hwnd, NULL, center.x, center.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
		SendMessage(hwnd, DM_REPOSITION, 0, 0L);
	}

	return TRUE;
}

void MessageBoxTweak::OverrideCtrlText(UINT ctrlId, LPCWSTR pszText)
{
	size_t index = overrideList.size();
	CTRLOVERRIDE *pOverride = NULL;
	while(index--)
	{
		if (overrideList[index].ctrlId == ctrlId)
		{
			LPWSTR p = overrideList[index].pszText;
			if (NULL != p && !IS_INTRESOURCE(p))
				free(p);
			if (NULL == pszText)
			{
				overrideList.eraseAt(index);
				return;
			}
			overrideList[index].pszText = NULL;
			pOverride = &overrideList[index];
		}
	}

	if (NULL == pOverride)
	{
		CTRLOVERRIDE t;
		t.ctrlId = ctrlId;
		t.pszText = NULL;
		overrideList.push_back(t);
		pOverride = &overrideList.back();
	}

	if (IS_INTRESOURCE(pszText))
		pOverride->pszText = (LPWSTR)pszText;
	else
	{
		INT cbLen = (lstrlen(pszText) + 1) * sizeof(WCHAR);
		pOverride->pszText = (LPWSTR)malloc(cbLen);
		if (NULL == pOverride->pszText)
		{
			OverrideCtrlText(pOverride->ctrlId, NULL);
			return;
		}
		CopyMemory(pOverride->pszText, pszText, cbLen);
	}
}


BOOL MessageBoxTweak::ReplaceCtrlText(UINT ctrlId, LPCWSTR pszText, LPWSTR pszBuffer, INT cchBufferMax)
{
	HWND hCtrl = GetDlgItem(hwnd, ctrlId);
	if (NULL == hCtrl) 
		return FALSE;

	if (IS_INTRESOURCE(pszText))
	{
		if (NULL == pszBuffer || NULL == WASABI_API_LNG)
			return FALSE;
		pszText = WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszText, pszBuffer, cchBufferMax);
	}
	
	return (0 != SetWindowText(hCtrl, pszText));
}

LRESULT MessageBoxTweak::PreviousWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (bUnicode) ? 
		CallWindowProcW(originalProc, hwnd, uMsg, wParam, lParam) : 
		CallWindowProcA(originalProc, hwnd, uMsg, wParam, lParam);
}

LRESULT MessageBoxTweak::DefaultWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return (bUnicode) ? 
		DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
		DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void MessageBoxTweak::OverrideText()
{	
	WCHAR szBuffer[4096];

	if (0 != (TWEAK_OVERRIDEBUTTONTEXT & flags))
	{
		for (size_t index = 0; index < ARRAYSIZE(szButtonOverrides); index++)
			ReplaceCtrlText(szButtonOverrides[index].ctrlId, szButtonOverrides[index].pszText, szBuffer, ARRAYSIZE(szBuffer));
	}

	for (size_t index = overrideList.size(); index--;)
	{
		if (NULL != overrideList[index].pszText)
			ReplaceCtrlText(overrideList[index].ctrlId, overrideList[index].pszText, szBuffer, ARRAYSIZE(szBuffer));
	}
}

void MessageBoxTweak::OnWindowActivate(UINT nState, HWND hwndOther, BOOL bMinimized)
{
	switch(nState)
	{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			if (firstActivate)
			{
				firstActivate = FALSE;
				OverrideText();
				if (0 != (TWEAK_CENTERPARENT & flags) && !bMinimized)
					CenterParent();
			}
			break;
	}
}

LRESULT MessageBoxTweak::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_ACTIVATE:
			OnWindowActivate(LOWORD(wParam), (HWND)lParam, (BOOL)HIWORD(wParam));
			break;
	}
	return PreviousWindowProc(uMsg, wParam, lParam);
}


static void CALLBACK UninitializeMessageBoxTweak(void)
{
	if (0 != MSGBOXTWEAKATOM)
	{
		GlobalDeleteAtom(MSGBOXTWEAKATOM);
		MSGBOXTWEAKATOM = 0;
	}
}

static INT CALLBACK MessageBoxTweak_SubclassProc(HWND hTarget, CREATESTRUCT *pcs, HWND hInsertAfter, ULONG_PTR user)
{
	if (NULL == hTarget) 
		return 0;

	if (0 == MSGBOXTWEAKATOM)
	{
		 MSGBOXTWEAKATOM = GlobalAddAtom(TEXT("MSGBOXTWEAK"));
		 if (0 == MSGBOXTWEAKATOM) return 0;
		 Plugin_RegisterUnloadCallback(UninitializeMessageBoxTweak);
	}

	MessageBoxTweak *pTweak = (MessageBoxTweak*)user;
	if (NULL == pTweak)
		return 0;

	pTweak->Attach(hTarget);
	return 0;
}

static LRESULT CALLBACK MessageBoxTweak_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MessageBoxTweak *pTweak = GetTweaker(hwnd);
	if (NULL == pTweak)
	{
		return (IsWindowUnicode(hwnd)) ? 
					DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
					DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	switch(uMsg)
	{
		case WM_DESTROY:
			{
				WNDPROC proc = pTweak->originalProc;
				pTweak->Detach();
				return (IsWindowUnicode(hwnd)) ? 
					CallWindowProcW(proc, hwnd, uMsg, wParam, lParam) : 
					CallWindowProcA(proc, hwnd, uMsg, wParam, lParam);
			}
			break;
	}
	return pTweak->WindowProc(uMsg, wParam, lParam);
}