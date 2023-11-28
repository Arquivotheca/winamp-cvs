#include "./main.h"
#include "./dropWindowInternal.h"
#include "./plugin.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./imageLoader.h"
#include "./formatData.h"
#include "./skinWindow.h"
#include <commctrl.h>
#include <strsafe.h>
#include <math.h>


static ATOM	WIDGETWND = 0;

#define PROGRESS_FRAMECOUNT		24
#define ANIMATETIMER_ID			64
#define ANIMATETIMER_INTERVAL	1000 / PROGRESS_FRAMECOUNT

typedef struct __CREATEWIDGETPARAM
{
	BOOL skinWindow;
} CREATEWIDGETPARAM;

typedef struct __WIDGET
{
	SIZE sizeMin;
	SIZE sizeMax;
	RECT clientInflate;
	BOOL windowSkinned;
	QUERYTHEMECOLOR GetThemeColor;
	QUERYTHEMEBRUSH GetThemeBrush;
	RECT validRect;
	RECT progressRect;
	HBITMAP progressBitmap;
	INT progressFrame;
} WIDGET;

#define GetWidget(__hwnd) ((WIDGET*)GetProp((__hwnd), MAKEINTATOM(WIDGETWND)))

static INT_PTR CALLBACK BusyWindowWidget_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND BusyWindowWidget_Create(HWND hParent, INT controlId, BOOL skinWindow)
{
	CREATEWIDGETPARAM createParam;
	createParam.skinWindow = skinWindow;
	HWND hdlg = WASABI_API_CREATEDIALOGPARAMW(IDD_ASYNCOPERATION, hParent, BusyWindowWidget_DialogProc, (LPARAM)&createParam);
	if (NULL != hdlg)
	{
		SetWindowLongPtrW(hdlg, GWLP_ID, controlId);
	}
	return hdlg;
}


static void CALLBACK UninitializeWidget(void)
{
	if (0 != WIDGETWND)
	{
		GlobalDeleteAtom(WIDGETWND);
		WIDGETWND = 0;
	}
}

#define RA_LEFT		0x0001
#define RA_RIGHT	0x0002
#define RA_HCENTER	0x0003
#define RA_TOP		0x0010
#define RA_BOTTOM	0x0020
#define RA_VCENTER	0x0030
#define RA_FITHORZ	0x0100
#define RA_FITVERT	0x0200


static void RectAlign(RECT *prcTarget, const RECT *prcBounds, UINT flags)
{
	if (0 != (0x0F & flags)) // horz
	{
		LONG targetWidth = prcTarget->right - prcTarget->left;
		LONG boundsWidth = prcBounds->right - prcBounds->left;
	
		if (0 != (RA_FITHORZ & flags) && targetWidth > boundsWidth)
			targetWidth = boundsWidth;
	
		if (targetWidth == boundsWidth)
		{
			prcTarget->left = prcBounds->left;
			prcTarget->right = prcBounds->right;
		}
		else
		{
			switch(0x0F & flags)
			{
				case RA_HCENTER:	prcTarget->left = prcBounds->left + (boundsWidth - targetWidth)/2; break;
				case RA_LEFT:		prcTarget->left = prcBounds->left; break;
				case RA_RIGHT:		prcTarget->left = prcBounds->right - targetWidth; break;
			}
			prcTarget->right = prcTarget->left + targetWidth; 
		}
	}

	if (0 != (0xF0 & flags)) // horz
	{
		LONG targetHeight = prcTarget->bottom - prcTarget->top;
		LONG boundsHeight = prcBounds->bottom - prcBounds->top;
	
		if (0 != (RA_FITVERT & flags) && targetHeight > boundsHeight)
			targetHeight = boundsHeight;
	
		if (targetHeight == boundsHeight)
		{
			prcTarget->top = prcBounds->top;
			prcTarget->bottom = prcBounds->bottom;
		}
		else
		{
			switch(0xF0 & flags)
			{
				case RA_VCENTER:	prcTarget->top = prcBounds->top + (boundsHeight - targetHeight)/2; break;
				case RA_TOP:		prcTarget->top = prcBounds->top; break;
				case RA_BOTTOM:		prcTarget->top = prcBounds->bottom - targetHeight; break;
			}
			prcTarget->bottom = prcTarget->top + targetHeight; 
		}
	}

}


