#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {36160612-9D0A-4f65-ABF5-A59AF1B1127B}
static const GUID IFC_ViewColumn = 
{ 0x36160612, 0x9d0a, 0x4f65, { 0xab, 0xf5, 0xa5, 0x9a, 0xf1, 0xb1, 0x12, 0x7b } };


#include <bfc/dispatch.h>

#include "./ifc_viewcolumninfo.h"
#include "./ifc_dataobject.h"


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_viewcolumn : public Dispatchable
{

protected:
	ifc_viewcolumn() {}
	~ifc_viewcolumn() {}

public:
	HRESULT GetInfo(ifc_viewcolumninfo **info);
	
	HRESULT Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize);
	int Compare(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2);

public:
	DISPATCH_CODES
	{
		API_GETINFO = 10,
		API_FORMAT = 20,
		API_COMPARE = 30,
	};
};

inline HRESULT ifc_viewcolumn::GetInfo(ifc_viewcolumninfo **info)
{
	return _call(API_GETINFO, (HRESULT)E_NOTIMPL, info);
}

inline HRESULT ifc_viewcolumn::Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize)
{
	return _call(API_FORMAT, (HRESULT)E_NOTIMPL, localeId, object, buffer, bufferSize);
}

inline int ifc_viewcolumn::Compare(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2)
{
	return _call(API_COMPARE, (int)COBJ_ERROR, localeId, object1, object2);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_COLUMN_INTERFACE_HEADER