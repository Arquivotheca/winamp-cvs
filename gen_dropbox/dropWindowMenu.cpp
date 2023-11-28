#include "./main.h"
#include "./dropWindowInternal.h"
#include "./wasabiApi.h"
#include "./resource.h"
#include "../nu/menuHelpers.h"

#include <strsafe.h>

#define SUBMENU_WINDOWCONTEXT_ID			0
#define SUBMENU_ITEMCONTEXT_ID			1
#define SUBMENU_PLAY_ID					2
#define SUBMENU_EDIT_ID					3
#define SUBMENU_ARRANGEBY_ID				4

HMENU DropWindowMenu_Initialize()
{	
	return WASABI_API_LOADMENUW(IDR_MENU_DROPWINDOW);
}


typedef struct __TEMPITEM
{
	INT menuId;
	LPTSTR pszText;
} TEMPITEM;

static INT __cdecl DropWindowMenu_CompareTempItemsText(const void *elem1, const void *elem2)
{
	INT r = CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, 
					((TEMPITEM*)elem1)->pszText, -1, ((TEMPITEM*)elem2)->pszText, -1);
	return (0 != r) ? (r - 2) : 0;
}

static void DropWindowMenu_SortTempItems(TEMPITEM *pszItems, size_t count)
{
	qsort(pszItems, count, sizeof(TEMPITEM), DropWindowMenu_CompareTempItemsText);
}


static LPCTSTR DropWindowMenu_ExpandString(LPCTSTR pszText, LPTSTR pszBuffer, INT cchBufferMax)
{
	if (IS_INTRESOURCE(pszText))
	{
		WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszText, pszBuffer, cchBufferMax);
		pszText = (0 != lstrlen(pszBuffer)) ? pszBuffer : NULL;
	}

	return pszText;
}

static INT DropWindowMenu_InsertArrangeByItems(HMENU hMenu, INT nPos)
{
	if (!hMenu) return 0;

	INT count = 0;
	
	TEMPITEM szItems[ARRAYSIZE(szRegisteredColumns)];
	LPCTSTR pszText;
	TCHAR szBuffer[512];
	
	for (int i = 0; i < ARRAYSIZE(szRegisteredColumns); i++)
	{
		if (!szRegisteredColumns[i].fnComparer)
			continue;

		szItems[count].menuId = MENU_ARRANGEBY_MIN + szRegisteredColumns[i].id;
		pszText = DropWindowMenu_ExpandString(szRegisteredColumns[i].pszTitleLong, szBuffer, ARRAYSIZE(szBuffer));
		if (NULL == pszText)
			pszText = DropWindowMenu_ExpandString(szRegisteredColumns[i].pszTitle, szBuffer, ARRAYSIZE(szBuffer));

		if (NULL != pszText && TEXT('\0') != *pszText)
			szItems[count].pszText = lfh_strdup(pszText);
		
		if (NULL != szItems[count].pszText)
			count++;
	}

	if (0 != count)
	{
		DropWindowMenu_SortTempItems(szItems, count);

		MENUITEMINFO mii = {sizeof(MENUITEMINFO), };
		mii.fMask = MIIM_STRING | MIIM_ID | MIIM_FTYPE;
		mii.fType = 0;

		for (int i = 0; i < count; i++)
		{		
			mii.dwTypeData = szItems[i].pszText;
			mii.wID = szItems[i].menuId;
			InsertMenuItem(hMenu, i, TRUE, &mii);
			lfh_free(szItems[i].pszText);
		}
	}
	return count;
}

static HMENU DropWindowMenu_GetWindowMenu(HWND hwndDB, HMENU hmenuDB)
{
	HMENU hMenu = GetSubMenu(hmenuDB, SUBMENU_WINDOWCONTEXT_ID);
	return hMenu;
}


