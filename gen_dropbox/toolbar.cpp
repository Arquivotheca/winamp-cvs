#include "main.h"
#include "./toolbar.h"
#include "./toolbarCallback.h"
#include "../nu/menuHelpers.h"



#include <strsafe.h>

#define DEFAULT_ITEMSPACING		0

Toolbar::Toolbar(INT barHeight) :
	height(barHeight), itemSpacing(DEFAULT_ITEMSPACING), flags(FlagVisible), 
	callback(NULL), pressedItem(NULL), activeMenu(NULL), imageList(NULL)
{
	SetBoundsIndirect(NULL);

	fontText = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	brushBk = GetSysColorBrush(COLOR_ACTIVECAPTION);
	rgbText = GetSysColor(COLOR_BTNTEXT);
	rgbTextBk = GetSysColor(COLOR_BTNFACE);
}

Toolbar::~Toolbar()
{
	size_t index = itemList.size();		
	while(index--)
		delete(itemList[index]);

	if (NULL != callback)
		callback->OnDestroy(this);

	if (NULL != imageList && 0 == (FlagImagelistShared & flags))
		ImageList_Destroy(imageList);
}

INT Toolbar::GetWidth()
{	
	size_t index = itemList.size();
	if (0 == index)
		return 0;
	
	INT width = (INT)((index - 1) * itemSpacing);
	while(index--)
	{
		width += itemList[index]->GetWidth();
		if (0 != (ToolbarItem::FlagGrouped & itemList[index]->GetFlags()) && 0 != index)
			width -= itemSpacing;
	}
	
	return width;
}

void Toolbar::SetFlags(UINT newFlags, UINT flagsMask)
{	
	flags &= ~flagsMask;
	flags |= (newFlags & flagsMask);
}

BOOL Toolbar::GetBounds(RECT *prc)
{
	if (NULL == prc)
		return FALSE;
	return CopyRect(prc, &boundsRect);
}

void Toolbar::SetBoundsIndirect(const RECT *prc)
{
	if (NULL == prc)
		SetRectEmpty(&boundsRect);
	else
		CopyRect(&boundsRect, prc);
}

void Toolbar::SetBounds(INT left, INT top, INT right, INT bottom)
{
	SetRect(&boundsRect, left, top, right, bottom);
}

INT Toolbar::InsertSpacer(INT insertPos)
{
	ToolbarSpacer *item = new ToolbarSpacer(this, FALSE);
	if (NULL == item)
		return -1;

	INT result = InsertItem(insertPos, item);
	if (-1 == result)
		delete(item);

	return result;
}

INT Toolbar::InsertFlexSpacer(INT insertPos)
{
	ToolbarSpacer *item = new ToolbarSpacer(this, TRUE);
	if (NULL == item)
		return -1;

	INT result = InsertItem(insertPos, item);
	if (-1 == result)
		delete(item);

	return result;
}

INT Toolbar::InsertSeparator(INT insertPos)
{
	ToolbarSeparator *item = new ToolbarSeparator();
	if (NULL == item)
		return -1;

	INT result = InsertItem(insertPos, item);
	if (-1 == result)
		delete(item);

	return result;
}
INT Toolbar::InsertButton(INT insertPos, UINT commandId, INT imageId, LPCTSTR pszTitle, LPCTSTR pszDescription, UINT buttonFlags)
{
	ToolbarButton *button = new ToolbarButton(this, commandId, imageId, pszTitle, pszDescription, buttonFlags);
	if (NULL == button)
		return -1;
		
	INT result = InsertItem(insertPos, button);
	if (-1 == result)
		delete(button);

	return result;
}

INT Toolbar::InsertItem(INT insertPos, ToolbarItem *toolbarItem)
{
	if (NULL == toolbarItem)
		return -1;

	if (insertPos < 0) insertPos = 0;

	if (((size_t)insertPos) >= itemList.size())
	{
		insertPos = (INT)itemList.size();
		itemList.push_back(toolbarItem);
	}
	else
	{
		itemList.insertBefore(insertPos, toolbarItem);
	}

	return insertPos;
}

