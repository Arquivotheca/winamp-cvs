#include "main.h"
#include "./groupedList.h"
#include "./groupedListView.h"
#include "./plugin.h"
#include "../nu/windowsTheme.h"

#include <windows.h>
#include <tmschema.h>
#include <shlwapi.h>

typedef struct __PAINTITEMENUMPARAM
{
	GroupedListView		*view;
	const PAINTSTRUCT	*paintStruct;
	RECT				paintRect;
	HRGN				eraseRegion;
} PAINTITEMENUMPARAM;

typedef struct __VIEWHEIGHTENUMPARAM
{
	GroupedListViewStyle		*style;
	LONG					viewHeight;
	LONG					minHeight;
	LONG					maxHeight;
} VIEWHEIGHTENUMPARAM;

typedef struct __HITTESTENUMPARAM
{
	GroupedListViewStyle		*style;
	POINT					testPoint;
	RECT					itemRect; // set it to client rect before call
	GLItem					*item;
} HITTESTENUMPARAM;

typedef struct __ITEMRECTENUMPARAM
{
	GroupedListViewStyle		*style;
	GLItem					*item;
	RECT					itemRect; // set it to client rect before call
	BOOL					visibleOnly; // will stop enumerating if itemRect.top >= itemRect.bottom
	BOOL					foundOk;	// will be set to TRUE if item found
} ITEMRECTENUMPARAM;

typedef struct __FINDITEMENUMPARAM
{
	GroupedListViewStyle		*style;
	GLItem					*foundItem;
	RECT					itemRect;
	LONG					testY;
}FINDITEMENUMPARAM;

#define GetGroupedView(__hwnd) ((GroupedListView*)(LONG_PTR)(LONGX86)GetWindowLongPtr((__hwnd), 0))
static LRESULT WINAPI GroupedListView_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct __STYLECHNAGEDENUMPARAM
{
	GLView *view;
	UINT	updateFlags;
} STYLECHNAGEDENUMPARAM;
BOOL GroupedListView_RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX  wc;
	if (GetClassInfoEx(hInstance, NWC_GRPLSTVIEW, &wc)) return TRUE;
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize		= sizeof(WNDCLASSEX);
	wc.hInstance		= hInstance;
	wc.lpszClassName	= NWC_GRPLSTVIEW;
	wc.lpfnWndProc	= GroupedListView_WindowProc;
	wc.style			= 0;
	wc.hIcon			= NULL;
	wc.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.cbWndExtra	= sizeof(GroupedListView*);
	
	return (0 != RegisterClassEx(&wc));
}

HWND GroupedListView_CreateWindow(DWORD dwExStyle, DWORD dwStyle, INT x, INT y, INT cx, INT cy, 
									HWND hwndParent, INT controlId, HINSTANCE hInstance, GLRoot *root, LPCTSTR pszPngImage)
{
	if (!GroupedListView_RegisterClass(hInstance))
		return NULL;

	HWND hwnd = CreateWindowEx(dwExStyle, NWC_GRPLSTVIEW, NULL, dwStyle, 	x, y, cx, cy,
								hwndParent, (HMENU)(INT_PTR)controlId, hInstance, NULL);
	
	if (NULL == hwnd)
	{
		DWORD errorCode = GetLastError();
		aTRACE_FMT("Error creating window (%d)\r\n", errorCode);
		return NULL;
	}
	SendMessage(hwnd, GLVM_SETROOT, 0, (LPARAM)root);
	SendMessage(hwnd, GLVM_SETBITMAP, (WPARAM)hInstance, (LPARAM)pszPngImage);
	return hwnd;
}



static INT GroupedListView_GetLineHeight(GLStyle *style, GLItem *item)
{
	if (NULL == item || NULL == style) return 0;
	INT h = item->GetHeight(style);

	GLGroup* parent = (GLGroup*)item->GetParent();

//	if (NULL == parent || NULL != parent->GetParent() || GLGroup::FlagTypeGroup == parent->GetType())
	{
		UINT metricIndex = (NULL != parent && item != parent->GetChild(0) && 
							GLGroup::FlagTypeGroup == item->GetType()) ? 
								GLStyle::uiMetricGroupInterval : GLStyle::uiMetricItemInterval;
		h += style->GetMetrics(metricIndex);
	}

	return h;
}

static BOOL CALLBACK PaintItem_EnumProc(GLItem *item, ULONG_PTR user)
{
	PAINTITEMENUMPARAM *param = (PAINTITEMENUMPARAM*)user;
	return param->view->PaintItem(item, param->paintStruct, &param->paintRect, param->eraseRegion);
}

static BOOL CALLBACK ViewHeight_EnumProc(GLItem *item, ULONG_PTR user)
{
	VIEWHEIGHTENUMPARAM *param = (VIEWHEIGHTENUMPARAM*)user;
	INT itemHeight = GroupedListView_GetLineHeight(param->style, item);
	param->viewHeight += itemHeight;
	
	if (param->minHeight > itemHeight)
		param->minHeight = itemHeight;
	
	if (param->maxHeight < itemHeight)
		param->maxHeight = itemHeight;

	return TRUE;
}

static BOOL CALLBACK HitTest_EnumProc(GLItem *item, ULONG_PTR user)
{
	HITTESTENUMPARAM *param = (HITTESTENUMPARAM*)user;
	INT itemHeight = GroupedListView_GetLineHeight(param->style, item);

	if (param->itemRect.top <= param->testPoint.y && 
		(param->itemRect.top + itemHeight) > param->testPoint.y)
	{
		param->itemRect.bottom = param->itemRect.top + itemHeight;
		RECT testRect;
		CopyRect(&testRect, &param->itemRect);
		item->AdjustRect(param->style, &testRect);
		if (PtInRect(&testRect, param->testPoint))
		{
			param->item = item;
			CopyRect(&param->itemRect, &testRect);
		}
		return FALSE;
	}
	
	param->itemRect.top += itemHeight;
	return TRUE;
}

static BOOL CALLBACK ItemRect_EnumProc(GLItem *item, ULONG_PTR user)
{
	ITEMRECTENUMPARAM *param = (ITEMRECTENUMPARAM*)user;
	INT itemHeight = GroupedListView_GetLineHeight(param->style, item);

	if (item == param->item)
	{
		param->itemRect.bottom = param->itemRect.top + itemHeight;
		//item->AdjustRect(param->style, &param->itemRect);
		param->foundOk = TRUE;
		return FALSE;
	}
	
	param->itemRect.top += itemHeight;
	if (param->visibleOnly && param->itemRect.top >= param->itemRect.bottom)
	{
        param->foundOk = FALSE;
		return FALSE;
	}
	return TRUE;
}

