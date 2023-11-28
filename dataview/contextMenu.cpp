#include "main.h"
#include "./contextMenu.h"
#include "./contextMenuOwner.h"

#define MENUITEM_BASE_ID		10000

ContextMenu::ContextMenu(ifc_viewactioncontextenum *_contextEnum, ifc_viewmenugroup *_root, BOOL _extendedMode)
	: ref(1), hMenu(NULL), contextEnum(_contextEnum), root(_root), extendedMode(_extendedMode)
{
	if (NULL != contextEnum)
		contextEnum->AddRef();

	if (NULL != root)
		root->AddRef();

	eventHandler.Init(this);

}

ContextMenu::~ContextMenu()
{
	if (NULL != hMenu)
	{
		size_t count;
		
		count = GetMenuItemCount(hMenu);
		if ((size_t)-1 != count)
		{
			MENUITEMINFO itemInfo;
			ContextMenu *subMenu;
			ifc_viewmenuitem *menuItem;

			itemInfo.cbSize = sizeof(itemInfo);
			itemInfo.fMask = MIIM_SUBMENU | MIIM_DATA;
			while(count--)
			{
				if (FALSE != GetMenuItemInfo(hMenu, count, TRUE, &itemInfo) && 
					NULL != itemInfo.hSubMenu &&
					SUCCEEDED(GetFromHandle(itemInfo.hSubMenu, &subMenu)))
				{
					subMenu->Release();
					subMenu->Release();

					// must vye
					itemInfo.hSubMenu = NULL;
					SetMenuItemInfo(hMenu, count, TRUE, &itemInfo);
				}

				menuItem = (ifc_viewmenuitem*)itemInfo.dwItemData;
				if (NULL != menuItem)
					menuItem->Release();

			}
			DestroyMenu(hMenu);
		}
		hMenu = NULL;
	}

	if (NULL != root)
	{
		eventHandler.UnregisterItem(root);
		root->Release();
	}

	SafeRelease(contextEnum);

}


HRESULT ContextMenu::CreateInstance(ifc_viewactioncontextenum *contextEnum, ifc_viewmenugroup *root, 
									BOOL extendedMode, ContextMenu **instance)
{
	HRESULT hr;

	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == contextEnum || NULL == root)
		return E_INVALIDARG;

	*instance = new (std::nothrow) ContextMenu(contextEnum, root, extendedMode);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	hr = (*instance)->Init();
	if (FAILED(hr))
	{
		(*instance)->Release();
		*instance = NULL;
	}

	return hr;
}

HRESULT ContextMenu::GetFromHandle(HMENU hMenu, ContextMenu **instance)
{
	MENUINFO info;

	if (NULL == instance)
		return E_POINTER;

	if (NULL == hMenu)
		return E_INVALIDARG;
	
	info.cbSize = sizeof(info);
	info.fMask = MIM_MENUDATA;

	if (FALSE == GetMenuInfo(hMenu, &info))
	{
		RETURN_HRESULT_FROM_LAST_ERROR();
	}

	if (NULL == info.dwMenuData)
		return E_FAIL;

	*instance = (ContextMenu*)info.dwMenuData;
	(*instance)->AddRef();

	return S_OK;
}


size_t ContextMenu::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t ContextMenu::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

HRESULT ContextMenu::GetRoot(ifc_viewmenugroup **group)
{
	if (NULL == group)
		return E_POINTER;

	*group = root;
	if (NULL == root)
		return S_FALSE;

	root->AddRef();
	return S_OK;
}

HMENU ContextMenu::GetHandle()
{
	return hMenu;
}