static HMENU DropWindowMenu_GetItemMenu(HWND hwndDB, HMENU hmenuDB)
{
	HMENU hMenu = GetSubMenu(hmenuDB, SUBMENU_ITEMCONTEXT_ID);
	if (NULL == hMenu)
		return NULL;

	hMenu = MenuHelper_DuplcateMenu(hMenu);
	MENUITEMINFO mii = {sizeof(MENUITEMINFO), };
	
	TCHAR szBuffer[512];
	HMENU hSubMenu;
	INT insertCount, itemsCount = 0;
	
	hSubMenu = CreatePopupMenu();
	if (NULL != hSubMenu)
	{
		itemsCount = DropWindowMenu_InsertArrangeByItems(hSubMenu, 0);
		if (0 != itemsCount)
		{
			if (MenuHelper_InsertSeparator(hSubMenu, itemsCount))
				itemsCount++;
		}

		MenuHelper_CopyMenu(hSubMenu, itemsCount, GetSubMenu(hmenuDB, SUBMENU_ARRANGEBY_ID));

		mii.fMask = MIIM_STRING;
		mii.dwTypeData = szBuffer;
		mii.cch = ARRAYSIZE(szBuffer);
		if (!GetMenuItemInfo(hmenuDB, SUBMENU_ARRANGEBY_ID, TRUE, &mii))
		{
			DestroyMenu(hSubMenu);
			hSubMenu = NULL;
		}
	}
	
	mii.fMask = MIIM_SUBMENU | MIIM_STRING;
	mii.dwTypeData = szBuffer;
	mii.hSubMenu = hSubMenu;
	mii.wID = (UINT)(UINT_PTR)hSubMenu;

	if (NULL != mii.hSubMenu)
	{

		if (TEXT('\0') != *mii.dwTypeData)
			itemsCount = InsertMenuItem(hMenu, 0, TRUE, &mii);
		if (0 == itemsCount)
			DestroyMenu(hSubMenu);
		else 
			MenuHelper_InsertSeparator(hMenu, itemsCount);
	}
	
	insertCount = MenuHelper_CopyMenu(hMenu, itemsCount, GetSubMenu(hmenuDB, SUBMENU_EDIT_ID));
	if (0 != insertCount)
	{
		if (0 != itemsCount && MenuHelper_InsertSeparator(hMenu, itemsCount))
			itemsCount++;
		itemsCount += insertCount;
	}

	insertCount = MenuHelper_CopyMenu(hMenu, 0, GetSubMenu(hmenuDB, SUBMENU_PLAY_ID));
	if (0 != insertCount)
	{
		if (0 != itemsCount && MenuHelper_InsertSeparator(hMenu, insertCount))
			itemsCount++;
		itemsCount += insertCount;
	}

	return hMenu;
}

