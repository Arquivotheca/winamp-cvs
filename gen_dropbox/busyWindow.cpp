#include "./main.h"
#include "./dropWindowInternal.h"
#include "./resource.h"
#include "./wasabiApi.h"
#include "./imageLoader.h"
#include "./skinWindow.h"
#include <commctrl.h>
#include <strsafe.h>

#define  IDC_WIDGET			1000

typedef struct _BUSYWINDOW
{
	HBITMAP		hbmp;
	HWND			hTarget;
	BOOL		showCommand;
	DWORD		showTime;
	BOOL		bitmapInvalid;
	HANDLE		bitmapThread;
	HANDLE		eventJob;
	HANDLE		eventKill;
	QUERYTHEMECOLOR GetThemeColor;
	QUERYTHEMEBRUSH GetThemeBrush;
	
} BUSYWINDOW;

typedef struct __BKIMAGETHREADPARAM
{
	HWND			hwndHost;
	HANDLE		hJob;
	HANDLE		hKill;
} BKIMAGETHREADPARAM;

#define CLOSEBKTREAD_TIMER		40
#define SHOWWINDOW_TIMER			41
#define CLOSEBKTREAD_DELAY		3000

static LRESULT CALLBACK BusyWindow_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static void CALLBACK BusyWindow_OnCloseBkThreadTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD dwTime);
static void CALLBACK BusyWindow_OnShowTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD dwTime);

extern HWND BusyWindowWidget_Create(HWND hParent, INT controlId, BOOL skinWindow);

#define GetBusyWindow(__hwnd) ((BUSYWINDOW*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))

BOOL BusyWindow_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	if (GetClassInfo(hInstance, NWC_BUSYWINDOW, &wc)) return TRUE;
	ZeroMemory(&wc, sizeof(WNDCLASS));

	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_BUSYWINDOW;
	wc.lpfnWndProc	= BusyWindow_WindowProc;
	wc.style			= CS_DBLCLKS;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(BUSYWINDOW*);
	
	return ( 0 != RegisterClass(&wc));
}

static HBITMAP BusyWindow_CreateBitmap(HWND hTarget, INT width, INT height, COLORREF rgbBk, BOOL allowBlur, HANDLE hKill)
{

//	LARGE_INTEGER freq, start, stop;
//	QueryPerformanceFrequency(&freq);
//	QueryPerformanceCounter(&start);

	HBITMAP hbmp;

	BITMAPINFOHEADER header;
	ZeroMemory(&header, sizeof(BITMAPINFOHEADER));

	header.biSize = sizeof(BITMAPINFOHEADER);
	header.biCompression = BI_RGB;
	header.biBitCount = 24;
	header.biPlanes = 1;
	header.biWidth = width;
	header.biHeight = -height;
	
	BYTE *pPixels;
	hbmp = CreateDIBSection(NULL, (LPBITMAPINFO)&header, DIB_RGB_COLORS, (void**)&pPixels, NULL, 0);
	if (NULL == hbmp)
		return NULL;

	HDC hdc = CreateCompatibleDC(NULL);
	if (NULL == hdc)
	{
		DeleteObject(hbmp);
		return NULL;
	}
	
	HBITMAP hbo = (HBITMAP)SelectObject(hdc, hbmp);

	if (NULL == hTarget)
	{
		RECT rcFill;
		SetRect(&rcFill, 0, 0, width, height);
		SetBkColor(hdc, rgbBk);
		ExtTextOut(hdc, 0, 0, OPAQUE, &rcFill, NULL, 0, NULL);
	}
	else
	{
		BYTE alpha = 160;
		BYTE blurRadius = 1;
		rgbBk = ((alpha << 24) | (0x00FFFFFF & rgbBk));

		SendMessage(hTarget, WM_PRINT, (WPARAM)hdc, (LPARAM) (PRF_NONCLIENT | PRF_CLIENT | PRF_CHILDREN | PRF_ERASEBKGND));
		if (allowBlur && WAIT_OBJECT_0 != WaitForSingleObject(hKill, 0))
			BoxBlur(pPixels, width, height, header.biBitCount, blurRadius, TRUE, hKill);
		
		if (WAIT_OBJECT_0 != WaitForSingleObject(hKill, 0))				
			ColorOverImageEx(pPixels, width, height, 0, 0, width, height, header.biBitCount, FALSE, rgbBk);

	}

	SelectObject(hdc, hbo);
	DeleteDC(hdc);

//	QueryPerformanceCounter(&stop);
//	INT duration = (INT)((stop.QuadPart - start.QuadPart) * 1000L / freq.QuadPart);
//	char buffer[128];
//	StringCchPrintfA(buffer, ARRAYSIZE(buffer), "boxblur %dx%d in %d\r\n", rc.right - rc.left, rc.bottom - rc.top, duration);
//	OutputDebugStringA(buffer);

	if (WAIT_OBJECT_0 == WaitForSingleObject(hKill, 0) && NULL != hbmp)
	{
		DeleteObject(hbmp);
		hbmp = NULL;
	}
	return hbmp;

}

