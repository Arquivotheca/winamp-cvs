#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_CHAIN_EVENT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_CHAIN_EVENT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {E5BA8A1A-B017-490e-8698-AE259F20F9AA}
static const GUID IFC_ViewFilterChainEvent = 
{ 0xe5ba8a1a, 0xb017, 0x490e, { 0x86, 0x98, 0xae, 0x25, 0x9f, 0x20, 0xf9, 0xaa } };

#include <bfc/dispatch.h>

class ifc_viewfilter;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewfilterchainevent : public Dispatchable
{

protected:
	ifc_viewfilterchainevent() {}
	~ifc_viewfilterchainevent() {}

public:
	void FilterChainEvent_Inserted(ifc_viewfilter *filter);
	void FilterChainEvent_Removed(ifc_viewfilter *filter);
		
public:
	DISPATCH_CODES
	{
		API_FILTERCHAINEVENT_INSERTED = 10,
		API_FILTERCHAINEVENT_REMOVED = 20,
		
	};
};

inline void ifc_viewfilterchainevent::FilterChainEvent_Inserted(ifc_viewfilter *filter)
{
	_voidcall(API_FILTERCHAINEVENT_INSERTED, filter);
}

inline void ifc_viewfilterchainevent::FilterChainEvent_Removed(ifc_viewfilter *filter)
{
	_voidcall(API_FILTERCHAINEVENT_REMOVED, filter);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_CHAIN_EVENT_INTERFACE_HEADER