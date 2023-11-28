#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_CONTEXT_ENUM_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_CONTEXT_ENUM_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {11EC1F16-FCCB-4e14-A948-5DE0F99ED73C}
static const GUID IFC_ViewActionContextEnum = 
{ 0x11ec1f16, 0xfccb, 0x4e14, { 0xa9, 0x48, 0x5d, 0xe0, 0xf9, 0x9e, 0xd7, 0x3c } };


#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewactioncontextenum : public Dispatchable
{

protected:
	ifc_viewactioncontextenum() {}
	~ifc_viewactioncontextenum() {}

public:
	HRESULT Next(Dispatchable **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);
	HRESULT Find(const GUID *contextId, Dispatchable **instance);
		
public:
	DISPATCH_CODES
	{
		API_NEXT = 10,
		API_RESET = 20,
		API_SKIP = 30,
		API_GETCOUNT = 40,
		API_FIND = 50,
	};
};

inline HRESULT ifc_viewactioncontextenum::Next(Dispatchable **buffer, size_t bufferMax, size_t *fetched)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, fetched);
}

inline HRESULT ifc_viewactioncontextenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewactioncontextenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}

inline HRESULT ifc_viewactioncontextenum::GetCount(size_t *count)
{
	return _call(API_GETCOUNT, (HRESULT)E_NOTIMPL, count);
}


inline HRESULT ifc_viewactioncontextenum::Find(const GUID *contextId, Dispatchable **instance)
{
	return _call(API_FIND, (HRESULT)E_NOTIMPL, contextId, instance);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_CONTEXT_ENUM_INTERFACE_HEADER