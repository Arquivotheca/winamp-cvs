#ifndef _NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_OBJECT_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_OBJECT_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_dataobject.h"
#include "./ifc_groupobject.h"
#include "./ifc_summarygroupobject.h"
#include "./hashTypes.h"

#include <bfc/multipatch.h>

#define MPIID_GFO_DATAOBJECT			10
#define MPIID_GFO_GROUPOBJECT			20
 #define MPIID_GFO_SUMMARYGROUPOBJECT	30

class GroupFilterObject : public MultiPatch<MPIID_GFO_DATAOBJECT, ifc_dataobject>,
						  public MultiPatch<MPIID_GFO_GROUPOBJECT, ifc_groupobject>,
						  public MultiPatch<MPIID_GFO_SUMMARYGROUPOBJECT, ifc_summarygroupobject>

{
public:
	typedef enum ValueId
	{
		ValueId_Group = 100000,
		ValueId_NextFilterCount,
		ValueId_TrackCount,
		ValueId_Size,
		ValueId_Length,
	} ValueId;

protected:
	GroupFilterObject(ifc_dataobject *group);
	virtual ~GroupFilterObject();

public:
	static HRESULT CreateInstance(ifc_dataobject *group,
								  GroupFilterObject **instance);

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

	/* ifc_summarygroupobject */
	HRESULT AddGroup(ifc_groupobject *group);
	HRESULT SubtractGroup(ifc_groupobject *group);

public:
	HRESULT GetGroup(ifc_dataobject **group);
	
	size_t Objects_GetCount();

	unsigned __int64 Length_Get();
	void Length_Add(unsigned __int64 length);
	void Length_Subtract(unsigned __int64 length);
	void Length_Reset();

	unsigned __int64 Size_Get();
	void Size_Add(unsigned __int64 size);
	void Size_Subtract(unsigned __int64 size);
	void Size_Reset();
	
	size_t NextFilters_GetCount();
	HRESULT NextFilters_Add(size_t groupId);
	HRESULT NextFilters_Subtract(size_t groupId);
	HRESULT NextFilters_Reset();
	
	void SearchIndex_Set(size_t searchIndex);
	size_t SearchIndex_Get();


protected:
	size_t ref;
	ifc_dataobject *groupData;
	ifc_groupobject *groupObject;
	size_t objectCount;
	unsigned __int64 size;
	unsigned __int64 length;
	size_t searchIndex;
	khash_t(sizet_map) *nextFilters;

protected:
	RECVS_MULTIPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_OBJECT_HEADER