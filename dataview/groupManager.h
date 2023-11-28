#ifndef _NULLSOFT_WINAMP_DATAVIEW_GROUP_MANAGER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GROUP_MANAGER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_groupmanager.h"

#include "../nu/ptrlist.h"

class GroupManager : public ifc_groupmanager
{

protected:
	GroupManager();
	~GroupManager();

public:
	static HRESULT CreateInstance(GroupManager **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_groupmanager */
	size_t Register(ifc_groupprovider **providers, size_t  count);
	HRESULT Unregister(const char *providerName);
	HRESULT Enumerate(ifc_groupenum **enumerator);
	HRESULT Find(const char *providerName, ifc_groupprovider **provider);

protected:
	typedef nu::PtrList<ifc_groupprovider> GroupList;

protected:
	size_t ref;
	GroupList list;

protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_MANAGER_HEADER