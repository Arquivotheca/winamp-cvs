#ifndef _NULLSOFT_WINAMP_DATAVIEW_STRING_GROUP_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_STRING_GROUP_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../ifc_dataobject.h"
#include "../ifc_groupobject.h"
#include "../ifc_sortkey.h"
#include "../ifc_groupprovider.h"

#include <bfc/multipatch.h>

#define MPIID_SG_DATAOBJECT			10
#define MPIID_SG_GROUPOBJECT		20

class StringGroup :	public MultiPatch<MPIID_SG_DATAOBJECT, ifc_dataobject>,
					public MultiPatch<MPIID_SG_GROUPOBJECT, ifc_groupobject>
{
public:
	typedef enum ValueId
	{
		ValueId_String = 0,
		ValueId_SortKey,
	} ValueId;

protected:
	StringGroup(ifc_groupprovider *provider, wchar_t *string, ifc_sortkey *sortKey);
	~StringGroup();

public:
	static HRESULT CreateInstance(ifc_groupprovider *provider, 
								  const wchar_t *string, 
								  ifc_sortkey *sortKey,
								  StringGroup **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_deviceobject */
	HRESULT GetValue(LCID localeId, size_t valueId, DataValue *value);
	HRESULT IsEqual(LCID localeId, ifc_dataobject *object2);
	int Compare(LCID localeId, size_t valueId, DataValue *value);
	int CompareTo(LCID localeId, size_t valueId, ifc_dataobject *object2);

	/* ifc_groupobject */
	HRESULT Add(ifc_dataobject *object);
	HRESULT Subtract(ifc_dataobject *object);
	HRESULT Reset();
	HRESULT IsUnknown();

protected:
	ifc_sortkey *GetSortKey(LCID localeId);

protected:
	size_t ref;
	wchar_t *string;
	ifc_sortkey *sortKey;
	ifc_groupprovider *provider;
	
protected:
	RECVS_MULTIPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_STRING_GROUP_HEADER