static BOOL CALLBACK FindTopItem_EnumProc(GLItem *item, ULONG_PTR user)
{
	FINDITEMENUMPARAM *param = (FINDITEMENUMPARAM*)user;
	INT itemHeight = GroupedListView_GetLineHeight(param->style, item);
	if (param->itemRect.top >= param->testY)
	{
		param->foundItem = item;
		return FALSE;
	}
	
	param->itemRect.top += itemHeight;
	return TRUE;
}

static BOOL CALLBACK FindBottomItem_EnumProc(GLItem *item, ULONG_PTR user)
{	
	FINDITEMENUMPARAM *param = (FINDITEMENUMPARAM*)user;
	
	param->itemRect.top += GroupedListView_GetLineHeight(param->style, item);
	if (param->itemRect.top > param->testY)
	{
		if (NULL == param->foundItem)
			param->foundItem = item;
		return FALSE;
	}
	param->foundItem = item;
	return TRUE;
}


static BOOL CALLBACK StyleChanged_EnumProc(GLItem *item, ULONG_PTR user)
{
	STYLECHNAGEDENUMPARAM *enumParam = (STYLECHNAGEDENUMPARAM*)user;

	if (NULL != item && NULL != enumParam)
		item->StyleChanged(enumParam->view, enumParam->updateFlags);

	return TRUE;
}
GroupedListView::GroupedListView(HWND hHost) : 
	hwnd(hHost), root(NULL), highlighted(NULL), pressed(NULL), focused(NULL), 
	wheelCarryover(0), continuousScroll(FALSE), backgroundBrush(NULL), hFont(NULL), listTheme(NULL), 
	backBuffer(NULL), backDC(NULL)
{
	style = new GroupedListViewStyle();
	UpdateStyle(GLStyle::updateFlagAll);
}

GroupedListView::~GroupedListView()
{
	if (NULL != root)
		root->Release();

	if (NULL != backgroundBrush)
		DeleteObject(backgroundBrush);

	if (NULL != hFont)
		DeleteObject(hFont);

	if (NULL != listTheme)
		UxCloseThemeData(listTheme);

	if (NULL != backBuffer)
		DeleteObject(backBuffer);
		
	if (NULL != backDC)
		DeleteDC(backDC);
	
	delete(style);
}

BOOL GroupedListView::PaintItem(GLItem *item, const PAINTSTRUCT *paintStruct, RECT *paintRect, HRGN eraseRegion)
{
	RECT rcItem;
	CopyRect(&rcItem, paintRect);
	rcItem.bottom = rcItem.top + GroupedListView_GetLineHeight(style, item);

	if (rcItem.bottom > paintStruct->rcPaint.top)
	{
		UINT drawFlags = GLItem::DrawFlagNormal;
		if (item == focused && 
			hwnd == GetFocus() &&
			0 == (UISF_HIDEFOCUS & DefWindowProc(hwnd, WM_QUERYUISTATE, 0, 0L))) 
		{
			drawFlags |= GLItem::DrawFlagFocus;
		}

		if (!IsWindowEnabled(hwnd))
			drawFlags |= GLItem::DrawFlagDisabled;
			
		item->AdjustRect(style, &rcItem);
	

		BITMAP bi;
		if (NULL != backDC &&
			(NULL == backBuffer ||
			sizeof(BITMAP) != GetObject(backBuffer, sizeof(BITMAP), &bi) ||
			bi.bmWidth < (rcItem.right - rcItem.left) || bi.bmHeight < (rcItem.bottom - rcItem.top)))
		{
			if (NULL != backBuffer)
				DeleteObject(backBuffer);
			backBuffer = CreateCompatibleBitmap(paintStruct->hdc, rcItem.right - rcItem.left, rcItem.bottom - rcItem.top);
		}

		BOOL drawSuccess = FALSE;
		if (NULL != backBuffer)
		{
			HBITMAP bitmapOrig = (HBITMAP)SelectObject(backDC, backBuffer);
			POINT viewport = *(POINT*)&rcItem;
			
			OffsetRect(&rcItem, -rcItem.left, -rcItem.top);
			drawSuccess = item->Draw(style, backDC, &rcItem, drawFlags);
			OffsetRect(&rcItem, viewport.x, viewport.y);
			
			if (drawSuccess)
			{
				drawSuccess = BitBlt(paintStruct->hdc, rcItem.left, rcItem.top, 
								rcItem.right - rcItem.left, rcItem.bottom - rcItem.top, backDC, 0, 0, SRCCOPY);
			}
			SelectObject(backDC, bitmapOrig);
		}
		else
		{
			drawSuccess = item->Draw(style, paintStruct->hdc, &rcItem, drawFlags);
		}

		if (drawSuccess)
		{
			HRGN itemRegion = CreateRectRgnIndirect(&rcItem);
			CombineRgn(eraseRegion, eraseRegion, itemRegion, RGN_DIFF);
			DeleteObject(itemRegion);
		}
	}
	paintRect->top = rcItem.bottom;

	return (rcItem.bottom < paintStruct->rcPaint.bottom);
		
}

void GroupedListView::Paint(const PAINTSTRUCT *paintStruct)
{
	HDC hdc = paintStruct->hdc;
	
	HRGN eraseRegion = CreateRectRgnIndirect(&paintStruct->rcPaint);
	

	if (NULL != root)
	{		
		PAINTITEMENUMPARAM param;
		param.view = this;
		param.paintStruct = paintStruct;
		param.eraseRegion = eraseRegion;
		GetClientRect(hwnd, &param.paintRect);

		SCROLLINFO scrollInfo;
		scrollInfo.cbSize = sizeof(SCROLLINFO);
		scrollInfo.fMask = SIF_POS;
		if (GetScrollInfo(hwnd, SB_VERT, &scrollInfo) && 0 != scrollInfo.nPos)
			OffsetRect(&param.paintRect, 0, -scrollInfo.nPos);

		if (NULL == backDC)
		{
			if (NULL != backBuffer)
			{
				DeleteObject(backBuffer);
				backBuffer = NULL;
			}

			backDC = CreateCompatibleDC(paintStruct->hdc);
		}
		
		root->EnumerateChidlren(PaintItem_EnumProc, (ULONG_PTR)&param);
	}

	if (paintStruct->fErase)
	{
		if (NULL == backgroundBrush)
			backgroundBrush = CreateSolidBrush(style->GetColor(GLStyle::uiColorWindow));

		FillRgn(hdc, eraseRegion, backgroundBrush);
	}
	
	DeleteObject(eraseRegion);
}

