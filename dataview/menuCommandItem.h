#ifndef _NULLSOFT_WINAMP_DATAVIEW_MENU_COMMAND_ITEM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_MENU_COMMAND_ITEM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewmenuitem.h"
#include "./ifc_viewcommand.h"
#include "../nu/ptrlist.h"


class MenuCommandItem : public ifc_viewmenuitem
{

protected:
	MenuCommandItem(ifc_viewcommand *command, MenuStyle style, MenuState state);
	~MenuCommandItem();

public:
	static HRESULT CreateInstance(ifc_viewcommand *command,
								  MenuStyle style,
								  MenuState state,
								  MenuCommandItem **instance);

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
	ifc_viewcommand *command;
	MenuStyle style;
	MenuState state;
	EventHandlerList eventHandlerList;
	
protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_MENU_COMMAND_ITEM_HEADER