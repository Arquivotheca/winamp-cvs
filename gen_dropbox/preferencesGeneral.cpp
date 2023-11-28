#include "main.h"
#include "./plugin.h"
#include "./resource.h"
#include "./preferences.h"
#include "./wasabiApi.h"
#include "./groupHeader.h"
#include "./editboxTweak.h"

#include "./configIniSection.h"
#include "./configManager.h"

#include <windows.h>
#include <strsafe.h>


#define IDC_GROUPHEADER		10001

#define PROFILE_PROP	TEXT("PROFLE_PROP")

static INT_PTR CALLBACK PreferencesGeneral_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL WINAPI PreferencesGeneral_RegisterPage()
{
	return Preferences_InsertPage(0, PREFPAGE_GENERAL, WASABI_API_LNG_HINST, 
							MAKEINTRESOURCE(IDS_PREFPAGE_GENERAL), 
							-1,
							MAKEINTRESOURCE(IDD_PREFPAGE_GENERAL), 
							PreferencesGeneral_DialogProc);
}

static void PreferencesGeneral_EnableNameButtons(HWND hwnd, BOOL fEnable)
{
	INT szButtons[] = { IDC_BUTTON_NAME_UPDATE, IDC_BUTTON_NAME_RESET, };
	for (INT i = 0; i < ARRAYSIZE(szButtons); i++)
	{
		HWND hControl = GetDlgItem(hwnd, szButtons[i]);
		if (NULL != hControl) EnableWindow(hControl, fEnable);
	}
}

static void PreferencesGeneral_EnableDescriptionButtons(HWND hwnd, BOOL fEnable)
{
	INT szButtons[] = { IDC_BUTTON_DESCRIPTION_UPDATE, IDC_BUTTON_DESCRIPTION_RESET, };
	for (INT i = 0; i < ARRAYSIZE(szButtons); i++)
	{
		HWND hControl = GetDlgItem(hwnd, szButtons[i]);
		if (NULL != hControl) EnableWindow(hControl, fEnable);
	}
}

static void PreferencesGeneral_ResetName(HWND hwnd)
{
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	TCHAR szBuffer[1024];
	if (NULL == profile || FAILED(profile->GetName(szBuffer, ARRAYSIZE(szBuffer))))
		szBuffer[0] = TEXT('\0');
	
	HWND hControl = GetDlgItem(hwnd, IDC_EDIT_NAME);
	if (NULL != hControl)
	{
		SetWindowText(hControl, szBuffer);
		SendMessage(hControl, EM_SETMODIFY, (WPARAM)FALSE, 0L);
	}
	PreferencesGeneral_EnableNameButtons(hwnd, FALSE);

	
}

static void PreferencesGeneral_ResetDescription(HWND hwnd)
{
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	TCHAR szBuffer[4096];
	if (NULL == profile || FAILED(profile->GetDescription(szBuffer, ARRAYSIZE(szBuffer))))
		szBuffer[0] = TEXT('\0');
	
	HWND hControl = GetDlgItem(hwnd, IDC_EDIT_DESCRIPTION);
	if (NULL != hControl)
	{
		SetWindowText(hControl, szBuffer);
		SendMessage(hControl, EM_SETMODIFY, (WPARAM)FALSE, 0L);
	}
	PreferencesGeneral_EnableDescriptionButtons(hwnd, FALSE);
}

static void PreferencesGeneral_UpdateProfileName(HWND hwnd)
{
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	HWND hControl = GetDlgItem(hwnd, IDC_EDIT_NAME);

	if (NULL == profile || NULL == hControl)
		return;
	
	TCHAR szBuffer[1024];
	GetWindowText(hControl, szBuffer, ARRAYSIZE(szBuffer));
	
	profile->SetName(szBuffer);
	profile->Save();

	SendMessage(hControl, EM_SETMODIFY, (WPARAM)FALSE, 0L);
	PreferencesGeneral_EnableNameButtons(hwnd, FALSE);
}

static void PreferencesGeneral_UpdateProfileDescription(HWND hwnd)
{
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	HWND hControl = GetDlgItem(hwnd, IDC_EDIT_DESCRIPTION);

	if (NULL == profile || NULL == hControl)
		return;
	
	TCHAR szBuffer[1024];
	GetWindowText(hControl, szBuffer, ARRAYSIZE(szBuffer));
	
	profile->SetDescription(szBuffer);
	profile->Save();

	SendMessage(hControl, EM_SETMODIFY, (WPARAM)FALSE, 0L);
	PreferencesGeneral_EnableDescriptionButtons(hwnd, FALSE);
}

static void PreferencesGeneral_UpdateLayout(HWND hwnd, BOOL bRedraw)
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

	/*if (NULL != (hctrl = GetDlgItem(hwnd, IDC_GROUPVIEW)))
	{		
		SetRect(&rcControl, rc.left, top, rc.right, rc.bottom);
		if (rcControl.top > rcControl.bottom) rcControl.top = rcControl.bottom;
		SetWindowPos(hctrl, NULL, rcControl.left, rcControl.top, 
						rcControl.right - rcControl.left, rcControl.bottom - rcControl.top, 
						windowposFlags);		
	}*/
}

