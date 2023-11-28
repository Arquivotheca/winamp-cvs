#ifndef _NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewcolumninfoenum.h"


class ColumnInfoEnum : public ifc_viewcolumninfoenum
{

protected:
	ColumnInfoEnum(ifc_viewcolumninfo **list, size_t size);
	~ColumnInfoEnum();

public:
	
	static HRESULT CreateInstance(ifc_viewcolumninfo **columns, size_t count,
								  ColumnInfoEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewcolumninfoenum */
	HRESULT Next(ifc_viewcolumninfo **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

protected:
	size_t ref;
	ifc_viewcolumninfo **list;
	size_t size;
	size_t cursor;


protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_COLUMN_INFO_ENUM_HEADER