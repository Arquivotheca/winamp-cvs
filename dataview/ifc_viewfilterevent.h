#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {1C78D417-97B0-4ff4-93DE-8CEDA1BFA946}
static const GUID IFC_ViewFilterEvent = 
{ 0x1c78d417, 0x97b0, 0x4ff4, { 0x93, 0xde, 0x8c, 0xed, 0xa1, 0xbf, 0xa9, 0x46 } };

#include <bfc/dispatch.h>

class ifc_viewfilter;

typedef enum ViewFilterAction
{
	ViewFilterAction_Block  = 0,
	ViewFilterAction_Allow = 1,
} ViewFilterAction;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewfilterevent : public Dispatchable
{

protected:
	ifc_viewfilterevent() {}
	~ifc_viewfilterevent() {}

public:
	void FilterEvent_BeginUpdate(ifc_viewfilter *instance);
	void FilterEvent_EndUpdate(ifc_viewfilter *instance);
	void FilterEvent_BlockAll(ifc_viewfilter *instance);
	void FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action);
	
public:
	DISPATCH_CODES
	{
		API_FILTEREVENT_BEGINUPDATE = 10,
		API_FILTEREVENT_ENDUPDATE = 20,
		API_FILTEREVENT_BLOCKALL = 30,
		API_FILTEREVENT_ACTIONCHANGED = 40,
	};
};

inline void ifc_viewfilterevent::FilterEvent_BeginUpdate(ifc_viewfilter *instance)
{
	_voidcall(API_FILTEREVENT_BEGINUPDATE, instance);
}

inline void ifc_viewfilterevent::FilterEvent_EndUpdate(ifc_viewfilter *instance)
{
	_voidcall(API_FILTEREVENT_ENDUPDATE, instance);
}

inline void ifc_viewfilterevent::FilterEvent_BlockAll(ifc_viewfilter *instance)
{
	_voidcall(API_FILTEREVENT_BLOCKALL, instance);
}

inline void ifc_viewfilterevent::FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action)
{
	_voidcall(API_FILTEREVENT_ACTIONCHANGED, instance, objectIndex, count, action);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_EVENT_INTERFACE_HEADER