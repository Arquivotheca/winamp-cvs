#include "main.h"
#include "./plugin.h"
#include "./resource.h"
#include "./preferences.h"
#include "./wasabiApi.h"
#include "./groupHeader.h"

#include <windows.h>
#include <strsafe.h>


#define IDC_GROUPHEADER		10001
#define IDC_FRAMECONTROL		10002

static INT_PTR CALLBACK PreferencesEmpty_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND PreferencesEmpty_CreateView(HWND hParent, INT_PTR controlId)
{
	HWND hView = WASABI_API_CREATEDIALOGPARAMW(IDD_PREFVIEW_EMPTY, hParent, 
								PreferencesEmpty_DialogProc, 0L);
	if (NULL != hView)
	{
		SetWindowLongPtr(hView, GWLP_ID, (LONGX86)controlId);
	}
	return hView;
}

static void PreferencesEmpty_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	HWND hctrl;
	RECT clientRect, controlRect;
	UINT windowposFlags = SWP_NOACTIVATE | SWP_NOZORDER | ((FALSE == bRedraw) ? SWP_NOREDRAW : 0);

	if (!GetClientRect(hwnd, &clientRect))
		return;


	
	if (NULL != (hctrl = GetDlgItem(hwnd, IDC_FRAMECONTROL)))
	{
		SetWindowPos(hctrl, NULL, clientRect.left, clientRect.top, 
			clientRect.right - clientRect.left, clientRect.bottom - clientRect.top,	windowposFlags);
	}


	InflateRect(&clientRect, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));
	INT messageBottom = clientRect.top;

	if (NULL != (hctrl = GetDlgItem(hwnd, IDC_GROUPHEADER)) &&
		0 != (WS_VISIBLE & GetWindowLongPtr(hctrl, GWL_STYLE)))
	{
		CopyRect(&controlRect, &clientRect);
		if (GroupHeader_AdjustRect(hctrl, &controlRect))
		{
			SetWindowPos(hctrl, NULL, controlRect.left, controlRect.top, 
							controlRect.right - controlRect.left, controlRect.bottom - controlRect.top, 
							windowposFlags);
			
			clientRect.top = controlRect.bottom;
		}
	}

	if (NULL != (hctrl = GetDlgItem(hwnd, IDC_LABEL_MESSAGE)) &&
		0 != (WS_VISIBLE & GetWindowLongPtr(hctrl, GWL_STYLE)) &&
		GetWindowRect(hctrl, &controlRect))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);
		INT offsetCX = (controlRect.left - clientRect.left);
		INT offsetCY = offsetCX;
        SetWindowPos(hctrl, NULL, controlRect.left, clientRect.top + offsetCY , 
							(clientRect.right - clientRect.left) - 2 * offsetCX, 
							controlRect.bottom - controlRect.top, 
							windowposFlags);
			
		messageBottom = clientRect.top + offsetCY + (controlRect.bottom - controlRect.top) + 12;
	}
	
	INT szButtons[] = { IDC_BUTTON_CREATE, IDC_BUTTON_LOAD, };
	WINDOWPOS szwp[ARRAYSIZE(szButtons)];
	
	INT buttonsTop = clientRect.bottom;
	INT buttonsBottom = clientRect.top;

	for (INT i = 0; i < ARRAYSIZE(szwp); i++)
	{
		ZeroMemory(&szwp[i], sizeof(WINDOWPOS));
		szwp[i].hwnd = GetDlgItem(hwnd, szButtons[i]);
	
		if (NULL != szwp[i].hwnd && 
			0 != (WS_VISIBLE & GetWindowLongPtr(szwp[i].hwnd, GWL_STYLE)) &&
			GetWindowRect(szwp[i].hwnd, &controlRect))
		{
			MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&controlRect, 2);
			
			szwp[i].cx = controlRect.right - controlRect.left;
			if (szwp[i].cx > (clientRect.right - clientRect.left))
				szwp[i].cx = (clientRect.right - clientRect.left);

			szwp[i].cy = controlRect.bottom - controlRect.top;

			szwp[i].x = clientRect.left + ((clientRect.right - clientRect.left) - szwp[i].cx)/2;
			szwp[i].y = controlRect.top;

			if (buttonsTop > szwp[i].y) 
				buttonsTop = szwp[i].y;
			if (buttonsBottom < (szwp[i].y + szwp[i].cy))
				buttonsBottom = szwp[i].y + szwp[i].cy;
		}
		else
		{
			szwp[i].hwnd = NULL;
		}
	}

	INT offsetY = clientRect.top + ((clientRect.bottom - clientRect.top) - (buttonsBottom - buttonsTop))/2;
	if (offsetY < messageBottom) offsetY = messageBottom;
	offsetY = (offsetY - buttonsTop);

	for (INT i = 0; i < ARRAYSIZE(szwp); i++)
	{
		if (NULL != szwp[i].hwnd)
		{
			szwp[i].y += offsetY;
			SetWindowPos(szwp[i].hwnd, NULL, szwp[i].x, szwp[i].y, szwp[i].cx, szwp[i].cy, windowposFlags);
		}
	}
}

