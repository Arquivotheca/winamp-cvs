#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_CHAIN_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_CHAIN_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewfilter.h"
#include "./ifc_viewfilterenum.h"
#include "./ifc_doublylinkednode.h"

class FilterChainEnum : public ifc_viewfilterenum
{
protected:
	FilterChainEnum(ifc_doublylinkednode *head);
	~FilterChainEnum();

public:
	static HRESULT CreateInstance(ifc_doublylinkednode *head,
								  FilterChainEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewfilterenum */
	HRESULT Next(ifc_viewfilter **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);

protected:
	size_t ref;
	ifc_doublylinkednode *head;
	ifc_doublylinkednode *cursor;

protected:
	RECVS_DISPATCH;
	
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_FILTER_CHAIN_ENUM_HEADER