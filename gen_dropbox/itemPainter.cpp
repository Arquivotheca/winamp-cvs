#include "./main.h"
#include "./itemPainter.h"
#include "./dropWindowInternal.h"
#include "./resource.h"
#include "./guiObjects.h"
#include <shlwapi.h>
#include <commctrl.h>
#include <strsafe.h>

static INT GetFieldText(IFileInfo *pFileInfo, UINT columnId, LPTSTR pszBuffer, INT cchBufferMax)
{		
	const LISTCOLUMN *pColumn = (columnId < COLUMN_LAST) ? &szRegisteredColumns[columnId] : NULL;
	if (NULL != pColumn && NULL != pColumn->fnFormatter)
	{
		LPCTSTR pText = pColumn->fnFormatter(pFileInfo, pszBuffer, cchBufferMax);
		if (NULL == pText) 
			return 0;
		else 
			return lstrlen(pText);
	}
	return 0;
}
ItemPainter::ItemPainter() : indexWidth(0), pImageList(NULL)
{
	style = (PAINTER_RIGHTCOLUMNMASK & COLUMN_TRACKLENGTH) | PAINTER_DRAWINDEX | PAINTER_DRAWTYPEICON | PAINTER_OPAQUE;
}

ItemPainter::~ItemPainter()
{
	if (NULL != pImageList)
		pImageList->Release();
}

void ItemPainter::SetStyle(DWORD newStyle, DWORD styleMask)
{
	style = ((style & ~styleMask) | (newStyle & styleMask));
}

DWORD ItemPainter::GetStyle(DWORD styleMask)
{
	return (style & styleMask);
}

BOOL ItemPainter::SetColumnWidth(INT paintColumn, INT width)
{
	switch(paintColumn)
	{
		case PAINTCOL_INDEX:
			indexWidth = width;
			return TRUE;
	}
	return FALSE;
}		

BOOL ItemPainter::SetColumnFont(INT paintColumn, HFONT hFont)
{
	return FALSE;
}

INT ItemPainter::FitTextInplace(HDC hdc, LPTSTR pszText, INT cchText, LONG width)
{
	INT textMax;
	SIZE size;
	if (GetTextExtentExPoint(hdc, pszText, cchText, width, &textMax, NULL, &size) &&
		textMax < cchText)
		
	{		
		const TCHAR szEllipsis[] = TEXT("...");
		
		INT offset, cchEllipsis;
		SIZE sizeEllipsis, sizeTest;

		cchEllipsis = ARRAYSIZE(szEllipsis) - 1;
		GetTextExtentPoint32(hdc, szEllipsis, cchEllipsis, &sizeEllipsis);
		GetTextExtentPoint32(hdc, pszText, textMax, &sizeTest);
			
		offset = 0;
		INT widthTotal = sizeTest.cx + sizeEllipsis.cx;

		for(; widthTotal >= width && offset <= textMax; offset++)
		{
			GetTextExtentPoint32(hdc, (pszText + (textMax - offset)), 1, &sizeTest);
			widthTotal -= sizeTest.cx;
		}
		
		if (offset > textMax)
		{
			offset = textMax;
			while (cchEllipsis--)
			{
				GetTextExtentPoint32(hdc, szEllipsis, cchEllipsis, &sizeEllipsis);
				if (sizeEllipsis.cx <= width)
					break;
			}
		}

		
		LPTSTR end = (pszText + (textMax - offset));
		if (cchEllipsis > 0)
		{
			CopyMemory(end, szEllipsis, sizeof(TCHAR) * cchEllipsis); 
			end += cchEllipsis;
		}
		*end = TEXT('\0');
		cchText = textMax - offset + cchEllipsis;
	}

	return cchText;
}

