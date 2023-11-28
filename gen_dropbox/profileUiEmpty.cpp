#include "./main.h"
#include "./plugin.h"
#include "./wasabiApi.h"
#include "./profileManager.h"
#include "./fontHelper.h"
#include "./skinWindow.h"
#include "./resource.h"
#include "./guiObjects.h"
#include "./preferences.h"

#include <shlwapi.h>
#include <strsafe.h>

#define BUTTON_SPACE	8

static INT_PTR CALLBACK ProfileUiEmpty_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND ProfileUiEmpty_CreateView(HWND hParent, INT_PTR controlId)
{
	HWND hView = WASABI_API_CREATEDIALOGPARAMW(IDD_PROFILEUI_EMPTY, hParent, 
								ProfileUiEmpty_DialogProc, 0L);
	if (NULL != hView)
	{
		SetWindowLongPtr(hView, GWLP_ID, (LONGX86)controlId);
	}
	return hView;
}

static INT ProfileUiEmpty_GetButtonHeight(HWND hwnd)
{
	RECT buttonRect;
	HWND hButton = GetDlgItem(hwnd, IDC_BUTTON_CREATE);
	return (NULL != hButton && GetWindowRect(hButton, &buttonRect)) ? 
		(buttonRect.bottom - buttonRect.top) : 0;	
}

static BOOL ProfileUiEmpty_GetTextEx(LPTSTR pszText, INT cchTextMax, HWND hwnd, HDC hdc, RECT *prcText)
{
	INT cchText = GetWindowText(hwnd, pszText, cchTextMax);


	if (NULL == prcText || 0 == cchText)
		return TRUE;

	HDC hdcMine = NULL;
	HFONT originalFont = NULL;
	if (NULL == hdc)
	{
		hdcMine = GetDCEx(hwnd, NULL, DCX_CACHE | DCX_NORESETATTRS);
		if (NULL != hdcMine)
		{
			HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
			originalFont = (HFONT)SelectObject(hdc, font);
		}
		hdc = hdcMine;
	}


	BOOL result = FALSE;

	INT buttonHeight = ProfileUiEmpty_GetButtonHeight(hwnd);

	if (NULL != hdc)
	{
		RECT clientRect;
		if (GetClientRect(hwnd, &clientRect))
		{
			
			SetRect(prcText, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
			DrawText(hdc, pszText, cchText, prcText, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_CALCRECT | DT_NOCLIP | DT_NOPREFIX); 
			
			INT blockHeight = (prcText->bottom - prcText->top) + BUTTON_SPACE + buttonHeight;

			INT x = clientRect.left + ((clientRect.right - clientRect.left) - (prcText->right - prcText->left))/2;
			INT y = clientRect.top + ((clientRect.bottom - clientRect.top) - blockHeight)/2;
			if (x < clientRect.left) x = clientRect.left;
			if (y < clientRect.top) y = clientRect.top;
			
			OffsetRect(prcText, x - prcText->left, y - prcText->top);
			if (prcText->right > clientRect.right) prcText->right = clientRect.right;
			if (prcText->bottom > (clientRect.bottom - buttonHeight)) prcText->bottom = (clientRect.bottom - buttonHeight);

			result = TRUE;
		}
	}

	if (NULL != hdcMine)
	{
		SelectObject(hdc, originalFont);
		ReleaseDC(hwnd, hdcMine);
	}

	return result;
}

static void ProfileUiEmpty_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	RECT clientRect, textRect;
	TCHAR szBuffer[1024];

	UINT swpFlags = SWP_NOZORDER | SWP_NOACTIVATE | ((FALSE == bRedraw) ? SWP_NOREDRAW : 0);

	if (GetClientRect(hwnd, &clientRect) && 
		ProfileUiEmpty_GetTextEx(szBuffer, ARRAYSIZE(szBuffer), hwnd, NULL, &textRect))
	{
		RECT buttonRect;
		HWND hButton = GetDlgItem(hwnd, IDC_BUTTON_CREATE);
		if (NULL != hButton && GetWindowRect(hButton, &buttonRect))
		{
			INT buttonHeight = (buttonRect.bottom - buttonRect.top);
			INT buttonWidth = (buttonRect.right - buttonRect.left);
			buttonRect.top = textRect.bottom + BUTTON_SPACE;
			if (buttonRect.top + buttonHeight > clientRect.bottom)
				buttonRect.top = clientRect.bottom - buttonHeight;
			if (buttonRect.top < clientRect.top) buttonRect.top = clientRect.top;

			buttonRect.left = clientRect.left + ((clientRect.right - clientRect.left) - buttonWidth)/2;
			if (buttonRect.left < clientRect.left) buttonRect.left = clientRect.left;

			
			SetWindowPos(hButton, NULL, buttonRect.left, buttonRect.top, buttonWidth, buttonHeight, swpFlags);
		}
	}
	InvalidateRect(hwnd, NULL, FALSE);
	
}