void GroupedListView::Invalidate(GLItem *item)
{
	SCROLLINFO scrollInfo;
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_POS;
	if (!GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		scrollInfo.nPos = 0;

	if (NULL == item || NULL == root)
		return;

	ITEMRECTENUMPARAM param;
	param.style = style;
	param.item  = item;
	param.visibleOnly = TRUE;
	param.foundOk = FALSE;
	if (!GetClientRect(hwnd, &param.itemRect))
		return;
	
	param.itemRect.top -= scrollInfo.nPos; 
			
	root->EnumerateChidlren(ItemRect_EnumProc, (ULONG_PTR)&param);
	
	if (param.foundOk && param.itemRect.top < param.itemRect.bottom && param.itemRect.bottom > 0)
		InvalidateRect(hwnd, &param.itemRect, TRUE);	
}

void GroupedListView::SetRoot(GLRoot *groupRoot)
{
	if (NULL != root)
		root->Release();

	root = groupRoot;

	if (NULL != root)
	{
		root->AddRef();
		root->RegisterCallback(this);
	}
		

	UpdateScrollInfo();
	InvalidateRect(hwnd, NULL, TRUE);
}

BOOL GroupedListView::SetBitmap(HINSTANCE resourceInstance, LPCTSTR resourceName)
{
	if (NULL == style)
		return FALSE;
	BOOL result = style->SetPngResource(resourceInstance, resourceName);
	
	if (NULL != result)
		UpdateStyle(GLStyle::updateFlagImages);

	return result;
}

void GroupedListView::UpdateScrollInfo()
{
	LONG viewHeight;
	LONG pageHeight;
	RECT clientRect;
	
	if (!GetClientRect(hwnd, &clientRect))
		SetRectEmpty(&clientRect);
	
	viewHeight = GetViewHeight(&lineHeight, NULL);
    if (lineHeight < 1) lineHeight = 1;
	
//	if (viewHeight > (clientRect.bottom - clientRect.top))
//		viewHeight += (lineHeight - viewHeight%lineHeight);
	
	if (viewHeight < 0) viewHeight = 0;

	pageHeight = clientRect.bottom - clientRect.top;
//	pageHeight -= pageHeight%lineHeight;
	if (pageHeight < 0) pageHeight = 0;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	if (!GetScrollInfo(hwnd, SB_VERT, &si))
		ZeroMemory(&si, sizeof(SCROLLINFO));
	
	if (si.nMin == 0 && si.nMax == viewHeight)
		si.fMask &= ~SIF_RANGE;
	else
	{
		si.nMin = 0;
		si.nMax = viewHeight;
	}

	if (si.nPage == pageHeight)
		si.fMask &= ~SIF_PAGE;
	else
		si.nPage = pageHeight;
	
	
	if (si.nPos < si.nMin) 
		si.nPos = si.nMin;
	else if ((si.nPos + si.nPage) > ((UINT)si.nMax)) 
		si.nPos =  ((UINT)si.nMax) - si.nPage;
	else 
		si.fMask &= ~SIF_POS;
	if (0 != si.fMask)
	{
		si.fMask |= SIF_DISABLENOSCROLL;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	}

}

LONG GroupedListView::GetViewHeight(INT *minHeightOut, INT *maxHeightOut)
{
	VIEWHEIGHTENUMPARAM param;
	param.style = this->style;
	param.viewHeight = 0;
	param.minHeight = 65555;
	param.maxHeight = 0;
	
	if (NULL != root)
		root->EnumerateChidlren(ViewHeight_EnumProc, (ULONG_PTR)&param);
	else
		param.minHeight = 0;

	if (NULL != minHeightOut)
		*minHeightOut = param.minHeight;

	if (NULL != maxHeightOut)
		*maxHeightOut = param.maxHeight;

	return param.viewHeight;
}


void GroupedListView::ScrollVert(UINT scrollCode, UINT trackPos)
{
	SCROLLINFO scrollInfo;
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	if (NULL == root ||
		!GetScrollInfo(hwnd, SB_VERT, &scrollInfo) || 
		scrollInfo.nMin >= scrollInfo.nMax ||
		scrollInfo.nPage > (UINT)((scrollInfo.nMax - scrollInfo.nMin)))
	{
		return;
	}

	LONG delta = 0;
	scrollInfo.fMask = 0;
	
	switch(scrollCode)
	{
		case SB_LINEDOWN:		delta = lineHeight; break;
		case SB_LINEUP:			delta = -lineHeight; break;
		case SB_PAGEDOWN:		delta = (scrollInfo.nPage - lineHeight); break;
		case SB_PAGEUP:			delta = -(LONG)((scrollInfo.nPage - lineHeight)); break;
		case SB_TOP:			delta = -scrollInfo.nPos; break;
		case SB_BOTTOM:			delta = scrollInfo.nMax - scrollInfo.nPage; break;
		case SB_THUMBTRACK:		delta = trackPos - scrollInfo.nPos; break;
		case SB_THUMBPOSITION:	delta = trackPos - scrollInfo.nPos; break;
		case SB_ENDSCROLL:		continuousScroll = FALSE;
			break;
	}

	ScrollVertDelta(delta);
}

INT GroupedListView::ScrollVertDelta(INT delta)
{
	if (0 == delta)
		return 0;

	SCROLLINFO scrollInfo;
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
	if (!GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		return 0;
	
	INT newPos = scrollInfo.nPos + delta;

	if (newPos < scrollInfo.nMin)
		newPos = scrollInfo.nMin;
	else if ((newPos + scrollInfo.nPage) > ((UINT)scrollInfo.nMax)) 
		newPos = ((UINT)scrollInfo.nMax) - scrollInfo.nPage;

	INT originalPos = scrollInfo.nPos;

	UINT scrollFlags = 0;
	if (FALSE == continuousScroll)
	{
		scrollFlags |= MAKELONG(SW_SMOOTHSCROLL, 0);
		UpdateWindow(hwnd); // ?
		continuousScroll = TRUE;
	}

	if (newPos != scrollInfo.nPos)
	{		
		scrollInfo.fMask |= SIF_POS;
		scrollInfo.nPos = newPos;
		newPos = SetScrollInfo(hwnd, SB_VERT, &scrollInfo, TRUE);
		
		if (newPos != originalPos)
		{
			HRGN rgnUpdate = CreateRectRgn(0, 0, 0, 0);
			

			if (ScrollWindowEx(hwnd, 0, originalPos - newPos, NULL, NULL, rgnUpdate, NULL, scrollFlags))
			{				
				InvalidateRgn(hwnd, rgnUpdate, TRUE);
			}
			DeleteObject(rgnUpdate);
		}
	}

	return (newPos - originalPos);
}

GLItem *GroupedListView::HitTest(POINT pt, RECT *prcItem)
{
	SCROLLINFO scrollInfo;
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_POS;
	if (!GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		scrollInfo.nPos = 0;

	HITTESTENUMPARAM param;
	ZeroMemory(&param, sizeof(HITTESTENUMPARAM));

	param.style = style;
	param.testPoint = pt;
	
	if (GetClientRect(hwnd, &param.itemRect))
	{
		OffsetRect(&param.itemRect, 0, -scrollInfo.nPos);
		if (NULL != root)
			root->EnumerateChidlren(HitTest_EnumProc, (ULONG_PTR)&param);
	}
	
	if (NULL != prcItem)
	{
		if (NULL != param.item)
			CopyRect(prcItem, &param.itemRect);
		else
			SetRectEmpty(prcItem);
	}
	
	return param.item;
}
void GroupedListView::MouseWheel(INT zDelta, UINT mouseFlags, POINT pt)
{	
	SCROLLINFO scrollInfo;
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_PAGE;
	if (!GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		return;

	UINT linesPerPage = scrollInfo.nPage / lineHeight;
	if (linesPerPage < 1)
		return;

	UINT uScroll;
	if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uScroll, 0))
		uScroll = 3;    /* default value */
	
	if (uScroll > linesPerPage)
		uScroll = linesPerPage;
    
	if (0 == uScroll)
		return;

	zDelta += wheelCarryover;

	int dLines = zDelta * (int)uScroll / WHEEL_DELTA;
	wheelCarryover = zDelta - dLines * WHEEL_DELTA / (int)uScroll;
		
	
	BOOL scrolled = FALSE;

	INT lineCY = style->GetMetrics(GLStyle::uiMetricItemFontCY);
	if (lineCY > lineHeight) lineCY = lineHeight;
	
	INT scrollDelta = (dLines > 0) ? -lineCY : lineCY;
	
	for (INT i = abs(dLines) * 1; i > 0; i--)
	{
		continuousScroll = TRUE;

        if (0 == ScrollVertDelta(scrollDelta))
			break;

		UpdateWindow(hwnd);
		Sleep(30);
		scrolled = TRUE;
	}
	if (scrolled)
		MouseMove(mouseFlags, pt);

}

void GroupedListView::MouseMove(UINT mouseFlags, POINT pt)
{
	if (0 != (0x8000 & GetAsyncKeyState(VK_SPACE)))
		return;

	RECT itemRect;
	GLItem *item = HitTest(pt, &itemRect);
	if (item != highlighted)
	{
		RemoveHighlight();

		if (NULL != item && 	(NULL == pressed || pressed == item))
		{
			item->MouseEnter(this);
			if (NULL == highlighted)
			{
				TRACKMOUSEEVENT tm;
				tm.cbSize = sizeof(TRACKMOUSEEVENT);
				tm.dwFlags = TME_LEAVE;
				tm.hwndTrack = hwnd;
				TrackMouseEvent(&tm);
			}
		}
		highlighted = item;
	}
	
	if (NULL != highlighted && (NULL == pressed || pressed == highlighted))
		highlighted->MouseMove(this, &itemRect, pt);
}

void GroupedListView::MouseLeave()
{
	RemoveHighlight();
}

void GroupedListView::LeftButtonDown(UINT mouseFlags, POINT pt)
{
	if (0 != (0x8000 & GetAsyncKeyState(VK_SPACE)))
		return;

	continuousScroll = FALSE;

	BOOL captureMouse = FALSE;
	RECT itemRect;
	GLItem *item = HitTest(pt, &itemRect);

	if (pressed != item)
	{
		if (NULL != pressed)
			pressed->LButtonUp(this, &itemRect, pt);
		
		pressed = item;
		
		if (NULL != pressed)
		{
			if (GLGroup::FlagTypeGroup != pressed->GetType())
			{
				BOOL pressedOk = pressed->LButtonDown(this, &itemRect, pt);
				if (pressedOk)
				{
					if (pressed != focused)
					{						
						Select(pressed);
					}
					captureMouse = (hwnd != GetCapture());
				}
				else 
					pressed = NULL;
			}
		}
	}


	if (captureMouse)
		SetCapture(hwnd);

}

void GroupedListView::LeftButtonUp(UINT mouseFlags, POINT pt)
{
	if (NULL != pressed)
	{
		RECT itemRect;
		GLItem *item = HitTest(pt, &itemRect);
	
		highlighted = pressed;

		pressed->LButtonUp(this, &itemRect, pt);

		if (item == pressed)
			pressed->LButtonClick(this);
		
		BOOL sendMove = (NULL != item && item != pressed);
		pressed = NULL;

		if (hwnd == GetCapture())
			ReleaseCapture();


		if (FALSE == sendMove && NULL != item)
		{
			INT delta = 0;
			RECT clientRect, pressedRect;
		
			if (GetClientRect(hwnd, &clientRect) && GetItemRect(item, &pressedRect)) 
			{ // hit test calls adjust rect and this is not what we want
				if (pressedRect.top < clientRect.top)
				{
					delta = -(pressedRect.bottom - pressedRect.top);
					if ((itemRect.bottom - delta) > clientRect.bottom)
						delta = itemRect.bottom - clientRect.bottom;
				}
				else
				{
					if (itemRect.bottom > clientRect.bottom)
					{
						BOOL showBottom = FALSE;
						GLItem *next = item->Next();
						
						if (NULL == next)
						{
							if (GLGroup::FlagTypeGroup == item->GetType())
                                next = ((GLGroup*)item)->Last();
							if (NULL == next) next = item;
							showBottom = TRUE;
						}

						if (NULL != next && GetItemRect(next, &pressedRect))
						{
							next->AdjustRect(style, &pressedRect);
							delta = ((showBottom) ? pressedRect.bottom : pressedRect.top) - itemRect.top;
							if ((itemRect.top - delta) < clientRect.top)
								delta = itemRect.top - clientRect.top;
						}
							
					}
				}
			}

			if (0 != delta)
			{		
				RemoveHighlight();
				continuousScroll = FALSE;
				ScrollVertDelta(delta);
			}
		}
		MouseMove(mouseFlags, pt);
	}
}

BOOL GroupedListView::GetItemRect(GLItem *item, RECT *itemRect)
{
	if (NULL == item || NULL == root) 
		return FALSE;

	SCROLLINFO scrollInfo;
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_POS;
	if (!GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		scrollInfo.nPos = 0;

	ITEMRECTENUMPARAM param;
	param.style = style;
	param.item  = item;
	param.visibleOnly = FALSE;
	param.foundOk = FALSE;

	if (!GetClientRect(hwnd, &param.itemRect))
		return FALSE;

	param.itemRect.top -= scrollInfo.nPos;
	root->EnumerateChidlren(ItemRect_EnumProc, (ULONG_PTR)&param);

	if (param.foundOk)
		CopyRect(itemRect, &param.itemRect);
	
	return param.foundOk;
}

void GroupedListView::RemoveHighlight()
{
	if (NULL != highlighted)
	{
		highlighted->MouseLeave(this);
		highlighted = NULL;
	}
}

void GroupedListView::UpdateHighlight()
{
	POINT pt;
	RECT clientRect;

	if (!GetCursorPos(&pt))
		ZeroMemory(&pt, sizeof(POINT));

	MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);

	if (GetClientRect(hwnd, &clientRect) && 
		PtInRect(&clientRect, pt))
	{
		UINT mouseFlags = 0;
		MouseMove(mouseFlags, pt);
	}
}

void GroupedListView::EnsureVisible(GLItem *item)
{
	if (NULL == item) return;

	RECT clientRect, itemRect;
	if (!GetClientRect(hwnd, &clientRect) || !GetItemRect(item, &itemRect))
		return;

	INT delta = 0;
	
	if (itemRect.top < clientRect.top)
		delta = itemRect.top - clientRect.top;
	else if (itemRect.bottom > clientRect.bottom)
		delta = itemRect.bottom - clientRect.bottom;
	
	if (0 != delta)
	{		
		RemoveHighlight();
		ScrollVertDelta(delta);
		UpdateHighlight();
	}
}

void GroupedListView::Select(GLItem *item)
{
	if (NULL == item) return;

	GLItem *prevFocused = focused;
	focused = item;

	if (NULL != prevFocused) Invalidate(prevFocused);
	Invalidate(focused);
}

void GroupedListView::SelectPrevious(GLItem *item, BOOL ensureVisible)
{
	if (NULL == item)
	{
		if (NULL != root) item = root->Last();
		if (NULL == item)
			return;
	}
	else
	{
		GLGroup *parent = (GLGroup*)item->GetParent();
		if (NULL == parent)
			return;
		
		item = (item == parent->First()) ? 
			parent->Previous() : parent->PreviousChild(item);
		if (NULL == item)
			return;
	}
		
	GLItem *prevFocused = focused;
	focused = NULL;

	if (NULL != prevFocused)  Invalidate(prevFocused);
	

	while(NULL != item && GLGroup::FlagTypeGroup == item->GetType())
	{
		if (FALSE != ensureVisible)
			EnsureVisible(item);
		item = (0 == (GLGroup::FlagGroupCollapsed & item->GetValue())) ?
			((GLGroup*)item)->Last() : item = item->Previous();
	}

	if (NULL == item) item = prevFocused;
	if (NULL != item) 
	{
		Select(item);
		if (FALSE != ensureVisible)
			EnsureVisible(item);
	}
	
}

void GroupedListView::SelectNext(GLItem *item, BOOL ensureVisible)
{	
	item = (NULL == item) ? 
			((NULL != root) ? root->First() : NULL) : 
			item->Next();

	GLItem *prevFocused = focused;
	focused = NULL;

	if (NULL != prevFocused)  Invalidate(prevFocused);
	

	while(NULL != item && GLGroup::FlagTypeGroup == item->GetType())
	{
		if (FALSE != ensureVisible)
			EnsureVisible(((GLGroup*)item)->Last());
		item = (0 == (GLGroup::FlagGroupCollapsed & item->GetValue())) ?
			((GLGroup*)item)->First() : item = item->Next();
	}

	if (NULL == item) item = prevFocused;
	if (NULL != item) 
	{
		Select(item);
		if (FALSE != ensureVisible)
			EnsureVisible(item);
	}
}

void GroupedListView::PageUp()
{
	if (NULL == root) return;

	if (NULL != focused)
		EnsureVisible(focused);

	SCROLLINFO scrollInfo;
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_POS;
	if (!GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		scrollInfo.nPos = 0;

	RECT clientRect, itemRect;


	FINDITEMENUMPARAM param;
	param.style = style;
	param.foundItem = NULL;
	if (!GetClientRect(hwnd, &clientRect))
		return;

	param.testY = clientRect.top;
	CopyRect(&param.itemRect, &clientRect);
	param.itemRect.top -= scrollInfo.nPos;

	root->EnumerateChidlren(FindTopItem_EnumProc, (ULONG_PTR)&param);

	GLItem *item = param.foundItem;
	
	if (NULL != item && GLGroup::FlagTypeGroup == item->GetType())
		item = ((GLGroup*)item)->First();

	while(NULL != item && GLGroup::FlagTypeGroup == item->GetType())
	{
		item = (0 == (GLGroup::FlagGroupCollapsed & item->GetValue())) ?
			((GLGroup*)item)->First() : item = item->Next();
	}
	if (NULL == item) return;
	
	if (focused == item)
	{
		if (GetItemRect(item, &itemRect) && itemRect.bottom < clientRect.bottom)
		{
			RemoveHighlight();
			INT scrolled = ScrollVertDelta(itemRect.bottom - clientRect.bottom);
			UpdateHighlight();
			if (0 != scrolled)
				PageUp();
		}
	}
	else
	{
		Select(item);
	}
}
void GroupedListView::PageDown()
{
	if (NULL == root) return;

	if (NULL != focused)
		EnsureVisible(focused);

	SCROLLINFO scrollInfo;
	scrollInfo.cbSize = sizeof(SCROLLINFO);
	scrollInfo.fMask = SIF_POS;
	if (!GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
		scrollInfo.nPos = 0;

	RECT clientRect, itemRect;

	FINDITEMENUMPARAM param;
	param.style = style;
	param.foundItem = NULL;
	if (!GetClientRect(hwnd, &clientRect))
		return;

	param.testY = clientRect.bottom;
	CopyRect(&param.itemRect, &clientRect);
	param.itemRect.top -= scrollInfo.nPos;

	root->EnumerateChidlren(FindBottomItem_EnumProc, (ULONG_PTR)&param);

	GLItem *item = param.foundItem;
	
	if (NULL != item && GLGroup::FlagTypeGroup == item->GetType())
		item = item->Previous();

	while(NULL != item && GLGroup::FlagTypeGroup == item->GetType())
	{
		item = (0 == (GLGroup::FlagGroupCollapsed & item->GetValue())) ?
			((GLGroup*)item)->Last() : item = item->Previous();
	}
	if (NULL == item) return;
	
	if (focused == item)
	{
		if (GetItemRect(item, &itemRect) && itemRect.top > clientRect.top)
		{
			RemoveHighlight();
			INT scrolled = ScrollVertDelta(itemRect.top - clientRect.top);
			UpdateHighlight();
			if (0 != scrolled)
				PageDown();
		}
	}
	else
	{
		Select(item);
	}
}

void GroupedListView::KeyDown(UINT virtualKey, UINT keyFlags)
{
	BOOL navigateOk = FALSE;
		
	switch(virtualKey)
	{			
		case VK_END:
			continuousScroll = FALSE;
			ScrollVert(SB_BOTTOM, 0);
			ScrollVert(SB_ENDSCROLL, 0);
			SelectPrevious(NULL, TRUE);
			navigateOk = TRUE;
			break;
		case VK_UP:
		case VK_LEFT:
            SelectPrevious(focused, TRUE);
			navigateOk = TRUE;
			break;

		case VK_HOME:
			continuousScroll = FALSE;
			ScrollVert(SB_TOP, 0);
			ScrollVert(SB_ENDSCROLL, 0);
			SelectNext(NULL, TRUE);
			navigateOk = TRUE;
			break;
		case VK_DOWN:
		case VK_RIGHT:
			SelectNext(focused, TRUE);
			navigateOk = TRUE;
			break;

		case VK_NEXT:
			PageDown();
			navigateOk = TRUE;
			break;

		case VK_PRIOR:
			PageUp();
			navigateOk = TRUE;
			break;

		case VK_SPACE:
			if (NULL != focused && NULL == pressed)
			{	
				pressed = focused;
				pressed->KeyDown(this, virtualKey);
			}
			break;

	}

	if (navigateOk)
	{
		if (0 != (UISF_HIDEFOCUS & SendMessage(hwnd, WM_QUERYUISTATE, 0, 0L)))
			SendMessage(hwnd, WM_CHANGEUISTATE, MAKELONG(UIS_CLEAR, UISF_HIDEFOCUS), 0L);
		
		UpdateWindow(hwnd);
	}
}

void GroupedListView::UpdateUiState(UINT actionId, UINT stateId)
{
	if (NULL != focused)
	{
		Invalidate(focused);
	}
}
void GroupedListView::KeyUp(UINT virtualKey, UINT keyFlags)
{
	continuousScroll = FALSE;

	switch(virtualKey)
	{
		case VK_SPACE:
			if (0 != (0x8000 & GetAsyncKeyState(VK_LBUTTON)) ||
				0 != (0x8000 & GetAsyncKeyState(VK_RBUTTON)))
			{
				focused = NULL;
			}

			if (NULL != focused && NULL != pressed)
			{
				if (NULL != pressed) 
				{		
					pressed->KeyUp(this, virtualKey);
					if (focused == pressed)
						pressed->KeyPressed(this, virtualKey);
				}
				pressed = NULL;
			}
			break;
	}
}

BOOL GroupedListView::UpdateStyle(UINT updateFlags)
{	
	if (NULL == style) return FALSE;
	if (0 == updateFlags) return TRUE;

	style->Update(hwnd, updateFlags);

	if (0 != (GLStyle::updateFlagColors & updateFlags))
	{
		if (NULL != listTheme)
			UxCloseThemeData(listTheme);
		listTheme = (UxIsAppThemed()) ? UxOpenThemeData(hwnd, L"Listbox") : NULL;
	
		if (NULL != backgroundBrush)
		{
			DeleteObject(backgroundBrush);
			backgroundBrush = NULL;
		}

		SetWindowPos(hwnd, NULL, 0, 0, 0, 0, 
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_FRAMECHANGED);
	}
	
	if (NULL != root)
	{
		STYLECHNAGEDENUMPARAM param;
		param.updateFlags = updateFlags;
		param.view = this;
		root->EnumerateChidlren(StyleChanged_EnumProc, (ULONG_PTR)&param);
	}
	
	UpdateScrollInfo();

	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_NOCHILDREN | RDW_UPDATENOW | RDW_FRAME);
	
	return TRUE;
}


void GroupedListView::FocusChanged(BOOL gainFocus)
{
	if (NULL == focused && gainFocus)
		SelectNext(NULL, FALSE);

	if (NULL != focused)
		Invalidate(focused);
}


void GroupedListView::SetFont(HFONT windowFont)
{
	hFont = windowFont;
	UpdateStyle(GLStyle::updateFlagFonts);
}

HFONT GroupedListView::GetFont()
{
	return hFont;
}

void *GroupedListView::GetTheme()
{
	return listTheme;
}
void GroupedListView::ItemStyleChanged(GLItem *item, UINT styleOld, UINT styleNew)
{
	NMGLITEM nmitem;
	nmitem.hdr.code = GLVN_ITEMSTYLECHANGED;
	nmitem.hdr.hwndFrom = hwnd;
	nmitem.hdr.idFrom = GetDlgCtrlID(hwnd);
	nmitem.item = item;
	nmitem.styleNew = styleNew;
	nmitem.styleOld = styleOld;

	HWND hParent = GetParent(hwnd);
	if (NULL != hParent)
		SendMessage(hParent, WM_NOTIFY, (WPARAM)nmitem.hdr.idFrom, (LPARAM)&nmitem);
		

}
static LRESULT GroupedListView_OnCreate(HWND hwnd, CREATESTRUCT *createParam)
{
	GroupedListView *view;

	view = new GroupedListView(hwnd);
	if (NULL == view)
	{
		DestroyWindow(hwnd);
		return -1;
	}
	
	SetLastError(ERROR_SUCCESS);
	if (!SetWindowLongPtr(hwnd, 0, (LONGX86)(LONG_PTR)view) && 
		ERROR_SUCCESS != GetLastError())
	{
		DestroyWindow(hwnd);
		return -1;
	}
	
	return 0;

}

static void GroupedListView_OnDestroy(HWND hwnd)
{
	GroupedListView *view = GetGroupedView(hwnd);
	SetWindowLongPtr(hwnd, 0, 0L);
	if (NULL == view) return;
	delete(view);
}

static void GroupedListView_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	GroupedListView *view = GetGroupedView(hwnd);

	if (BeginPaint(hwnd, &ps))
	{
		if (ps.rcPaint.left != ps.rcPaint.right && NULL != view) 
			view->Paint(&ps);
		
		EndPaint(hwnd, &ps);
	}

}

static void GroupedListView_OnSetFocus(HWND hwnd, HWND lostFocus)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->FocusChanged(TRUE);

}
static void GroupedListView_OnKillFocus(HWND hwnd, HWND receiveFocus)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->FocusChanged(FALSE);
	
}
static void GroupedListView_OnSetFont(HWND hwnd, HFONT newFont, BOOL redrawFlag)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->SetFont(newFont);	
}