BOOL Toolbar::ToolbarItemRect_InternalRead(ToolbarItemRect *ptir)
{
	ptir->pItem = itemList[ptir->index];

		
	ptir->rcItem.right = ptir->rcItem.left + ptir->pItem->GetWidth();

	INT count = (INT)itemList.size();
	LONG test = ptir->rcItem.right;

	if (0 != (ToolbarItem::FlagFlexSpacer & ptir->pItem->GetFlags()))
	{		
		for (INT t = (ptir->index + 1); t < count && test <= boundsRect.right; t++)
		{
			ToolbarItem *nextItem = itemList[t];
			test += nextItem->GetWidth();
			if (0 == (ToolbarItem::FlagGrouped & nextItem->GetFlags()) && t < (count - 1))
				test += itemSpacing;
		}
		if (test < boundsRect.right)
			ptir->rcItem.right += (boundsRect.right - test);

		test = ptir->rcItem.right;
	}

	
	LONG groupRight = test;


	for (INT t = (ptir->index + 1); t < count && test <= boundsRect.right; t++)
	{			
		ToolbarItem *nextItem = itemList[t];
		test += nextItem->GetWidth();
						
		if (0 == (ToolbarItem::FlagGrouped & nextItem->GetFlags()))
		{ 
			if (t < (count - 1)) test += itemSpacing;
			break;
		}
		else
			groupRight += nextItem->GetWidth();
	}

	if (test > boundsRect.right && 
		groupRight > (boundsRect.right - chevron.GetWidth()))
	{
		ptir->index = -1;
		ptir->rcItem.right = boundsRect.right;
		ptir->rcItem.left = ptir->rcItem.right - chevron.GetWidth();
		if (ptir->rcItem.left >= boundsRect.left)
		{
			ptir->pItem = &chevron;
			return TRUE;
		}
		ptir->pItem = NULL;
		SetRectEmpty(&ptir->rcItem);	
		return FALSE;
	}
	
	return TRUE;
}

BOOL Toolbar::GetFirstItemRect(ToolbarItemRect *ptir)
{	
    if (0 == itemList.size())
	{	
		ptir->index = -1;
		ptir->pItem = NULL;
		SetRectEmpty(&ptir->rcItem);
		return FALSE;
	}
	
	ptir->index = 0;
	CopyRect(&ptir->rcItem, &boundsRect);
	
	return ToolbarItemRect_InternalRead(ptir);
	
}

BOOL Toolbar::GetNextItemRect(ToolbarItemRect *ptir)
{
	if (-1 == ptir->index || NULL == ptir->pItem ||
		&chevron == ptir->pItem || ((size_t)(++ptir->index)) >= itemList.size()) 
	{
		ptir->index = -1;
		ptir->pItem = NULL;
		SetRectEmpty(&ptir->rcItem);
		return FALSE;
	}
	
	UINT itemFlags = itemList[ptir->index]->GetFlags();

	ptir->rcItem.left = ptir->rcItem.right;
	if (0 == (ToolbarItem::FlagGrouped & itemFlags))
		ptir->rcItem.left += itemSpacing;

	return ToolbarItemRect_InternalRead(ptir);	
}

BOOL Toolbar::Draw(HDC hdc, const RECT *prcUpdate)
{
	RECT rcPaint;
	
	if (0 == (FlagVisible & flags))
		return FALSE;

	if (NULL == prcUpdate)
		prcUpdate = &boundsRect;

	if (!IntersectRect(&rcPaint, &boundsRect, prcUpdate))
		return TRUE;


	ToolbarItemRect itemRect;
	if (GetFirstItemRect(&itemRect))
	{
		COLORREF rgbBkOld = ::SetBkColor(hdc, rgbTextBk);
		COLORREF rgbFgOld = ::SetTextColor(hdc, rgbText);
		HFONT fontTextOld = (HFONT)SelectObject(hdc, fontText);
		
		RECT rcFill;
		CopyRect(&rcFill, &rcPaint);
		
		do
		{
			if (rcFill.left < itemRect.rcItem.left)
			{
				rcFill.right = itemRect.rcItem.left;
				FillRect(hdc, &rcFill, brushBk);
			}
			rcFill.left = itemRect.rcItem.right;

			if (itemRect.rcItem.left > rcPaint.right)
				break;
			
			if (itemRect.rcItem.right > rcPaint.left)
			{
				BOOL fillMe = TRUE;

				if (NULL != itemRect.pItem)
				{
					UINT drawFlags = itemRect.pItem->GetFlags();
					if (0 != (FlagDisabled & flags))
						drawFlags = (drawFlags & ~(ToolbarItem::FlagHighlighted | ToolbarItem::FlagPressed)) | ToolbarItem::FlagDisabled;
					fillMe = !itemRect.pItem->Draw(this, hdc, &itemRect.rcItem, &rcPaint, drawFlags);
				}
				if (fillMe)
					FillRect(hdc, &itemRect.rcItem, brushBk);
				
			}
						
		}while(GetNextItemRect(&itemRect));

		if (rcFill.left < rcPaint.right)
		{
			rcFill.right = rcPaint.right;
			FillRect(hdc, &rcFill, brushBk);
		}
		
		if (rgbBkOld != rgbTextBk) ::SetBkColor(hdc, rgbBkOld);
		if (rgbFgOld != rgbText) ::SetTextColor(hdc, rgbFgOld);
		if (fontTextOld != fontText) SelectObject(hdc, fontTextOld);
	}
	else
	{
		FillRect(hdc, &rcPaint, brushBk);
	}
	return TRUE;	
}

