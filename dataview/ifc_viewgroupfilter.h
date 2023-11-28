#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_GROUP_FILTER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_GROUP_FILTER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {D76C91AA-834A-4c9b-9B15-5824D791BEA7}
static const GUID IFC_ViewGroupFilter = 
{ 0xd76c91aa, 0x834a, 0x4c9b, { 0x9b, 0x15, 0x58, 0x24, 0xd7, 0x91, 0xbe, 0xa7 } };


#include <bfc/dispatch.h>
#include "./ifc_dataobjectlist.h"
#include "./ifc_viewgroupfilterevent.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewgroupfilter : public Dispatchable
{

protected:
	ifc_viewgroupfilter() {}
	~ifc_viewgroupfilter() {}

public:
	HRESULT GetProvider(ifc_dataprovider **provider);
	HRESULT GetObjects(ifc_dataobjectlist **list);
	HRESULT GetGroupProvider(ifc_groupprovider **groupProvider);
	HRESULT GetSummaryObject(ifc_dataobject **object);
	HRESULT UpdateSelection(ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed);
	size_t GetGroupId(size_t objectIndex, BOOL ignoreUnknownGroup);
	HRESULT EnableBypass(BOOL enable);
	HRESULT IsBypassEnabled();
	HRESULT RegisterEventHandler(ifc_viewgroupfilterevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewgroupfilterevent *eventHandler);

public:
	DISPATCH_CODES
	{
		API_GETPROVIDER = 10,
		API_GETOBJECTS = 20,
		API_GETGROUPPROVIDER = 30,
		API_GETSUMMARYOBJECT = 40,
		API_UPDATESELECTION = 50,
		API_GETGROUPID = 60,
		API_ENABLEBYPASS = 70,
		API_ISBYPASSENABLED = 80,
		API_REGISTEREVENTHANDLER = 90,
		API_UNREGISTEREVENTHANDLER = 100,
	};
};

inline HRESULT ifc_viewgroupfilter::GetProvider(ifc_dataprovider **provider)
{
	return _call(API_GETPROVIDER, (HRESULT)E_NOTIMPL, provider);
} 

inline HRESULT ifc_viewgroupfilter::GetObjects(ifc_dataobjectlist **list)
{
	return _call(API_GETOBJECTS, (HRESULT)E_NOTIMPL, list);
} 

inline HRESULT ifc_viewgroupfilter::GetGroupProvider(ifc_groupprovider **groupProvider)
{
	return _call(API_GETGROUPPROVIDER, (HRESULT)E_NOTIMPL, groupProvider);
} 

inline HRESULT ifc_viewgroupfilter::GetSummaryObject(ifc_dataobject **object)
{
	return _call(API_GETSUMMARYOBJECT, (HRESULT)E_NOTIMPL, object);
}

inline HRESULT ifc_viewgroupfilter::UpdateSelection(ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed)
{
	return _call(API_UPDATESELECTION, (HRESULT)E_NOTIMPL, selection, appended, removed);
}

inline size_t ifc_viewgroupfilter::GetGroupId(size_t objectIndex, BOOL ignoreUnknownGroup)
{
	return _call(API_GETGROUPID, (size_t)-1, objectIndex, ignoreUnknownGroup);
}

inline HRESULT ifc_viewgroupfilter::EnableBypass(BOOL enable)
{
	return _call(API_ENABLEBYPASS, (HRESULT)E_NOTIMPL, enable);
}

inline HRESULT ifc_viewgroupfilter::IsBypassEnabled()
{
	return _call(API_ISBYPASSENABLED, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewgroupfilter::RegisterEventHandler(ifc_viewgroupfilterevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_viewgroupfilter::UnregisterEventHandler(ifc_viewgroupfilterevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_GROUP_FILTER_INTERFACE_HEADER