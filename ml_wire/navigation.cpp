#include "main.h"
#include "./navigation.h"
#include "./util.h"
#include "./resource.h"
#include "./api.h"
#include "./service.h"
#include "./subscriptionView.h"
#include "./downloadsDialog.h"

#include "../omBrowser/browserView.h"
#include "../winamp/wa_ipc.h"

#include "../gen_ml/ml_ipc_0313.h"

#include <strsafe.h>


#define NAVITEM_PREFIX	L"podcast_svc_"

#define E_NAVITEM_UNKNOWN		E_NOINTERFACE

static Nullsoft::Utility::LockGuard navigationLock;

static INT Navigation_RegisterIcon(HWND hLibrary, INT iconIndex, LPCWSTR pszImage)
{
	HMLIMGLST hmlilNavigation = MLNavCtrl_GetImageList(hLibrary);
	if (NULL == hmlilNavigation) return -1;
	
	MLIMAGESOURCE mlis;
	ZeroMemory(&mlis, sizeof(MLIMAGESOURCE));
	mlis.cbSize = sizeof(MLIMAGESOURCE);
	mlis.hInst = NULL;
	mlis.bpp = 24;
	mlis.lpszName = pszImage;
	mlis.type = SRC_TYPE_PNG;
	mlis.flags = ISF_FORCE_BPP | ISF_PREMULTIPLY | ISF_LOADFROMFILE;
	
	MLIMAGELISTITEM item;
	ZeroMemory(&item, sizeof(MLIMAGELISTITEM));
	item.cbSize = sizeof(MLIMAGELISTITEM);
	item.hmlil = hmlilNavigation;
	item.filterUID = MLIF_FILTER3_UID;
	item.pmlImgSource = &mlis;

	if (iconIndex >= 0)
	{
		INT count = MLImageList_GetImageCount(hLibrary, item.hmlil);
		if (iconIndex < count)
		{
			item.mlilIndex = iconIndex;
			return (FALSE != MLImageList_Replace(hLibrary, &item)) ? iconIndex : -1;
		}
	}
	
	
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
	
	WCHAR szIcon[512];
	INT iIcon = (SUCCEEDED(service->GetIcon(szIcon, ARRAYSIZE(szIcon)))) ? 
				Navigation_RegisterIcon(hLibrary, -1, szIcon) : -1;
	
	nis.item.cbSize = sizeof(NAVITEM);
	nis.item.mask = NIMF_TEXT | NIMF_STYLE | NIMF_TEXTINVARIANT | NIMF_PARAM | NIMF_IMAGE | NIMF_IMAGESEL;
		
	nis.item.id = 0;
	nis.item.pszText = szName;
	nis.item.pszInvariant = szInvariant;
	nis.item.lParam = (LPARAM)service;
	
	nis.item.style = 0;
	UINT serviceFlags = service->GetFlags();
	if (0 != (OmService::flagRoot & serviceFlags))
	{
		nis.item.style |= (NIS_HASCHILDREN | NIS_DEFAULTIMAGE);
	}

	nis.item.styleMask = nis.item.style;

	nis.item.iImage = iIcon;
	nis.item.iSelectedImage = iIcon;

		
	HNAVITEM hItem = MLNavCtrl_InsertItem(hLibrary,  &nis);
	if (NULL != hItem)
		service->AddRef();
	
	return hItem;
}

static HNAVITEM Navigation_GetMessageItem(INT msg, INT_PTR param1)
{
	HWND hLibrary = plugin.hwndLibraryParent;
	HNAVITEM hItem  = (msg < ML_MSG_NAVIGATION_FIRST) ? 	MLNavCtrl_FindItemById(hLibrary, param1) : (HNAVITEM)param1;
	return hItem;
}

static BOOL Navigation_CheckInvariantName(LPCWSTR pszInvarian)
{
	INT cchInvariant = (NULL != pszInvarian) ? lstrlen(pszInvarian) : 0;
	INT cchPrefix = ARRAYSIZE(NAVITEM_PREFIX) - 1;
	return (cchInvariant > cchPrefix &&
		CSTR_EQUAL == CompareString(CSTR_INVARIANT, 0, NAVITEM_PREFIX, cchPrefix, pszInvarian, cchPrefix));
}