BOOL Toolbar::HitTest(POINT pt, ToolbarItem **item, RECT *prcItem)
{
	if (!PtInRect(&boundsRect, pt))
	{
		if (NULL != item) *item = NULL;
		if (NULL != prcItem) SetRectEmpty(prcItem);
		return FALSE;
	}

	ToolbarItemRect itemRect;
	if (NULL != item &&
		GetFirstItemRect(&itemRect))
	{
		do
		{
			if (PtInRect(&itemRect.rcItem, pt))
			{
				*item = itemRect.pItem;
				if (NULL != prcItem) 
					CopyRect(prcItem, &itemRect.rcItem);
				return TRUE;
			}
		} while(GetNextItemRect(&itemRect));
	}

	if (NULL != item) *item = NULL;
	if (NULL != prcItem) SetRectEmpty(prcItem);

	return TRUE;
}

BOOL Toolbar::GetItemRect(ToolbarItem *item, RECT *prc)
{
	if (NULL == prc)
		return FALSE;
	
	ToolbarItemRect itemRect;
	if (NULL != item && 
		GetFirstItemRect(&itemRect))
	{
		do
		{
			if (itemRect.pItem == item)
			{
				CopyRect(prc, &itemRect.rcItem);
				return TRUE;
			}
		} while(GetNextItemRect(&itemRect));
	}

	SetRectEmpty(prc);
	return FALSE;
}

BOOL Toolbar::SetItemStyle(ToolbarItem *pItem, UINT style, BOOL bSet)
{
	UINT itemFlags, newFlags;

	itemFlags = pItem->GetFlags();
	newFlags = itemFlags;

	if (bSet)
		newFlags |= style;
	else
		newFlags &= ~style;

	if (newFlags == itemFlags)
		return FALSE;
	
	pItem->SetFlags(newFlags, style);

	if (NULL != callback)
	{
		RECT rcItem;
		if (GetItemRect(pItem, &rcItem))
			callback->Invalidate(&rcItem);
	}
	
	return TRUE;
}

void Toolbar::MouseMove(POINT pt, UINT mouseFlags)
{	
	ToolbarItem *item;
	RECT rcItem;

	if (FlagVisible != ((FlagDisabled | FlagVisible) & flags))
		return;

	HitTest(pt, &item, &rcItem);
		
	BOOL displayTip = FALSE;
	UINT itemStateMask = ToolbarItem::FlagHighlighted;
	if (NULL != item)
	{
		UINT itemFlags = item->GetFlags();
		if (0 != ((ToolbarItem::FlagDisabled | ToolbarItem::FlagSpacer | ToolbarItem::FlagFlexSpacer) & itemFlags))
			item = NULL;
		
		if (NULL != item && 
			0 == (ToolbarItem::FlagHighlighted & itemFlags))
		{
			if (NULL == callback || ToolbarCallback::Success != callback->TrackMouseLeave(this))
				item = NULL;
			else
				displayTip = TRUE;	
		}

		if (NULL != pressedItem)
		{
			if (item == pressedItem)
				itemStateMask |= ToolbarItem::FlagPressed;
			item = pressedItem;
		}
	}
	else
	{
		if (NULL != pressedItem && 0 != ((MK_LBUTTON | MK_RBUTTON) & mouseFlags))
			itemStateMask = ToolbarItem::FlagPressed;
	}

	SetItemStyle(&chevron, itemStateMask, (item == &chevron));
	
	size_t index = itemList.size();
	while(index--)
	{	
		SetItemStyle(itemList[index], itemStateMask, (item == itemList[index]));
	}

	if (displayTip && NULL != callback)
	{
		TCHAR szTip[MAX_PATH];
		if (NULL != item && item->GetTip(szTip, ARRAYSIZE(szTip)))
			callback->ShowTip(szTip, &rcItem);
	}
	
}

