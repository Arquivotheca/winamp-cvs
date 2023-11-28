#include "main.h"
#include "./selectionTransaction.h"

SelectionTransaction::SelectionTransaction(Selection *_selection)
	: ref(1), selection(_selection)
{
	selection->AddRef();
}

SelectionTransaction::~SelectionTransaction()
{
}

HRESULT SelectionTransaction::CreateInstance(Selection *selection, SelectionTransaction **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = NULL;

	if (NULL == selection)
		return E_INVALIDARG;

	*instance = new (std::nothrow) SelectionTransaction(selection);
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;

}

size_t SelectionTransaction::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t SelectionTransaction::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int SelectionTransaction::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewSelectionTransaction))
		*object = static_cast<ifc_viewselectiontransaction*>(this);
	else
	{
		*object = NULL;
		return E_NOINTERFACE;
	}

	if (NULL == *object)
		return E_UNEXPECTED;

	AddRef();
	return S_OK;
}

HRESULT SelectionTransaction::Set(size_t first, size_t last)
{
	Record record;

	record.action = Action_Set;
	record.range.first = first;
	record.range.last = last;

	list.clear();
	list.push_back(record);
	
	return S_OK;
}

HRESULT SelectionTransaction::Add(size_t first, size_t last)
{
	Record *lastRecord;
	
	lastRecord = (false == list.empty()) ? &list.back() : NULL;

	if (NULL != lastRecord && 
		(Action_Add == lastRecord->action || Action_Set == lastRecord->action) &&
		first <= (lastRecord->range.last + 1) && (last + 1) >= lastRecord->range.first)
	{
		if (first < lastRecord->range.first)
			lastRecord->range.first = first;

		if (last > lastRecord->range.last)
			lastRecord->range.last = last;

	}
	else
	{
		Record record;

		record.action = Action_Add;
		record.range.first = first;
		record.range.last = last;

		list.push_back(record);
	}
	
	return S_OK;
}

HRESULT SelectionTransaction::Remove(size_t first, size_t last)
{
	Record *lastRecord;
	
	lastRecord = (false == list.empty()) ? &list.back() : NULL;

	if (NULL != lastRecord && 
		Action_Remove == lastRecord->action &&
		first <= (lastRecord->range.last + 1) && (last + 1) >= lastRecord->range.first)
	{
		if (first < lastRecord->range.first)
			lastRecord->range.first = first;

		if (last > lastRecord->range.last)
			lastRecord->range.last = last;

	}
	else
	{
		Record record;

		record.action = Action_Remove;
		record.range.first = first;
		record.range.last = last;

		list.push_back(record);
	}
	return S_OK;
}

HRESULT SelectionTransaction::RemoveAll()
{
	Record record;

	record.action = Action_RemoveAll;

	list.clear();
	list.push_back(record);

	return S_OK;
}

HRESULT SelectionTransaction::Commit()
{
	IndexRange *selectionList;
	size_t selectionListSize, index, count;
	Record *record;
	SelectionNotifier *notifier;
	BOOL freeSelectionList;

	
	selectionListSize = selection->GetListSize();
	if (selectionListSize <= 1024)
	{
		__try
		{
			selectionList = (IndexRange*)_alloca(selectionListSize * sizeof(IndexRange));
		}
		__except(STATUS_STACK_OVERFLOW == GetExceptionCode())
		{
			_resetstkoflw();
			selectionList = NULL;
		}
	}
	else
		selectionList = NULL;

	if (NULL == selectionList)
	{
		selectionList = (IndexRange*)malloc(selectionListSize * sizeof(IndexRange));
		if (NULL == selectionList)
			return E_OUTOFMEMORY;
		
		freeSelectionList = TRUE;
	}
	else
		freeSelectionList = FALSE;
	
		

	selection->CopyTo(selectionList, selectionListSize);
	selection->BlockNotifications();

	count = list.size();
	for(index = 0; index < count; index++)
	{
		record = &list[index];
		switch(record->action)
		{
			case Action_Set:
				selection->Set(record->range.first, record->range.last);
				break;
			case Action_Add:
				selection->Add(record->range.first, record->range.last);
				break;
			case Action_Remove:
				selection->Remove(record->range.first, record->range.last);
				break;
			case Action_RemoveAll:
				selection->RemoveAll();
				break;
		}
	}


	selection->UnblockNotifications();

	notifier = selection->GetNotifier();
	if (NULL != notifier)
	{
		notifier->ResetTrackers();
		selection->NotifyDifference(notifier, selectionList, selectionListSize);
		notifier->Notify(selection, ifc_viewselectionevent::Selection_Commit);
	}

	if (FALSE != freeSelectionList)
		free(selectionList);

	return S_OK;
}

#define CBCLASS SelectionTransaction
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_SET, Set)
CB(API_ADD, Add)
CB(API_REMOVE, Remove)
CB(API_REMOVEALL, RemoveAll)
CB(API_COMMIT, Commit)
END_DISPATCH;
#undef CBCLAS