#ifndef _NULLSOFT_WINAMP_DATAVIEW_COLUMN_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COLUMN_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewcolumnenum.h"


class ColumnEnum : public ifc_viewcolumnenum
{

protected:
	ColumnEnum(ifc_viewcolumn **list, size_t size);
	~ColumnEnum();

public:
	
	static HRESULT CreateInstance(ifc_viewcolumn **columns, 
								  size_t count,
								  ColumnEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewcolumnenum */
	HRESULT Next(ifc_viewcolumn **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

protected:
	size_t ref;
	ifc_viewcolumn **list;
	size_t size;
	size_t cursor;


protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_COLUMN_ENUM_HEADER