static HMENU DropWindowMenu_GetViewMenu(HWND hwndDB, HMENU hmenuDB)
{
	HMENU hMenu = CreatePopupMenu();
	if (NULL == hMenu)
		return NULL;

	hMenu = MenuHelper_DuplcateMenu(hMenu);
	MENUITEMINFO mii = {sizeof(MENUITEMINFO), };
	
	TCHAR szBuffer[512];
	HMENU hSubMenu;
	INT itemsCount = 0;

	HMENU hmEdit = GetSubMenu(hmenuDB, SUBMENU_EDIT_ID);
	if (NULL != hmEdit)
	{
		INT itemPos;
				

		if (MenuHelper_GetMenuItemPos(hmEdit, ID_PASTE, &itemPos))
		{			
			itemsCount = MenuHelper_CopyMenuEx(hMenu, 0, hmEdit, itemPos, 1);
			if (itemsCount > 0 && MenuHelper_InsertSeparator(hMenu, itemsCount))
				itemsCount++;
		}
		if (MenuHelper_GetMenuItemPos(hmEdit, ID_SELECTALL, &itemPos))	
			itemsCount += MenuHelper_CopyMenuEx(hMenu, itemsCount, hmEdit, itemPos, 1);
		if (MenuHelper_GetMenuItemPos(hmEdit, ID_INVERTSELECTION, &itemPos))	
			itemsCount += MenuHelper_CopyMenuEx(hMenu, itemsCount, hmEdit, itemPos, 1);
	
	}
	
	hSubMenu = CreatePopupMenu();
	if (NULL != hSubMenu)
	{
		itemsCount = DropWindowMenu_InsertArrangeByItems(hSubMenu, 0);
		if (0 != itemsCount)
		{
			MenuHelper_InsertSeparator(hSubMenu, itemsCount);
			itemsCount++;
		}

		MenuHelper_CopyMenu(hSubMenu, itemsCount, GetSubMenu(hmenuDB, SUBMENU_ARRANGEBY_ID));

		mii.fMask = MIIM_STRING;
		mii.dwTypeData = szBuffer;
		mii.cch = ARRAYSIZE(szBuffer);
		if (!GetMenuItemInfo(hmenuDB, SUBMENU_ARRANGEBY_ID, TRUE, &mii))
		{
			DestroyMenu(hSubMenu);
			hSubMenu = NULL;
		}
	}
	
	mii.fMask = MIIM_SUBMENU | MIIM_STRING;
	mii.dwTypeData = szBuffer;
	mii.hSubMenu = hSubMenu;

	if (NULL != mii.hSubMenu)
	{
		if (TEXT('\0') == *mii.dwTypeData || !InsertMenuItem(hMenu, 0, TRUE, &mii))
			DestroyMenu(hSubMenu);
		else
			MenuHelper_InsertSeparator(hMenu, 1);
	}
	
	
	
	return hMenu;
}

static HMENU DropWindowMenu_GetArrangeByMenu(HWND hwndDB, HMENU hmenuDB)
{
	HMENU hMenu = CreatePopupMenu();
	if (NULL == hMenu)
		return NULL;
	
	INT itemsCount = DropWindowMenu_InsertArrangeByItems(hMenu, 0);
	if (0 != itemsCount)
	{
		if (0 != MenuHelper_InsertSeparator(hMenu, itemsCount))
			itemsCount++;
	}

	MenuHelper_CopyMenu(hMenu, itemsCount, GetSubMenu(hmenuDB, SUBMENU_ARRANGEBY_ID));

	return hMenu;

}

HMENU DropWindowMenu_GetSubMenu(HWND hwndDB, HMENU hmenuDB, UINT menuType)
{
	switch(menuType)
	{
		case DBMENU_WINDOWCONTEXT:
			return DropWindowMenu_GetWindowMenu(hwndDB, hmenuDB);
		case DBMENU_ITEMCONTEXT:
			return DropWindowMenu_GetItemMenu(hwndDB, hmenuDB);
		case DBMENU_SELECTIONCONTEXT:
			return DropWindowMenu_GetItemMenu(hwndDB, hmenuDB);
		case DBMENU_VIEWCONTEXT:
			return DropWindowMenu_GetViewMenu(hwndDB, hmenuDB);
		case DBMENU_ARRANGEBY:
			return DropWindowMenu_GetArrangeByMenu(hwndDB, hmenuDB);
	}
	return NULL;
}

void DropWindowMenu_ReleaseSubMenu(HWND hwndDB, UINT menuType, HMENU hmenu)
{
	if (NULL == hmenu)
		return;
	switch(menuType)
	{
		case DBMENU_WINDOWCONTEXT:
			break;
		case DBMENU_ITEMCONTEXT:
            DestroyMenu(hmenu);
			break;
		case DBMENU_VIEWCONTEXT:
            DestroyMenu(hmenu);
			break;
		case DBMENU_SELECTIONCONTEXT:
            DestroyMenu(hmenu);
			break;
		case DBMENU_ARRANGEBY:
            DestroyMenu(hmenu);
			break;
	}
}