static HRESULT Navigation_GetServiceInt(HWND hLibrary, HNAVITEM hItem, OmService **service)
{
	WCHAR szBuffer[64];
	
	if (NULL == service) return E_POINTER;
	*service = NULL;

	if (NULL == hLibrary || NULL == hItem) 
		return E_INVALIDARG;

	NAVITEM itemInfo;
	itemInfo.cbSize = sizeof(NAVITEM);
	itemInfo.hItem = hItem;
	itemInfo.pszInvariant = szBuffer;
	itemInfo.cchInvariantMax = ARRAYSIZE(szBuffer);
	itemInfo.mask = NIMF_PARAM | NIMF_TEXTINVARIANT;

	if (FALSE == MLNavItem_GetInfo(hLibrary, &itemInfo))
		return E_FAIL;

	if (FALSE == Navigation_CheckInvariantName(szBuffer))
		return E_NAVITEM_UNKNOWN;

	*service = (OmService*)itemInfo.lParam;
	(*service)->AddRef();
	return S_OK;
}

HRESULT Navigation_GetService(HNAVITEM hItem, OmService **service)
{
	return Navigation_GetServiceInt(plugin.hwndLibraryParent, hItem, service);
}

static HRESULT Navigation_CreateView(HNAVITEM hItem, HWND hParent, HWND *hView)
{
	if (NULL == hView) return E_POINTER;
	*hView = NULL;

	if (NULL == hItem || NULL == hParent) 
		return E_INVALIDARG;

	HRESULT hr;

	OmService *service;
	hr = Navigation_GetServiceInt(plugin.hwndLibraryParent, hItem, &service);
	if (SUCCEEDED(hr))
	{
		hr = service->CreateView(hParent, hView);		
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

HNAVITEM Navigation_FindService(UINT serviceId, HNAVITEM hStart, OmService **serviceOut)
{
	HWND hLibrary = plugin.hwndLibraryParent;

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

HRESULT Navigation_ShowService(UINT serviceId, INT showMode)
{
	if (SERVICE_DOWNLOADS == serviceId && downloadsViewLoaded)
		return S_OK;

	Nullsoft::Utility::AutoLock lock3(navigationLock);

	HNAVITEM hRoot = Navigation_FindService(SERVICE_PODCAST, NULL, NULL);
	if(NULL == hRoot) return E_UNEXPECTED;

	switch(serviceId)
	{
		case SERVICE_SUBSCRIPTION:
			if (SHOWMODE_AUTO == showMode) 
				return E_INVALIDARG;
			break;

		case SERVICE_DOWNLOADS:
			if (SHOWMODE_AUTO == showMode)
			{
				Nullsoft::Utility::AutoLock lock1(downloadedFiles.downloadedLock);
				Nullsoft::Utility::AutoLock lock2(downloadStatus.statusLock);
				showMode = (0 != downloadedFiles.downloadList.size() || 0 != downloadStatus.downloads.size()) ?
							SHOWMODE_SHOW : SHOWMODE_HIDE;
			}
			break;
		default:
			return E_INVALIDARG;
	}

	if (SHOWMODE_HIDE != showMode && SHOWMODE_SHOW != showMode)
		return E_INVALIDARG;

	HWND hLibrary = plugin.hwndLibraryParent;

	MLNavCtrl_BeginUpdate(hLibrary, NUF_LOCK_TOP);

	OmService *service;
	HNAVITEM hItem = MLNavItem_GetChild(hLibrary, hRoot); 
	hItem = Navigation_FindService(serviceId, hItem, &service);

	HRESULT hr = S_OK;

	if (SHOWMODE_HIDE == showMode)
	{
		if (NULL == hItem) 
			hr = S_FALSE;
		else if (FALSE == MLNavCtrl_DeleteItem(hLibrary, hItem))
			hr = E_FAIL;
	}
	else
	{
		if (NULL != hItem)
			hr = S_FALSE;
		else
		{
			switch(serviceId)
			{
				case SERVICE_SUBSCRIPTION:
					hr = OmService::CreateLocal(SERVICE_SUBSCRIPTION, MAKEINTRESOURCE(IDS_SUBSCRIPTIONS), 
							MAKEINTRESOURCE(IDR_SUBSCRIPTION_ICON), SubscriptionView_Create, &service);
					break;
				case SERVICE_DOWNLOADS:
					hr = OmService::CreateLocal(SERVICE_DOWNLOADS, MAKEINTRESOURCE(IDS_DOWNLOADS), 
							MAKEINTRESOURCE(IDR_DOWNLOAD_ICON), DownloadDialog_Create, &service);
					break;
				default:
					hr = E_UNEXPECTED;
					break;
			}

			if (SUCCEEDED(hr))
			{
				if (NULL == Navigation_CreateItem(hLibrary, hRoot, service))
					hr = E_FAIL;
			}
		}
	}

	if(NULL != service) 
		service->Release();

	MLNavCtrl_EndUpdate(hLibrary);
	
	return hr;
}

BOOL Navigation_Initialize(void)
{
	HNAVITEM hParent = NULL;
	OmService *service;
	HWND hLibrary = plugin.hwndLibraryParent;

	MLNavCtrl_BeginUpdate(hLibrary, NUF_LOCK_TOP);

	if (SUCCEEDED(OmService::CreateRemote(SERVICE_PODCAST, MAKEINTRESOURCE(IDS_PODCAST_DIRECTORY), 
					MAKEINTRESOURCE(IDR_DISCOVER_ICON), L"http://client.winamp.com/wire/tagbrowse.php", &service)))
	{
		service->SetFlags(OmService::flagRoot, OmService::flagRoot);
		hParent = Navigation_CreateItem(hLibrary, hParent, service);
		service->Release();
	}

	if (NULL != hParent)
	{	
		Navigation_ShowService(SERVICE_SUBSCRIPTION, SHOWMODE_SHOW);
		//Navigation_ShowService(SERVICE_DOWNLOADS, SHOWMODE_AUTO);
	}
	
	MLNavCtrl_EndUpdate(hLibrary);

	return TRUE;
}

static void Navigation_OnDeleteItem(HNAVITEM hItem)
{
	if (NULL == hItem) return;

	HWND hLibrary = plugin.hwndLibraryParent;
	if (NULL == hLibrary) return;

	WCHAR szBuffer[64];
	NAVITEM itemInfo;

	ZeroMemory(&itemInfo, sizeof(itemInfo));
	itemInfo.cbSize = sizeof(itemInfo);
    itemInfo.hItem = hItem;
	itemInfo.pszInvariant = szBuffer;
	itemInfo.cchInvariantMax = ARRAYSIZE(szBuffer);
	itemInfo.mask = NIMF_PARAM | NIMF_TEXTINVARIANT | NIMF_IMAGE;

	if (FALSE != MLNavItem_GetInfo(hLibrary, &itemInfo) && 
		FALSE != Navigation_CheckInvariantName(szBuffer))
	{
		
		OmService *service = (OmService*)itemInfo.lParam;

		itemInfo.mask = NIMF_PARAM;
		itemInfo.lParam = 0L;
		MLNavItem_SetInfo(hLibrary, &itemInfo);

		service->Release();
	}
}


BOOL Navigation_ProcessMessage(INT msg, INT_PTR param1, INT_PTR param2, INT_PTR param3, INT_PTR *result)
{
	if (msg < ML_MSG_TREE_BEGIN || msg > ML_MSG_TREE_END)
		return FALSE;

	HRESULT hr;

	switch(msg)
	{
		case ML_MSG_TREE_ONCREATEVIEW: 
			{
				HWND hView;
				hr = Navigation_CreateView(Navigation_GetMessageItem(msg, param1), (HWND)param2, &hView);
				*result = (SUCCEEDED(hr)) ? (INT_PTR)hView : NULL;
			}
			return TRUE;
			
 		case ML_MSG_NAVIGATION_ONDESTROY:
			Navigation_OnDestroy();
			return TRUE;

		case ML_MSG_NAVIGATION_ONDELETE:
			Navigation_OnDeleteItem(Navigation_GetMessageItem(msg, param1));
			return TRUE;
	}
	return FALSE;
}
