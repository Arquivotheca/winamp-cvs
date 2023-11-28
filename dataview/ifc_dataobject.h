#ifndef _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {4680C028-C553-43d7-806E-70CD561F133D}
static const GUID IFC_DataObject = 
{ 0x4680c028, 0xc553, 0x43d7, { 0x80, 0x6e, 0x70, 0xcd, 0x56, 0x1f, 0x13, 0x3d } };


#include <bfc/dispatch.h>
#include "./dataValue.h"


#define VALUE_E_NOTFOUND		DISP_E_PARAMNOTFOUND
#define VALUE_E_BADTYPE			DISP_E_TYPEMISMATCH
#define VALUE_E_OUTOFMEMORY		E_OUTOFMEMORY
#define VALUE_E_UNKNOWNNAME		DISP_E_UNKNOWNNAME 

// CompareXXX return values
#define COBJ_ERROR			0
#define COBJ_LESS_THAN		CSTR_LESS_THAN
#define COBJ_EQUAL			CSTR_EQUAL
#define COBJ_GREATER_THAN	CSTR_GREATER_THAN

#define COBJ_COMPARE(_a, _b)\
		(((_a) > (_b)) ? COBJ_GREATER_THAN : (((_a) < (_b)) ? COBJ_LESS_THAN : COBJ_EQUAL))

// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_dataobject : public Dispatchable
{
protected:
	ifc_dataobject() {}
	~ifc_dataobject() {}

public:
	HRESULT GetValue(LCID localeId, size_t valueId, DataValue *value); // return S_FALSE if valueId is correct but value is undefined (never set)
	HRESULT IsEqual(LCID localeId, ifc_dataobject *object2);
	int Compare(LCID localeId, size_t valueId, DataValue *value);
	int CompareTo(LCID localeId, size_t valueId, ifc_dataobject *object2);
	

public:
	DISPATCH_CODES
	{
		API_GETVALUE = 10,
		API_ISEQUAL = 20,
		API_COMPARE = 30,
		API_COMPARETO = 40,
	};
};


inline HRESULT ifc_dataobject::GetValue(LCID localeId, size_t valueId, DataValue *value)
{
	return _call(API_GETVALUE, (HRESULT)E_NOTIMPL, localeId, valueId, value);
}

inline HRESULT ifc_dataobject::IsEqual(LCID localeId, ifc_dataobject *object2)
{
	return _call(API_ISEQUAL, (HRESULT)E_NOTIMPL, localeId, object2);
}

inline int ifc_dataobject::Compare(LCID localeId, size_t valueId, DataValue *value)
{
	return _call(API_COMPARE, (int)COBJ_ERROR, localeId, valueId, value);
}

inline int ifc_dataobject::CompareTo(LCID localeId, size_t valueId, ifc_dataobject *object2)
{
	return _call(API_COMPARETO, (int)COBJ_ERROR, localeId,  valueId, object2);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_DATA_OBJECT_INTERFACE_HEADER