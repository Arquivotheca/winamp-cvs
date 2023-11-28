#include "main.h"
#include "./menu.h"
#include "./wasabi.h"
#include "./resource.h"
#include "./navigation.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "../nu/menuHelpers.h"

#define SUBMENU_NAVIGATIONCONTEXT			0

static HMENU Menu_GetNavigationContext(HMENU baseMenu)
{
	HMENU hMenu = GetSubMenu(baseMenu, SUBMENU_NAVIGATIONCONTEXT);
	if (NULL == hMenu) return NULL;

	hMenu = MenuHelper_DuplcateMenu(hMenu);
	if (NULL == hMenu) return NULL;

	HNAVITEM hActive = Navigation_GetActive(NULL);
	if (NULL != hActive)
	{
		EnableMenuItem(hMenu, ID_NAVIGATION_OPEN, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	}
	else
	{
		EnableMenuItem(hMenu, ID_NAVIGATION_OPEN, MF_BYCOMMAND | MF_ENABLED);
		SetMenuDefaultItem(hMenu, ID_NAVIGATION_OPEN, FALSE);
	}

	return hMenu;
}

HMENU Menu_GetMenu(UINT menuKind)
{
	HMENU baseMenu = WASABI_API_LOADMENUW(IDR_CONTEXTMENU);
	if (NULL == baseMenu) 
		return NULL;

	switch(menuKind)
	{
		case MENU_NAVIGATIONCONTEXT:
			return Menu_GetNavigationContext(baseMenu);
	}

	return NULL;
}

BOOL Menu_ReleaseMenu(HMENU hMenu, UINT menuKind)
{
	if (NULL == hMenu) return FALSE;

	switch(menuKind)
	{
		case MENU_NAVIGATIONCONTEXT:
			return DestroyMenu(hMenu);
	}
	return FALSE;
}

BOOL Menu_TrackPopup(HMENU hMenu, UINT fuFlags, INT x, INT y,  HWND hwnd, LPTPMPARAMS lptpm)
{
	if (NULL == hMenu)
		return NULL;
	
	MLSKINNEDPOPUP popup;
	ZeroMemory(&popup, sizeof(MLSKINNEDPOPUP));
	popup.cbSize = sizeof(MLSKINNEDPOPUP);
	popup.hmenu = hMenu;
    popup.fuFlags = fuFlags;
    popup.x = x;
    popup.y = y;
    popup.hwnd = hwnd;
	popup.lptpm = lptpm;
    popup.skinStyle = SMS_USESKINFONT/*SMS_SYSCOLORS*/;
	popup.customProc = NULL;
	popup.customParam = 0L;
	return (INT)SENDMLIPC(Plugin_GetLibrary(), ML_IPC_TRACKSKINNEDPOPUPEX, &popup);
}