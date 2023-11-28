#ifndef _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_HEADER
#define _NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_HEADER

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <wtypes.h>
#include "./ifc_viewselection.h"
#include "./selectionNotifier.h"

#include "../nu/vector.h"

class Selection : public ifc_viewselection
{

protected:
	Selection();
	~Selection();

public:
	
	static HRESULT CreateInstance(Selection **instance);

public:
	/* Dispatchable */
	size_t AddRef();
	size_t Release();
	int QueryInterface(GUID interface_guid, void **object);

	/* ifc_viewselection */
	HRESULT Set(size_t first, size_t last);
	HRESULT Add(size_t first, size_t last);
	HRESULT Remove(size_t first, size_t last);
	HRESULT RemoveAll();
	HRESULT Shift(size_t start, int length);
	HRESULT IsEditAllowed();
	size_t GetCount();
	HRESULT IsSelected(size_t index);
	HRESULT Enumerate(ifc_viewselectionenum **enumerator);
	HRESULT CreateTransaction(ifc_viewselectiontransaction **transaction);
	HRESULT RegisterEventHandler(ifc_viewselectionevent *eventHandler);
	HRESULT UnregisterEventHandler(ifc_viewselectionevent *eventHandler);

public:
	void Reserve(size_t size);
	size_t GetListSize();
	size_t CopyTo(IndexRange *buffer, size_t bufferSize);
	HRESULT NotifyDifference(SelectionNotifier *notifier, const IndexRange *buffer, size_t bufferCount);
	void BlockNotifications();
	void UnblockNotifications();
	void AllowEdit(BOOL allowEdit);

protected:
	SelectionNotifier *GetNotifier();
	HRESULT NotifyDifferenceEx(SelectionNotifier *notifier, const IndexRange *buffer, size_t bufferCount,
							   size_t shiftStart, int shiftLength);
	
	const IndexRange *TransformRange(IndexRange *range, size_t shiftStart, int shiftLength, int *countDelta);
	IndexRange GetNextTransformedRange(size_t *index, size_t shiftStart, int shiftLength);

protected:
	typedef Vector<IndexRange> SelectionList;
	friend class SelectionTransaction;

protected:
	size_t ref;
	SelectionList list;
	size_t totalCount;
	SelectionNotifier *notifierInstance;
	size_t notificationBlock;
	BOOL editAllowed;


protected:
	RECVS_DISPATCH;
};


#endif //_NULLSOFT_WINAMP_DATAVIEW_VIEW_SELECTION_ENUM_HEADER