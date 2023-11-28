#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_MANAGER_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_MANAGER_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {1A9961BD-FC64-41c1-A3AD-4490A42D6966}
static const GUID IFC_ViewColumnManager = 
{ 0x1a9961bd, 0xfc64, 0x41c1, { 0xa3, 0xad, 0x44, 0x90, 0xa4, 0x2d, 0x69, 0x66 } };


#include <bfc/dispatch.h>

#include "./ifc_viewcolumninfoenum.h"

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcolumnmanager : public Dispatchable
{

protected:
	ifc_viewcolumnmanager() {}
	~ifc_viewcolumnmanager() {}

public:
	size_t Register(ifc_viewcolumninfo **columnInfo, size_t  count);
	HRESULT Unregister(const char *columnName);
	HRESULT Enumerate(ifc_viewcolumninfoenum **enumerator);
	HRESULT Find(const char *columnName, ifc_viewcolumninfo **columnInfo);

public:
	DISPATCH_CODES
	{
		API_REGISTER = 10,
		API_UNREGISTER = 20,
		API_ENUMERATE = 30,
		API_FIND = 40,
	};
};

inline size_t ifc_viewcolumnmanager::Register(ifc_viewcolumninfo **columnInfo, size_t  count)
{
	return _call(API_REGISTER, (size_t)0, columnInfo, count);
}

inline HRESULT ifc_viewcolumnmanager::Unregister(const char *columnName)
{
	return _call(API_UNREGISTER, (HRESULT)E_NOTIMPL, columnName);
}

inline HRESULT ifc_viewcolumnmanager::Enumerate(ifc_viewcolumninfoenum **enumerator)
{
	return _call(API_ENUMERATE, (HRESULT)E_NOTIMPL, enumerator);
}

inline HRESULT ifc_viewcolumnmanager::Find(const char *columnName, ifc_viewcolumninfo **columnInfo)
{
	return _call(API_FIND, (HRESULT)E_NOTIMPL, columnName, columnInfo);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_MANAGER_INTERFACE_HEADER