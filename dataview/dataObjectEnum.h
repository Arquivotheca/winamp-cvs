#ifndef _NULLSOFT_WINAMP_DATAVIEW_OBJECT_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_OBJECT_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_dataobject.h"
#include "./ifc_dataobjectenum.h"


class DataObjectEnum : public ifc_dataobjectenum
{
protected:
	DataObjectEnum(ifc_dataobject **objects, size_t count);
	~DataObjectEnum();

public:
	static HRESULT CreateInstance(ifc_dataobject **objects, 
								  size_t count, 
								  DataObjectEnum **instance);

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
	size_t count;
	size_t cursor;

protected:
	RECVS_DISPATCH;
};



#endif //_NULLSOFT_WINAMP_DATAVIEW_FILTERED_OBJECT_ENUM_HEADER