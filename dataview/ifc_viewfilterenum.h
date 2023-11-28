#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_ENUMERATOR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>


// {B41F04CD-390A-4f12-939D-78305D1BED37}
static const GUID IFC_ViewFilterEnum = 
{ 0xb41f04cd, 0x390a, 0x4f12, { 0x93, 0x9d, 0x78, 0x30, 0x5d, 0x1b, 0xed, 0x37 } };


#include <bfc/dispatch.h>

class ifc_viewfilter;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewfilterenum : public Dispatchable
{
protected:
	ifc_viewfilterenum() {}
	~ifc_viewfilterenum() {}

public:
	HRESULT Next(ifc_viewfilter **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);

public:
	DISPATCH_CODES
	{
		API_NEXT = 10,
		API_RESET = 20,
		API_SKIP = 30,
	};
};

inline HRESULT ifc_viewfilterenum::Next(ifc_viewfilter **buffer, size_t bufferMax, size_t *fetched)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, fetched);
}

inline HRESULT ifc_viewfilterenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewfilterenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_ENUMERATOR_INTERFACE_HEADER