static void PreferencesEmpty_CreateProfile(HWND hwnd)
{	
	Profile *profile = Profile::Create(hwnd);
	if (NULL != profile)
		profile->Release();
}

static void PreferencesEmpty_LoadDefaultProfiles(HWND hwnd)
{
	PLUGIN_PROFILEMNGR->RegisterDefault();
}

static INT_PTR PreferencesEmpty_OnInit(HWND hwnd, HWND hFocus, LPARAM param)
{		

	TCHAR szBuffer[1024];

	WASABI_API_LNGSTRINGW_BUF(IDS_PREFVIEW_EMPTY, szBuffer, ARRAYSIZE(szBuffer));
	GroupHeader_RegisterClass(plugin.hDllInstance);

	HWND groupHeader = GroupHeader_CreateWindow(WS_EX_NOPARENTNOTIFY, 
							szBuffer, 
							WS_CHILD | WS_VISIBLE | GHS_DEFAULTCOLORS,
							0, 0, 1, 1, hwnd, IDC_GROUPHEADER, plugin.hDllInstance); 
	
	HWND hFrame = CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE, WC_LISTBOX, NULL, 
					WS_VISIBLE | WS_CHILD | WS_DISABLED |
					LBS_NOREDRAW | LBS_NODATA | LBS_NOINTEGRALHEIGHT | LBS_NOSEL , 
					0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)IDC_FRAMECONTROL, NULL, NULL);
	
	if (NULL != hFrame)
	{
		SetWindowPos(hFrame, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE | SWP_NOOWNERZORDER);
	}
	
	

	return FALSE;
}

static void PreferencesEmpty_OnDestroy(HWND hwnd)
{
		
}

static void PreferencesEmpty_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;	
	PreferencesEmpty_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)));
}

static void PreferencesEmpty_OnCommand(HWND hwnd, INT controlId, INT eventId, HWND hControl)
{
	switch(controlId)
	{
		case IDC_BUTTON_CREATE: PreferencesEmpty_CreateProfile(hwnd); break;
		case IDC_BUTTON_LOAD:	PreferencesEmpty_LoadDefaultProfiles(hwnd); break;
	}
}

static LRESULT PreferencesEmpty_OnStaticColor(HWND hwnd, HDC hdc, HWND hStatic)
{
	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
	SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	return (LRESULT)(INT_PTR)GetSysColorBrush(COLOR_WINDOW);
}

static INT_PTR CALLBACK PreferencesEmpty_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return PreferencesEmpty_OnInit(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:				PreferencesEmpty_OnDestroy(hwnd); return 0;
		case WM_WINDOWPOSCHANGED:	PreferencesEmpty_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_COMMAND:				PreferencesEmpty_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case WM_CTLCOLORSTATIC:		return PreferencesEmpty_OnStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
	}
	return 0;
}