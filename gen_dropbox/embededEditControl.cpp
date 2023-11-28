#define OEMRESOURCE

#include "./main.h"
#include "./embededEditControl.h"
#include "./plugin.h"
#include <windows.h>


static ATOM  EMBEDEDITCTRL = 0;

typedef struct 
{
	WNDPROC originalWndProc;
	EMBEDCONTROLCALLBACK callback;
	UINT_PTR	timerId;
	BOOL		showCursor;
	BOOL		keyDown;
} EMBEDEDIT;

static HHOOK mouseHook = NULL;

#define GetEmdedEdit(__hwnd) ((EMBEDEDIT*)GetProp((__hwnd), MAKEINTATOM(EMBEDEDITCTRL)))

static LRESULT CALLBACK EmdedEdit_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


static void CALLBACK UninitializeEmbedEditHandler(void)
{
	if (0 != EMBEDEDITCTRL)
	{
		GlobalDeleteAtom(EMBEDEDITCTRL);
		EMBEDEDITCTRL = 0;
	}
}

static BOOL InitializeEmbedEditHandler(void)
{
	if (0 != EMBEDEDITCTRL)
		return TRUE;
	
	EMBEDEDITCTRL = GlobalAddAtom(TEXT("EMBEDEDITCTRL"));
	if (0 == EMBEDEDITCTRL) return FALSE;
		
	Plugin_RegisterUnloadCallback(UninitializeEmbedEditHandler);
	return TRUE;
}

BOOL EmbedEditControl(HWND hwndEdit, EMBEDCONTROLCALLBACK callback)
{
	if (0 == EMBEDEDITCTRL && !InitializeEmbedEditHandler())
		return FALSE;

	if (NULL == hwndEdit || !IsWindow(hwndEdit))
		return FALSE;

	if (NULL != GetEmdedEdit(hwndEdit))
		return TRUE;

	EMBEDEDIT *pee = (EMBEDEDIT*)malloc(sizeof(EMBEDEDIT));
	if (NULL == pee)
		return FALSE;

	ZeroMemory(pee, sizeof(EMBEDEDIT));
	
	pee->callback = callback;    	
	pee->originalWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONGX86)(LONG_PTR)EmdedEdit_WindowProc);

	if (NULL == pee->originalWndProc || !SetProp(hwndEdit, MAKEINTATOM(EMBEDEDITCTRL), pee))
	{
		if (NULL != pee->originalWndProc)
			SetWindowLongPtr(hwndEdit, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pee->originalWndProc);
		free(pee);
		return FALSE;
	}
	return TRUE;
}

static void EmdedEdit_PatchCursor()
{	
	ShowCursor(FALSE);
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	ShowCursor(TRUE);

	CURSORINFO cursorInfo;
	cursorInfo.cbSize = sizeof(CURSORINFO);
	if (GetCursorInfo(&cursorInfo) && 
		0 != (CURSOR_SHOWING & cursorInfo.flags))
	{
		POINT pt;
		GetCursorPos(&pt);
		HWND hTarget= WindowFromPoint(pt);
		if (NULL != hTarget)
		{
			UINT hitTest = (UINT)SendMessage(hTarget, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
			UINT uMsg = (HTCLIENT == hitTest) ? WM_MOUSEMOVE : WM_NCMOUSEMOVE;
			SendMessage(hTarget, WM_SETCURSOR,  (WPARAM)hTarget, MAKELPARAM(hitTest, uMsg));
		}
		
	}
}

static void EmedEdit_Detach(HWND hwnd, INT closeCode)
{	
	EMBEDEDIT *pee = GetEmdedEdit(hwnd);
	RemoveProp(hwnd, MAKEINTATOM(EMBEDEDITCTRL));

	EmdedEdit_PatchCursor();

	if (NULL != pee && pee->showCursor)
	{
		ShowCursor(TRUE);
		pee->showCursor = FALSE;
	}

	if (NULL != mouseHook)
	{
		UnhookWindowsHookEx(mouseHook);
		mouseHook = NULL;
	}

	if (NULL == pee) return;

	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)pee->originalWndProc);

	if (NULL != pee->callback)
		pee->callback(hwnd, closeCode);
	
	free(pee);
}

static void EmbedEdit_OnGetDlgCode(HWND hwnd, INT vkCode, MSG *pMsg)
{
	switch(vkCode)
	{
		case VK_ESCAPE:
			EmedEdit_Detach(hwnd, IDCLOSE);
			DestroyWindow(hwnd);
			break;
		case VK_TAB:
		case VK_RETURN:
			EmedEdit_Detach(hwnd, IDOK);
			DestroyWindow(hwnd);
			break;

	}
}

static LRESULT CALLBACK EmbedEdit_MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;
	MOUSEHOOKSTRUCT *pmh = (MOUSEHOOKSTRUCT*)lParam;
	
	EmdedEdit_PatchCursor();

	EMBEDEDIT *pee = GetEmdedEdit(GetFocus());
	if (NULL != pee && pee->showCursor)
	{
		ShowCursor(TRUE);
		pee->showCursor = FALSE;
	}

	result = CallNextHookEx(mouseHook, nCode, wParam, lParam);
	UnhookWindowsHookEx(mouseHook);
	mouseHook = NULL;
	return result;
}

static LRESULT CALLBACK EmdedEdit_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EMBEDEDIT *pee = GetEmdedEdit(hwnd);
	if (NULL == pee || NULL == pee->originalWndProc)
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	WNDPROC wndProc = pee->originalWndProc;

	

	switch(uMsg)
	{
		case WM_KILLFOCUS:	
			EmedEdit_Detach(hwnd, IDOK);
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:			
			EmedEdit_Detach(hwnd, 0);
			break;

		case WM_GETDLGCODE:
			if (NULL != lParam) EmbedEdit_OnGetDlgCode(hwnd, (INT)wParam, (MSG*)lParam);
			return (DLGC_HASSETSEL | DLGC_WANTALLKEYS);
	
		case WM_SETCURSOR:
			SetCursor((HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(OCR_IBEAM), IMAGE_CURSOR, 
							0, 0, LR_DEFAULTCOLOR | LR_SHARED | LR_DEFAULTSIZE));
			return TRUE;
		
		case WM_WINDOWPOSCHANGED:
			if (pee->keyDown)
			{	
				if (NULL != mouseHook)
				{
					UnhookWindowsHookEx(mouseHook);
					mouseHook = NULL;
				}

				if (!pee->showCursor)
				{
					CURSORINFO cursorInfo;
					cursorInfo.cbSize = sizeof(CURSORINFO);
					if (GetCursorInfo(&cursorInfo) && 
						0 == (CURSOR_SHOWING & cursorInfo.flags))
					{
						ShowCursor(FALSE);
						pee->showCursor = TRUE;
					}
				}
			}
			break;
		
		case WM_KEYDOWN:
			pee->keyDown = TRUE;
			break;

		case WM_KEYUP:
			CallWindowProc(wndProc, hwnd, uMsg, wParam, lParam);
			if (NULL == mouseHook)
			{
				mouseHook = SetWindowsHookEx(WH_MOUSE, EmbedEdit_MouseHook, NULL, GetCurrentThreadId());
			}
			pee->keyDown = FALSE;
			return 0;
	}


	return CallWindowProc(wndProc, hwnd, uMsg, wParam, lParam);
}