void Toolbar::MouseLeave()
{
	SetItemStyle(&chevron, ToolbarItem::FlagHighlighted, FALSE);
	size_t index = itemList.size();
	while(index--)
		SetItemStyle(itemList[index], ToolbarItem::FlagHighlighted | ToolbarItem::FlagPressed, FALSE);
}

BOOL Toolbar::ButtonDown(UINT mouseButton, POINT pt, UINT mouseFlags)
{
	ToolbarItem *item, *prevPressed;
	
	RECT rcItem;
	BOOL toolbarHit = FALSE;

	if (0 == (FlagVisible & flags))
		return FALSE;

	prevPressed = pressedItem;
    
	if (HitTest(pt, &item, &rcItem))
	{	
		toolbarHit = TRUE;
		if ( 0 != (FlagDisabled & flags))
			item = NULL;

		BOOL itemHit = FALSE;

		if (NULL != item &&
			0 == ((ToolbarItem::FlagDisabled | ToolbarItem::FlagSpacer | ToolbarItem::FlagFlexSpacer) & item->GetFlags()))
		{
            if (item->ButtonDown(this, mouseButton, pt, mouseFlags))
			{
				SetItemStyle(item, ToolbarItem::FlagHighlighted | ToolbarItem::FlagPressed, TRUE);
				pressedItem = item;
				itemHit = TRUE;
				if (NULL != callback)
					callback->Invalidate(&rcItem);
			}
		}

		if (FALSE == itemHit)
		{
			// do nothing 
		}
	}

	if (NULL != prevPressed && prevPressed != pressedItem)
	{
		SetItemStyle(prevPressed, ToolbarItem::FlagHighlighted | ToolbarItem::FlagPressed, FALSE);
		if (NULL != callback && GetItemRect(prevPressed, &rcItem))
			callback->Invalidate(&rcItem);
	}
	return toolbarHit;
}

BOOL Toolbar::ButtonUp(UINT mouseButton, POINT pt, UINT mouseFlags)
{
	ToolbarItem *item = NULL;
	RECT rcItem;
	BOOL handled = FALSE;

	if (0 == (FlagVisible & flags))
		return FALSE;

	if (NULL != callback)
		callback->CancelTrackMouseLeave(this);

	if (HitTest(pt, &item, &rcItem))
	{
		if ( 0 != (FlagDisabled & flags))
			item = NULL;

		BOOL toolbarHandler = TRUE;
		if (NULL != item)
		{
			UINT itemFlags = item->GetFlags();
			if (0 == ((ToolbarItem::FlagDisabled | ToolbarItem::FlagSpacer | ToolbarItem::FlagFlexSpacer) & itemFlags))
			{
				toolbarHandler = FALSE;
				if (0 != (ToolbarItem::FlagPressed & itemFlags) && 
					item->ButtonUp(this, mouseButton, pt, mouseFlags))
				{
					SetItemStyle(item, ToolbarItem::FlagPressed, FALSE);
					if (NULL != callback)
					{
                        callback->Invalidate(&rcItem);
						callback->TrackMouseLeave(this);
					}
					
				}
			}
		}

		if (toolbarHandler)
		{
			// if rClick - display toolbar context menu here
		}
		handled = TRUE;
	}
	
	if (NULL != pressedItem)
	{
		if (pressedItem != item)
		{
			SetItemStyle(pressedItem, ToolbarItem::FlagHighlighted | ToolbarItem::FlagPressed, FALSE);
			if (NULL != callback && GetItemRect(pressedItem, &rcItem))
				callback->Invalidate(&rcItem);
		}
		pressedItem = NULL;
	}
	
	return handled;
}



