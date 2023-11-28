#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_ITEM_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_ITEM_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {A9E4DEA0-CBCA-42b4-AF0E-9F7B20531958}
static const GUID IFC_ViewMenuItemEvent = 
{ 0xa9e4dea0, 0xcbca, 0x42b4, { 0xaf, 0xe, 0x9f, 0x7b, 0x20, 0x53, 0x19, 0x58 } };


#include <bfc/dispatch.h>

class ifc_viewmenuitem;
typedef enum MenuState MenuState;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewmenuitemevent : public Dispatchable
{
protected:
	ifc_viewmenuitemevent() {}
	~ifc_viewmenuitemevent() {}

public:
	void MenuItemEvent_DisplayNameChanged(ifc_viewmenuitem *instance, const wchar_t *displayName);
	void MenuItemEvent_DescriptionChanged(ifc_viewmenuitem *instance, const wchar_t *description);
	void MenuItemEvent_IconChanged(ifc_viewmenuitem *instance);
	void MenuItemEvent_StateChanged(ifc_viewmenuitem *instance, MenuState oldState, MenuState newState);
		
public:
	DISPATCH_CODES
	{
		API_MENUITEMEVENT_DISPLAYNAMECHANGED = 10,
		API_MENUITEMEVENT_DESCRIPTIONCHANGED = 20,
		API_MENUITEMEVENT_ICONCHANGED = 30,
		API_MENUITEMEVENT_STATECHANGED = 40,
	};
};

inline void ifc_viewmenuitemevent::MenuItemEvent_DisplayNameChanged(ifc_viewmenuitem *instance, const wchar_t *displayName)
{
	_voidcall(API_MENUITEMEVENT_DISPLAYNAMECHANGED, instance, displayName);
}

inline void ifc_viewmenuitemevent::MenuItemEvent_DescriptionChanged(ifc_viewmenuitem *instance, const wchar_t *description)
{
	_voidcall(API_MENUITEMEVENT_DESCRIPTIONCHANGED, instance, description);
}

inline void ifc_viewmenuitemevent::MenuItemEvent_IconChanged(ifc_viewmenuitem *instance)
{
	_voidcall(API_MENUITEMEVENT_ICONCHANGED, instance);
}

inline void ifc_viewmenuitemevent::MenuItemEvent_StateChanged(ifc_viewmenuitem *instance, MenuState oldState, MenuState newState)
{
	_voidcall(API_MENUITEMEVENT_STATECHANGED, instance, oldState, newState);
}



#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_ITEM_EVENT_INTERFACE_HEADER