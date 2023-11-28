#include "./main.h"
#include "./editboxTweak.h"
#include "./plugin.h"

#include <commctrl.h>

#define ETF_USERFLAGSMASK	0x00FFFFFF
#define ETF_UNICODE			0x01000000

typedef struct __EDITBOXTWEAK
{
	WNDPROC originalProc;
	UINT	flags;
} EDITBOXTWEAK;

static ATOM EDITBOXTWEAK_PROP = 0;
static LRESULT CALLBACK EditboxTweak_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static void CALLBACK EditboxTweak_Uninitialize()
{
	if (0 != EDITBOXTWEAK_PROP)
	{
		GlobalDeleteAtom(EDITBOXTWEAK_PROP);
		EDITBOXTWEAK_PROP = 0;
	}
}

static EDITBOXTWEAK *EditboxTweak_GetData(HWND hwnd, BOOL fCreate)
{
	if (!IsWindow(hwnd)) 
		return  NULL;

	if (0 == EDITBOXTWEAK_PROP)
	{
		 EDITBOXTWEAK_PROP = GlobalAddAtom(TEXT("waDropboxEditboxTweak"));
		 if (0 == EDITBOXTWEAK_PROP) return  NULL;
		 Plugin_RegisterUnloadCallback(EditboxTweak_Uninitialize);
	}
	
	EDITBOXTWEAK *tweak = (EDITBOXTWEAK*)GetProp(hwnd, MAKEINTATOM(EDITBOXTWEAK_PROP));
	if (NULL == tweak && FALSE != fCreate)
	{
		tweak = (EDITBOXTWEAK*)malloc(sizeof(EDITBOXTWEAK));
		if (NULL == tweak) return  NULL;
		
		ZeroMemory(tweak, sizeof(EDITBOXTWEAK));
		
		tweak->originalProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)EditboxTweak_WindowProc);
				
		if (NULL == tweak->originalProc || !SetProp(hwnd, MAKEINTATOM(EDITBOXTWEAK_PROP), tweak))
		{
			if (NULL != tweak->originalProc)
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)tweak->originalProc);
			return NULL;
		}
		if (IsWindowUnicode(hwnd)) tweak->flags |= ETF_UNICODE;
	}
	
	return tweak;
}

static void EditboxTweak_RemoveData(HWND hwnd, BOOL fDetroyWindow)
{
	EDITBOXTWEAK *tweak = (EDITBOXTWEAK*)GetProp(hwnd, MAKEINTATOM(EDITBOXTWEAK_PROP));
	RemoveProp(hwnd, MAKEINTATOM(EDITBOXTWEAK_PROP));
	if (NULL == tweak) return;
	
	if (NULL != tweak->originalProc)
	{
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONGX86)(LONG_PTR)tweak->originalProc);

		if (FALSE != fDetroyWindow)
		{
			if (0 != (ETF_UNICODE & tweak->flags))
				CallWindowProcW(tweak->originalProc, hwnd, WM_DESTROY, 0, 0L);
			else
				CallWindowProcA(tweak->originalProc, hwnd, WM_DESTROY, 0, 0L);
		}
	}	
	free(tweak);
}


BOOL EditboxTweak_Enable(HWND hEdit, UINT tweakFlags)
{
	if (0 == tweakFlags)
	{
		EditboxTweak_RemoveData(hEdit, FALSE);
		return TRUE;
	}

	EDITBOXTWEAK *tweak = EditboxTweak_GetData(hEdit, TRUE);
	if (NULL == tweak) return FALSE;

	tweak->flags &= ~ETF_USERFLAGSMASK;
	tweak->flags = (ETF_USERFLAGSMASK & tweakFlags);

	return TRUE;
}

static LRESULT EditboxTweak_CallOrigWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	EDITBOXTWEAK *tweak = (EDITBOXTWEAK*)GetProp(hwnd, MAKEINTATOM(EDITBOXTWEAK_PROP));

	if (NULL == tweak || NULL == tweak->originalProc)
	{
		return (0 != (ETF_UNICODE & tweak->flags)) ? 
				DefWindowProcW(hwnd, uMsg, wParam, lParam) : 
				DefWindowProcA(hwnd, uMsg, wParam, lParam);
	}

	return (0 != (ETF_UNICODE & tweak->flags)) ? 
			CallWindowProcW(tweak->originalProc, hwnd, uMsg, wParam, lParam) : 
			CallWindowProcA(tweak->originalProc, hwnd, uMsg, wParam, lParam);
}

static BOOL EditboxTweak_NotifyReturn(HWND hwnd)
{
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent)  return FALSE;

	NMHDR nmhdr;
	nmhdr.code = NM_RETURN;
	nmhdr.hwndFrom = hwnd;
	nmhdr.idFrom = GetDlgCtrlID(hwnd);

	return (0 != SendMessage(hParent, WM_NOTIFY, (WPARAM)nmhdr.idFrom, (LPARAM)&nmhdr));

}
static void EditboxTweak_OnDestroy(HWND hwnd)
{
	EditboxTweak_RemoveData(hwnd, TRUE);
}

static LRESULT EditboxTweak_OnGetDlgCode(HWND hwnd, INT virtualKey, MSG *pMsg)
{
	LRESULT result = EditboxTweak_CallOrigWindowProc(hwnd, WM_GETDLGCODE, (WPARAM)virtualKey, (LPARAM)pMsg);

	EDITBOXTWEAK *tweak = (EDITBOXTWEAK*)GetProp(hwnd, MAKEINTATOM(EDITBOXTWEAK_PROP));
		
	if (NULL != tweak && 0 != (ETF_NOTIFY_ENTERKEY & tweak->flags))
	{
		if (NULL == pMsg || VK_RETURN == virtualKey)
			result |= DLGC_WANTALLKEYS;
	}

	return result;
}

static LRESULT EditboxTweak_OnChar(HWND hwnd, INT charCode, UINT flags)
{
	switch(charCode)
	{
		case VK_RETURN:
			return 0;
	}
	return EditboxTweak_CallOrigWindowProc(hwnd, WM_CHAR, (WPARAM)charCode, (LPARAM)flags);
}

static LRESULT EditboxTweak_OnKeyDown(HWND hwnd, INT virtualKey, UINT flags)
{
	switch(virtualKey)
	{
		case VK_RETURN:
			if (EditboxTweak_NotifyReturn(hwnd)) return 0;
			break;
	}
	return EditboxTweak_CallOrigWindowProc(hwnd, WM_KEYDOWN, (WPARAM)virtualKey, (LPARAM)flags);
}

static LRESULT CALLBACK EditboxTweak_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_DESTROY:
			EditboxTweak_OnDestroy(hwnd);
			return 0;

		case WM_GETDLGCODE:
			return EditboxTweak_OnGetDlgCode(hwnd, (INT)wParam, (MSG*)lParam);
		
		case WM_CHAR:
			return EditboxTweak_OnChar(hwnd, (INT)wParam, (UINT)lParam);

		case WM_KEYDOWN:
			return EditboxTweak_OnKeyDown(hwnd, (INT)wParam, (UINT)lParam);


	}
	
	return EditboxTweak_CallOrigWindowProc(hwnd, uMsg, wParam, lParam);
}