typedef struct __TOOLBARMENUITEM
{
	UINT id;
	UINT state;
}TOOLBARMENUITEM;

static INT Toolbar_GetMenuItems(ToolbarItem **toolbarItems, INT count, UINT idBase, TOOLBARMENUITEM *menuItems, INT *columnsMax, INT *linesMax)
{
	INT total = 0;

	INT lineLength = 0;
	INT lines = 0, columns = 0;

	if (NULL != menuItems)
		columns = *columnsMax;
	
	for (INT i = 0; i < count; i++)
	{							
		DWORD itemFlags = toolbarItems[i]->GetFlags();
		if (0 != ((ToolbarItem::FlagSpacer | ToolbarItem::FlagFlexSpacer) & itemFlags) || 4 == lineLength)
		{			
			if (lineLength > 0)
			{
				if (NULL == menuItems)
				{
					if (lineLength > columns)
						columns = lineLength;
				}
				else
				{
					for (int k = lineLength; k < columns; k++)
					{
						TOOLBARMENUITEM *mi = &menuItems[k + (lines * columns)];
						mi->id = 0xFFFFFFFF;
						mi->state = MFS_DISABLED;
					}
				}

				lines++;
				lineLength = 0;
			}
			if (0 != ((ToolbarItem::FlagSpacer | ToolbarItem::FlagFlexSpacer) & itemFlags))
				continue;
		}		

		if (NULL != menuItems)
		{
			TOOLBARMENUITEM *mi = &menuItems[lineLength + (lines * columns)];
			mi->id = idBase + i;
			mi->state = (0 != (ToolbarItem::FlagDisabled & itemFlags)) ? MFS_DISABLED : 0;
		}
		
		lineLength++;
	}

	if (lineLength > 0)
	{
		lines++;
		if (NULL == menuItems && lineLength > columns)
			columns = lineLength;
	}


	if (NULL != columnsMax) *columnsMax = columns;
	if (NULL != linesMax) *linesMax = lines;
	return lines * columns;
}

static INT Toolbar_InsertMenuItems(Toolbar *instance, HMENU hMenu, ToolbarItem **toolbarItems, INT count)
{
	MENUITEMINFO mi;
	mi.cbSize = sizeof(MENUITEMINFO);
	WCHAR szBuffer[80];
	UINT flags;
	ToolbarItem *item;

	UINT insertedCount = 0;
	BOOL insertBreak = FALSE;

	for (INT i = 0; i < count; i++)
	{					
		item = toolbarItems[i];
		if (NULL != item) 
		{
			flags = toolbarItems[i]->GetFlags();
			if (FALSE != item->FillMenuInfo(instance, &mi, szBuffer, ARRAYSIZE(szBuffer)))
			{
				if (MIIM_FTYPE == mi.fMask && MFT_MENUBREAK == mi.fType)
				{
					if (insertedCount > 0)
						insertBreak = TRUE;
				}
				else
				{
					if (FALSE != InsertMenuItem(hMenu, insertedCount, TRUE, &mi)) 
					{
						if (insertBreak)
						{
							mi.fMask = MIIM_FTYPE;
							mi.fType = MFT_MENUBREAK;
							if (InsertMenuItem(hMenu, insertedCount, TRUE, &mi))
								insertedCount++;
							insertBreak = FALSE;
						}
						insertedCount++;
					}
				}
			}
		}
	}

	return insertedCount;
}

INT Toolbar::DisplayButtonsMenu(ToolbarItem *item)
{
	HMENU hMenu = CreatePopupMenu();
	if (NULL == hMenu || NULL == callback)
		return 0;
		
	BOOL result = 0;

	ToolbarItemRect itemRect;
	size_t lastIndex = 0;
	if (GetFirstItemRect(&itemRect))
	{
		do 
		{ 
			if (-1 != itemRect.index)
				lastIndex++;
		} 
		while(GetNextItemRect(&itemRect));
	}
	else
		lastIndex = 0;

		
	UINT itemsCount = (UINT)(itemList.size() - lastIndex);
	ToolbarItem **toolbarItems = itemList.begin() + lastIndex;
	INT insertedCount = Toolbar_InsertMenuItems(this, hMenu, toolbarItems, itemsCount);

	if (insertedCount > 0)
	{
	//	activeMenu = hMenu;
		UINT menuStyle = TPM_RIGHTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION | TPM_RETURNCMD;
		result = callback->TrackPopupMenuEx(hMenu, menuStyle, boundsRect.right, boundsRect.bottom, NULL);
	//	activeMenu = NULL;
	}
	
	DestroyMenu(hMenu);

	if (0 != result)
		FireCommand(result);

	return result;
}

