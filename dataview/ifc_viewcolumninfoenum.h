#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INFO_ENUM_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INFO_ENUM_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {E1110956-B005-4ca8-BC7C-45648FE8DCD1}
static const GUID IFC_ViewColumnInfoEnum = 
{ 0xe1110956, 0xb005, 0x4ca8, { 0xbc, 0x7c, 0x45, 0x64, 0x8f, 0xe8, 0xdc, 0xd1 } };


#include <bfc/dispatch.h>
#include "./ifc_viewcolumninfo.h"


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcolumninfoenum : public Dispatchable
{

protected:
	ifc_viewcolumninfoenum() {}
	~ifc_viewcolumninfoenum() {}

public:
	HRESULT Next(ifc_viewcolumninfo **buffer, size_t bufferMax, size_t *fetched);
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

inline HRESULT ifc_viewcolumninfoenum::Next(ifc_viewcolumninfo **buffer, size_t bufferMax, size_t *fetched)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, fetched);
}

inline HRESULT ifc_viewcolumninfoenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewcolumninfoenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}

inline HRESULT ifc_viewcolumninfoenum::GetCount(size_t *count)
{
	return _call(API_GETCOUNT, (HRESULT)E_NOTIMPL, count);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INFO_ENUM_INTERFACE_HEADER