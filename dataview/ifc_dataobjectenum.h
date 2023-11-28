#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_ENUMERATOR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {E1EA9F18-2592-4207-B24E-09470C51A04D}
static const GUID IFC_DataObjectEnum = 
{ 0xe1ea9f18, 0x2592, 0x4207, { 0xb2, 0x4e, 0x9, 0x47, 0xc, 0x51, 0xa0, 0x4d } };

#include <bfc/dispatch.h>

class ifc_dataobject;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_dataobjectenum : public Dispatchable
{
protected:
	ifc_dataobjectenum() {}
	~ifc_dataobjectenum() {}

public:
	HRESULT Next(ifc_dataobject **buffer, size_t bufferMax, size_t *fetched);
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

inline HRESULT ifc_dataobjectenum::Next(ifc_dataobject **buffer, size_t bufferMax, size_t *fetched)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, fetched);
}

inline HRESULT ifc_dataobjectenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_dataobjectenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}

inline HRESULT ifc_dataobjectenum::GetCount(size_t *count)
{
	return _call(API_GETCOUNT, (HRESULT)E_NOTIMPL, count);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_ENUMERATOR_INTERFACE_HEADER