#include "main.h"
#include "./resource.h"
#include "./wasabi.h"
#include "./service.h"
#include "./menu.h"

#include "../winamp/wa_ipc.h"

#define _ML_HEADER_IMPMLEMENT
#include "../gen_ml/ml_ipc_0313.h"
#undef _ML_HEADER_IMPMLEMENT

#include <strsafe.h>

#define MAINSERVICE_URL		L"http://winamp.orb.com/orb/html/index.html?app=audio"
#define AUDIOSERVICE_URL	L"http://winamp.orb.com/orb/html/index.html?app=audio"
#define VIDEOSERVICE_URL	L"http://winamp.orb.com/orb/html/index.html?app=video"


#define NAVITEM_PREFIX	L"mlorb_svc_"

#define E_NAVITEM_UNKNOWN		E_NOINTERFACE

static INT Navigation_GetIconIndex(LPCWSTR pszImage)
{
	HWND hLibrary = Plugin_GetLibrary();	
	if (NULL == hLibrary) return -1;

	HMLIMGLST hmlilNavigation = MLNavCtrl_GetImageList(hLibrary);
	if (NULL == hmlilNavigation) return -1;
	
	MLIMAGESOURCE mlis;
	ZeroMemory(&mlis, sizeof(mlis));
	mlis.cbSize = sizeof(mlis);
	mlis.hInst = NULL;
	mlis.bpp = 24;
	mlis.lpszName = pszImage;
	mlis.type = SRC_TYPE_PNG;
	mlis.flags = ISF_FORCE_BPP | ISF_PREMULTIPLY | ISF_LOADFROMFILE;
	
	MLIMAGELISTITEM item;
	ZeroMemory(&item, sizeof(item));
	item.cbSize = sizeof(item);
	item.hmlil = hmlilNavigation;
	item.filterUID = MLIF_FILTER3_UID;
	item.pmlImgSource = &mlis;
	
	return MLImageList_Add(hLibrary, &item);
}

static HNAVITEM Navigation_CreateItem(HWND hLibrary, HNAVITEM hParent, OmService *service)
{
	if (NULL == hLibrary || NULL == service) 
		return NULL; 

	WCHAR szName[256], szInvariant[64];
    if (FAILED(service->GetName(szName, ARRAYSIZE(szName))))
		return NULL;

	if (FAILED(StringCchPrintf(szInvariant, ARRAYSIZE(szInvariant), NAVITEM_PREFIX L"%u", service->GetId())))
		return NULL;

	NAVINSERTSTRUCT nis;
	nis.hInsertAfter = NULL;
	nis.hParent = hParent;
	
	INT iIcon = -1;
	if (0 == (OmService::flagRoot & service->GetFlags()))
	{
		WCHAR szIcon[512];
		if (SUCCEEDED(service->GetIcon(szIcon, ARRAYSIZE(szIcon))))
			iIcon = Navigation_GetIconIndex(szIcon);
	}

	
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.mask = NIMF_TEXT | NIMF_STYLE | NIMF_TEXTINVARIANT | NIMF_PARAM | NIMF_IMAGE | NIMF_IMAGESEL;
		
	
	nis.item.id = 0;
	nis.item.pszText = szName;
	nis.item.pszInvariant = szInvariant;
	nis.item.style = 0;
	nis.item.styleMask = nis.item.style;
	nis.item.lParam = (LPARAM)service;
	nis.item.iImage = iIcon;
	nis.item.iSelectedImage = iIcon;

	if (0 != (OmService::flagRoot & service->GetFlags()))
		nis.item.style |= NIS_HASCHILDREN | NIS_ALLOWCHILDMOVE;

		
	HNAVITEM hItem = MLNavCtrl_InsertItem(hLibrary,  &nis);
	if (NULL != hItem)
		service->AddRef();
	
	return hItem;
}

static HNAVITEM Navigation_GetMessageItem(INT msg, INT_PTR param1)
{
	HWND hLibrary = Plugin_GetLibrary();
	HNAVITEM hItem  = (msg < ML_MSG_NAVIGATION_FIRST) ? 	MLNavCtrl_FindItemById(hLibrary, param1) : (HNAVITEM)param1;
	return hItem;
}

