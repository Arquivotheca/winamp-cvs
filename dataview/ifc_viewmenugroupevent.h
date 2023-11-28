#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_GROUP_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_GROUP_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {AD55F13B-6CC1-4c04-937D-F69DA2C55CFA}
static const GUID IFC_ViewMenuGroupEvent = 
{ 0xad55f13b, 0x6cc1, 0x4c04, { 0x93, 0x7d, 0xf6, 0x9d, 0xa2, 0xc5, 0x5c, 0xfa } };

#include <bfc/dispatch.h>

class ifc_viewmenugroup;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewmenugroupevent : public Dispatchable
{
protected:
	ifc_viewmenugroupevent() {}
	~ifc_viewmenugroupevent() {}

public:
	void MenuGroupEvent_ItemAdded(ifc_viewmenugroup *instance, ifc_viewmenuitem *item, size_t position);
	void MenuGroupEvent_ItemRemoved(ifc_viewmenugroup *instance, ifc_viewmenuitem *item, size_t position);
			
public:
	DISPATCH_CODES
	{
		API_MENUGROUPEVENT_ITEMADDED = 10,
		API_MENUGROUPEVENT_ITEMREMOVED = 20,
	};
};

inline void ifc_viewmenugroupevent::MenuGroupEvent_ItemAdded(ifc_viewmenugroup *instance, ifc_viewmenuitem *item, size_t position)
{
	_voidcall(API_MENUGROUPEVENT_ITEMADDED, instance, item, position);
}

inline void ifc_viewmenugroupevent::MenuGroupEvent_ItemRemoved(ifc_viewmenugroup *instance, ifc_viewmenuitem *item, size_t position)
{
	_voidcall(API_MENUGROUPEVENT_ITEMREMOVED, instance, item, position);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_MENU_GROUP_EVENT_INTERFACE_HEADER