static LRESULT GroupedListView_OnGetFont(HWND hwnd)
{
	GroupedListView *view = GetGroupedView(hwnd);
	return (NULL != view) ? (LRESULT)view->GetFont() : NULL;
}

static void GroupedListView_OnPrintClient(HWND hwnd, HDC hdc, UINT printOptions)
{
	PAINTSTRUCT ps;
	ZeroMemory(&ps, sizeof(PAINTSTRUCT));
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
	ps.fErase = (0 != (PRF_ERASEBKGND & printOptions));

	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->Paint(&ps);

}

static void GroupedListView_OnWindowPosChanged(HWND hwnd, WINDOWPOS *pwp)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->UpdateScrollInfo();

}
static void GroupedListView_OnVertScroll(HWND hwnd, UINT scrollCode, UINT trackPos)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->ScrollVert(scrollCode, trackPos);

}
static LRESULT GroupedListView_OnSetRoot(HWND hwnd, GLRoot *groupRoot)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL == view)
		return FALSE;
	
	view->SetRoot(groupRoot);
	return TRUE;
}

static LRESULT GroupedListView_OnSetBitmap(HWND hwnd, HINSTANCE hInstance, LPCTSTR pszResource)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL == view)
		return FALSE;

	return view->SetBitmap(hInstance, pszResource);
}

