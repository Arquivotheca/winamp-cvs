#ifndef _NULLSOFT_WINAMP_DATAVIEW_DOUBLY_LINKED_NODE_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DOUBLY_LINKED_NODE_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {FB2B4B0F-297D-426b-A03E-1E1177743858}
static const GUID IFC_DoublyLinkedNode = 
{ 0xfb2b4b0f, 0x297d, 0x426b, { 0xa0, 0x3e, 0x1e, 0x11, 0x77, 0x74, 0x38, 0x58 } };


#include <bfc/dispatch.h>

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_doublylinkednode : public Dispatchable
{

protected:
	ifc_doublylinkednode() {}
	~ifc_doublylinkednode() {}

public:
	HRESULT SetPrevious(ifc_doublylinkednode *node);
	HRESULT SetNext(ifc_doublylinkednode *node);
	HRESULT GetPrevious(ifc_doublylinkednode **node);	// return S_FALSE if no previous node
	HRESULT GetNext(ifc_doublylinkednode **node);		// return S_FALSE if no next node
		
public:
	DISPATCH_CODES
	{
		API_SETPREVIOUS = 10,
		API_SETNEXT= 20,
		API_GETPREVIOUS = 30,
		API_GETNEXT = 40,
	};
};

inline HRESULT ifc_doublylinkednode::SetPrevious(ifc_doublylinkednode *node)
{
	return _call(API_SETPREVIOUS, (HRESULT)E_NOTIMPL, node);
}

inline HRESULT ifc_doublylinkednode::SetNext(ifc_doublylinkednode *node)
{
	return _call(API_SETNEXT, (HRESULT)E_NOTIMPL, node);
}

inline HRESULT ifc_doublylinkednode::GetPrevious(ifc_doublylinkednode **node)
{
	return _call(API_GETPREVIOUS, (HRESULT)E_NOTIMPL, node);
}

inline HRESULT ifc_doublylinkednode::GetNext(ifc_doublylinkednode **node)
{
	return _call(API_GETNEXT, (HRESULT)E_NOTIMPL, node);
}


#endif //_NULLSOFT_WINAMP_DATAVIEW_DOUBLY_LINKED_NODE_INTERFACE_HEADER