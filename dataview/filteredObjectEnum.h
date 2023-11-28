#ifndef _NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_dataobject.h"
#include "./ifc_dataobjectenum.h"


class FilteredObjectEnum : public ifc_dataobjectenum
{
protected:
	FilteredObjectEnum(ifc_dataobject **objects, const size_t *map, size_t count);
	~FilteredObjectEnum();

public:
	static HRESULT CreateInstance(ifc_dataobject **objects, 
								  const size_t *map, 
								  size_t count, 
								  FilteredObjectEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_dataobjectenum */
	HRESULT Next(ifc_dataobject **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);
	
protected:
	size_t ref;
	ifc_dataobject **objects;
	const size_t *map;
	size_t count;
	size_t cursor;

protected:
	RECVS_DISPATCH;
};



#endif //_NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_ENUM_HEADER