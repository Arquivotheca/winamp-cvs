#ifndef NULLSOFT_DROPBOX_PLUGIN_ITEM_PAINTER_HEADER
#define NULLSOFT_DROPBOX_PLUGIN_ITEM_PAINTER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "./imageList.h"


typedef enum __PAINTERSTYLES
{
	PAINTER_RIGHTCOLUMNMASK = 0x0000FFFF,
	PAINTER_DRAWINDEX = 0x00010000,
	PAINTER_DRAWTYPEICON = 0x00020000,
	PAINTER_OPAQUE = 0x00040000,
	
} PAINTERSTYLES;

typedef enum __PAINTCOLUMN
{
	PAINTCOL_INDEX = 0,	// 
	PAINTCOL_TITLE,	// atf
	PAINTCOL_EXTRA,	//	track length, size, 
	PAINTCOL_LAST,
} PAINTCOLUMN;

class ItemPainter
{
public:
	ItemPainter();
	~ItemPainter();
public:
	void Paint(HDC hdc, IFileInfo *pItem, UINT iItem, UINT itemState, RECT *prcItem);
	BOOL SetColumnWidth(INT paintColumn, INT width);  // right now only index column supports this call
	BOOL SetColumnFont(INT paintColumn, HFONT hFont);

public:
	static BOOL MeasureString(HWND hwnd, LPCTSTR pszText, INT cchText, SIZE *pSize);
	static BOOL MeasureLineHeight(HWND hwnd, LONG *pnHeight);
	
	void SetStyle(DWORD newStyle, DWORD styleMask);
	DWORD GetStyle(DWORD styleMask);

protected:
	INT FitTextInplace(HDC hdc, LPTSTR pszText, INT cchText, LONG width);

protected:
	DWORD		style;
	INT			indexWidth;
	HFONT		szFonts[PAINTCOL_LAST];
	COLORREF	szColors[PAINTCOL_LAST];
	TCHAR		szBuffer[1024];
	FileImageList *pImageList;
};

#endif // NULLSOFT_DROPBOX_PLUGIN_ITEM_PAINTER_HEADER

