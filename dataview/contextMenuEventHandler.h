#ifndef _NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_EVENT_HANDLER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_EVENT_HANDLER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewmenuitem.h"
#include "./ifc_viewmenugroup.h"

#include <bfc/multipatch.h>

class ContextMenu;

#define MPIID_VIEWMENUITEM_EVENT			10
#define MPIID_VIEWMENUGROUP_EVENT			20

class ContextMenuEventHandler :  public MultiPatch<MPIID_VIEWMENUITEM_EVENT, ifc_viewmenuitemevent>,
								 public MultiPatch<MPIID_VIEWMENUGROUP_EVENT, ifc_viewmenugroupevent>

{
public:
	ContextMenuEventHandler();
	~ContextMenuEventHandler();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);
	
protected:
	/* ifc_viewmenuitemevent */
	void MenuItemEvent_DisplayNameChanged(ifc_viewmenuitem *instance, const wchar_t *displayName);
	void MenuItemEvent_DescriptionChanged(ifc_viewmenuitem *instance, const wchar_t *description);
	void MenuItemEvent_IconChanged(ifc_viewmenuitem *instance);
	void MenuItemEvent_StateChanged(ifc_viewmenuitem *instance, MenuState oldState, MenuState newState);

	/* ifc_viewmenugroupevent */
	void MenuGroupEvent_ItemAdded(ifc_viewmenugroup *instance, ifc_viewmenuitem *item, size_t position);
	void MenuGroupEvent_ItemRemoved(ifc_viewmenugroup *instance, ifc_viewmenuitem *item, size_t position);

public:
	HRESULT Init(ContextMenu *contextMenu);
	HRESULT RegisterItem(ifc_viewmenuitem *item);
	HRESULT RegisterItem(ifc_viewmenugroup *group);
	HRESULT UnregisterItem(ifc_viewmenuitem *item);
	HRESULT UnregisterItem(ifc_viewmenugroup *group);

protected:
	ContextMenu *contextMenu;

protected:
	RECVS_MULTIPATCH;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_CONTEXT_MENU_EVENT_HANDLER_HEADER
