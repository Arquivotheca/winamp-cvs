#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_TRANSACTION_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_TRANSACTION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewselectiontransaction.h"
#include "./selection.h"
#include "../nu/vector.h"


class SelectionTransaction: public ifc_viewselectiontransaction
{
public:
	typedef enum Action
	{
		Action_Set = 0,
		Action_Add = 1,
		Action_Remove = 2,
		Action_RemoveAll = 3,
	} Action;

	typedef struct Record
	{
		IndexRange range;
		Action action;
	} Record;

protected:
	SelectionTransaction(Selection *selection);
	~SelectionTransaction();

public:
	
	static HRESULT CreateInstance(Selection *selection,
								  SelectionTransaction **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewselectiontransaction */
	HRESULT Set(size_t first, size_t last);
	HRESULT Add(size_t first, size_t last);
	HRESULT Remove(size_t first, size_t last);
	HRESULT RemoveAll();
	
	HRESULT Commit();

protected:
	typedef Vector<Record> RecordList;

protected:
	size_t ref;
	Selection *selection;
	RecordList list;
	
protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_TRANSACTION_HEADER