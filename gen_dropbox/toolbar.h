#ifndef NULLOSFT_DROPBOX_PLUGIN_TOOLBAR_HEADER
#define NULLOSFT_DROPBOX_PLUGIN_TOOLBAR_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./toolbarItem.h"
#include "../nu/ptrlist.h"

#include <commctrl.h>

class ToolbarCallback;

typedef struct __ToolbarItemRect
{
	INT index;				// can be -1 (in case of chevron)
	ToolbarItem *pItem;
	RECT rcItem;	
} ToolbarItemRect; // do not modify this struct between calls to GetFirstItemRect() / GetNextItemRect()

class Toolbar
{
public:
	typedef enum
	{
		FlagVisible = 0x0001,
		FlagDisabled = 0x0002,
		FlagImagelistShared = 0x0004,
	} ToolbarFlags;

	typedef enum
	{
		MouseButtonLeft = 0,
		MouseButtonRight = 1,
		MouseButtonMiddle = 2,
		MouseButtonX = 3,
	} MouseButton;

public: 
	Toolbar(INT barHeight);
	~Toolbar();

public:
	INT GetHeight() { return height; }
	
	INT GetSpacing() { return itemSpacing; }
	void SetSpacing(INT newSpacing) { itemSpacing = newSpacing; }
	
	INT GetWidth();
	BOOL GetBounds(RECT *prc);
	void SetBoundsIndirect(const RECT *prc);
	void SetBounds(INT left, INT top, INT right, INT bottom);

	UINT GetFlags() { return flags; }
	void SetFlags(UINT newFlags, UINT flagsMask);

	BOOL Draw(HDC hdc, const RECT *prcUpdate);

	void SetTextColor(COLORREF rgb) { rgbText = rgb; }
	COLORREF GetTextColor() { return rgbText; }
	void SetTextBkColor(COLORREF rgb) { rgbTextBk = rgb; }
	COLORREF GetTextBkColor() { return rgbTextBk; }
	
	void SetBkBrush(HBRUSH brush) { brushBk = brush; } // dosn't create copy
	HBRUSH GetBkBrush() { return brushBk; }

	void SetTextFont(HFONT hFont) { fontText = hFont; } // dosn't create copy
	HFONT GetTextFont() { return fontText; } // dosn't create copy
	
	INT InsertSpacer(INT insertPos);
	INT InsertFlexSpacer(INT insertPos);
	INT InsertSeparator(INT insertPos);
	INT InsertButton(INT insertPos, UINT commandId, INT imageId, LPCTSTR pszTitle, LPCTSTR pszDescription, UINT buttonFlags);

	void MouseMove(POINT pt, UINT mouseFlags);
	void MouseLeave();
	BOOL ButtonDown(UINT mouseButton, POINT pt, UINT mouseFlags);
	BOOL ButtonUp(UINT mouseButton, POINT pt, UINT mouseFlags);
	
	void RegisterCallback(ToolbarCallback *callbackInstance) { callback = callbackInstance; }
	
	BOOL HitTest(POINT pt, ToolbarItem **item, RECT *prcItem);
	BOOL GetItemRect(ToolbarItem *item, RECT *prc);
	BOOL GetFirstItemRect(ToolbarItemRect *pir);
	BOOL GetNextItemRect(ToolbarItemRect *pir);

	void ShowTip(LPCTSTR pszText, const RECT *prcBounds);

	INT DisplayButtonsMenu(ToolbarItem *item); 

	BOOL RelayMeasureItem(MEASUREITEMSTRUCT *pmis);
	BOOL RelayDrawItem(DRAWITEMSTRUCT *pdis);

	HIMAGELIST GetImageList() { return imageList; }
	void SetImageList(HIMAGELIST newImageList);

	BOOL Invalidate();

	void FireCommand(UINT commandId);
	INT TrackPopupMenuEx(ToolbarItem *item, UINT menuId);
	HMENU GetMenuCopy(UINT menuId);

protected:
	INT InsertItem(INT insertPos, ToolbarItem *toolbarItem);
	BOOL SetItemStyle(ToolbarItem *pItem, UINT style, BOOL bSet); // return TRUE if style changed
	
private:
	BOOL ToolbarItemRect_InternalRead(ToolbarItemRect *ptir);
protected:
	typedef nu::PtrList<ToolbarItem> ITEMLIST;

protected:
	ITEMLIST	 itemList;
	INT height;
	INT itemSpacing;
	RECT boundsRect;
	UINT flags;
	ToolbarItem *pressedItem;
	HIMAGELIST imageList;
	
	HFONT fontText;
	HBRUSH brushBk;
	COLORREF rgbText;
	COLORREF rgbTextBk;

	HMENU activeMenu;

	ToolbarChevron chevron;
	ToolbarCallback *callback;
};

#endif // NULLOSFT_DROPBOX_PLUGIN_TOOLBAR_HEADER