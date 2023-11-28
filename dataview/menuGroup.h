#ifndef _NULLSOFT_WINAMP_DATAVIEW_MENU_GROUP_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_MENU_GROUP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewmenugroup.h"

#include "../nu/ptrlist.h"

#include <bfc/multipatch.h>

#define MPIID_VIEWMENUITEM			10
#define MPIID_VIEWMENUGROUP			20

class MenuGroup : public MultiPatch<MPIID_VIEWMENUITEM, ifc_viewmenuitem>,
				  public MultiPatch<MPIID_VIEWMENUGROUP, ifc_viewmenugroup>
{

protected:
	MenuGroup(const char *name, const wchar_t *displayName, const wchar_t *description, ifc_viewaction *action);
	~MenuGroup();

public:
	static HRESULT CreateInstance(const char *name, 
								  const wchar_t *displayName,
								  const wchar_t *description,
								  ifc_viewaction *action,
								  MenuGroup **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewmenuitem */
	const char *GetName();
	HRESULT GetDisplayName(wchar_t *buffer, size_t bufferSize);
	HRESULT GetDescription(wchar_t *buffer, size_t bufferSize);
	HRESULT GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height);
	MenuStyle GetStyle();
	MenuState GetState();
	HRESULT SetState(MenuState state, MenuState mask);
	HRESULT GetAction(ifc_viewaction **action);
	HRESULT RegisterItemEventHandler(ifc_viewmenuitemevent *eventHandler);
	HRESULT UnregisterItemEventHandler(ifc_viewmenuitemevent *eventHandler);

	/* ifc_viewmenugroup */
	size_t GetCount();
	HRESULT Get(size_t position, ifc_viewmenuitem **item);
	size_t Locate(const char *name, size_t start);
	HRESULT Insert(size_t position, ifc_viewmenuitem *item);
	HRESULT Remove(size_t position);
	HRESULT RemoveAll();
	HRESULT RegisterGroupEventHandler(ifc_viewmenugroupevent *eventHandler);
	HRESULT UnregisterGroupEventHandler(ifc_viewmenugroupevent *eventHandler);

protected:
	void Notify_ItemAdded(ifc_viewmenuitem *item, size_t position);
	void Notify_ItemRemoved(ifc_viewmenuitem *item, size_t position);

protected:
	typedef nu::PtrList<ifc_viewmenuitem> ItemList;
	typedef nu::PtrList<ifc_viewmenuitemevent> ItemEventHandlerList;
	typedef nu::PtrList<ifc_viewmenugroupevent> GroupEventHandlerList;

protected:
	size_t ref;
	ItemList list;
	char *name;
	wchar_t *displayName;
	wchar_t *description;
	MenuState state;
	ifc_viewaction *action;
	ItemEventHandlerList itemEventHandlerList;
	GroupEventHandlerList groupEventHandlerList;

protected:
	RECVS_MULTIPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_MENU_GROUP_HEADER