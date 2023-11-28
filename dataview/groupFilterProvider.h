#ifndef _NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_PROVIDER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_PROVIDER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_dataprovider.h"
#include "./ifc_groupprovider.h"

#include "../nu/ptrlist.h"

class GroupFilterProvider : public ifc_dataprovider
{

protected:
	GroupFilterProvider(ifc_groupprovider *groupProvider);
	~GroupFilterProvider();

public:
	static HRESULT CreateInstance(ifc_groupprovider *groupProvider,
								  GroupFilterProvider **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_dataprovider */
	HRESULT ResolveNames(const char **names, size_t count, size_t *valueIds);
	HRESULT Enumerate(ifc_dataobjectenum **enumerator);
	HRESULT GetColumnDisplayName(const char *name, wchar_t *buffer, size_t bufferSize);
	HRESULT RegisterEventHandler(ifc_dataproviderevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_dataproviderevent *eventHandler);

public:
	size_t ResolveName(const char *name);
	HRESULT SetObjects(ifc_dataobjectlist *objectList);
	HRESULT GetGroupProvider(ifc_groupprovider **groupProvider);
	HRESULT SetNextFilter(ifc_groupprovider *groupProvider);


	void Notify_FieldsChanged();
	void Notify_ObjectsChanged(ifc_dataobject **objects, size_t count);
		
protected:
	typedef nu::PtrList<ifc_dataproviderevent> EventHandlerList;

protected:
	size_t ref;
	ifc_groupprovider *groupProvider;
	ifc_dataobjectlist *objectList;
	ifc_groupprovider *nextGroupProvider;
	EventHandlerList eventHandlerList;
	
protected:
	RECVS_DISPATCH;
};

#endif //_NULLSOFT_WINAMP_DATAVIEW_GROUP_FILTER_PROVIDER_HEADER