HRESULT ContextMenu::UpdateMenuInfo()
{
	MENUINFO menuInfo;

	if (NULL == hMenu)
		return E_UNEXPECTED;

	menuInfo.cbSize = sizeof(menuInfo);
	menuInfo.fMask = MIM_MENUDATA | MIM_STYLE;
	if (FALSE == GetMenuInfo(hMenu, &menuInfo))
		RETURN_HRESULT_FROM_LAST_ERROR();

	menuInfo.fMask = 0;
	if ((ULONG_PTR)this != menuInfo.dwMenuData)
	{
		menuInfo.dwMenuData = (ULONG_PTR)(this);
		menuInfo.fMask |= MIM_MENUDATA;
	}

	if (0 == (MNS_NOTIFYBYPOS & menuInfo.dwStyle))
	{
		menuInfo.dwStyle |= MNS_NOTIFYBYPOS;
		menuInfo.fMask |= MIM_STYLE;
	}

	if (0 != menuInfo.fMask && 
		FALSE == SetMenuInfo(hMenu, &menuInfo))
	{
		RETURN_HRESULT_FROM_LAST_ERROR();
	}

	return S_OK;

}
HRESULT ContextMenu::Show(unsigned int flags, int x, int y, HWND hwnd, TPMPARAMS *params)
{
	BOOL result;
	ContextMenuOwner *owner;
	HRESULT hr;

	if (NULL == hMenu)
		return E_UNEXPECTED;
	
	hr = UpdateMenuInfo();
	if (FAILED(hr))
		return hr;
	
	flags &= ~(TPM_NONOTIFY | TPM_RETURNCMD);

	hr = ContextMenuOwner::CreateInstance(this, hwnd, &owner);
	if (FAILED(hr))
		return hr;
	
	//
	result = FALSE;

	Theme *theme;
	if (SUCCEEDED(Plugin_GetTheme(&theme)))
	{
		if (S_OK == theme->PopupMenu_IsEnabled())
		{
			result = theme->PopupMenu_Track(hMenu, flags, x, y, hwnd, params);
		}
		theme->Release();
	}
	
	if (FALSE == result)
		result = TrackPopupMenuEx(hMenu, flags, x, y, hwnd, params);

	if (FALSE == result)
	{
		unsigned long errorCode;

		errorCode = GetLastError();
		hr = HRESULT_FROM_WIN32(errorCode);

		owner->Detach();
	}
	else
		hr = S_OK; 

	owner->Release();

	return hr;
}

BOOL ContextMenu::IsExtendedMode()
{
	return extendedMode;
}

HRESULT ContextMenu::Init()
{
	size_t index, count, inserted;
	ifc_viewmenuitem *item;
	HRESULT hr;
		
	hMenu = CreatePopupMenu();
	if (NULL == hMenu)
		RETURN_HRESULT_FROM_LAST_ERROR();

	hr = UpdateMenuInfo();
	if (FAILED(hr))
		return hr;

	eventHandler.RegisterItem(root);
		
	count = root->GetCount();
	inserted = 0;

	for (index = 0; index < count; index++)
	{
		if (SUCCEEDED(root->Get(index, &item)))
		{
			if (SUCCEEDED(InsertItem(index, FALSE, item)))
				inserted++;

			item->Release();
		}
	}
	
	return S_OK;
	//(count == inserted) ? S_OK : E_FAIL;
}

