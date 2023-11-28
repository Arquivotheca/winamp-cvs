#include "./skinnedstatic.h"
#include "../winamp/wa_dlg.h"
#include "./skinning.h"
#include <strsafe.h>

	
SkinnedStatic::SkinnedStatic(void) : SkinnedWnd(FALSE)
{
}

SkinnedStatic::~SkinnedStatic(void)
{
}

BOOL SkinnedStatic::Attach(HWND hwndStatic)
{
	if(!SkinnedWnd::Attach(hwndStatic)) return FALSE;
	SetType(SKINNEDWND_TYPE_STATIC);
	
	HWND hwndParent = GetParent(hwndStatic);
	if (hwndParent) SkinWindow(hwndParent, SWS_NORMAL); 

	SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	return TRUE;
}


LRESULT SkinnedStatic::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (SWS_USESKINCOLORS & style)
	{
		switch(uMsg)
		{
			case REFLECTED_CTLCOLORSTATIC:
				{
					COLORREF rgbText, rgbTextBk;
					rgbText = WADlg_getColor(WADLG_WNDFG);
					rgbTextBk = WADlg_getColor(WADLG_WNDBG);

					if(!IsWindowEnabled(hwnd))
					{		
						rgbText = RGB((GetRValue(rgbText)+GetRValue(rgbTextBk))/2,
									(GetGValue(rgbText)+GetGValue(rgbTextBk))/2,
									(GetBValue(rgbText)+GetBValue(rgbTextBk))/2);
					}
	
					SetBkColor((HDC)wParam, rgbTextBk);
					SetTextColor((HDC)wParam, rgbText);
				}
				((REFLECTPARAM*)lParam)->result = (LRESULT)MlStockObjects_Get(WNDBCK_BRUSH);
				return TRUE;
		}
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}