BOOL Toolbar::RelayMeasureItem(MEASUREITEMSTRUCT *pmis)
{
	return FALSE;
	if (ODT_MENU != pmis->CtlType || NULL == activeMenu)
		return FALSE;

	ToolbarItem  *item = NULL;
	if (pmis->itemID > 0 && pmis->itemID <= itemList.size())
		item = itemList[pmis->itemID - 1];

	pmis->itemHeight = height;
	pmis->itemWidth = (NULL != item) ? item->GetWidth() : 0;
	if (pmis->itemWidth > 12)
		pmis->itemWidth -= 12;

	return TRUE;
}

BOOL Toolbar::RelayDrawItem(DRAWITEMSTRUCT *pdis)
{
	return FALSE;

	if (ODT_MENU != pdis->CtlType)
		return FALSE;

	ToolbarItem  *item = NULL;
	if (pdis->itemID > 0 && pdis->itemID <= itemList.size())
		item = itemList[pdis->itemID - 1];

	if (NULL != item)
	{
		COLORREF rgbBkOld = ::SetBkColor(pdis->hDC, rgbTextBk);
		COLORREF rgbFgOld = ::SetTextColor(pdis->hDC, rgbText);
		HFONT fontTextOld = (HFONT)SelectObject(pdis->hDC, fontText);
		
		UINT itemFlags = 0;
		if (0 != (ODS_SELECTED & pdis->itemState))
			itemFlags |= ToolbarItem::FlagHighlighted;

		if (0 != (ODS_DISABLED & pdis->itemState))
			itemFlags |= ToolbarItem::FlagDisabled;

		if (NULL == item || 
			!item->Draw(this, pdis->hDC, &pdis->rcItem, &pdis->rcItem, itemFlags))
		{
			ExtTextOut(pdis->hDC, 0, 0, ETO_OPAQUE, &pdis->rcItem, NULL, 0, NULL);
		}
				
		

		if (rgbBkOld != rgbTextBk) ::SetBkColor(pdis->hDC, rgbBkOld);
		if (rgbFgOld != rgbText) ::SetTextColor(pdis->hDC, rgbFgOld);
		if (fontTextOld != fontText) SelectObject(pdis->hDC, fontTextOld);
	}

	return TRUE;
}

void Toolbar::SetImageList(HIMAGELIST newImageList)
{
	if (NULL != imageList && 0 == (FlagImagelistShared & flags))
		ImageList_Destroy(imageList);
	imageList = newImageList;
}

BOOL Toolbar::Invalidate()
{
	if (NULL == callback)
		return FALSE;
	callback->Invalidate(&boundsRect);
	return TRUE;
}

void Toolbar::FireCommand(UINT commandId)
{
	if (NULL != callback)
		callback->OnCommand(this, commandId);
}

HMENU Toolbar::GetMenuCopy(UINT menuId)
{
	if (NULL == callback)
		return NULL;

	HMENU hMenu = callback->ResolveMenuId(menuId);
	if (NULL == hMenu) return NULL;

	HMENU hDuplicate =  MenuHelper_DuplcateMenu(hMenu);

	callback->ReleaseMenu(menuId, hMenu);
	return hDuplicate;

}

INT Toolbar::TrackPopupMenuEx(ToolbarItem *item, UINT menuId)
{
	if (NULL == callback)
		return 0;

	HMENU hMenu = callback->ResolveMenuId(menuId);
	if (NULL == hMenu)
		return 0;

	RECT rcItem;
	if (NULL == item || !GetItemRect(item, &rcItem))
		CopyRect(&rcItem, &boundsRect);

	INT result = callback->TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION | TPM_RETURNCMD | TPM_RECURSE, 
												rcItem.left, rcItem.bottom, NULL);
	
	callback->ReleaseMenu(menuId, hMenu);

	return result;
}