static INT_PTR PreferencesGeneral_OnInit(HWND hwnd, HWND hFocus, LPARAM param)
{
	Profile *profile = (Profile*)param;
	if (NULL != profile &&
		FALSE != SetProp(hwnd, PROFILE_PROP, (HANDLE)profile))
	{
		profile->AddRef();
	}
	else
	{
		INT szControls[] = { IDC_EDIT_NAME, IDC_BUTTON_NAME_UPDATE, IDC_BUTTON_NAME_RESET, 
			IDC_EDIT_DESCRIPTION, IDC_BUTTON_DESCRIPTION_UPDATE, IDC_BUTTON_DESCRIPTION_RESET, };
		for (INT i = 0; i < ARRAYSIZE(szControls); i++)
		{
			HWND hControl = GetDlgItem(hwnd, szControls[i]);
			if (NULL != hControl) EnableWindow(hControl, FALSE);
		}
	}

	
	TCHAR szBuffer[1024];
	WASABI_API_LNGSTRINGW_BUF(IDS_PREFPAGE_GENERAL_DESC, szBuffer, ARRAYSIZE(szBuffer));
	GroupHeader_RegisterClass(plugin.hDllInstance);
	HWND groupHeader = GroupHeader_CreateWindow(WS_EX_NOPARENTNOTIFY, 
							szBuffer, 
							WS_CHILD | WS_VISIBLE | GHS_DEFAULTCOLORS,
							0, 0, 1, 1, hwnd, IDC_GROUPHEADER, plugin.hDllInstance); 

	if (!SENDWAIPC(plugin.hwndParent, IPC_USE_UXTHEME_FUNC, IPC_ISWINTHEMEPRESENT))
		SENDWAIPC(plugin.hwndParent, IPC_USE_UXTHEME_FUNC, hwnd);

	HWND hEdit = GetDlgItem(hwnd, IDC_EDIT_NAME);
	if (NULL != hEdit) EditboxTweak_Enable(hEdit, ETF_NOTIFY_ENTERKEY);

	PreferencesGeneral_ResetName(hwnd);
	PreferencesGeneral_ResetDescription(hwnd);
	return FALSE;
}

static void PreferencesGeneral_OnDestroy(HWND hwnd)
{
	Profile *profile = (Profile*)GetProp(hwnd, PROFILE_PROP);
	RemoveProp(hwnd, PROFILE_PROP);

	if (NULL != profile)
	{
		profile->Release();
	}

}

static void PreferencesGeneral_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(controlId)
	{
		case IDC_BUTTON_NAME_UPDATE: 
			if (BN_CLICKED == eventId)
				PreferencesGeneral_UpdateProfileName(hwnd);
			break;
		case IDC_BUTTON_NAME_RESET:			
			if (BN_CLICKED == eventId)
				PreferencesGeneral_ResetName(hwnd); 
			break;
		case IDC_BUTTON_DESCRIPTION_UPDATE: 
			if (BN_CLICKED == eventId)
				PreferencesGeneral_UpdateProfileDescription(hwnd);
			break;
		case IDC_BUTTON_DESCRIPTION_RESET: 	
			if (BN_CLICKED == eventId)
				PreferencesGeneral_ResetDescription(hwnd); 
			break;
		case IDC_EDIT_NAME:
			switch(eventId)
			{
				case EN_CHANGE:
					PreferencesGeneral_EnableNameButtons(hwnd, TRUE);
					break;
			}
			break;
		case IDC_EDIT_DESCRIPTION:
			switch(eventId)
			{
				case EN_CHANGE:
					PreferencesGeneral_EnableDescriptionButtons(hwnd, TRUE);
					break;
			}
			break;
	}
}

static void PreferencesGeneral_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;	
	PreferencesGeneral_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)));
}

static INT_PTR PreferencesGeneral_OnDialogColor(HWND hwnd, HDC hdc, HWND hDialog)
{
	SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
	return(INT_PTR)GetSysColorBrush(COLOR_WINDOW);
}

static INT_PTR PreferencesGeneral_OnStaticColor(HWND hwnd, HDC hdc, HWND hStatic)
{
	SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
	return(INT_PTR)GetSysColorBrush(COLOR_WINDOW);
}

static void PreferencesGeneral_OnNameEnter(HWND hwnd, HWND hEdit)
{
	if (0 != SendMessage(hEdit, EM_GETMODIFY, 0, 0L))
		PreferencesGeneral_UpdateProfileName(hwnd);
	
	HWND hDetails = GetDlgItem(hwnd, IDC_EDIT_DESCRIPTION);

	if (NULL != hDetails && IsWindowEnabled(hDetails))
		PostMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hDetails, TRUE);
	else
		PostMessage(hwnd, WM_NEXTDLGCTL, 0, 0L);
}

static LRESULT PreferencesGeneral_OnNotify(HWND hwnd, INT controlId, NMHDR *pnmh)
{
	switch(controlId)
	{
		case IDC_EDIT_NAME:
			switch(pnmh->code)
			{
				case NM_RETURN:
					PreferencesGeneral_OnNameEnter(hwnd, pnmh->hwndFrom);
					return TRUE;
			}
			break;
	}
	return 0;
}


static INT_PTR CALLBACK PreferencesGeneral_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return PreferencesGeneral_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:				PreferencesGeneral_OnDestroy(hwnd); return 0;
		case WM_COMMAND:				PreferencesGeneral_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); break;
		case WM_WINDOWPOSCHANGED:	PreferencesGeneral_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_CTLCOLORDLG:			return PreferencesGeneral_OnDialogColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_CTLCOLORSTATIC:		return PreferencesGeneral_OnStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_NOTIFY:				MSGRESULT(hwnd, PreferencesGeneral_OnNotify(hwnd, (INT)wParam, (NMHDR*)lParam));
	}
	return 0;
}