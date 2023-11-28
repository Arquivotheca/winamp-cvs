#ifndef _NULLSOFT_WINAMP_DATAVIEW_FILTER_CHAIN_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_FILTER_CHAIN_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewfilter.h"
#include "./ifc_doublylinkednode.h"
#include "./ifc_viewfilterevent.h"
#include "./ifc_viewfilterenum.h"
#include "./ifc_viewfilterchainevent.h"

#include "../nu/ptrlist.h"

#include <bfc/multipatch.h>

#define MPIID_VIEWFILTER			10
#define MPIID_VIEWFILTEREVENT		20

class FilterChain:	public MultiPatch<MPIID_VIEWFILTER, ifc_viewfilter>,
					public MultiPatch<MPIID_VIEWFILTEREVENT, ifc_viewfilterevent>
{

protected:
	FilterChain();
	~FilterChain();

public:
	static HRESULT CreateInstance(FilterChain **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewfilter */
	const char *GetName();
	HRESULT Bind(ifc_dataprovider *provider);
	HRESULT Init(ifc_dataobjectlist *objectList); 
	HRESULT IsAllowed(size_t objectIndex);
	HRESULT Update();
	HRESULT RegisterEventHandler(ifc_viewfilterevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewfilterevent *eventHandler);

	/* ifc_viewfilterevent */
	void FilterEvent_BeginUpdate(ifc_viewfilter *instance);
	void FilterEvent_EndUpdate(ifc_viewfilter *instance);
	void FilterEvent_BlockAll(ifc_viewfilter *instance);
	void FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action);

public:
	HRESULT InsertFilter(size_t insertAt, ifc_viewfilter *filter);
	HRESULT RemoveFilter(ifc_viewfilter *filter);
	HRESULT EnumerateFilters(ifc_viewfilterenum **enumerator);
	HRESULT Destroy();

protected:
	typedef nu::PtrList<ifc_viewfilterevent> EventHandlerList;

protected:
	size_t ref;
	ifc_doublylinkednode *head;
	EventHandlerList eventHandlerList;

protected:
	RECVS_MULTIPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_FILTER_CHAIN_HEADER