static HRESULT Navigation_GetService(HWND hLibrary, HNAVITEM hItem, OmService **service)
{
	WCHAR szBuffer[64];
	
	if (NULL == service) return E_POINTER;
	*service = NULL;

	if (NULL == hLibrary || NULL == hItem) return E_INVALIDARG;

	NAVITEM itemInfo;
	itemInfo.cbSize = sizeof(NAVITEM);
	itemInfo.hItem = hItem;
	itemInfo.pszInvariant = szBuffer;
	itemInfo.cchInvariantMax = ARRAYSIZE(szBuffer);
	itemInfo.mask = NIMF_PARAM | NIMF_TEXTINVARIANT;

	if (FALSE == MLNavItem_GetInfo(hLibrary, &itemInfo))
		return E_FAIL;

	INT cchInvariant = lstrlen(szBuffer);
	INT cchPrefix = ARRAYSIZE(NAVITEM_PREFIX) - 1;
	if (cchInvariant <= cchPrefix ||
		CSTR_EQUAL != CompareString(CSTR_INVARIANT, 0, NAVITEM_PREFIX, cchPrefix, szBuffer, cchPrefix))
	{
		return E_NAVITEM_UNKNOWN;
	}

	*service = (OmService*)itemInfo.lParam;
	(*service)->AddRef();
	
	return S_OK;
}

static HRESULT Navigation_CreateView(HNAVITEM hItem, HWND hParent, HWND *hView)
{
	if (NULL == hView) return E_POINTER;
	*hView = NULL;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_UNEXPECTED;

	if (NULL == hItem || NULL == hParent) return E_INVALIDARG;

	HRESULT hr;

	OmService *service;
	hr = Navigation_GetService(hLibrary, hItem, &service);
	if (SUCCEEDED(hr))
	{
		if (NULL == OMBROWSERMNGR) 
			hr = E_UNEXPECTED;

		if (SUCCEEDED(hr))
		{
			hr = OMBROWSERMNGR->Initialize(NULL, Plugin_GetWinamp());
			if (SUCCEEDED(hr))
			{				
				hr = OMBROWSERMNGR->CreateView(service, hParent, NULL, 0, hView);
			}
		}
		
		service->Release();
	}
	return hr;
}

static BOOL Navigation_GetViewRect(RECT *rect)
{
	if (NULL == rect) return FALSE;
	
	HWND hWinamp = Plugin_GetWinamp();
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hWinamp || NULL == hLibrary) 
		return FALSE;

	HWND hFrame = (HWND)SENDMLIPC(hLibrary, ML_IPC_GETCURRENTVIEW, 0);
	if (NULL == hFrame) 
		hFrame = hLibrary;
	
	return GetWindowRect(hFrame, rect);
}

static HRESULT Navigation_CreatePopup(HNAVITEM hItem, HWND *hWindow)
{
	if (NULL == hWindow) return E_POINTER;
	*hWindow = NULL;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_UNEXPECTED;

	if (NULL == hItem) return E_INVALIDARG;

	HRESULT hr;

	OmService *service;
	hr = Navigation_GetService(hLibrary, hItem, &service);
	if (SUCCEEDED(hr))
	{
		HWND hWinamp = Plugin_GetWinamp();

		if (NULL == OMBROWSERMNGR) 
			hr = E_UNEXPECTED;

		if (SUCCEEDED(hr))
		{
			hr = OMBROWSERMNGR->Initialize(NULL, hWinamp);
			if (SUCCEEDED(hr))
			{
				RECT rect;
				if (FALSE == Navigation_GetViewRect(&rect))
					hr = E_FAIL;
				
				if (SUCCEEDED(hr))
				{
					rect.left += 16;
					rect.top += 16;
					
					hr = OMBROWSERMNGR->CreatePopup(service, rect.left, rect.top, 
									rect.right - rect.left, rect.bottom - rect.top,	hWinamp, NULL, 0, hWindow);
				}
			}
		}
		
		service->Release();
	}

	return hr;	
}


static void Navigation_OnDestroy()
{
	if (NULL != OMBROWSERMNGR)
	{
		OMBROWSERMNGR->Finish();
	}
}

