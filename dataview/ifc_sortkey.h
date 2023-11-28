#ifndef _NULLSOFT_WINAMP_DATAVIEW_SORT_KEY_INTERFACE_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_SORT_KEY_INTERFACE_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <bfc/platform/guid.h>

// {336C060D-C738-4032-B7AF-9EF975F1FED9}
static const GUID IFC_SortKey = 
{ 0x336c060d, 0xc738, 0x4032, { 0xb7, 0xaf, 0x9e, 0xf9, 0x75, 0xf1, 0xfe, 0xd9 } };

#include <bfc/dispatch.h>
#include "./ifc_dataobject.h"


// supports AddRef(), Release(), QueryInterface()
class __declspec(novtable) ifc_sortkey : public Dispatchable
{
protected:
	ifc_sortkey() {}
	~ifc_sortkey() {}

public:
	const void *GetValue();
	size_t GetSize();
	LCID GetLocaleId();
	int Compare(const void *value, size_t size);

public:
	DISPATCH_CODES
	{
		API_GETVALUE = 10,
		API_GETSIZE = 20,
		API_GETLOCALEID = 30,
		API_COMPARE = 40,
	};
};


inline const void *ifc_sortkey::GetValue()
{
	return _call(API_GETVALUE, (const void*)NULL);
}

inline size_t ifc_sortkey::GetSize()
{
	return _call(API_GETSIZE, (size_t)0);
}

inline LCID ifc_sortkey::GetLocaleId()
{
	return _call(API_GETLOCALEID, (LCID)0);
}

inline int ifc_sortkey::Compare(const void *value, size_t size)
{
	return _call(API_COMPARE, (int)COBJ_ERROR, value, size);
}

#endif //_NULLSOFT_WINAMP_DATAVIEW_SORT_KEY_INTERFACE_HEADER