static INT GetWindowTextWidth(HWND hwnd, HFONT hFont, SIZE *pSize) // if hFont == NULL window font will be used, pSize can be NULL
{
	if (NULL != pSize)
		ZeroMemory(pSize, sizeof(SIZE));
		

	HFONT hfo;
	if (NULL == hFont)
	{
		hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0L);
		if (NULL == hFont) 
			hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}

	HDC hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (NULL == hdc)
		return 0;
	hfo = (NULL != hFont) ? (HFONT)SelectObject(hdc, hFont) : NULL;

	TCHAR szText[128];
	LPTSTR pszText = NULL;
	INT cchText = GetWindowTextLength(hwnd);
	
	SIZE textSize;
	ZeroMemory(&textSize, sizeof(SIZE));

	if (cchText > 0)
	{
		pszText = (cchText < ARRAYSIZE(szText)) ? szText : (LPTSTR)malloc(sizeof(TCHAR) * (cchText + 1));
		cchText = (NULL != pszText) ? GetWindowText(hwnd, szText, cchText + 1) : 0;
	}

	if (0 >= cchText)
	{
		if (NULL != pSize)
		{
			TEXTMETRIC tm;
			GetTextMetrics(hdc, &tm);
			textSize.cy = tm.tmHeight;
		}
	}
	else
	{		
		if (!GetTextExtentPoint32(hdc, pszText, cchText, &textSize))
			ZeroMemory(&textSize, sizeof(SIZE));
		if (NULL != pSize)
			CopyMemory(pSize, &textSize, sizeof(SIZE));
	}
	
	if (NULL != hfo)
		SelectObject(hdc, hfo);
	ReleaseDC(hwnd, hdc);

	if (NULL != pszText && pszText != szText)
		lfh_free(pszText);

	return textSize.cx;

}
static BOOL BusyWindowWidget_DeferControlPos(WINDOWPOS *pControls, INT controlsCount)
{
	HDWP hdwp = BeginDeferWindowPos(controlsCount);
	if (NULL == hdwp)
		return FALSE;

	for(INT i = 0; i < controlsCount && NULL != hdwp; i++)
	{
		if (NULL == pControls[i].hwnd)
			continue;

		hdwp = DeferWindowPos(hdwp, pControls[i].hwnd, NULL, pControls[i].x, pControls[i].y, 
					pControls[i].cx, pControls[i].cy, pControls[i].flags);
	}

	BOOL success = FALSE;

	if (NULL != hdwp)
		success = EndDeferWindowPos(hdwp);
	
	return success;
}

