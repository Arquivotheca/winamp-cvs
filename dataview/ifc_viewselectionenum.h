#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_ENUMERATOR_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_ENUMERATOR_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {CA598D7E-999E-4ca5-9508-92725DB8E733}
static const GUID IFC_ViewSelectionEnum = 
{ 0xca598d7e, 0x999e, 0x4ca5, { 0x95, 0x8, 0x92, 0x72, 0x5d, 0xb8, 0xe7, 0x33 } };


#include <bfc/dispatch.h>

typedef struct IndexRange
{
	size_t first;
	size_t last;
} IndexRange;

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewselectionenum : public Dispatchable
{
protected:
	ifc_viewselectionenum() {}
	~ifc_viewselectionenum() {}

public:
	HRESULT Next(IndexRange *buffer, size_t bufferMax, size_t *fetched);
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

inline HRESULT ifc_viewselectionenum::Next(IndexRange *buffer, size_t bufferMax, size_t *fetched)
{
	return _call(API_NEXT, (HRESULT)E_NOTIMPL, buffer, bufferMax, fetched);
}

inline HRESULT ifc_viewselectionenum::Reset(void)
{
	return _call(API_RESET, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewselectionenum::Skip(size_t count)
{
	return _call(API_SKIP, (HRESULT)E_NOTIMPL, count);
}

inline HRESULT ifc_viewselectionenum::GetCount(size_t *count)
{
	return _call(API_GETCOUNT, (HRESULT)E_NOTIMPL, count);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_ENUMERATOR_INTERFACE_HEADER