static void GroupedListView_OnMouseWheel(HWND hwnd, INT zDelta, UINT mouseFlags, POINTS pts)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
	{
		POINT pt;
		POINTSTOPOINT(pt, pts);
		MapWindowPoints(HWND_DESKTOP, hwnd, &pt, 1);
		view->MouseWheel(zDelta, mouseFlags, pt);
	}
}

static void GroupedListView_OnMouseMove(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
	{
		POINT pt;
		POINTSTOPOINT(pt, pts);
		view->MouseMove(mouseFlags, pt);
	}
}

static void GroupedListView_OnMouseLeave(HWND hwnd)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->MouseLeave();
}

static void GroupedListView_OnLButtonDown(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	if (IsWindowEnabled(hwnd) && hwnd != GetFocus())
		SetFocus(hwnd);

	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
	{
		POINT pt;
		POINTSTOPOINT(pt, pts);
		view->LeftButtonDown(mouseFlags, pt);
	}


}

static void GroupedListView_OnRButtonDown(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	if (IsWindowEnabled(hwnd) && hwnd != GetFocus())
		SetFocus(hwnd);
}

static void GroupedListView_OnMButtonDown(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	if (IsWindowEnabled(hwnd) && hwnd != GetFocus())
		SetFocus(hwnd);
}