static void ProfileUiEmpty_Paint(HWND hwnd, HDC hdc, const RECT *prcPaint, BOOL fErase)
{
	SendMessage(hwnd, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hwnd);

	if (FALSE != fErase)
	{
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, prcPaint, NULL, 0, NULL);
	}

	TCHAR szText[512];
	RECT textRect;
	if (ProfileUiEmpty_GetTextEx(szText, ARRAYSIZE(szText), hwnd, hdc, &textRect))
	{
		HFONT font = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
		HFONT originalFont = (HFONT)SelectObject(hdc, font);
		DrawText(hdc, szText, -1, &textRect, DT_CENTER | DT_TOP | DT_NOPREFIX | DT_WORDBREAK);
		SelectObject(hdc, originalFont);
	}

}


static INT_PTR ProfileUiEmpty_OnInit(HWND hwnd, HWND hFocus, LPARAM param)
{	
	TCHAR szBuffer[1024];
	WASABI_API_LNGSTRINGW_BUF(IDS_PROFILEUI_EMPTY_MESSAGE, szBuffer, ARRAYSIZE(szBuffer));
	SetWindowText(hwnd, szBuffer);

	HWND hButton = GetDlgItem(hwnd, IDC_BUTTON_CREATE);
	if (NULL != hButton) MlSkinWindow(hButton, SWS_USESKINCURSORS | SWS_USESKINCOLORS | SWS_USESKINFONT);


	return FALSE;
}

static void ProfileUiEmpty_OnDestroy(HWND hwnd)
{
		
}

static void ProfileUiEmpty_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;	
	ProfileUiEmpty_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)));
}

static void ProfileUiEmpty_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(controlId)
	{
		case IDC_BUTTON_CREATE: 
			switch(eventId)
			{
				case BN_CLICKED:
					Plugin_ShowPreferences();
					break;
			}
			break;
	}
}

static INT_PTR ProfileUiEmpty_OnDialogColor(HWND hwnd, HDC hdc, HWND hDialog)
{
	HWND hParent = GetParent(hwnd);
	return (NULL != hParent) ? SendMessage(hParent, WM_CTLCOLORDLG, (WPARAM)hdc, (LPARAM)hDialog) : NULL;
}

static void ProfileUiEmpty_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if (NULL == hdc)
		return;
	ProfileUiEmpty_Paint(hwnd, ps.hdc, &ps.rcPaint, ps.fErase);
	EndPaint(hwnd, &ps);
}

static void ProfileUiEmpty_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);
	ProfileUiEmpty_Paint(hwnd, hdc, &clientRect, 0 != (PRF_ERASEBKGND & options));
}



static INT_PTR CALLBACK ProfileUiEmpty_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return ProfileUiEmpty_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:				ProfileUiEmpty_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	ProfileUiEmpty_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_CTLCOLORDLG:		return ProfileUiEmpty_OnDialogColor(hwnd, (HDC)wParam, (HWND)lParam); 
		case WM_COMMAND:				ProfileUiEmpty_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_PAINT:				ProfileUiEmpty_OnPaint(hwnd); return TRUE;
		case WM_PRINTCLIENT:		ProfileUiEmpty_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return TRUE;
		case WM_ERASEBKGND:			MSGRESULT(hwnd, 0);
	}
	return 0;
}