static void BusyWindowWidget_UpdateLayout(HWND hwnd, BOOL bRedraw)
{	
	RECT rc, rcControl;
	
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;

	INT ctrlList[] = { IDC_LABEL_TITLE, IDCANCEL, IDC_LABEL_PROCESSED};
	WINDOWPOS szwp[ARRAYSIZE(ctrlList)];

	if (!GetClientRect(hwnd, &rc))
		return;

	rc.left += pWidget->clientInflate.left;
	rc.top += pWidget->clientInflate.top;
	rc.right -= pWidget->clientInflate.right;
	rc.bottom -= pWidget->clientInflate.bottom;

	INT controlsCount = 0;
	WINDOWPOS *pwp;
	DWORD controlStyle;

	for (INT i = 0; i < ARRAYSIZE(ctrlList); i++)
	{
		pwp = &szwp[controlsCount];
		if (NULL == (pwp->hwnd = GetDlgItem(hwnd, ctrlList[i])) ||
			!GetWindowRect(pwp->hwnd, &rcControl))
		{	
			pwp->hwnd = NULL;
			continue;
		}

		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rcControl, 2); 

		pwp->x = rcControl.left;
		pwp->y = rcControl.top;
		pwp->cx = rcControl.right - rcControl.left;
		pwp->cy = rcControl.bottom - rcControl.top;

		switch(ctrlList[i])
		{
			case IDC_LABEL_TITLE:
				rcControl.right = rcControl.left + GetWindowTextWidth(pwp->hwnd, NULL, NULL);
				RectAlign(&rcControl, &rc, RA_HCENTER | RA_TOP | RA_FITHORZ);
				break;

			case IDCANCEL:
				RectAlign(&rcControl, &rc, RA_HCENTER | RA_BOTTOM);
				break;
			
			case IDC_LABEL_PROCESSED:
				rcControl.right = rcControl.left + GetWindowTextWidth(pwp->hwnd, NULL, NULL);
				RectAlign(&rcControl, &rc, RA_HCENTER | RA_BOTTOM | RA_FITHORZ);
				break;
		}

		controlStyle = GetWindowStyle(pwp->hwnd);
		if ((rcControl.top < rc.top) != (0 == (WS_VISIBLE & controlStyle)))
		{
			if (rcControl.top > rc.top) controlStyle |= WS_VISIBLE;
			else controlStyle &= ~WS_VISIBLE;
			SetWindowLongPtrW(pwp->hwnd, GWL_STYLE, controlStyle);
			if (bRedraw)
			{
				if (0 != (WS_VISIBLE & controlStyle))
					InvalidateRect(pwp->hwnd, NULL, TRUE);
				else
				{
					RECT rcInvlaidate;
					SetRect(&rcInvlaidate, pwp->x, pwp->y, pwp->x + pwp->cx, pwp->y + pwp->cy);
					InvalidateRect(hwnd, &rcInvlaidate, TRUE);
				}
			}
		}

		if (0 != (WS_VISIBLE & controlStyle))
		{
			switch(ctrlList[i])
			{
				case IDC_LABEL_TITLE:		rc.top = rcControl.bottom + 8; break;
				case IDCANCEL:				rc.bottom = rcControl.top - 8; break;
				case IDC_LABEL_PROCESSED:	rc.bottom = rcControl.top - 8;break;
			}
		}

		pwp->flags = SWP_NOACTIVATE | SWP_NOZORDER | ((bRedraw) ? 0 : SWP_NOREDRAW);
		
		if (rcControl.left == pwp->x && rcControl.top == pwp->y )
			pwp->flags |= SWP_NOMOVE;
		else
		{
			pwp->x = rcControl.left;
			pwp->y = rcControl.top;
		}

		if ((rcControl.right - rcControl.left) == pwp->cx && (rcControl.bottom - rcControl.top) == pwp->cy )
			pwp->flags |= SWP_NOSIZE;
		else
		{
			pwp->cx = rcControl.right - rcControl.left;
			pwp->cy = rcControl.bottom - rcControl.top;
			if (bRedraw)
				InvalidateRect(pwp->hwnd, NULL, TRUE);
		}

		if ((SWP_NOSIZE | SWP_NOMOVE) != ((SWP_NOSIZE | SWP_NOMOVE) & pwp->flags))
			controlsCount++;
	}

	
	SetRectEmpty(&rcControl);
	if (NULL != pWidget->progressBitmap)
	{
		BITMAP bm;
		if (GetObject(pWidget->progressBitmap, sizeof(BITMAP), &bm))
		{
			bm.bmHeight = bm.bmHeight / PROGRESS_FRAMECOUNT;

			if (bm.bmWidth < (rc.right - rc.left) && 
				bm.bmHeight < (rc.bottom - rc.top))
			{
				SetRect(&rcControl, 0, 0, bm.bmWidth, bm.bmHeight);
				RectAlign(&rcControl, &rc, RA_HCENTER | RA_VCENTER);
			}
		}
	}

	if (!EqualRect(&rcControl, &pWidget->progressRect))
	{
		if (bRedraw)
		{
			if (!IsRectEmpty(&pWidget->progressRect)) InvalidateRect(hwnd, &pWidget->progressRect, TRUE);
			if (!IsRectEmpty(&rcControl)) InvalidateRect(hwnd, &rcControl, FALSE);
		}			
		CopyRect(&pWidget->progressRect, &rcControl);
	}

	if (controlsCount > 0)
		BusyWindowWidget_DeferControlPos(szwp, controlsCount);
}