static void GroupedListView_OnLButtonUp(HWND hwnd, UINT mouseFlags, POINTS pts)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
	{
		POINT pt;
		POINTSTOPOINT(pt, pts);
		view->LeftButtonUp(mouseFlags, pt);
	}
}

static void GroupedListView_OnKeyDown(HWND hwnd, UINT virtualKey, UINT keyFlags)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->KeyDown(virtualKey, keyFlags);
}

static void GroupedListView_OnKeyUp(HWND hwnd, UINT virtualKey, UINT keyFlags)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->KeyUp(virtualKey, keyFlags);
}
static LRESULT GroupedListView_OnGetDlgCode(HWND hwnd, UINT virtualKey, MSG *pMsg)
{	
	if (NULL == pMsg)
		return DLGC_WANTALLKEYS;
	
	if (VK_TAB != virtualKey)
		return TRUE;
	
	return FALSE;
}

static void GroupedListView_OnThemeChanged(HWND hwnd)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->UpdateStyle(GLStyle::updateFlagThemes);	

}
static void GroupedListView_OnSysColorChanged(HWND hwnd)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->UpdateStyle(GLStyle::updateFlagColors);	
}


static LRESULT GroupListView_OnNcCalcSize(HWND hwnd, BOOL bValidRects, NCCALCSIZE_PARAMS *pncp)
{
		
	GroupedListView *view = GetGroupedView(hwnd);
	void *theme = (NULL != view) ? view->GetTheme() : NULL;
	
	if (0 && NULL != theme && 
		(0 != (WS_BORDER & GetWindowLongPtr(hwnd, GWL_STYLE)) || 
			0 != (WS_EX_CLIENTEDGE & GetWindowLongPtr(hwnd, GWL_EXSTYLE))))
	{
		RECT *boundsRect = &pncp->rgrc[0];
		INT borderSize = 0;
		if (SUCCEEDED(UxGetThemeInt(theme, EP_EDITTEXT, ETS_NORMAL, TMT_BORDERSIZE, &borderSize)))
		{
			InflateRect(boundsRect, -borderSize, -borderSize);
			if (!bValidRects) return 0;
			return 0;
		}
		
	}

	return DefWindowProc(hwnd, WM_NCCALCSIZE, (WPARAM)bValidRects, (LPARAM)pncp);
}

