#include "./menu.h"
#include "../gen_ml/ml.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "./resource1.h"

extern winampMediaLibraryPlugin plugin;

#define RATING_MARKER			 MAKELONG(MAKEWORD('R','A'),MAKEWORD('T','E'))

#define RATING_MINSPACECX		16
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)))
#endif

static BOOL Menu_IsRatingStar(HMENU hMenu, INT itemId, INT *valueOut)
{
	WCHAR szBuffer[8];
	INT cchBuffer = GetMenuStringW(hMenu, itemId, szBuffer, ARRAYSIZE(szBuffer), MF_BYCOMMAND);
	if (cchBuffer < 1 || cchBuffer > 5) 
        return FALSE;
	
	for (INT i = 1; i < cchBuffer; i++)
	{
		if (szBuffer[i -1] != szBuffer[i])
			return FALSE;
	}

	if (NULL != valueOut)
		*valueOut = cchBuffer;

	return TRUE;
}
static BOOL Menu_MeasureRating(HMENU hMenu, HDC hdc, MEASUREITEMSTRUCT *pmis)
{
	if (NULL == hdc || !Menu_IsRatingStar(hMenu, pmis->itemID, NULL))
		return FALSE;
	
	RECT rect;
	if (!MLRating_CalcRect(plugin.hwndLibraryParent, NULL, 5, &rect))
		return FALSE;
	
	pmis->itemHeight = rect.bottom - rect.top + 6;
	
	TEXTMETRIC tm;
	if (GetTextMetrics(hdc, &tm) && 
		(UINT)(tm.tmHeight + 2) > pmis->itemHeight)
	{
		pmis->itemHeight = tm.tmHeight + 2;
	}
    
	INT spaceCX = (pmis->itemHeight > RATING_MINSPACECX) ? pmis->itemHeight : RATING_MINSPACECX;
	pmis->itemWidth = rect.right - rect.left + (2 * spaceCX) - (GetSystemMetrics(SM_CXMENUCHECK) - 1);
	return TRUE;
}

static BOOL Menu_DrawRating(HMENU hMenu, HDC hdc, DRAWITEMSTRUCT *pdis)
{
	INT ratingValue;
	if (NULL == hdc || !Menu_IsRatingStar(hMenu, pdis->itemID, &ratingValue))
		return FALSE;

	INT spaceCX = ((pdis->rcItem.bottom - pdis->rcItem.top) > RATING_MINSPACECX) ? 
					(pdis->rcItem.bottom - pdis->rcItem.top) : 
					RATING_MINSPACECX;

	RATINGDRAWPARAMS rdp;
	rdp.cbSize = sizeof(RATINGDRAWPARAMS);
	rdp.hdcDst = hdc;
	rdp.rc = pdis->rcItem;
	rdp.rc.left += spaceCX;
	rdp.value = ratingValue;
	rdp.maxValue = 5;

	UINT menuState = GetMenuState(hMenu, pdis->itemID, MF_BYCOMMAND);
	
	rdp.trackingValue = (0 != (ODS_SELECTED & pdis->itemState) && 
						0 == ((MF_DISABLED | MF_GRAYED) & menuState)) ? 
						rdp.value :
						0;

	rdp.fStyle = RDS_LEFT | RDS_VCENTER | RDS_HOT;
	rdp.hMLIL = NULL;
	rdp.index = 0;

	return MLRating_Draw(plugin.hwndLibraryParent, &rdp);
}

static BOOL CALLBACK Menu_CustomDrawProc(INT action, HMENU hMenu, HDC hdc, LPARAM param, ULONG_PTR user)
{
	switch(action)
	{
		case MLMENU_ACTION_MEASUREITEM:
			if (hMenu == (HMENU)user)
				return Menu_MeasureRating(hMenu, hdc, (MEASUREITEMSTRUCT*)param);
			break;
		case MLMENU_ACTION_DRAWITEM:
			if (hMenu == (HMENU)user)
				return MLMENU_WANT_DRAWPART;
			break;
		case MLMENU_ACTION_DRAWBACK:
			break;
		case MLMENU_ACTION_DRAWICON:
			break;
		case MLMENU_ACTION_DRAWTEXT:
			if (hMenu == (HMENU)user)
				return Menu_DrawRating(hMenu, hdc, (DRAWITEMSTRUCT*)param);
			break;
	}
	return FALSE;
}