static void BusyWindowWidget_CalculateBounds(HWND hwnd)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;

	SetRectEmpty(&pWidget->clientInflate);

	RECT rc, rcControl;
	if (!GetClientRect(hwnd, &rc))
		return;

	HWND hctrl;
	
	if (NULL != (hctrl = GetDlgItem(hwnd, IDC_LABEL_TITLE)) &&
		GetWindowRect(hctrl, &rcControl))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rcControl, 2);
		pWidget->clientInflate.left = rcControl.left - rc.left;
		pWidget->clientInflate.top = rcControl.top - rc.top;
		pWidget->clientInflate.right = rc.right - rcControl.right;
	}

	if (NULL != (hctrl = GetDlgItem(hwnd, IDCANCEL)) &&
		GetWindowRect(hctrl, &rcControl))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rcControl, 2);
		pWidget->clientInflate.bottom = rc.bottom - rcControl.bottom;
	}
}

static void BusyWindowWidget_CalculateMaxMin(HWND hwnd)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;

	HWND hctrl;
	RECT rc, rcControl;

	ZeroMemory(&pWidget->sizeMax, sizeof(SIZE));
	ZeroMemory(&pWidget->sizeMin, sizeof(SIZE));

	if (GetWindowRect(hwnd, &rc))
	{
		pWidget->sizeMax.cx = rc.right - rc.left;
		pWidget->sizeMax.cy = rc.bottom - rc.top;

		pWidget->sizeMin.cx = pWidget->clientInflate.left + pWidget->clientInflate.right;
		pWidget->sizeMin.cy = pWidget->clientInflate.top + pWidget->clientInflate.bottom;

		if (NULL != (hctrl = GetDlgItem(hwnd, IDCANCEL)) &&
			GetWindowRect(hctrl, &rcControl))
		{
			pWidget->sizeMin.cx += (rcControl.right - rcControl.left);
		}

		if (NULL != (hctrl = GetDlgItem(hwnd, IDC_LABEL_TITLE)) &&
			GetWindowRect(hctrl, &rcControl))
		{
			pWidget->sizeMin.cy += (rcControl.bottom - rcControl.top) + 8;
		}

		if (GetClientRect(hwnd, &rcControl))
		{
			MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rcControl, 2);
			pWidget->sizeMin.cx += ((rcControl.left - rc.left) + (rc.right - rcControl.right));
			pWidget->sizeMin.cy += ((rcControl.top - rc.top) + (rc.bottom - rcControl.bottom));
		}

	}
}