static DWORD CALLBACK BusyWindow_BackImageThread(VOID *param)
{
	BKIMAGETHREADPARAM *threadParam = (BKIMAGETHREADPARAM*)param;
	if (NULL == param)
		return 0;
	HANDLE szEvents[] = { threadParam->hJob, threadParam->hKill };
	
	for(;;)
	{
		DWORD eventId = WaitForMultipleObjects(ARRAYSIZE(szEvents), szEvents, FALSE, INFINITE);
		ResetEvent(threadParam->hJob);
		
		if (WAIT_OBJECT_0 != eventId || 
			WAIT_OBJECT_0 == WaitForSingleObject(threadParam->hKill,0))
		{
			break;
		}
		
		HWND hTarget = BusyWindow_GetTargetWindow(threadParam->hwndHost);
		if (NULL != hTarget)
		{
			RECT rc;
			if (GetClientRect(threadParam->hwndHost, &rc))
			{
				COLORREF rgbBk = BusyWindow_GetBkColor(threadParam->hwndHost);
				BOOL allowBlur = (0 != (BWS_ENABLEBLUR & GetWindowLongPtr(threadParam->hwndHost, GWL_STYLE)));
				HBITMAP hbmp = BusyWindow_CreateBitmap(hTarget, rc.right - rc.left, rc.bottom - rc.top, rgbBk, 
														allowBlur, threadParam->hJob);
				if (WAIT_OBJECT_0 == WaitForSingleObject(threadParam->hKill, 0))
				{
					DeleteObject(hbmp);
					hbmp = NULL;
				}

				if (NULL != hbmp && 	0 == BusyWindow_BkImageReady(threadParam->hwndHost, hbmp))
				{
					DeleteObject(hbmp);
					hbmp = NULL;
				}
			}
		}
		
		if (WAIT_OBJECT_0 != WaitForSingleObject(threadParam->hKill,0))
			SetTimer(threadParam->hwndHost, CLOSEBKTREAD_TIMER, CLOSEBKTREAD_DELAY, BusyWindow_OnCloseBkThreadTimer);
		else
			break;
		
	}
	
	free(threadParam);
	return 0;
}

static BOOL BusyWindow_RequestBkImage(HWND hwnd)
{
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL == pbw)
		return FALSE;
	
	KillTimer(hwnd, CLOSEBKTREAD_TIMER);
	if (NULL == pbw->eventJob)
		pbw->eventJob = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (NULL == pbw->eventKill)
		pbw->eventKill = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (NULL == pbw->bitmapThread && 
		NULL != pbw->eventJob && 
		NULL != pbw->eventKill)
	{
		DWORD threadId;
		BKIMAGETHREADPARAM *threadParam = (BKIMAGETHREADPARAM*)malloc(sizeof(BKIMAGETHREADPARAM));
		threadParam->hwndHost = hwnd;
		
		threadParam->hJob = pbw->eventJob;
		threadParam->hKill = pbw->eventKill;
		
		pbw->bitmapThread = CreateThread(NULL, 0, BusyWindow_BackImageThread, threadParam, 0, &threadId);
		if (NULL == pbw->bitmapThread)
			free(threadParam);
	}
	
	if (NULL == pbw->bitmapThread)
	{
		if (NULL != pbw->eventJob)
		{
			CloseHandle(pbw->eventJob);
			pbw->eventJob = NULL;
		}

		if (NULL != pbw->eventKill)
		{
			CloseHandle(pbw->eventKill);
			pbw->eventKill = NULL;
		}
	}

	BOOL success = FALSE;

	if (NULL != pbw->eventJob)
		success = SetEvent(pbw->eventJob);

	return success;
}

