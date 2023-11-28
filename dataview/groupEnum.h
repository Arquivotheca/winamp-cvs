#ifndef _NULLSOFT_WINAMP_DATAVIEW_GROUP_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GROUP_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_groupenum.h"


class GroupEnum : public ifc_groupenum
{

protected:
	GroupEnum(ifc_groupprovider **list, size_t size);
	~GroupEnum();

public:
	
	static HRESULT CreateInstance(ifc_groupprovider **providers, 
								  size_t count,
								  GroupEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_groupenum */
	HRESULT Next(ifc_groupprovider **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

protected:
	size_t ref;
	ifc_groupprovider **list;
	size_t size;
	size_t cursor;


protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_ENUM_HEADER