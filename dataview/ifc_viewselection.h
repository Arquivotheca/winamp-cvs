#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {BB500B9D-B221-4cc8-B173-FE4BFADCD75B}
static const GUID IFC_ViewSelection = 
{ 0xbb500b9d, 0xb221, 0x4cc8, { 0xb1, 0x73, 0xfe, 0x4b, 0xfa, 0xdc, 0xd7, 0x5b } };


#include <bfc/dispatch.h>
#include "./ifc_viewselectionenum.h"
#include "./ifc_viewselectiontransaction.h"
#include "./ifc_viewselectionevent.h"

#define SELECTION_E_WRITE_PROTECT  HRESULT_FROM_WIN32(ERROR_WRITE_PROTECT)

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewselection : public Dispatchable
{

protected:
	ifc_viewselection() {}
	~ifc_viewselection() {}

public:
	HRESULT Set(size_t first, size_t last);
	HRESULT Add(size_t first, size_t last);
	HRESULT Remove(size_t first, size_t last);
	HRESULT RemoveAll();
	HRESULT IsEditAllowed();

	size_t GetCount();
	HRESULT IsSelected(size_t index);
	HRESULT Enumerate(ifc_viewselectionenum **enumerator);
	
	HRESULT CreateTransaction(ifc_viewselectiontransaction **transaction);

	HRESULT RegisterEventHandler(ifc_viewselectionevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewselectionevent *eventHandler);
	
	
public:
	DISPATCH_CODES
	{
		API_SET = 10,
		API_ADD = 20,
		API_REMOVE = 30,
		API_REMOVEALL = 40,
		API_ISEDITALLOWED = 50,
		API_GETCOUNT = 60,
		API_ISSELECTED = 70,
		API_ENUMERATE = 80,
		API_CREATETRANSACTION = 90,
		API_REGISTEREVENTHANDLER = 100,
		API_UNREGISTEREVENTHANDLER = 110,
	};
};

inline HRESULT ifc_viewselection::Set(size_t first, size_t last)
{
	return _call(API_SET, (HRESULT)E_NOTIMPL, first, last);
}

inline HRESULT ifc_viewselection::Add(size_t first, size_t last)
{
	return _call(API_ADD, (HRESULT)E_NOTIMPL, first, last);
}

inline HRESULT ifc_viewselection::Remove(size_t first, size_t last)
{
	return _call(API_REMOVE, (HRESULT)E_NOTIMPL, first, last);
}

inline HRESULT ifc_viewselection::RemoveAll()
{
	return _call(API_REMOVEALL, (HRESULT)E_NOTIMPL);
}

inline HRESULT ifc_viewselection::IsEditAllowed()
{
	return _call(API_ISEDITALLOWED, (HRESULT)E_NOTIMPL);
}

inline size_t ifc_viewselection::GetCount()
{
	return _call(API_GETCOUNT, (size_t)0);
}

inline HRESULT ifc_viewselection::IsSelected(size_t index)
{
	return _call(API_ISSELECTED, (HRESULT)E_NOTIMPL, index);
}

inline HRESULT ifc_viewselection::Enumerate(ifc_viewselectionenum **enumerator)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}
	
inline HRESULT ifc_viewselection::CreateTransaction(ifc_viewselectiontransaction **transaction)
{
	return _call(API_CREATETRANSACTION, (HRESULT)E_NOTIMPL, transaction);
}

inline HRESULT ifc_viewselection::RegisterEventHandler(ifc_viewselectionevent *eventHandler)
{
	return _call(API_REGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

inline HRESULT ifc_viewselection::UnregisterEventHandler(ifc_viewselectionevent *eventHandler)
{
	return _call(API_UNREGISTEREVENTHANDLER, (HRESULT)E_NOTIMPL, eventHandler);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_INTERFACE_HEADER