static void BusyWindow_CloseBkThread(HWND hwnd, BUSYWINDOW *pbw)
{
	if (NULL == pbw) return;

	if (NULL != pbw->bitmapThread)
	{
		if (NULL != pbw->eventKill)
			SetEvent(pbw->eventKill);

		if (NULL != pbw->eventJob)
			SetEvent(pbw->eventJob);
		
		for(;;)
		{
			DWORD eventId = MsgWaitForMultipleObjects(1, &pbw->bitmapThread, FALSE, INFINITE, QS_SENDMESSAGE);
			if (WAIT_OBJECT_0 == eventId)
			{
				break;
			}
			else if ((WAIT_OBJECT_0 + 1) == eventId)
			{
				MSG msg;
				while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
				{
					if (WM_QUIT == msg.message) break;
					if (!CallMsgFilter(&msg, 0)) DispatchMessage(&msg);
				}

				if (WM_QUIT == msg.message) 
				{
					PostQuitMessage((int)msg.wParam);
					break;
				}
			}
		}

		CloseHandle(pbw->bitmapThread);
		pbw->bitmapThread = NULL;
	}

	if (NULL != pbw->eventJob)
	{
		CloseHandle(pbw->eventJob);
		pbw->eventJob = NULL;
	}

	if (NULL != pbw->eventKill)
	{
		CloseHandle(pbw->eventKill);
		pbw->eventKill = NULL;
	}
}
static BYTE *CreateGrayBuffer(BYTE *pPixels, INT width, INT height, WORD bpp)
{
	BYTE *data, *cursor, *px, *scan;
	INT x,y, line, pitch, r, g, b;
	
	pitch = bpp >> 3;

	line = width * pitch;
	if (0 != line % 4)
		line += ( 4 - line%4);
	
	data = (BYTE*)malloc(width * height);
	if (NULL == data)
		return NULL;

	cursor = data;
	scan = pPixels;

	for (y = 0; y < height; y++, scan += line)
	{			
		for (x = 0, px = scan; x < width; x++, px += pitch, cursor++) 
		{
			DWORD clr = *((COLORREF*)px);
			r = *(px + 2);
			g = *(px + 1);
			b = *px;
			*cursor = (r*299 + g*587 + b*114)/1000;
		}
	}

	return data;
}

static void CrayBufferToImage(BYTE *pGray, INT width, INT height, BYTE *pImage, WORD bpp, COLORREF rgbBk, COLORREF rgbFg)
{
	BYTE *px;
	INT x,y, line, pitch;
	
	pitch = bpp >> 3;

	line = width * pitch;
	if (0 != line % 4)
		line += ( 4 - line%4);

	BYTE rFg, gFg, bFg, rK, gK, bK, clr;
	rFg = GetRValue(rgbFg); gFg = GetGValue(rgbFg); bFg = GetBValue(rgbFg);
	rK = rFg - GetRValue(rgbBk); gK = gFg - GetGValue(rgbBk); bK = bFg - GetBValue(rgbBk);
	
	for (y = 0; y < height; y++, pImage += line)
	{			
		for (x = 0, px = pImage; x < width; x++, px += pitch, pGray++) 
		{
			clr = (255 - *pGray);
			*((COLORREF*)px) = RGB(bFg - ((bK*clr)>>8), gFg - ((gK*clr)>>8), rFg - ((rK*clr)>>8));
		}
	}
}


static void BusyWindow_SkinWindow(HWND hwnd, BOOL enableSkin)
{
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL == pbw) return;
	
	HFONT font = NULL;
	if (enableSkin)
	{	
		pbw->GetThemeColor = GetSkinColor;
		pbw->GetThemeBrush = GetSkinBrush;
		font = GetSkinFont();
	}
	else
	{
		pbw->GetThemeColor = GetSystemColor;
		pbw->GetThemeBrush = GetSystemBrush;
		font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}
	if (NULL != font)
		SendMessage(hwnd, WM_SETFONT, (WPARAM)font, TRUE);
}

