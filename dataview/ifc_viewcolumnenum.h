#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_ENUM_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_ENUM_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {9B889397-0601-40fc-9C9A-FAFA51C4CFCD}
static const GUID IFC_ViewColumnEnum = 
{ 0x9b889397, 0x601, 0x40fc, { 0x9c, 0x9a, 0xfa, 0xfa, 0x51, 0xc4, 0xcf, 0xcd } };


#include <bfc/dispatch.h>
#include "./ifc_viewcolumn.h"


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcolumnenum : public Dispatchable
{

protected:
	ifc_viewcolumnenum() {}
	~ifc_viewcolumnenum() {}

public:
	HRESULT Next(ifc_viewcolumn **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

public:
	DISPATCH_CODES
	{
		API_NEXT = 10,
		API_RESET = 20,
		API_SKIP = 30,
		API_GETCOUNT = 40,
	};
};

inline HRESULT ifc_viewcolumnenum::Next(ifc_viewcolumn **buffer, size_t bufferMax, size_t *fetched)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, fetched);
}

inline HRESULT ifc_viewcolumnenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewcolumnenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}

inline HRESULT ifc_viewcolumnenum::GetCount(size_t *count)
{
	return _call(API_GETCOUNT, (HRESULT)E_NOTIMPL, count);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_ENUM_INTERFACE_HEADER