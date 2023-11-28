#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {42538003-AE0D-4f3c-8393-935A4435C81D}
static const GUID IFC_ViewAction = 
{ 0x42538003, 0xae0d, 0x4f3c, { 0x83, 0x93, 0x93, 0x5a, 0x44, 0x35, 0xc8, 0x1d } };


#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewaction: public Dispatchable
{

protected:
	ifc_viewaction() {}
	~ifc_viewaction() {}

public:
	HRESULT GetContextId(GUID *contextId);
	HRESULT Execute(Dispatchable *context, Dispatchable *source, HWND hostWindow);
	
public:
	DISPATCH_CODES
	{
		API_GETCONTEXTID = 10,
		API_EXECUTE = 20,
	};
};

inline HRESULT ifc_viewaction::GetContextId(GUID *contextId)
{
	return _call(API_GETCONTEXTID, (HRESULT)E_NOTIMPL, contextId);
}

inline HRESULT ifc_viewaction::Execute(Dispatchable *context, Dispatchable *source, HWND hostWindow)
{
	return _call(API_EXECUTE, (HRESULT)E_NOTIMPL, context, source, hostWindow);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_INTERFACE_HEADER