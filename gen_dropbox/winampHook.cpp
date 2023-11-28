#include "./main.h"
#include "./winampHook.h"
#include "./plugin.h"
#include "./skinWindow.h"
#include "./guiObjects.h"
#include "../winamp/wa_ipc.h"

#include <windows.h>

#define WINAMP_REFRESHSKIN              40291

typedef enum
{
	WAHOOK_UNICODE = 0x0001,
} WAHOOK_FLAGS;

typedef struct __SUBSCRIBER SUBSCRIBER;
typedef struct __WNDHOOK WNDHOOK;

typedef struct __SUBSCRIBER
{
	ULONG_PTR	user;
	WHCALLBACK callback;
	SUBSCRIBER	*previous;
	SUBSCRIBER	*next;
	WNDHOOK		*wndHook;
}SUBSCRIBER;

typedef struct __WNDHOOK
{
	HWND		hwnd;
	WNDPROC	originalWndProc;
	DWORD	flags;
	SUBSCRIBER *firstSubscriber;
} WNDHOOK;

static ATOM  WAWNDATOM = 0;

static LRESULT CALLBACK WinampWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CALLBACK UninitializeWinampHook(void);
static LRESULT CALLBACK DefWaHookCallback(HWAHOOK hWaHook, UINT whcbId, WPARAM param, ULONG_PTR user);

#define GetWindowHook(__hwnd) ((WNDHOOK*)GetProp((__hwnd), MAKEINTATOM(WAWNDATOM)))


static HWAHOOK WindowHook_AddSubscriber(WNDHOOK *pwh, WHCALLBACK callback, ULONG_PTR user)
{
	SUBSCRIBER *ps = (SUBSCRIBER*)malloc(sizeof(SUBSCRIBER));
	if (NULL == ps) return NULL;
	ZeroMemory(ps, sizeof(SUBSCRIBER));
	ps->callback = callback;
	ps->user = user;
	ps->previous = pwh->firstSubscriber;
	if (NULL != ps->previous)
		ps->previous->next = ps;

	ps->next = NULL;
	ps->wndHook = pwh;
	pwh->firstSubscriber = ps;
	return (HWAHOOK)ps;
}

static void WindowHook_RemoveSubscriber(WNDHOOK *pwh, SUBSCRIBER *ps)
{
	if (NULL != ps->previous)
		ps->previous->next = ps->next;
	if (NULL != ps->next)
		ps->next->previous = ps->previous;
	
	if (pwh->firstSubscriber == ps)
	{
		if (NULL != ps->previous)
			pwh->firstSubscriber = ps->previous;
		else
			pwh->firstSubscriber = ps->next;
	}

	free(ps);

	if (pwh->firstSubscriber->previous == NULL)
	{
		ps = pwh->firstSubscriber;
		pwh->firstSubscriber = NULL;
		free(ps);
	}
}

HWAHOOK AttachWinampHook(HWND hwndWa, WHCALLBACK callback, ULONG_PTR user)
{
	if (!IsWindow(hwndWa) || NULL == callback)
		return NULL;

	if (0 == WAWNDATOM)
	{
		 WAWNDATOM = GlobalAddAtom(TEXT("DROPBOXWAWNDHOOK"));
		 if (0 == WAWNDATOM) return NULL;
		 Plugin_RegisterUnloadCallback(UninitializeWinampHook);
	}

	WNDHOOK *pwh = GetWindowHook(hwndWa);
	if (NULL == pwh)
	{
		pwh = (WNDHOOK*)malloc(sizeof(WNDHOOK));
		ZeroMemory(pwh, sizeof(WNDHOOK));
		
		pwh->hwnd = hwndWa;
		if (IsWindowUnicode(hwndWa))
			pwh->flags |= WAHOOK_UNICODE;
		
		pwh->originalWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwndWa, GWLP_WNDPROC, (LONGX86)(LONG_PTR)WinampWindowProc);
		if (NULL == pwh->originalWndProc || !SetProp(hwndWa, MAKEINTATOM(WAWNDATOM), pwh))
		{
			if (NULL != pwh->originalWndProc)
				SetWindowLongPtr(hwndWa, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pwh->originalWndProc);
			free(pwh);
			return NULL;
		}
		UpdateSkinCache(UPDATESKIN_COLOR | UPDATESKIN_FONT);
		WindowHook_AddSubscriber(pwh, DefWaHookCallback, 0);
	}
	return WindowHook_AddSubscriber(pwh, callback, user);;
}

void ReleaseWinampHook(HWND hwndWa, HWAHOOK hook)
{
	if (!IsWindow(hwndWa))
		return;

	SUBSCRIBER *ps = (SUBSCRIBER*)hook;
	if (NULL == ps) return;

	WNDHOOK *pHook = GetWindowHook(hwndWa);
	WNDHOOK *pwh = ps->wndHook;
	if (NULL == pwh || pHook == NULL || pwh != pHook)
		return;
	
	WindowHook_RemoveSubscriber(pwh, ps);
	if (NULL != pwh->firstSubscriber)
		return;
		
	if (IsWindow(pwh->hwnd) && NULL != pwh->originalWndProc)
	{
		WNDPROC topWndProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(pwh->hwnd, GWLP_WNDPROC);
		if (topWndProc != WinampWindowProc)
			return;
		
		SetWindowLongPtr(pwh->hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pwh->originalWndProc);
		RemoveProp(pwh->hwnd, MAKEINTATOM(WAWNDATOM));
	}
	
	free(pwh);
	UpdateSkinCache(UPDATESKIN_COLOR | UPDATESKIN_FONT); 
}


