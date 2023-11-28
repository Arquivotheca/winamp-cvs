#ifndef _NULLSOFT_WINAMP_DATAVIEW_TEXT_FILTER_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_TEXT_FILTER_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewfilter.h"
#include "./ifc_doublylinkednode.h"
#include "./ifc_viewfilterevent.h"

#include "../nu/ptrlist.h"

#include <bfc/multipatch.h>

#define MPIID_VIEWFILTER			10
#define MPIID_DOUBLYLINKEDNODE		20
#define MPIID_VIEWFILTEREVENT		30


class TextFilter :	public MultiPatch<MPIID_VIEWFILTER, ifc_viewfilter>,
					public MultiPatch<MPIID_DOUBLYLINKEDNODE, ifc_doublylinkednode>,
					public MultiPatch<MPIID_VIEWFILTEREVENT, ifc_viewfilterevent>
{

protected:
	TextFilter();
	~TextFilter();

public:
	static HRESULT CreateInstance(TextFilter **instance);

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
	HRESULT RegisterEventHandler(ifc_viewfilterevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewfilterevent *eventHandler);

	/* ifc_viewfilterevent */
	void FilterEvent_BeginUpdate(ifc_viewfilter *instance);
	void FilterEvent_EndUpdate(ifc_viewfilter *instance);
	void FilterEvent_ActionChanged(ifc_viewfilter *instance, const size_t *objectIndex, size_t count, ViewFilterAction action);

	/* ifc_doublylinkednode */
	HRESULT SetPrevious(ifc_doublylinkednode *node);
	HRESULT SetNext(ifc_doublylinkednode *node);
	HRESULT GetPrevious(ifc_doublylinkednode **node);
	HRESULT GetNext(ifc_doublylinkednode **node);


protected:
	typedef nu::PtrList<ifc_viewfilterevent> EventHandlerList;

protected:
	size_t ref;
	ifc_doublylinkednode *previous;
	ifc_doublylinkednode *next;
	EventHandlerList eventHandlerList;

protected:
	RECVS_MULTIPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_TEXT_FILTER_HEADER