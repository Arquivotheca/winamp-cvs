#include "main.h"
#include "./dropWindow.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include <strsafe.h>


static INT_PTR CALLBACK RenamePlDlg_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR DropboxWindow_RenamePlaylsitDialog(HWND hDropbox)
{
	return WASABI_API_DIALOGBOXPARAMW(IDD_RENAMEPLAYLIST, hDropbox, RenamePlDlg_DialogProc, (LPARAM)hDropbox);
}

static INT_PTR RenamePlDlg_OnInit(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_EDIT_NAME);

	if (NULL == hEdit)
	{
		EndDialog(hwnd, 0);
		return 0;
	}

	BOOL success;
	TCHAR szTitle[1024];
	HWND hDropbox = (HWND)lParam;
	
	SetProp(hwnd, TEXT("Dropbox"), hDropbox);
	success = (NULL != hDropbox) ? DropboxWindow_GetDocumentName(hDropbox, szTitle, ARRAYSIZE(szTitle)) : FALSE;
	
	if (!success)
	{
		StringCchCopy(szTitle, ARRAYSIZE(szTitle), TEXT("Unable to read playlist name"));
		EnableWindow(hEdit, FALSE);
	}
	
	SetWindowText(hEdit, szTitle);
	if (success && hEdit != hFocus)
	{
		SendMessage(hwnd, WM_NEXTDLGCTL, (WPARAM)hEdit, (LPARAM)TRUE);
		SendMessage(hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
	}
			
	return (!success || hEdit == hFocus);
}

static void RenamePlDlg_OnDestroy(HWND hwnd)
{
	RemoveProp(hwnd, TEXT("Dropbox"));
}

static BOOL RenamePlDlg_ValidateName(LPCTSTR pszName)
{
	if (NULL == pszName || TEXT('\0') == *pszName)
		return FALSE;

	return TRUE;
}

static BOOL RenamePlDlg_UpdateName(HWND hwnd)
{
	HWND hDropbox = (HWND)GetProp(hwnd, TEXT("Dropbox"));
	if (NULL == hDropbox)
		return FALSE;
	HWND hEdit = GetDlgItem(hwnd, IDC_EDIT_NAME);
	if (NULL == hEdit)
		return FALSE;

	TCHAR szName[1024];
	if (!GetWindowText(hEdit, szName, ARRAYSIZE(szName)))
		return FALSE;
		
	return RenamePlDlg_ValidateName(szName) &&
			DropboxWindow_SetDocumentName(hDropbox, szName);
}

static void RenamePlDlg_OnCommand(HWND hwnd, INT ctrlId, INT eventId, HWND hctrl)
{
	switch(ctrlId)
	{
		case IDOK:
			if (!RenamePlDlg_UpdateName(hwnd))
			{
				FLASHWINFO flashInfo;
				flashInfo.cbSize = sizeof(FLASHWINFO);
				flashInfo.dwFlags = FLASHW_CAPTION;
				flashInfo.dwTimeout = 100;
				flashInfo.hwnd = hwnd;
				flashInfo.uCount = 2;
				FlashWindowEx(&flashInfo);
				return;
			}
		case IDCANCEL:
			EndDialog(hwnd, (IDOK == ctrlId));
			return;

		case IDC_EDIT_NAME:
			switch(eventId)
			{
				case EN_CHANGE:
				{
					TCHAR szName[1024];
					HWND hOk = GetDlgItem(hwnd, IDOK);
					if (NULL != hOk)
					{
						BOOL bEnable = (0 != GetWindowText(hctrl, szName, ARRAYSIZE(szName)) &&
										0 != RenamePlDlg_ValidateName(szName));

						if (bEnable != (0 ==  (WS_DISABLED & GetWindowLongPtr(hOk, GWL_STYLE))))
							EnableWindow(hOk, bEnable);
					}	
				}
				break;
			}
			break;
	}

}
static INT_PTR CALLBACK RenamePlDlg_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG: return RenamePlDlg_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:		RenamePlDlg_OnDestroy(hwnd); break;
		case WM_COMMAND:		RenamePlDlg_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
	}
	return 0;
}