static LRESULT CallPrevWndProc(WNDHOOK *pwh, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return ( 0 != (WAHOOK_UNICODE & pwh->flags)) ? 
		CallWindowProcW(pwh->originalWndProc, pwh->hwnd, uMsg, wParam, lParam) : 
		CallWindowProcA(pwh->originalWndProc, pwh->hwnd, uMsg, wParam, lParam);
}

static LRESULT CALLBACK DefWaHookCallback(HWAHOOK hWaHook, UINT whcbId, WPARAM param, ULONG_PTR user)
{
	switch(whcbId)
	{
		case WHCB_RESETFONT:
			break;
		case WHCB_OKTOQUIT:
			return 1;
		case WHCB_SKINCHANGED:
			break;
		case WHCB_SKINCHANGING:
			{
				SUBSCRIBER *ps = (SUBSCRIBER *)hWaHook;
				return CallPrevWndProc(ps->wndHook, WM_COMMAND, WINAMP_REFRESHSKIN, param);
			}
		case WHCB_FILEMETACHANGED:
			break;
	}
	return 0;
}

LRESULT CallNextWinampHook(HWAHOOK hWaHook, UINT whcbId, WPARAM param)
{
	SUBSCRIBER *ps = (SUBSCRIBER *)hWaHook;
	if (NULL == ps || NULL == ps->previous)
		return 0;
	ps = ps->previous;
	return ps->callback((HWAHOOK)ps, whcbId, param, ps->user);
}

static LRESULT WinampHook_Callback(WNDHOOK *pwh, UINT whcbId, WPARAM param)
{		
	SUBSCRIBER *ps = pwh->firstSubscriber;
	if (NULL != ps)
	{
		return ps->callback((HWAHOOK)ps, whcbId, param, ps->user);
	}
	return DefWaHookCallback((HWAHOOK)ps, whcbId, param, NULL);
}

static BOOL WinampHook_OnWinampIPC(WNDHOOK *pwh, UINT ipcId, WPARAM param, LRESULT *pResult)
{
	switch(ipcId)
	{
		case IPC_CB_RESETFONT:
			UpdateSkinCache(UPDATESKIN_FONT);
			WinampHook_Callback(pwh, WHCB_RESETFONT, param);
			break;
		case IPC_HOOK_OKTOQUIT:
			*pResult = WinampHook_Callback(pwh, WHCB_OKTOQUIT, param);
			if (*pResult > 0)
				*pResult = CallPrevWndProc(pwh, WM_WA_IPC, param, (LPARAM)ipcId);
			return TRUE;
		case IPC_SKIN_CHANGED:
		case IPC_FF_ONCOLORTHEMECHANGED:
			UpdateSkinCache(UPDATESKIN_COLOR);
			WinampHook_Callback(pwh, WHCB_SKINCHANGED, param);
			break;
		case IPC_FILE_TAG_MAY_HAVE_UPDATED:
			if (NULL != param)
			{
				wchar_t szBuffer[1024];
				if (MultiByteToWideChar(CP_ACP, 0, (char*)param, -1, szBuffer, ARRAYSIZE(szBuffer)))
					WinampHook_Callback(pwh, WHCB_FILEMETACHANGED, (WPARAM)szBuffer);
			}
			break;
		case IPC_FILE_TAG_MAY_HAVE_UPDATEDW:
			WinampHook_Callback(pwh, WHCB_FILEMETACHANGED, param);
			break;
	}
	return FALSE;
}

static LRESULT CALLBACK WinampWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDHOOK *pwh = GetWindowHook(hwnd);
	if (NULL == pwh)
	{
		return (IsWindowUnicode(hwnd)) ? 
					DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
					DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	if (NULL == pwh->firstSubscriber) // pretend to be dettached
		return CallPrevWndProc(pwh, uMsg, wParam, lParam);
	
	switch(uMsg)
	{
		case WM_DESTROY:
			{
				WNDPROC proc = pwh->originalWndProc;

				while(NULL != pwh->firstSubscriber)
					WindowHook_RemoveSubscriber(pwh, pwh->firstSubscriber);

				return (IsWindowUnicode(hwnd)) ? 
					CallWindowProcW(proc, hwnd, uMsg, wParam, lParam) : 
					CallWindowProcA(proc, hwnd, uMsg, wParam, lParam);
			}
			break;
		case WM_WA_IPC:
			{
				LRESULT result;
				if (WinampHook_OnWinampIPC(pwh, (UINT)lParam, wParam, &result))
					return result;
			}
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case WINAMP_REFRESHSKIN:
					return WinampHook_Callback(pwh, WHCB_SKINCHANGING, lParam);
			}
			break;

		case WM_SYSCOLORCHANGE:
			WinampHook_Callback(pwh, WHCB_SYSCOLORCHANGE, 0L);
			break;
	}
	return CallPrevWndProc(pwh, uMsg, wParam, lParam);
}

static void CALLBACK UninitializeWinampHook(void)
{
	if (0 != WAWNDATOM)
	{
		GlobalDeleteAtom(WAWNDATOM);
		WAWNDATOM = 0;
	}
}
