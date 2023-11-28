#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {5F0A7759-10E0-4647-BA85-B229F3340022}
static const GUID IFC_ViewFilter = 
{ 0x5f0a7759, 0x10e0, 0x4647, { 0xba, 0x85, 0xb2, 0x29, 0xf3, 0x34, 0x0, 0x22 } };


#include <bfc/dispatch.h>
#include "./ifc_viewfilterevent.h"
#include "./ifc_dataprovider.h"
#include "./ifc_dataobject.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewfilter : public Dispatchable
{

protected:
	ifc_viewfilter() {}
	~ifc_viewfilter() {}

public:
	const char *GetName();
	HRESULT Bind(ifc_dataprovider *provider);
	HRESULT Init(ifc_dataobjectlist *objectList); 
	HRESULT IsAllowed(size_t objectIndex);
	HRESULT Update();
	HRESULT RegisterEventHandler(ifc_viewfilterevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewfilterevent *eventHandler);
	
public:
	DISPATCH_CODES
	{
		API_GETNAME = 10,
		API_BIND = 20,
		API_INIT = 30,
		API_ISALLOWED = 40,
		API_UPDATE = 50,
		API_REGISTEREVENTHANDLER = 60,
		API_UNREGISTEREVENTHANDLER = 70,
	};
};

inline const char *ifc_viewfilter::GetName()
{
	return _call(API_GETNAME, (const char*)NULL);
}


inline HRESULT ifc_viewfilter::Bind(ifc_dataprovider *provider)
{
	return _call(API_BIND, (HRESULT)E_NOTIMPL, provider);
} 


inline HRESULT ifc_viewfilter::Init(ifc_dataobjectlist *objectList)
{
	return _call(API_INIT, (HRESULT)E_NOTIMPL, objectList);
} 

inline HRESULT ifc_viewfilter::IsAllowed(size_t objectIndex)
{
	return _call(API_ISALLOWED, (HRESULT)E_NOTIMPL, objectIndex);
}

inline HRESULT ifc_viewfilter::Update()
{
	return _call(API_UPDATE, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewfilter::RegisterEventHandler(ifc_viewfilterevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_viewfilter::UnregisterEventHandler(ifc_viewfilterevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_INTERFACE_HEADER