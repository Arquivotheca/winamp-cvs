#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_GROUP_FILTER_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_GROUP_FILTER_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {55DD212F-85A1-40ed-96E6-A44F4E18DBF7}
static const GUID IFC_ViewGroupFilterEvent = 
{ 0x55dd212f, 0x85a1, 0x40ed, { 0x96, 0xe6, 0xa4, 0x4f, 0x4e, 0x18, 0xdb, 0xf7 } };

#include <bfc/dispatch.h>

class ifc_viewgroupfilter;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewgroupfilterevent : public Dispatchable
{

protected:
	ifc_viewgroupfilterevent() {}
	~ifc_viewgroupfilterevent() {}

public:
	void GroupFilterEvent_BypassModeChanged(ifc_viewgroupfilter *instance, BOOL bypassEnabled);
	
public:
	DISPATCH_CODES
	{
		API_GROUPFILTEREVENT_BYPASSMODECHANGED = 10,
	};
};

inline void ifc_viewgroupfilterevent::GroupFilterEvent_BypassModeChanged(ifc_viewgroupfilter *instance, BOOL bypassEnabled)
{
	_voidcall(API_GROUPFILTEREVENT_BYPASSMODECHANGED, instance, bypassEnabled);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_GROUP_FILTER_EVENT_INTERFACE_HEADER