static void BusyWindow_Paint(HWND hwnd, PAINTSTRUCT *pps)
{	
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL == pbw)
		return;

	if (pbw->bitmapInvalid)
	{
		BusyWindow_RequestBkImage(hwnd);
		pbw->bitmapInvalid = FALSE;
	}

	if (NULL != pbw->hbmp)
	{
		HDC hdcSrc = CreateCompatibleDC(pps->hdc);
		if (NULL != hdcSrc)
		{
			HBITMAP hbo = (HBITMAP)SelectObject(hdcSrc, pbw->hbmp);
			BITMAP bm;
			GetObject(pbw->hbmp, sizeof(BITMAP), &bm);
			RECT rcImage, rcBlit;
			SetRect(&rcImage, 0, 0, bm.bmWidth, abs(bm.bmHeight));
			if (IntersectRect(&rcBlit, &rcImage, &pps->rcPaint))
			{
				BOOL bSuccess = BitBlt(pps->hdc, rcBlit.left, rcBlit.top, 
						rcBlit.right - rcBlit.left, rcBlit.bottom - rcBlit.top, 
						hdcSrc, rcBlit.left, rcBlit.top, SRCCOPY);
				if (bSuccess)
					ExcludeClipRect(pps->hdc, rcBlit.left, rcBlit.top, rcBlit.right, rcBlit.bottom);
				
			}
			SelectObject(hdcSrc, pbw->hbmp);
			DeleteDC(hdcSrc);
			
		}
	}

	
	SetBkColor(pps->hdc, pbw->GetThemeColor(COLOR_WINDOW));
	ExtTextOut(pps->hdc, 0, 0, ETO_OPAQUE, &pps->rcPaint, NULL, 0, NULL);

	
		
}

static void BusyWindow_UpdateLayout(HWND hwnd, BOOL bRedraw)
{
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL == pbw) 	return;

	RECT rc, rcWidgetInvalid;
	GetClientRect(hwnd, &rc);

	HWND hWidget = GetDlgItem(hwnd, IDC_WIDGET);

	if (bRedraw && NULL != hWidget &&
		GetWindowRect(hWidget, &rcWidgetInvalid))
	{
		MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rcWidgetInvalid, 2);
	}
	else
	{
		SetRectEmpty(&rcWidgetInvalid);
	}


	if (NULL != hWidget)
	{
		MINMAXINFO minmax;
		ZeroMemory(&minmax, sizeof(MINMAXINFO));
				
		DWORD widgetStyle = GetWindowStyle(hwnd); 
		RECT rcWidget;
		CopyRect(&rcWidget, &rc);
		InflateRect(&rcWidget, -12, -12);
		
		LONG width = rcWidget.right - rcWidget.left;
		LONG height = rcWidget.bottom - rcWidget.top;
		
		minmax.ptMaxTrackSize.x = width;
		minmax.ptMaxTrackSize.y = height;
		minmax.ptMinTrackSize.x = width;
		minmax.ptMinTrackSize.y = height;
		
		SendMessage(hWidget, WM_GETMINMAXINFO, 0, (LPARAM)&minmax); 

		if (width > minmax.ptMaxTrackSize.x)
			width = minmax.ptMaxTrackSize.x;
		else if (width < minmax.ptMinTrackSize.x)
			width = minmax.ptMinTrackSize.x;
		
		if (height> minmax.ptMaxTrackSize.y)
			height = minmax.ptMaxTrackSize.y;
		else if (height < minmax.ptMinTrackSize.y)
			height = minmax.ptMinTrackSize.y;
		
		BOOL forceWidgetVisible = FALSE;
				
		if ((rcWidget.right - rcWidget.left) >= width && (rcWidget.bottom - rcWidget.top) >= height)
		{
			if (0 == (WS_VISIBLE & widgetStyle))
				forceWidgetVisible = TRUE;
		}
		else
		{
			if (0 != (WS_VISIBLE & widgetStyle))
			{
				SetWindowLongPtr(hWidget, GWL_STYLE, widgetStyle & ~WS_VISIBLE);
				if (bRedraw)
				{
					RECT rcInvalid;
					GetWindowRect(hWidget, &rcInvalid);
					MapWindowPoints(HWND_DESKTOP, hwnd, (POINT*)&rcInvalid, 2);
					InvalidateRect(hwnd, &rcInvalid, TRUE);
				}
			}
		}

		rcWidget.left += ((rcWidget.right - rcWidget.left) - width)/2;
		rcWidget.top += ((rcWidget.bottom - rcWidget.top) - height)/2;
		rcWidget.right = rcWidget.left + width;
		rcWidget.bottom = rcWidget.top + height;

        SetWindowPos(hWidget, NULL, rcWidget.left, rcWidget.top, 
				rcWidget.right - rcWidget.left, rcWidget.bottom - rcWidget.top, 
				SWP_NOACTIVATE | SWP_NOZORDER | ((bRedraw) ? 0 : SWP_NOREDRAW));

		if (forceWidgetVisible)
		{
			SetWindowLongPtr(hWidget, GWL_STYLE, widgetStyle | WS_VISIBLE);
			if (bRedraw)
				RedrawWindow(hWidget, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
		}

	}

	pbw->bitmapInvalid = TRUE;
	if (bRedraw)
	{
		if (!IsRectEmpty(&rcWidgetInvalid))
			InvalidateRect(hwnd, &rcWidgetInvalid, TRUE);
	}
}

