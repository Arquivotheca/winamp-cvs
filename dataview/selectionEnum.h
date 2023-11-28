#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewselectionenum.h"


class SelectionEnum : public ifc_viewselectionenum
{

protected:
	SelectionEnum(IndexRange *list, size_t size);
	~SelectionEnum();

public:
	
	static HRESULT CreateInstance(IndexRange *list, 
								  size_t count,
								  SelectionEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewselectionenum */
	HRESULT Next(IndexRange *buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

protected:
	size_t ref;
	const IndexRange *list;
	size_t count;
	size_t cursor;


protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_ENUM_HEADER