HRESULT ContextMenu::InsertItem(size_t position, BOOL verifyPosition, ifc_viewmenuitem *item)
{
	HRESULT hr;
	MENUITEMINFO itemInfo;
	MenuState itemState;
	MenuStyle itemStyle;
	wchar_t buffer[256];
	ifc_viewmenugroup *group;
		
	if (NULL == item)
		return E_INVALIDARG;

	itemInfo.cbSize = sizeof(itemInfo);

	if (FALSE != verifyPosition)
	{
		ifc_viewmenuitem *neighbor;
		size_t positionTest, positionBest, positionMenuMax;

		itemInfo.fMask = MIIM_DATA;
		
		positionTest = position;
		positionBest = -1;

		positionMenuMax = GetMenuItemCount(hMenu);
		if ((size_t)-1 != positionMenuMax && 0 != positionMenuMax)
		{			
			if (positionMenuMax > position)
				positionMenuMax = position;

			positionMenuMax--;

			while(positionTest--)
			{
				if(S_OK == root->Get(positionTest, &neighbor))
				{
					
					positionBest = LocateItem(neighbor->GetName(), positionMenuMax, TRUE, NULL);
					neighbor->Release();

					if ((size_t)-1 != positionBest)
					{
						positionBest++;
						break;
					}
				}
			}
		}

		if ((size_t)-1 == positionBest)
			position = 0;
		else
			position = positionBest;
	}

	itemInfo.fMask = MIIM_ID | MIIM_FTYPE | MIIM_STATE | MIIM_DATA | MIIM_STRING;
	itemInfo.fType = 0;
	itemInfo.wID = (unsigned int)(uintptr_t)item;

	itemStyle = item->GetStyle();
	
	if (0 != (MenuStyle_BarBreak & itemStyle))
		itemInfo.fType |= MFT_MENUBARBREAK;
	else if (0 != (MenuStyle_Break & itemStyle))
		itemInfo.fType |= MFT_MENUBREAK;

	if (0 != (MenuStyle_Separator & itemStyle))
		itemInfo.fType |= MFT_SEPARATOR;
	else if (0 != (MenuStyle_RadioCheck & itemStyle))
		itemInfo.fType |= MFT_RADIOCHECK;

	itemInfo.fState = MFS_ENABLED | MFS_UNCHECKED;
	itemState = item->GetState();
	if (0 != (MenuState_Disabled & itemState))
		itemInfo.fState |= (MFS_DISABLED | MFS_GRAYED);

	if (0 != (MenuState_Checked & itemState))
		itemInfo.fState |= MFS_CHECKED;
	
	itemInfo.dwItemData = (ULONG_PTR)item;
	if (0 != (MIIM_STRING & itemInfo.fMask))
	{
		if (FAILED(item->GetDisplayName(buffer, ARRAYSIZE(buffer))))
			return E_FAIL;

		itemInfo.dwTypeData = buffer;
	}

	if (SUCCEEDED(item->QueryInterface(IFC_ViewMenuGroup, (void**)&group)))
	{
		ContextMenu *groupContext;
		if (SUCCEEDED(ContextMenu::CreateInstance(contextEnum, group, extendedMode, &groupContext)))
		{
			itemInfo.fMask |= MIIM_SUBMENU;
			itemInfo.hSubMenu = groupContext->GetHandle();
		}
		group->Release();
	}

	
	if (0 == (MFT_SEPARATOR & itemInfo.fType))
	{		
		ifc_viewaction *action;

		hr = item->GetAction(&action);
		if (S_OK == hr)
		{
			Dispatchable *context;
			hr = GetActionContext(action, &context);
			if (SUCCEEDED(hr))
				context->Release();
			
			action->Release();
		}
		else if (S_FALSE == hr)
		{
			if (0 != (MIIM_SUBMENU & itemInfo.fMask))
				hr = S_OK;
			else
				hr = E_FAIL;
		}
	}
	else
		hr = S_OK;


	if (SUCCEEDED(hr) &&
		FALSE == InsertMenuItem(hMenu, position, TRUE, &itemInfo))
	{		
		unsigned long errorCode;

		errorCode = GetLastError();
		hr = HRESULT_FROM_WIN32(errorCode);
	}

	if (FAILED(hr))
	{
		ContextMenu *subMenu;

		if (0 != (MIIM_SUBMENU & itemInfo.fMask) && 
			SUCCEEDED(GetFromHandle(itemInfo.hSubMenu, &subMenu)))
		{
			subMenu->Release();
			subMenu->Release();
		}
	}
	else
	{
		item->AddRef();
	}
	
	return hr;
}

HRESULT ContextMenu::RemoveItem(const char *name, size_t position)
{
	ifc_viewmenuitem *item;
	MENUITEMINFO itemInfo;
	ContextMenu *subMenu;

	if (IS_STRING_EMPTY(name))
		return E_INVALIDARG;

	if ((size_t)-1 != position)
		position = LocateItem(name, position, FALSE, NULL);

	if ((size_t)-1 == position)
		position = LocateItem(name, 0, FALSE, NULL);

	if ((size_t)-1 == position)
		return S_FALSE;

	
	
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_SUBMENU | MIIM_DATA;
	
	if (FALSE == GetMenuItemInfo(hMenu, position, TRUE, &itemInfo))
	{
		RETURN_HRESULT_FROM_LAST_ERROR();
	}

	itemInfo.fMask = 0;

	if (NULL != itemInfo.hSubMenu &&
		SUCCEEDED(GetFromHandle(itemInfo.hSubMenu, &subMenu)))
	{
		subMenu->Release();
		subMenu->Release();
		
		itemInfo.fMask |= MIIM_SUBMENU;
		itemInfo.hSubMenu = NULL;
	}

	item = (ifc_viewmenuitem*)itemInfo.dwItemData;
	if (NULL != item)
	{
		itemInfo.fMask |= MIIM_DATA;
		itemInfo.dwItemData = NULL;
		item->Release();
	}

	if (0 != itemInfo.fMask)
		SetMenuItemInfo(hMenu, position, TRUE, &itemInfo);
	
	if (FALSE == DeleteMenu(hMenu, position, MF_BYPOSITION))
	{
		RETURN_HRESULT_FROM_LAST_ERROR();
	}

	return S_OK;
}