static LRESULT BusyWindow_OnCreateWindow(HWND hwnd, CREATESTRUCT *pcs)
{
	BUSYWINDOW *pbw;

	pbw = (BUSYWINDOW*)malloc(sizeof(BUSYWINDOW));
	if (NULL == pbw)
	{
		DestroyWindow(hwnd);
		return -1;
	}

	ZeroMemory(pbw, sizeof(BUSYWINDOW));
	
	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)pbw) && ERROR_SUCCESS != GetLastError())
	{
		free(pbw);
		return -1;
	}

	DWORD windowStyle = GetWindowLong(hwnd, GWL_STYLE);
    
	pbw->showCommand = SW_HIDE;

	BOOL skinWindow = (0 != (DBS_SKINWINDOW & windowStyle));
	BusyWindow_SkinWindow(hwnd, skinWindow);

	HWND hWidget = BusyWindowWidget_Create(hwnd, IDC_WIDGET, skinWindow);
	BusyWindow_UpdateLayout(hwnd, TRUE);

	if (NULL != hWidget)
	{
		TCHAR szText[256];
		SendMessage(hwnd, WM_GETTEXT, ARRAYSIZE(szText), (LPARAM)szText);
		SendMessage(hWidget, WM_SETTEXT, 0, (LPARAM)szText);
		ShowWindow(hWidget, SW_SHOW);
	}

	return 0;
	
}

static void BusyWindow_OnDestroy(HWND hwnd)
{
	KillTimer(hwnd, CLOSEBKTREAD_TIMER);
	KillTimer(hwnd, SHOWWINDOW_TIMER);

	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);

	if (!pbw) return;
	
	BusyWindow_CloseBkThread(hwnd, pbw);
	if (NULL != pbw->hbmp)
		DeleteObject(pbw->hbmp);
		
	free(pbw);
	
}

static void BusyWindow_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right) 
			BusyWindow_Paint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}
}

static void BusyWindow_OnPrintClient(HWND hwnd, HDC hdc, UINT options)
{
	PAINTSTRUCT ps;
	ZeroMemory(&ps, sizeof(PAINTSTRUCT));
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
	ps.fErase = (0 != (PRF_ERASEBKGND & options));
	BusyWindow_Paint(hwnd, &ps);
}

static void BusyWindow_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	if (SWP_NOSIZE == ((SWP_NOSIZE | SWP_FRAMECHANGED) & pwp->flags)) return;
	BusyWindow_UpdateLayout(hwnd, (0 == (SWP_NOREDRAW & pwp->flags)));
}