void ItemPainter::Paint(HDC hdc, IFileInfo *pItem, UINT iItem, UINT itemState, RECT *prcItem)
{	
	
	INT len;
	RECT line, element;
	SIZE size;
	size_t remaining;

	COLORREF rgbText, rgbBk, rgbDarkerText;

	CopyRect(&line, prcItem);
	InflateRect(&line, -1, 0);
	line.bottom -= 1;
	line.right -= 2;

	if (PAINTER_OPAQUE & style)
		ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, prcItem, NULL, 0, 0);

	rgbText = GetTextColor(hdc);
	rgbBk = GetBkColor(hdc);
	
	LONG alpha = 140;

	rgbDarkerText = BlendColors(rgbText, rgbBk, alpha);
	UINT alignStyle = GetTextAlign(hdc);
	if (PAINTER_DRAWINDEX & style)
	{	
		StringCchPrintfEx(szBuffer, ARRAYSIZE(szBuffer), NULL, &remaining, STRSAFE_NULL_ON_FAILURE, TEXT("%d."), iItem);
		len = (INT)(ARRAYSIZE(szBuffer) - remaining);
		
		if (GetTextExtentPoint32(hdc, szBuffer, len, &size))
		{
			if (indexWidth < size.cx) 
					indexWidth = size.cx;
						
			SetRect(&element, line.left, line.top, line.left + indexWidth, line.bottom);
			if ((line.right - element.right) > 180)
			{
				SetTextAlign(hdc, TA_BOTTOM | TA_RIGHT);
				SetTextColor(hdc, rgbDarkerText);
				TextOut(hdc,element.right, element.bottom, szBuffer, len);
				SetTextColor(hdc, rgbText);
			//	ExtTextOut(hdc, element.right, element.bottom, ETO_CLIPPED, &element, szBuffer, len, NULL);
				line.left = element.right + 1;
			}
		}
	}

	if (PAINTER_DRAWTYPEICON & style)
	{
		DWORD itemType;

		if (SUCCEEDED(pItem->GetType(&itemType)))
		{			
			if (NULL == pImageList)
			{
				pImageList = new FileImageList(plugin.hDllInstance, MAKEINTRESOURCE(IDR_SMALLFILETYPES_IMAGE), 4);
				if (NULL != pImageList)
					pImageList->Load();
			}
				
			INT cx, cy;
			if (NULL != pImageList && pImageList->GetImageSize(&cx, &cy))
			{	
				SetRect(&element, line.left, line.top, line.left + cx, line.bottom + 1);
				if ((line.right - element.right) > 140)
				{
					if (cy > (element.bottom - element.top ))
					{
						float k = ((float)(element.bottom - element.top)) / cy;
						cy = element.bottom - element.top;
						cx = (INT)(k * cx);
					}

					element.right = element.left + cx;
					element.top = element.bottom - cy; //((element.bottom  - element.top) -  cy) / 2;
					//element.bottom  = element.top + cy;
	
					pImageList->Draw(hdc, itemType, element.left, element.top, 
									element.right - element.left, element.bottom - element.top, 
									GetBkColor(hdc), GetTextColor(hdc));
					line.left = element.right + 1;
				}
			}
		}
	}

	INT rIndex = ((INT)(SHORT)(PAINTER_RIGHTCOLUMNMASK & style));
	if (rIndex >= 0 && rIndex < COLUMN_LAST)
	{
		len = GetFieldText(pItem, rIndex, szBuffer, ARRAYSIZE(szBuffer));
		if (len > 0)
		{
			if (GetTextExtentPoint32(hdc, szBuffer, len, &size))
			{
				SetRect(&element, line.right - size.cx, line.top, line.right, line.bottom);
				if ((element.left - line.left) > 120)
				{
					SetTextAlign(hdc, TA_BOTTOM | TA_RIGHT);
					SetTextColor(hdc, rgbDarkerText);
					TextOut(hdc,element.right, element.bottom, szBuffer, len);
					SetTextColor(hdc, rgbText);
					line.right = element.left - 2;
				}
			}
		}
	}
	 
	len = GetFieldText(pItem, COLUMN_FORMATTEDTITLE, szBuffer, ARRAYSIZE(szBuffer));

	if (len > 0)
	{
		SetTextAlign(hdc, TA_LEFT | TA_BOTTOM);
		len = FitTextInplace(hdc, szBuffer, len, line.right - line.left);
		TextOut(hdc, line.left, line.bottom, szBuffer, len);
		//DrawText(hdc, szBuffer, len, &line, /*DT_END_ELLIPSIS |*/ DT_LEFT |/* DT_BOTTOM |*/ DT_NOPREFIX | DT_SINGLELINE);
	}
	SetTextAlign(hdc, alignStyle);
}

BOOL ItemPainter::MeasureString(HWND hwnd, LPCTSTR pszText, INT cchText, SIZE *pSize)
{
	BOOL br = FALSE;
	HDC hdc;
	hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (NULL == hdc)
		return FALSE;

	HFONT hf, hfo;
	hf =(HFONT)SNDMSG(hwnd, WM_GETFONT, 0, 0L);
	if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hfo = (NULL != hf) ? (HFONT)SelectObject(hdc, hf) : NULL;

	if (cchText < 0) 
		cchText = lstrlen(pszText);

	br = GetTextExtentPoint32(hdc, pszText, cchText, pSize);

	if (NULL != hfo) SelectObject(hdc, hfo);
	ReleaseDC(hwnd, hdc);

	return br;
}


BOOL ItemPainter::MeasureLineHeight(HWND hwnd, LONG *pnHeight)
{
	HDC hdc;
	hdc = GetDCEx(hwnd, NULL, DCX_CACHE);
	if (NULL == hdc) return FALSE;

	BOOL bSuccess =  FALSE;
	HFONT hf, hfo;
	hf =(HFONT)SNDMSG(hwnd, WM_GETFONT, 0, 0L);
	if (NULL == hf) hf = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	hfo = (NULL != hf) ? (HFONT)SelectObject(hdc, hf) : NULL;

	TEXTMETRIC tm;
	if (GetTextMetrics(hdc, &tm))
	{
		*pnHeight = tm.tmHeight + 1;
		bSuccess = TRUE;
	}
	
	if (NULL != hfo) SelectObject(hdc, hfo);
	ReleaseDC(hwnd, hdc);
	
	return bSuccess;
}