static HRESULT Navigation_ShowContextMenu(HNAVITEM hItem, HWND hHost, POINTS pts)
{
	if (NULL == hItem || NULL == hHost)
		return E_INVALIDARG;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return E_UNEXPECTED;

	HRESULT hr;

	OmService *service;
	hr = Navigation_GetService(hLibrary, hItem, &service);
	if (FAILED(hr)) return hr;
	
	POINT pt;
	POINTSTOPOINT(pt, pts);
	if (-1 == pt.x || -1 == pt.y)
	{
		NAVITEMGETRECT itemRect;
		itemRect.fItem = FALSE;
		itemRect.hItem = hItem;
		if (MLNavItem_GetRect(hLibrary, &itemRect))
		{
			MapWindowPoints(hHost, HWND_DESKTOP, (POINT*)&itemRect.rc, 2);
			pt.x = itemRect.rc.left + 2;
			pt.y = itemRect.rc.top + 2;
		}
	}
	
	HMENU hMenu = Menu_GetMenu(MENU_NAVIGATIONCONTEXT);
	if (NULL != hMenu)
	{
		INT commandId = Menu_TrackPopup(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD, 
							pt.x, pt.y, hHost, NULL);
		
		Menu_ReleaseMenu(hMenu, MENU_NAVIGATIONCONTEXT);

		switch(commandId)
		{
			case ID_NAVIGATION_OPEN:
				MLNavItem_Select(hLibrary, hItem);
				break;
	
			case ID_NAVIGATION_OPENNEWWINDOW:
				{
					HWND hWindow;
					if (SUCCEEDED(Navigation_CreatePopup(hItem, &hWindow)))
					{
						ShowWindow(hWindow, SW_SHOWNORMAL);
					}
				}
				break;
		}
	}

	service->Release();
	
	return hr;
}

BOOL Navigation_Initialize(void)
{
	HNAVITEM hParent = NULL;
	OmService *service;
	HWND hLibrary = Plugin_GetLibrary();

	MLNavCtrl_BeginUpdate(hLibrary, NUF_LOCK_TOP);

	if (SUCCEEDED(OmService::CreateInstance(SERVICE_MAIN, MAKEINTRESOURCE(IDS_MAINSERVICE_NAME), 
					MAKEINTRESOURCE(IDR_MAINSERVICE_ICON), MAINSERVICE_URL, &service)))
	{
		service->SetFlags(OmService::flagRoot, OmService::flagRoot);
		hParent = Navigation_CreateItem(hLibrary, hParent, service);
		service->Release();
	}

	if (NULL != hParent)
	{
		if (SUCCEEDED(OmService::CreateInstance(SERVICE_AUDIO, MAKEINTRESOURCE(IDS_AUDIOSERVICE_NAME), 
					MAKEINTRESOURCE(IDR_AUDIOSERVICE_ICON), AUDIOSERVICE_URL, &service)))
		{
			Navigation_CreateItem(hLibrary, hParent, service);
			service->Release();
		}

		if (SUCCEEDED(OmService::CreateInstance(SERVICE_VIDEO, MAKEINTRESOURCE(IDS_VIDESERVICE_NAME), 
					MAKEINTRESOURCE(IDR_VIDEOSERVICE_ICON), VIDEOSERVICE_URL, &service)))
		{
			Navigation_CreateItem(hLibrary, hParent, service);
			service->Release();
		}
	}
	
	
	MLNavCtrl_EndUpdate(hLibrary);

	return TRUE;
}

static void Navigation_OnDeleteItem(HNAVITEM hItem)
{
	if (NULL == hItem) return;

	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return;

	OmService *service;
	if (SUCCEEDED(Navigation_GetService(hLibrary, hItem, &service)))
	{
		
		NAVITEM itemInfo;
		itemInfo.cbSize = sizeof(NAVITEM);
		itemInfo.hItem = hItem;
		itemInfo.mask = NIMF_PARAM;
		itemInfo.lParam = 0L;
		MLNavItem_SetInfo(hLibrary, &itemInfo);

		service->Release(); // create
		service->Release(); // Navigation_GetService
	}
}