static void BusyWindow_OnSetTargetWindow(HWND hwnd, HWND hTarget)
{
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL != pbw)
	{
		pbw->hTarget = hTarget;
		pbw->bitmapInvalid = TRUE;
	}
}

static void BusyWindow_OnSkinChanged(HWND hwnd)
{
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL != pbw)
	{
		pbw->bitmapInvalid = TRUE;
		HWND hWidget = GetDlgItem(hwnd, IDC_WIDGET);
		if (NULL != hWidget)
			SendMessage(hWidget, DBM_SKINCHANGED, 0, 0L);

		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
	}
}

static LRESULT BusyWindow_OnSetText(HWND hwnd, LPCTSTR pszText)
{
	HWND hWidget = GetDlgItem(hwnd, IDC_WIDGET);
	if (NULL != hWidget)
		SendMessage(hWidget, WM_SETTEXT, 0, (LPARAM)pszText);
	return DefWindowProc(hwnd, WM_SETTEXT, 0, (LPARAM)pszText);
}

static LRESULT BusyWindow_OnSetOperationInfo(HWND hwnd, BWOPERATIONINFO *poi)
{
	HWND hWidget = GetDlgItem(hwnd, IDC_WIDGET);
	if (NULL == hWidget || NULL == poi)
		return FALSE;
	
	HWND hCancelButton = GetDlgItem(hWidget, IDCANCEL);
	if (NULL != hCancelButton && (TRUE == poi->cancelable) != IsWindowEnabled(hCancelButton))
		EnableWindow(hCancelButton, (TRUE == poi->cancelable));
	
	TCHAR szText[256], szTemplate[224];
	if (((size_t)-1) == poi->total)
	{	// print out "%d items processed"
		WASABI_API_LNGSTRINGW_BUF(IDS_PROCESSED_ITEMS, szTemplate, ARRAYSIZE(szTemplate));
		StringCchPrintf(szText, ARRAYSIZE(szText), szTemplate, poi->processed);
	}
	else
	{ // printout "%d%% completed"
		WASABI_API_LNGSTRINGW_BUF(IDS_PROCESSED_PERCENT, szTemplate, ARRAYSIZE(szTemplate));
		StringCchPrintf(szText, ARRAYSIZE(szText), szTemplate, ((poi->processed * 100) / poi->total));
	}
	BusyWindowWidget_UpdateProcessed(hWidget, szText);

	return TRUE;
}

static void BusyWindow_OnCancelOperation(HWND hwnd)
{
	HWND hParent = GetParent(hwnd);
	if (NULL != hParent)
	{
		DropboxWindow_CancellActiveOperation(hParent);
	}

}

static LRESULT BusyWindow_OnBkImageReady(HWND hwnd, HBITMAP hbmp)
{
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL == pbw) return 0;

	if (NULL != pbw->hbmp)
		DeleteObject(pbw->hbmp);

	pbw->hbmp = hbmp;
	InvalidateRect(hwnd, NULL, TRUE);

	if (SW_HIDE != pbw->showCommand)
	{
		DWORD windowStyle = GetWindowStyle(hwnd);
		if (0 == (WS_VISIBLE & windowStyle))
		{
			UINT current = GetTickCount();
			if (current >= pbw->showTime)
			{
				ShowWindow(hwnd, pbw->showCommand);
				HWND hParent = GetParent(hwnd);
				if (NULL != hParent && hParent == GetActiveWindow())
					SetFocus(hParent);
				
				pbw->showCommand = SW_HIDE;
				pbw->showTime = 0;
			}
			else
			{
				UINT elapsed = pbw->showTime - current;
				if (elapsed > 10000)
					elapsed = 10000;
				SetTimer(hwnd, SHOWWINDOW_TIMER, elapsed, BusyWindow_OnShowTimer);
			}
		}
		else
		{
			pbw->showCommand = SW_HIDE;
			pbw->showTime = 0;
		}
		

	}


	return TRUE;
}

static LRESULT BusyWindow_OnGetTargetWindow(HWND hwnd)
{
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	return (LRESULT)((NULL != pbw) ? pbw->hTarget : NULL);
}

