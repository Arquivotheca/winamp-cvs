#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_CONTEXT_ENUM_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_CONTEXT_ENUM_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "./ifc_viewactioncontextenum.h"
#include "../nu/ptrlist.h"

class ActionContextEnum : public ifc_viewactioncontextenum
{
protected:
	ActionContextEnum();
	~ActionContextEnum();

public:
	static HRESULT CreateInstance(ActionContextEnum **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewactioncontextenum */
	HRESULT Next(Dispatchable **buffer, size_t bufferMax, size_t *fetched);
	HRESULT Reset(void);
	HRESULT Skip(size_t count);
	HRESULT GetCount(size_t *count);
	HRESULT Find(const GUID *contextId, Dispatchable **instance);

public:
	HRESULT Add(Dispatchable *context);
	HRESULT Remove(Dispatchable *context);
	HRESULT Clear();

protected:
	typedef nu::PtrList<Dispatchable> ContextList;

protected:
	size_t ref;
	size_t cursor;
	ContextList list;
	
protected:
	RECVS_DISPATCH;
	
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_ACTION_CONTEXT_ENUM_HEADER