static void BusyWindowWidget_SkinWindow(HWND hwnd, BOOL enableSkin)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;
	
	pWidget->windowSkinned = enableSkin;

	HFONT font = NULL;
	if (enableSkin)
	{	
		pWidget->GetThemeColor = GetSkinColor;
		pWidget->GetThemeBrush = GetSkinBrush;
		font = GetSkinFont();

		HWND hctrl = GetDlgItem(hwnd, IDCANCEL);
		if (NULL != hctrl)
			MlSkinWindow(hctrl, SWS_USESKINCOLORS | SWS_USESKINCURSORS);
	}
	else
	{
		pWidget->GetThemeColor = GetSystemColor;
		pWidget->GetThemeBrush = GetSystemBrush;
		font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}
	if (NULL != font)
		SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);

}
static void BusyWindowWidget_Paint(HWND hwnd, PAINTSTRUCT *pps)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;

	RECT rc;
	GetClientRect(hwnd, &rc);
		
	BOOL blitSuccess = FALSE;
	if (NULL != pWidget->progressBitmap && !IsRectEmpty(&pWidget->progressRect))
	{
		RECT rcBlit;
		if (IntersectRect(&rcBlit, &pWidget->progressRect, &pps->rcPaint))
		{
			HDC hdcSrc = CreateCompatibleDC(pps->hdc);
			if (NULL != hdcSrc)
			{
				HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcSrc, pWidget->progressBitmap);

				blitSuccess = BitBlt(pps->hdc, rcBlit.left, rcBlit.top, 
							rcBlit.right - rcBlit.left, rcBlit.bottom - rcBlit.top, 
							hdcSrc, 
							rcBlit.left - pWidget->progressRect.left, 
							(pWidget->progressRect.bottom - pWidget->progressRect.top)*pWidget->progressFrame + 
								rcBlit.top - pWidget->progressRect.top, 
							SRCCOPY);

				SelectObject(hdcSrc, hbmpOld);
				DeleteDC(hdcSrc);
				
			}

			if (blitSuccess)
			{
				if (EqualRect(&pWidget->progressRect, &pps->rcPaint))
					return; // we done
				ExcludeClipRect(pps->hdc, rcBlit.left, rcBlit.top, rcBlit.right, rcBlit.bottom);
			}
		}
	}

	if(rc.left >= pps->rcPaint.left || rc.top >= pps->rcPaint.top ||
		rc.right <= pps->rcPaint.right || rc.bottom <= pps->rcPaint.bottom)
	{
		FrameRect(pps->hdc, &rc, pWidget->GetThemeBrush(COLOR_WINDOWFRAME));
	}

	if (pps->fErase)
	{		
		RECT rcFill;
		CopyRect(&rcFill, &pps->rcPaint);
		
		if (rcFill.left == rc.left) rcFill.left++;
		if (rcFill.top == rc.top) rcFill.top++;
		if (rcFill.right == rc.right) rcFill.right--;
		if (rcFill.bottom == rc.bottom) rcFill.bottom--;

		SetBkColor(pps->hdc, pWidget->GetThemeColor(COLOR_DIALOG));
		ExtTextOut(pps->hdc, 0, 0, OPAQUE, &rcFill, NULL, 0, NULL);	
	}

	
}
static HBITMAP BusyWindowWidget_LoadImage(HWND hwnd, COLORREF rgbBk, COLORREF rgbFg)
{	
	HBITMAP hbmp;
	BITMAPINFOHEADER bitmapHeader;
	BYTE *pixels;

	hbmp = NULL;
	HBITMAP hbmpFrame = ImageLoader_LoadPngEx(plugin.hDllInstance, MAKEINTRESOURCE(IDR_PROGRESSCIRCLE_IMAGE),  
									&bitmapHeader, (void**)&pixels);

	if (NULL != hbmpFrame)
	{
		ColorizeImage(pixels, bitmapHeader.biWidth, abs(bitmapHeader.biHeight), 
			bitmapHeader.biBitCount, rgbBk, rgbFg);

		HDC hdcFrame = GetDCEx(hwnd, NULL, DCX_CACHE);
		if (NULL != hdcFrame)
		{
			HDC hdc = CreateCompatibleDC(hdcFrame);
			HDC hdcSrc = CreateCompatibleDC(hdcFrame);
			if (NULL != hdc && NULL != hdcSrc)
			{
				INT side = (INT)ceil(_hypot(bitmapHeader.biWidth, abs(bitmapHeader.biHeight)));
				hbmp = CreateCompatibleBitmap(hdcFrame, side, side * PROGRESS_FRAMECOUNT);
				if (NULL != hbmp)
				{
					LONG  centerX, centerY;
					centerX = side/2;
					centerY = side/2;

					XFORM xForm;
					ZeroMemory(&xForm, sizeof(XFORM));

					HBITMAP hbmpFrameOld = (HBITMAP)SelectObject(hdcSrc, hbmpFrame);
					HBITMAP hbmpOld = (HBITMAP)SelectObject(hdc, hbmp);
		
					SetBkColor(hdc, rgbBk);
					RECT rcFill;
					SetRect(&rcFill, 0, 0, side, side * PROGRESS_FRAMECOUNT);
					ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcFill, NULL, 0, NULL);
					INT graphicsMode = SetGraphicsMode(hdc, GM_ADVANCED);
					INT top = (side - abs(bitmapHeader.biHeight))/2;
					INT left = (side - bitmapHeader.biWidth)/2;

					for (INT i = 0; i < PROGRESS_FRAMECOUNT; i++)
					{
						double fangle = (double)(360/PROGRESS_FRAMECOUNT * i)/180. * 3.1415926;
						xForm.eM11 = (float)cos(fangle);
						xForm.eM12 = (float)sin(fangle);
						xForm.eM21 = (float)-sin(fangle);
						xForm.eM22 = (float)cos(fangle);
						xForm.eDx = (float)(centerX - cos(fangle)*centerX + sin(fangle)*centerY);
						xForm.eDy = (float)(centerY - cos(fangle)*centerY - sin(fangle)*centerX);
						SetWorldTransform(hdc, &xForm);
						BitBlt(hdc, left, top, bitmapHeader.biWidth, abs(bitmapHeader.biHeight), hdcSrc, 0, 0, SRCCOPY);
						centerY += side;
						top += side;

					}

					SetGraphicsMode(hdc, graphicsMode);
					ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);

					SelectObject(hdcSrc, hbmpFrameOld);
					SelectObject(hdc, hbmpOld);
					
				}	
			}
			if (NULL != hdc) DeleteDC(hdc);
			if (NULL != hdcSrc) DeleteDC(hdcSrc);
			ReleaseDC(hwnd, hdcFrame);
		}
		DeleteObject(hbmpFrame);
	}
				
	return hbmp;
}