static LRESULT BusyWindow_OnGetBkColor(HWND hwnd)
{
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	return (LRESULT)((NULL != pbw) ? pbw->GetThemeColor(COLOR_DIALOG) : RGB(127, 127, 127));
}

static void CALLBACK BusyWindow_OnCloseBkThreadTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD dwTime)
{
	KillTimer(hwnd, eventId);
	
	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL != pbw)
		BusyWindow_CloseBkThread(hwnd, pbw);
}

static void CALLBACK BusyWindow_OnShowTimer(HWND hwnd, UINT uMsg, UINT_PTR eventId, DWORD dwTime)
{
	KillTimer(hwnd, eventId);

	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL == pbw) return;
		
	if (SW_HIDE != pbw->showCommand)
	{
		DWORD windowStyle = GetWindowStyle(hwnd);
		if (0 == (WS_VISIBLE & windowStyle))
		{
			ShowWindow(hwnd, pbw->showCommand);
			HWND hParent = GetParent(hwnd);
			if (NULL != hParent && hParent == GetFocus())
				SetFocus(hParent);
		}
		pbw->showCommand = SW_HIDE;
		pbw->showTime = 0;
	}
}

static void BusyWindow_OnPrepareAndShow(HWND hwnd, INT nCmdShow, INT delayMs)
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle) || SW_HIDE == nCmdShow)
		return;

	BUSYWINDOW *pbw = GetBusyWindow(hwnd);
	if (NULL == pbw)
		return;

	pbw->showCommand = nCmdShow;
	pbw->showTime = GetTickCount() + delayMs;
	if (BusyWindow_RequestBkImage(hwnd))
	{
		pbw->bitmapInvalid = FALSE;
	}
	else
	{
		pbw->showCommand = SW_HIDE;
		ShowWindow(hwnd, nCmdShow);
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent && hParent == GetFocus())
			SetFocus(hParent);
	}

}
static void BusyWindow_OnSetFocus(HWND hwnd, HWND hLost, BOOL bRedraw)
{
	HWND hWidget = GetDlgItem(hwnd, IDC_WIDGET);
	HWND hButton =  (NULL != hWidget) ? GetDlgItem(hWidget, IDCANCEL) : NULL;
	
	if (NULL != hButton &&
		IsWindowVisible(hButton) &&
		IsWindowEnabled(hButton))
	{
		PostMessage(hWidget, WM_NEXTDLGCTL, (WPARAM)hButton, TRUE);
	}
}

static LRESULT CALLBACK BusyWindow_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{		
		case WM_CREATE:				return BusyWindow_OnCreateWindow(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:				BusyWindow_OnDestroy(hwnd); return 0;
		case WM_PAINT:				BusyWindow_OnPaint(hwnd); return 0;
		case WM_PRINTCLIENT:			BusyWindow_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_SETTEXT:				return BusyWindow_OnSetText(hwnd, (LPCTSTR)lParam);
		case WM_WINDOWPOSCHANGED:	BusyWindow_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_SETFOCUS:			BusyWindow_OnSetFocus(hwnd, (HWND)wParam, (BOOL)lParam); return 0;
		case BWM_SETTARGETWINDOW:	BusyWindow_OnSetTargetWindow(hwnd, (HWND)lParam); return 0;
		case BWM_SETOPERATIONINFO:	return BusyWindow_OnSetOperationInfo(hwnd, (BWOPERATIONINFO*)lParam);
		case BWM_CANCELOPERATION:	BusyWindow_OnCancelOperation(hwnd); return 0;
		case BWM_BKIMAGEREADY:		return BusyWindow_OnBkImageReady(hwnd, (HBITMAP)lParam);
		case BWM_GETTARGETWINDOW:	return BusyWindow_OnGetTargetWindow(hwnd);
		case BWM_GETBKCOLOR:			return BusyWindow_OnGetBkColor(hwnd);
		case BWM_PREPANDSHOW:		BusyWindow_OnPrepareAndShow(hwnd, (INT)wParam, (INT)lParam); return 0;
		case DBM_SKINCHANGED:		BusyWindow_OnSkinChanged(hwnd); return 0;
		

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
