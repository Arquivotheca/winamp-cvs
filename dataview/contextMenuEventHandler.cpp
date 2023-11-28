#include "main.h"
#include "./contextMenu.h"
#include "./contextMenuEventHandler.h"


ContextMenuEventHandler::ContextMenuEventHandler()
	: contextMenu(NULL)
{
}

ContextMenuEventHandler::~ContextMenuEventHandler()
{

}

size_t ContextMenuEventHandler::AddRef()
{
	return 1;
}

size_t ContextMenuEventHandler::Release()
{
	return 1;
}

int ContextMenuEventHandler::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewMenuItemEvent))
		*object = static_cast<ifc_viewmenuitemevent*>(this);
	else if (IsEqualIID(interface_guid, IFC_ViewMenuGroupEvent))
		*object = static_cast<ifc_viewmenugroupevent*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT ContextMenuEventHandler::Init(ContextMenu *_contextMenu)
{
	contextMenu = _contextMenu;
	return S_OK;
}

HRESULT ContextMenuEventHandler::RegisterItem(ifc_viewmenuitem *item)
{
	HRESULT hr;
	
	if (NULL == item)
		return E_INVALIDARG;
	
	hr = item->RegisterEventHandler(this);
	if (SUCCEEDED(hr))
	{
		ifc_viewmenugroup *group;
		if (SUCCEEDED(item->QueryInterface(IFC_ViewMenuGroup, (void**)&group)))
		{
			hr = group->RegisterEventHandler(this);
			group->Release();
		}
	}
	
	return hr;
}

HRESULT ContextMenuEventHandler::RegisterItem(ifc_viewmenugroup *group)
{
	HRESULT hr;
	ifc_viewmenuitem *item;

	if (NULL == group)
		return E_INVALIDARG;
	
	if (SUCCEEDED(group->QueryInterface(IFC_ViewMenuItem, (void**)&item)))
	{
		hr = RegisterItem(item);
		item->Release();
	}
	else
		hr = group->RegisterEventHandler(this);

	return hr;
}

HRESULT ContextMenuEventHandler::UnregisterItem(ifc_viewmenuitem *item)
{
	HRESULT hr;
	
	if (NULL == item)
		return E_INVALIDARG;
	
	hr = item->UnregisterEventHandler(this);
	if (SUCCEEDED(hr))
	{
		ifc_viewmenugroup *group;
		if (SUCCEEDED(item->QueryInterface(IFC_ViewMenuGroup, (void**)&group)))
		{
			hr = group->UnregisterEventHandler(this);
			group->Release();
		}
	}
	
	return hr;
}

HRESULT ContextMenuEventHandler::UnregisterItem(ifc_viewmenugroup *group)
{
	HRESULT hr;
	ifc_viewmenuitem *item;

	if (NULL == group)
		return E_INVALIDARG;
	
	if (SUCCEEDED(group->QueryInterface(IFC_ViewMenuItem, (void**)&item)))
	{
		hr = UnregisterItem(item);
		item->Release();
	}
	else
		hr = group->UnregisterEventHandler(this);

	return hr;
}

void ContextMenuEventHandler::MenuItemEvent_DisplayNameChanged(ifc_viewmenuitem *instance, const wchar_t *displayName)
{
}

void ContextMenuEventHandler::MenuItemEvent_DescriptionChanged(ifc_viewmenuitem *instance, const wchar_t *description)
{
}

void ContextMenuEventHandler::MenuItemEvent_IconChanged(ifc_viewmenuitem *instance)
{
}

void ContextMenuEventHandler::MenuItemEvent_StateChanged(ifc_viewmenuitem *instance, MenuState oldState, MenuState newState)
{
}

void ContextMenuEventHandler::MenuGroupEvent_ItemAdded(ifc_viewmenugroup *instance, ifc_viewmenuitem *item, size_t position)
{
	contextMenu->InsertItem(position, TRUE, item);
}

void ContextMenuEventHandler::MenuGroupEvent_ItemRemoved(ifc_viewmenugroup *instance, ifc_viewmenuitem *item, size_t position)
{
	if (NULL != item)
		contextMenu->RemoveItem(item->GetName(), position);
}

#define CBCLASS ContextMenuEventHandler
START_MULTIPATCH
	START_PATCH(MPIID_VIEWMENUITEM_EVENT)
		M_CB(MPIID_VIEWMENUITEM_EVENT, ifc_viewmenuitemevent, ADDREF, AddRef)
		M_CB(MPIID_VIEWMENUITEM_EVENT, ifc_viewmenuitemevent, RELEASE, Release)
		M_CB(MPIID_VIEWMENUITEM_EVENT, ifc_viewmenuitemevent, QUERYINTERFACE, QueryInterface)
		M_VCB(MPIID_VIEWMENUITEM_EVENT, ifc_viewmenuitemevent, API_MENUITEMEVENT_DISPLAYNAMECHANGED, MenuItemEvent_DisplayNameChanged)
		M_VCB(MPIID_VIEWMENUITEM_EVENT, ifc_viewmenuitemevent, API_MENUITEMEVENT_DESCRIPTIONCHANGED, MenuItemEvent_DescriptionChanged)
		M_VCB(MPIID_VIEWMENUITEM_EVENT, ifc_viewmenuitemevent, API_MENUITEMEVENT_ICONCHANGED, MenuItemEvent_IconChanged)
		M_VCB(MPIID_VIEWMENUITEM_EVENT, ifc_viewmenuitemevent, API_MENUITEMEVENT_STATECHANGED, MenuItemEvent_StateChanged)
	NEXT_PATCH(MPIID_VIEWMENUGROUP_EVENT)
		M_CB(MPIID_VIEWMENUGROUP_EVENT, ifc_viewmenugroupevent, ADDREF, AddRef)
		M_CB(MPIID_VIEWMENUGROUP_EVENT, ifc_viewmenugroupevent, RELEASE, Release)
		M_CB(MPIID_VIEWMENUGROUP_EVENT, ifc_viewmenugroupevent, QUERYINTERFACE, QueryInterface)
		M_VCB(MPIID_VIEWMENUGROUP_EVENT, ifc_viewmenugroupevent, API_MENUGROUPEVENT_ITEMADDED, MenuGroupEvent_ItemAdded)
		M_VCB(MPIID_VIEWMENUGROUP_EVENT, ifc_viewmenugroupevent, API_MENUGROUPEVENT_ITEMREMOVED, MenuGroupEvent_ItemRemoved)
	END_PATCH
END_MULTIPATCH;