static void CALLBACK BaseWindowWidget_AnimateTimerElpased(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;

	if (NULL != pWidget->progressBitmap && 
		!IsRectEmpty(&pWidget->progressRect))
	{

		pWidget->progressFrame++;
		if (pWidget->progressFrame >= PROGRESS_FRAMECOUNT)
			pWidget->progressFrame = 0;

		InvalidateRect(hwnd, &pWidget->progressRect, FALSE);
		
	}

}
 
static INT_PTR BusyWindowWidget_OnInitDialog(HWND hwnd, HWND hFocus, LPARAM lParam)
{
	if (0 == WIDGETWND)
	{
		 WIDGETWND = GlobalAddAtom(TEXT("WABUSYWIDGETWND"));
		 if (0 == WIDGETWND) 
		 {
			 DestroyWindow(hwnd);
			 return FALSE;
		 }
		 Plugin_RegisterUnloadCallback(UninitializeWidget);
	}

	CREATEWIDGETPARAM *createParam = (CREATEWIDGETPARAM*)lParam;

	BOOL success = TRUE;
	WIDGET *pWidget = (WIDGET*)malloc(sizeof(WIDGET));
	if (NULL == pWidget)
		success = TRUE;

	if (success)
	{
		ZeroMemory(pWidget, sizeof(WIDGET));
		if (!SetProp(hwnd, MAKEINTATOM(WIDGETWND), pWidget))
			success = FALSE;
	}
	
	if (!success)
	{
		DestroyWindow(hwnd);
		return FALSE;
	}

	BusyWindowWidget_SkinWindow(hwnd, NULL != createParam && createParam->skinWindow);
	BusyWindowWidget_CalculateBounds(hwnd);
	BusyWindowWidget_CalculateMaxMin(hwnd);

	pWidget->progressBitmap = BusyWindowWidget_LoadImage(hwnd, pWidget->GetThemeColor(COLOR_DIALOG), 
														pWidget->GetThemeColor(COLOR_DIALOGTEXT));

	TCHAR szText[256];
	SendMessage(hwnd, WM_GETTEXT, ARRAYSIZE(szText), (LPARAM)szText);
	SetDlgItemText(hwnd, IDC_LABEL_TITLE, szText);
	SetDlgItemText(hwnd, IDC_LABEL_PROCESSED, TEXT(""));
	EnableWindow(GetDlgItem(hwnd, IDCANCEL), FALSE);

    BusyWindowWidget_UpdateLayout(hwnd, TRUE);

	SetTimer(hwnd, ANIMATETIMER_ID, ANIMATETIMER_INTERVAL, BaseWindowWidget_AnimateTimerElpased);
	return TRUE;
}