BOOL Navigation_ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result)
{
	if (msg == ML_MSG_NO_CONFIG)
	{
		*result = TRUE;
		return TRUE;
	}

	if (msg < ML_MSG_TREE_BEGIN || msg > ML_MSG_TREE_END)
		return FALSE;

	switch(msg)
	{
		case ML_MSG_TREE_ONCREATEVIEW: 
			{
				HWND hView;
				HNAVITEM hItem = Navigation_GetMessageItem(msg, param1);
				HRESULT hr = Navigation_CreateView(hItem, (HWND)param2, &hView);
				if (SUCCEEDED(hr)) 
				{
					*result = (INT_PTR)hView;
					return TRUE;
				}
			}
			break;
			
 		case ML_MSG_NAVIGATION_ONDESTROY:
			Navigation_OnDestroy();
			break;

		case ML_MSG_NAVIGATION_CONTEXTMENU:
			{
				HNAVITEM hItem = Navigation_GetMessageItem(msg, param1);
				HRESULT hr = Navigation_ShowContextMenu(hItem, (HWND)param2, MAKEPOINTS(param3));
				if (SUCCEEDED(hr)) 
				{
					*result = TRUE;
					return TRUE;
				}
			}
			break;
		case ML_MSG_NAVIGATION_ONDELETE:
			{				
				HNAVITEM hItem = Navigation_GetMessageItem(msg, param1);
				Navigation_OnDeleteItem(hItem);
				break;
			}
	}

	return FALSE;
}

HNAVITEM Navigation_GetActive(OmService **serviceOut)
{
	HWND hLibrary = Plugin_GetLibrary();
	
	OmService *service;
	HNAVITEM hActive = (NULL != hLibrary) ?  MLNavCtrl_GetSelection(hLibrary) : NULL;
	if (NULL == hActive || FAILED(Navigation_GetService(hLibrary, hActive, &service)))
	{
		hActive = NULL;
		service = NULL;
	}
		
	if (NULL != serviceOut) 
		*serviceOut = service;
	else if (NULL != service)
		service->Release();

	return hActive;
}

HNAVITEM Navigation_FindService(UINT serviceId, HNAVITEM hStart, OmService **serviceOut)
{
	HWND hLibrary = Plugin_GetLibrary();

	INT cchPrefix = ARRAYSIZE(NAVITEM_PREFIX) - 1;
	
	WCHAR szBuffer[256];
	NAVITEM itemInfo;
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.mask = NIMF_TEXTINVARIANT | NIMF_PARAM;
	itemInfo.cchInvariantMax = ARRAYSIZE(szBuffer);
	itemInfo.pszInvariant = szBuffer;

	if (NULL == hStart)
		hStart = MLNavCtrl_GetFirst(hLibrary);

	itemInfo.hItem = hStart;
	while(NULL != itemInfo.hItem)
	{
		if (FALSE != MLNavItem_GetInfo(hLibrary, &itemInfo) && 
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, NORM_IGNORECASE, itemInfo.pszInvariant, cchPrefix, 
						NAVITEM_PREFIX, cchPrefix))
		{
			OmService *service = (OmService*)itemInfo.lParam;
			if (NULL != service && service->GetId() == serviceId)
			{
				if (NULL != serviceOut)
				{
					*serviceOut = service;
					service->AddRef();
				}
				return itemInfo.hItem;
			}
		}

		itemInfo.hItem = MLNavItem_GetNext(hLibrary, itemInfo.hItem);
	}

	if (NULL != serviceOut)
		*serviceOut = NULL;
	return NULL;
}

BOOL Navigation_RemoveService(UINT serviceId)
{
	HNAVITEM hItem;
	
	HWND hLibrary = Plugin_GetLibrary();
	if (NULL == hLibrary) return FALSE;

	HNAVITEM hRoot = Navigation_FindService(SERVICE_MAIN, NULL, NULL);
	if (serviceId == SERVICE_MAIN)
	{
		hItem = hRoot;
	}
	else
	{
		hRoot = MLNavItem_GetChild(hLibrary, hRoot); 
		hItem = Navigation_FindService(serviceId, hRoot, NULL);
	}

	return (NULL != hItem) ? MLNavCtrl_DeleteItem(hLibrary, hItem) : FALSE;
}