static HMENU Menu_FindRatingByChild(HMENU hMenu, MENUINFO *pmi, MENUITEMINFO *pmii, UINT childId)
{
	INT count = GetMenuItemCount(hMenu);
	for(INT i = 0; i < count; i++)
	{
		if (GetMenuItemInfo(hMenu, i, TRUE, pmii))
		{
			if (childId == pmii->wID) return hMenu;
			if (NULL != pmii->hSubMenu)
			{								
				HMENU hRating = Menu_FindRatingByChild(pmii->hSubMenu, pmi, pmii, childId);
				if (NULL != hRating) 
					return hRating;
			}
		}
	}
	return NULL;
}

static HMENU Menu_FindRatingByMarker(HMENU hMenu, MENUINFO *pmi, MENUITEMINFO *pmii)
{
	if (GetMenuInfo(hMenu, pmi) && RATING_MARKER == pmi->dwMenuData)
		return hMenu;

	INT count = GetMenuItemCount(hMenu);
	for(INT i = 0; i < count; i++)
	{
		if (GetMenuItemInfo(hMenu, i, TRUE, pmii) && NULL != pmii->hSubMenu)
		{								
			HMENU hRating = Menu_FindRatingByMarker(pmii->hSubMenu, pmi, pmii);
			if (NULL != hRating) 
				return hRating;
		}
	}
	return NULL;
}

HMENU Menu_FindRatingMenu(HMENU hMenu, BOOL fUseMarker)
{
	if (NULL == hMenu) 
		return NULL;

	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_SUBMENU;

	if (FALSE == fUseMarker)
		mii.fMask |= MIIM_ID;

	MENUINFO mi;
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;
	
	return (FALSE == fUseMarker) ? 
			Menu_FindRatingByChild(hMenu, &mi, &mii, ID_RATE_5) : 
			Menu_FindRatingByMarker(hMenu, &mi, &mii);
}

BOOL Menu_SetRatingValue(HMENU ratingMenu, INT ratingValue)
{
	if (NULL == ratingMenu) return FALSE;

	INT ratingList[] = { ID_RATE_0, ID_RATE_1, ID_RATE_2, 
						ID_RATE_3, ID_RATE_4, ID_RATE_5};
	
	/// set rating marker
	MENUINFO mi;
	mi.cbSize = sizeof(MENUINFO);
	mi.fMask = MIM_MENUDATA;
	mi.dwMenuData = RATING_MARKER;
	if (!SetMenuInfo(ratingMenu, &mi))
		return FALSE;


	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	
	UINT type, state;
	for (INT i = 0; i < ARRAYSIZE(ratingList); i++)
	{
		mii.fMask = MIIM_STATE | MIIM_FTYPE;
		if (GetMenuItemInfo(ratingMenu, ratingList[i], FALSE, &mii))
		{
			if (ratingValue == i)
			{
				type = mii.fType | MFT_RADIOCHECK;
				state = mii.fState | MFS_CHECKED;
			}
			else
			{
				type = mii.fType & ~MFT_RADIOCHECK;
				state = mii.fState & ~MFS_CHECKED;
			}

			mii.fMask = 0;
			if (type != mii.fType)
			{
				mii.fType = type;
				mii.fMask |= MIIM_FTYPE;
			}
			
			if (state != mii.fState)
			{
				mii.fState = state;
				mii.fMask |= MIIM_STATE;
			}

			if (0 != mii.fMask)
				SetMenuItemInfo(ratingMenu, ratingList[i], FALSE, &mii);
		}
	}
	return TRUE;
}

INT Menu_TrackSkinnedPopup(HMENU hMenu, UINT fuFlags, INT x, INT y,  HWND hwnd, LPTPMPARAMS lptpm)
{
	if (NULL == hMenu)
		return NULL;
	
	HMENU ratingMenu = Menu_FindRatingMenu(hMenu, TRUE);

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
	popup.customProc = Menu_CustomDrawProc;
	popup.customParam = (ULONG_PTR)ratingMenu;

	return (INT)SENDMLIPC(plugin.hwndLibraryParent, ML_IPC_TRACKSKINNEDPOPUPEX, &popup);
}