static void BusyWindowWidget_OnDestroy(HWND hwnd)
{
	WIDGET *pWidget = GetWidget(hwnd);

	KillTimer(hwnd, ANIMATETIMER_ID);

	RemoveProp(hwnd, MAKEINTATOM(WIDGETWND));
	if (NULL != pWidget)
	{
		if (NULL != pWidget->progressBitmap)
		{
			DeleteObject(pWidget->progressBitmap);
		}
		free(pWidget);
	}

}

static void BusyWindowWidget_OnWindowPosChanging(HWND hwnd, WINDOWPOS* pwp)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;
	
	if (0 != (SWP_NOSIZE & pwp->flags) || !GetClientRect(hwnd, &pWidget->validRect)) 
		SetRectEmpty(&pWidget->validRect);
}

static void BusyWindowWidget_OnWindowPosChanged(HWND hwnd, WINDOWPOS* pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;
	
	BusyWindowWidget_UpdateLayout(hwnd, ( 0 == (SWP_NOREDRAW & pwp->flags)));
	if (0 == (SWP_NOREDRAW & pwp->flags) && 
		IsWindowVisible(hwnd))
	{

		WIDGET *pWidget = GetWidget(hwnd);
		if (NULL != pWidget)
		{

			RECT rc, rcInvalidate;
			if (GetClientRect(hwnd, &rc))
			{
				if (rc.right > pWidget->validRect.right)
				{
					SetRect(&rcInvalidate, pWidget->validRect.right - 1, rc.top, rc.right, rc.bottom);
					InvalidateRect(hwnd, &rcInvalidate, TRUE);
				}
				else if (rc.right < pWidget->validRect.right)
				{
					SetRect(&rcInvalidate, rc.right - 1, rc.top, rc.right, rc.bottom);
					InvalidateRect(hwnd, &rcInvalidate, TRUE);
				}

				if (rc.bottom > pWidget->validRect.bottom)
				{
					SetRect(&rcInvalidate, rc.left, pWidget->validRect.bottom - 1, pWidget->validRect.right, rc.bottom);
					InvalidateRect(hwnd, &rcInvalidate, TRUE);
				}
				else if (rc.bottom < pWidget->validRect.bottom)
				{
					SetRect(&rcInvalidate, rc.left, rc.bottom -1, rc.right, rc.bottom);
					InvalidateRect(hwnd, &rcInvalidate, TRUE);
				}
			}

		}

	}
}

static void BusyWindowWidget_OnGetMinMaxInfo(HWND hwnd, MINMAXINFO *pmm)
{

	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;

	pmm->ptMaxTrackSize.x = pWidget->sizeMax.cx;
	pmm->ptMaxTrackSize.y = pWidget->sizeMax.cy;
	pmm->ptMinTrackSize.x = pWidget->sizeMin.cx;
	pmm->ptMinTrackSize.y = pWidget->sizeMin.cy;
	pmm->ptMaxSize.x = pWidget->sizeMax.cx;
	pmm->ptMaxSize.y = pWidget->sizeMax.cy;
}

static void BusyWindowWidget_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right) 
			BusyWindowWidget_Paint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
}

static void BusyWindowWidget_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{
	PAINTSTRUCT ps;
	ZeroMemory(&ps, sizeof(PAINTSTRUCT));
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
	ps.fErase = (0 != (PRF_ERASEBKGND & options));
	BusyWindowWidget_Paint(hwnd, &ps);
}

static INT_PTR BusyWindowWidget_OnStaticColor(HWND hwnd, HDC hdc, HWND hCtrl)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL != pWidget && pWidget->windowSkinned) 
	{
		SetTextColor(hdc, pWidget->GetThemeColor(COLOR_DIALOGTEXT));
		SetBkColor(hdc, pWidget->GetThemeColor(COLOR_DIALOG));
		return (INT_PTR)pWidget->GetThemeBrush(COLOR_DIALOG);
	}
	return 0L;

}

