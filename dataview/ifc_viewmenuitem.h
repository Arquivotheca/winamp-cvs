#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_ITEM_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_ITEM_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {758210F4-2126-4e71-AB5B-129E96F9CE12}
static const GUID IFC_ViewMenuItem = 
{ 0x758210f4, 0x2126, 0x4e71, { 0xab, 0x5b, 0x12, 0x9e, 0x96, 0xf9, 0xce, 0x12 } };


#include <bfc/dispatch.h>

#include "./ifc_viewmenuitemevent.h"
#include "./ifc_viewaction.h"

typedef enum MenuStyle
{
	MenuStyle_Normal = 0,
	MenuStyle_BarBreak = (1 << 0),
	MenuStyle_Break = (1 << 1),
	MenuStyle_RadioCheck = (1 << 2),
	MenuStyle_Separator = (1 << 3),
	MenuStyle_Rating = (1 << 4),
} MenuStyle;
DEFINE_ENUM_FLAG_OPERATORS(MenuStyle)

typedef enum MenuState
{
	MenuState_Normal = 0, 
	MenuState_Disabled = (1 << 0),
	MenuState_Checked = (1 << 1)
} MenuState;
DEFINE_ENUM_FLAG_OPERATORS(MenuState)

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewmenuitem : public Dispatchable
{

protected:
	ifc_viewmenuitem() {}
	~ifc_viewmenuitem() {}

public:
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
	
public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_GETDISPLAYNAME = 20,
		API_GETDESCRIPTION = 30,
		API_GETICON = 40,
		API_GETSTYLE = 50,
		API_GETSTATE = 60,
		API_SETSTATE = 70,
		API_GETACTION = 80,
		API_REGISTEREVENTHANDLER = 90,
		API_UNREGISTEREVENTHANDLER = 100,
	};
};

inline const char *ifc_viewmenuitem::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}

inline HRESULT ifc_viewmenuitem::GetDisplayName(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDISPLAYNAME, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_viewmenuitem::GetDescription(wchar_t *buffer, size_t bufferSize)
{
	return _call(API_GETDESCRIPTION, (HRESULT)E_NOTIMPL, buffer, bufferSize);
}

inline HRESULT ifc_viewmenuitem::GetIcon(wchar_t *buffer, size_t bufferSize, int width, int height)
{
	return _call(API_GETICON, (HRESULT)E_NOTIMPL, buffer, bufferSize, width, height);
}

inline MenuStyle ifc_viewmenuitem::GetStyle()
{
	return _call(API_GETSTYLE, (MenuStyle)MenuStyle_Normal);
}

inline MenuState ifc_viewmenuitem::GetState()
{
	return _call(API_GETSTATE, (MenuState)MenuState_Normal);
}

inline HRESULT ifc_viewmenuitem::SetState(MenuState state, MenuState mask)
{
	return _call(API_SETSTATE, (HRESULT)E_NOTIMPL, state, mask);
}

inline HRESULT ifc_viewmenuitem::GetAction(ifc_viewaction **action)
{
	return _call(API_GETACTION, (HRESULT)E_NOTIMPL, action);
}

inline HRESULT ifc_viewmenuitem::RegisterEventHandler(ifc_viewmenuitemevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_viewmenuitem::UnregisterEventHandler(ifc_viewmenuitemevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_ITEM_INTERFACE_HEADER