static void GroupListView_OnNcPaint(HWND hwnd, HRGN rgnUpdate)
{
	GroupedListView *view = GetGroupedView(hwnd);
	void *theme = (NULL != view) ? view->GetTheme() : NULL;
	
	if (NULL != theme && 
		(0 != (WS_BORDER & GetWindowLongPtr(hwnd, GWL_STYLE)) || 
			0 != (WS_EX_CLIENTEDGE & GetWindowLongPtr(hwnd, GWL_EXSTYLE))))
	{
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);

	
		INT borderCX = GetSystemMetrics(SM_CXEDGE);
		INT borderCY = GetSystemMetrics(SM_CYEDGE);
		
		UINT flags = DCX_PARENTCLIP | DCX_CACHE | DCX_WINDOW  | DCX_INTERSECTUPDATE | DCX_VALIDATE;
		HDC hdc = GetDCEx(hwnd, ((HRGN)NULLREGION != rgnUpdate) ? rgnUpdate : NULL, flags);
		
		if (NULL != hdc)
		{	
			RECT frameRect;
			SetRect(&frameRect, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
			ExcludeClipRect(hdc, frameRect.left + borderCX, frameRect.top + borderCY, 
				frameRect.right - borderCX, frameRect.bottom - borderCY);

			if (SUCCEEDED(UxDrawThemeBackground(theme, hdc, LVP_LISTITEM, 0, &frameRect, NULL)))
			{
				
			}
			ReleaseDC(hwnd, hdc);
		

			HRGN rgn = CreateRectRgn(windowRect.left + borderCX, windowRect.top + borderCY, 
				windowRect.right - borderCX, windowRect.bottom - borderCY);

			if (NULL != rgn)
			{
				if (((HRGN)NULLREGION != rgnUpdate))
					CombineRgn(rgn, rgn, rgnUpdate, RGN_AND);

				DefWindowProc(hwnd, WM_NCPAINT, (WPARAM)rgn, 0L);
				DeleteObject(rgn);
			}
			
			
				

			return;
		}
	}

	
	DefWindowProc(hwnd, WM_NCPAINT, (WPARAM)rgnUpdate, 0L);
}
static void GroupedListView_OnEnable(HWND hwnd, BOOL fEnabled)
{
	InvalidateRect(hwnd, NULL, TRUE);
}
static void GroupListView_OnUpdateUiState(HWND hwnd, UINT actionId, UINT stateId)
{
	GroupedListView *view = GetGroupedView(hwnd);
	if (NULL != view)
		view->UpdateUiState(actionId, stateId);

}
static LRESULT WINAPI GroupedListView_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:				return GroupedListView_OnCreate(hwnd, (CREATESTRUCT*)lParam);
		case WM_DESTROY:				GroupedListView_OnDestroy(hwnd); return 0;
		case WM_PAINT:				GroupedListView_OnPaint(hwnd); return 0;
		case WM_SETFOCUS:			GroupedListView_OnSetFocus(hwnd, (HWND)wParam); return 0;
		case WM_KILLFOCUS:			GroupedListView_OnKillFocus(hwnd, (HWND)wParam); return 0;
		case WM_SETFONT:				GroupedListView_OnSetFont(hwnd, (HFONT)wParam, (BOOL)LOWORD(lParam)); return 0;
		case WM_GETFONT:				return GroupedListView_OnGetFont(hwnd); 
		case WM_PRINTCLIENT:			GroupedListView_OnPrintClient(hwnd, (HDC)wParam, (UINT)lParam); return 0;
		case WM_WINDOWPOSCHANGED:	GroupedListView_OnWindowPosChanged(hwnd, (WINDOWPOS*)lParam); return 0;
		case WM_ERASEBKGND:			return 0;
		case WM_VSCROLL:				GroupedListView_OnVertScroll(hwnd, LOWORD(wParam), HIWORD(wParam)); return 0;
		case WM_MOUSEWHEEL:			GroupedListView_OnMouseWheel(hwnd, ((SHORT)HIWORD(wParam)), LOWORD(wParam), MAKEPOINTS(lParam)); return 0;
		case WM_MOUSEMOVE:			GroupedListView_OnMouseMove(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MOUSELEAVE:			GroupedListView_OnMouseLeave(hwnd); return 0;
		case WM_LBUTTONDOWN:			GroupedListView_OnLButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_RBUTTONDOWN:			GroupedListView_OnRButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_MBUTTONDOWN:			GroupedListView_OnMButtonDown(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_LBUTTONUP:			GroupedListView_OnLButtonUp(hwnd, (UINT)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_KEYDOWN:				GroupedListView_OnKeyDown(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_KEYUP:				GroupedListView_OnKeyUp(hwnd, (UINT)wParam, (UINT)lParam); return 0;
		case WM_GETDLGCODE:			return GroupedListView_OnGetDlgCode(hwnd, (UINT)wParam, (MSG*)lParam);
		case WM_ENABLE:				GroupedListView_OnEnable(hwnd, (BOOL)wParam); return 0;

		case 0x031A/*WM_THEMECHANGED*/:	GroupedListView_OnThemeChanged(hwnd); break;
		case WM_SYSCOLORCHANGE:			GroupedListView_OnSysColorChanged(hwnd); break;

		case WM_NCCALCSIZE:			return GroupListView_OnNcCalcSize(hwnd, (BOOL)wParam, (NCCALCSIZE_PARAMS*)lParam);
		case WM_NCPAINT:				GroupListView_OnNcPaint(hwnd, (HRGN)wParam); return 0;
		case WM_UPDATEUISTATE:		GroupListView_OnUpdateUiState(hwnd, LOWORD(wParam), HIWORD(wParam)); break;
		case GLVM_SETROOT:			return GroupedListView_OnSetRoot(hwnd, (GLRoot*)lParam);
		case GLVM_SETBITMAP:			return GroupedListView_OnSetBitmap(hwnd, (HINSTANCE)wParam,(LPCTSTR)lParam);

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}