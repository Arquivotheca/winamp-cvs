#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_TRANSACTION_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_TRANSACTION_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {E94785D0-B8B2-45d8-B03E-194CB48050C4}
static const GUID IFC_ViewSelectionTransaction = 
{ 0xe94785d0, 0xb8b2, 0x45d8, { 0xb0, 0x3e, 0x19, 0x4c, 0xb4, 0x80, 0x50, 0xc4 } };


#include <bfc/dispatch.h>


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewselectiontransaction : public Dispatchable
{

protected:
	ifc_viewselectiontransaction() {}
	~ifc_viewselectiontransaction() {}

public:
	HRESULT Set(size_t first, size_t last);
	HRESULT Add(size_t first, size_t last);
	HRESULT Remove(size_t first, size_t last);
	HRESULT RemoveAll();
	
	HRESULT Commit();
	
	
public:
	DISPATCH_CODES
	{
		API_SET = 10,
		API_ADD = 20,
		API_REMOVE = 30,
		API_REMOVEALL = 40,
		API_COMMIT = 50,
	};
};

inline HRESULT ifc_viewselectiontransaction::Set(size_t first, size_t last)
{
	return _call(API_SET, (HRESULT)E_NOTIMPL, first, last);
}

inline HRESULT ifc_viewselectiontransaction::Add(size_t first, size_t last)
{
	return _call(API_ADD, (HRESULT)E_NOTIMPL, first, last);
}

inline HRESULT ifc_viewselectiontransaction::Remove(size_t first, size_t last)
{
	return _call(API_REMOVE, (HRESULT)E_NOTIMPL, first, last);
}

inline HRESULT ifc_viewselectiontransaction::RemoveAll()
{
	return _call(API_REMOVEALL, (HRESULT)E_NOTIMPL);
}
	
inline HRESULT ifc_viewselectiontransaction::Commit()
{
	return _call(API_COMMIT, (HRESULT)E_NOTIMPL);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_TRANSACTION_INTERFACE_HEADER