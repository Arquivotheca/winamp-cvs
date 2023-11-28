#ifndef _NULLSOFT_WINAMP_DATAVIEW_MENU_ITEM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_MENU_ITEM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewmenuitem.h"
#include "../nu/ptrlist.h"


class MenuItem : public ifc_viewmenuitem
{

protected:
	MenuItem(const char *name, MenuStyle style, const wchar_t *displayName, 
			 const wchar_t *description, ifc_viewaction *action, MenuState state);
	~MenuItem();

public:
	static HRESULT CreateInstance(const char *name,
								  MenuStyle style,
								  const wchar_t *displayName,
								  const wchar_t *description,
								  ifc_viewaction *action,
								  MenuState state,
								  MenuItem **instance);

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
	HRESULT RegisterEventHandler(ifc_viewmenuitemevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewmenuitemevent *eventHandler);

protected:
	typedef nu::PtrList<ifc_viewmenuitemevent> EventHandlerList;
	
protected:
	size_t ref;
	char *name;
	MenuStyle style;
	wchar_t *displayName;
	wchar_t *description;
	ifc_viewaction *action;
	MenuState state;
	EventHandlerList eventHandlerList;
	
protected:
	RECVS_DISPATCH;
};


#define MenuGroup_InsertItem(_group, _position, _name, _displayName, _type, _state, _action)\
		{ MenuItem *_item;\
		  if (SUCCEEDED(MenuItem::CreateInstance((_name), (_type), (_displayName), NULL, (_action), (_state), &_item)))\
		  {	(_group)->Insert((_position), _item);\
		  _item->Release();}}


#endif //_NULLSOFT_WINAMP_DATAVIEW_MENU_COMMAND_ITEM_HEADER