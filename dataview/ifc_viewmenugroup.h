#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_GROUP_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_GROUP_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {C1149774-B584-4385-B46E-C3591AB9E715}
static const GUID IFC_ViewMenuGroup = 
{ 0xc1149774, 0xb584, 0x4385, { 0xb4, 0x6e, 0xc3, 0x59, 0x1a, 0xb9, 0xe7, 0x15 } };


#include <bfc/dispatch.h>

#include "./ifc_viewmenuitem.h"
#include "./ifc_viewmenugroupevent.h"

#define MENUGROUP_E_DUPLICATE_ITEM		HRESULT_FROM_WIN32(ERROR_DUP_NAME)

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewmenugroup : public Dispatchable
{
public:
	
protected:
	ifc_viewmenugroup() {}
	~ifc_viewmenugroup() {}

public:
	size_t GetCount();
	HRESULT Get(size_t position, ifc_viewmenuitem **item);

	size_t Locate(const char *name, size_t start);

	HRESULT Insert(size_t position, ifc_viewmenuitem *item);
	HRESULT Remove(size_t position);
	HRESULT RemoveAll();

	HRESULT RegisterEventHandler(ifc_viewmenugroupevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewmenugroupevent *eventHandler);
	
public:
	DISPATCH_CODES
	{
		API_GETCOUNT = 10,
		API_GET = 20,
		API_LOCATE = 30,
		API_INSERT = 40,
		API_REMOVE = 50,
		API_REMOVEALL = 60,
		API_REGISTEREVENTHANDLER = 70,
		API_UNREGISTEREVENTHANDLER = 80,
	};
};

inline size_t ifc_viewmenugroup::GetCount()
{
	return _call(API_GETCOUNT, (size_t)0);
}

inline HRESULT ifc_viewmenugroup::Get(size_t position, ifc_viewmenuitem **item)
{
	return _call(API_GET, (HRESULT)E_NOTIMPL, position, item);
}

inline size_t ifc_viewmenugroup::Locate(const char *name, size_t start)
{
	return _call(API_LOCATE, (size_t)-1, name, start);
}

inline HRESULT ifc_viewmenugroup::Insert(size_t position, ifc_viewmenuitem *item)
{
	return _call(API_INSERT, (HRESULT)E_NOTIMPL, position, item);
}

inline HRESULT ifc_viewmenugroup::Remove(size_t position)
{
	return _call(API_REMOVE, (HRESULT)E_NOTIMPL, position);
}

inline HRESULT ifc_viewmenugroup::RemoveAll()
{
	return _call(API_REMOVEALL, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewmenugroup::RegisterEventHandler(ifc_viewmenugroupevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_viewmenugroup::UnregisterEventHandler(ifc_viewmenugroupevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_ITEM_INTERFACE_HEADER