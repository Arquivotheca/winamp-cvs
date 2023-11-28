#ifndef _NULLSOFT_WINAMP_DATAVIEW_GROUP_ENUM_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GROUP_ENUM_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {29CEFC5B-E2D7-4237-BCB2-BCAC52F01D18}
static const GUID IFC_GroupEnum = 
{ 0x29cefc5b, 0xe2d7, 0x4237, { 0xbc, 0xb2, 0xbc, 0xac, 0x52, 0xf0, 0x1d, 0x18 } };


#include <bfc/dispatch.h>
#include "./ifc_groupprovider.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_groupenum : public Dispatchable
{

protected:
	ifc_groupenum() {}
	~ifc_groupenum() {}

public:
	HRESULT Next(ifc_groupprovider **buffer, size_t bufferMax, size_t *fetched);
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

inline HRESULT ifc_groupenum::Next(ifc_groupprovider **buffer, size_t bufferMax, size_t *fetched)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, fetched);
}

inline HRESULT ifc_groupenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_groupenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}

inline HRESULT ifc_groupenum::GetCount(size_t *count)
{
	return _call(API_GETCOUNT, (HRESULT)E_NOTIMPL, count);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_ENUM_INTERFACE_HEADER