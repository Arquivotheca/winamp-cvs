#ifndef _NULLSOFT_WINAMP_DATAVIEW_COLUMN_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_COLUMN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewcolumn.h"
#include "./columnInfo.h"


class _declspec(novtable) Column : public ifc_viewcolumn
{
protected:
	Column(size_t valueId, ifc_viewcolumninfo *columnInfo);
	virtual ~Column();

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewcolumn */
	HRESULT GetInfo(ifc_viewcolumninfo **info);
	virtual HRESULT Format(LCID localeId, ifc_dataobject *object, wchar_t *buffer, size_t bufferSize);
	virtual int Compare(LCID localeId, ifc_dataobject *object1, ifc_dataobject *object2);

protected:
	size_t ref;
	size_t valueId;
	ifc_viewcolumninfo *columnInfo;
	
protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_COLUMN_HEADER