static LRESULT BusyWindowWidget_OnSetText(HWND hwnd, LPCTSTR pszText)
{
	HWND hTitle = GetDlgItem(hwnd, IDC_LABEL_TITLE);
	if (NULL != hTitle)
	{
		SendMessage(hTitle, WM_SETTEXT, 0, (LPARAM)pszText);
		BusyWindowWidget_UpdateLayout(hwnd, TRUE);
	}
	return DefWindowProc(hwnd, WM_SETTEXT, 0, (LPARAM)pszText);
}

static void BusyWindowWidget_OnUpdateProcessed(HWND hwnd, LPCTSTR pszText)
{
	HWND hProcessed = GetDlgItem(hwnd, IDC_LABEL_PROCESSED);
	if (NULL != hProcessed)
	{
		SendMessage(hProcessed, WM_SETTEXT, 0, (LPARAM)pszText);
		BusyWindowWidget_UpdateLayout(hwnd, TRUE);
	}
}

static void BusyWindowWidget_OnCommand(HWND hwnd, INT ctrlId, INT eventId, HWND hCtrl)
{
	switch(ctrlId)
	{
		case IDCANCEL:
			BusyWindow_CancelOperation(GetParent(hwnd));
			break;
	}
}

static void BusyWindowWidget_OnSkinChanged(HWND hwnd)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return;

	if (NULL != pWidget->progressBitmap)
	{
		DeleteObject(pWidget->progressBitmap);
	}

	pWidget->progressBitmap = BusyWindowWidget_LoadImage(hwnd, pWidget->GetThemeColor(COLOR_DIALOG), 
														pWidget->GetThemeColor(COLOR_DIALOGTEXT));

	BusyWindowWidget_UpdateLayout(hwnd, FALSE);
	InvalidateRect(hwnd, NULL, TRUE);
}

static INT_PTR BusyWindowWidget_OnEraseBackground(HWND hwnd, HDC hdc)
{
	WIDGET *pWidget = GetWidget(hwnd);
	if (NULL == pWidget) return FALSE;
	
	if (pWidget->windowSkinned)
	{
		SetWindowLongPtr(hwnd, DWLP_MSGRESULT, 0); 
		return TRUE;
	}
	return FALSE;
}

static INT_PTR CALLBACK BusyWindowWidget_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:			return BusyWindowWidget_OnInitDialog(hwnd, (HWND)wParam, lParam);
		case WM_DESTROY:				BusyWindowWidget_OnDestroy(hwnd); break;
		case WM_WINDOWPOSCHANGING:	BusyWindowWidget_OnWindowPosChanging(hwnd, (WINDOWPOS*)lParam); return FALSE;
		case WM_WINDOWPOSCHANGED:	BusyWindowWidget_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return TRUE;
		case WM_GETMINMAXINFO:		BusyWindowWidget_OnGetMinMaxInfo(hwnd, (MINMAXINFO*)lParam); return TRUE;
		case WM_ERASEBKGND:			return BusyWindowWidget_OnEraseBackground(hwnd, (HDC)wParam); 
		case WM_PAINT:				BusyWindowWidget_OnPaint(hwnd); return TRUE;
		case WM_PRINTCLIENT:			BusyWindowWidget_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return TRUE;
		case WM_CTLCOLORSTATIC:		return BusyWindowWidget_OnStaticColor(hwnd, (HDC)wParam, (HWND)lParam);
		case WM_SETTEXT:				MSGRESULT(hwnd, BusyWindowWidget_OnSetText(hwnd, (LPCTSTR)lParam));
		case WM_COMMAND:				BusyWindowWidget_OnCommand(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return TRUE;
		case BWWM_UPDATEPROCESSED:	BusyWindowWidget_OnUpdateProcessed(hwnd, (LPCTSTR)lParam); return TRUE;
		case DBM_SKINCHANGED:		BusyWindowWidget_OnSkinChanged(hwnd); return TRUE;

	}
	return FALSE;
}