size_t ContextMenu::LocateItem(const char *name, size_t start, BOOL reverse, ifc_viewmenuitem **itemOut)
{
	size_t count;
	MENUITEMINFO itemInfo;
	ifc_viewmenuitem *item;
	const char *itemName;
	
	if (IS_STRING_EMPTY(name))
		return (size_t)-1;

	count = GetMenuItemCount(hMenu);
	if ((size_t)-1 == count || 
		start >= count)
	{
		return (size_t)-1;
	}
	
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_DATA;

	
	for(;;)
	{
		if (FALSE != GetMenuItemInfo(hMenu, start, TRUE, &itemInfo))
		{
			item = (ifc_viewmenuitem*)itemInfo.dwItemData;
			if (NULL != item)
			{
				itemName = item->GetName();
				if (FALSE == IS_STRING_EMPTY(itemName) &&
					CSTR_EQUAL == CompareStringA(CSTR_INVARIANT, NORM_IGNORECASE, itemName, -1, name, -1))
				{
					if (NULL != itemOut)
					{
						*itemOut = item;
						item->AddRef();
					}
					return start;
				}
			}
		}

		if (FALSE == reverse)
		{
			if (++start >= count)
				break;
		}
		else
		{
			if (0 == start--)
				break;
		}
	}

	if (NULL != itemOut)
		*itemOut = NULL;

	return  (size_t)-1;
}

void ContextMenu::OnMenuInit(int position, HWND ownerWindow)
{	
	ifc_viewmenuitem *item;
	if (NULL != root &&
		SUCCEEDED(root->QueryInterface(IFC_ViewMenuItem, (void**)&item)))
	{		
		Execute(item, ownerWindow);
		item->Release();
	}
}

void ContextMenu::OnMenuUninit(HWND ownerWindow)
{	
}

void ContextMenu::OnMenuCommand(int position, HWND ownerWindow)
{
	ifc_viewmenuitem *item;
	MENUITEMINFO itemInfo;
	
	itemInfo.cbSize = sizeof(itemInfo);
	itemInfo.fMask = MIIM_DATA;
	if (FALSE == GetMenuItemInfo(hMenu, position, TRUE, &itemInfo))
		return;

	item = (ifc_viewmenuitem*)itemInfo.dwItemData;
	if (NULL == item)
		return;

	Execute(item, ownerWindow);
}

void ContextMenu::OnMenuSelect(int position, unsigned int flags, HWND ownerWindow)
{
//	aTRACE_FMT("menu select (hMenu=0x%08X, pos=%d, flags=0x%04X)\r\n", hMenu, position, flags);
}

HRESULT ContextMenu::GetActionContext(ifc_viewaction *action, Dispatchable **context)
{
	HRESULT hr;
	GUID contextId;
	
	if (NULL == context)
		return E_POINTER;

	*context = NULL;

	if (NULL == action)
		return E_INVALIDARG;
	
	hr = action->GetContextId(&contextId);
	if (FAILED(hr))
		return hr;
				
	if (FALSE != IsEqualGUID(GUID_NULL, contextId))
	{
		*context = NULL;
	}
	else if (FALSE != IsEqualGUID(IFC_ViewActionContextEnum, contextId))
	{
		*context = contextEnum;
		contextEnum->AddRef();
	}
	else
	{
		hr = contextEnum->Find(&contextId, context);
		if (S_FALSE == hr)
			hr = E_INVALIDARG;
	}

	return hr;
}

HRESULT ContextMenu::Execute(ifc_viewmenuitem *item, HWND ownerWindow)
{
	HRESULT hr;
	Dispatchable *context;
	ifc_viewaction *action;
	
	if (NULL == item)
		return E_INVALIDARG;

	hr = item->GetAction(&action);
	if(S_OK != hr)
		return hr;
		
	hr = GetActionContext(action, &context);
	if (SUCCEEDED(hr))
	{
		hr = action->Execute(context, item, ownerWindow);

		if (NULL != context)
			context->Release();
	}

	action->Release();

	return hr;
}