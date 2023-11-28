#include "main.h"
#include "./selection.h"
#include "./selectionEnum.h"
#include "./selectionTransaction.h"

Selection::Selection()
	: ref(1), totalCount(0), notifierInstance(NULL), notificationBlock(0),
	  editAllowed(TRUE)
{
}

Selection::~Selection()
{
	SafeDelete(notifierInstance);
}

HRESULT Selection::CreateInstance(Selection **instance)
{
	if (NULL == instance)
		return E_POINTER;

	*instance = new (std::nothrow) Selection();
	if (NULL == *instance)
		return E_OUTOFMEMORY;

	return S_OK;
}

size_t Selection::AddRef()
{
	return InterlockedIncrement((LONG*)&ref);
}

size_t Selection::Release()
{
	if (0 == ref)
		return ref;
	
	LONG r = InterlockedDecrement((LONG*)&ref);
	if (0 == r)
		delete(this);
	
	return r;
}

int Selection::QueryInterface(GUID interface_guid, void **object)
{
	if (NULL == object) 
		return E_POINTER;
	
	if (IsEqualIID(interface_guid, IFC_ViewSelection))
		*object = static_cast<ifc_viewselection*>(this);
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

HRESULT Selection::Set(size_t first, size_t last)
{
	IndexRange insertRange;
	SelectionNotifier *notifier;
		
	if (FALSE == editAllowed) 
		return SELECTION_E_WRITE_PROTECT;

	if (first > last)
	{
		size_t index;
		index = last;
		last = first;
		first = index;
	}

	notifier = GetNotifier();
	if (NULL != notifier)
	{
		size_t count;
		IndexRange *range;
		Selection *appendedTracker, *removedTracker;

		count = list.size();
	
		notifier->ResetTrackers();

		appendedTracker = notifier->GetAppendedTracker();
		appendedTracker->RemoveAll();
		appendedTracker->Add(first, last);

		removedTracker = notifier->GetRemovedTracker();
		removedTracker->RemoveAll();
		removedTracker->Reserve(count);

		while(count--)
		{
			range = &list[count];
			appendedTracker->Remove(range->first, range->last);
			removedTracker->Add(range->first, range->last);
		}

		removedTracker->Remove(first, last);
	}
	
	insertRange.first = first;
	insertRange.last = last;

	list.clear();
	list.push_back(insertRange);

	totalCount = (last - first) + 1;

	if (NULL != notifier)
		notifier->Notify(this, ifc_viewselectionevent::Selection_Set);

	return S_OK;
}

HRESULT Selection::Add(size_t first, size_t last)
{
	size_t index, count;
	IndexRange *range;
	IndexRange insertRange;
	Selection *appendedTracker; 
	SelectionNotifier *notifier;
		
	if (FALSE == editAllowed) 
		return SELECTION_E_WRITE_PROTECT;

	if (first > last)
	{
		size_t temp;
		temp = last;
		last = first;
		first = temp;
	}

	insertRange.first = first;
	insertRange.last = last;

	count = list.size();

	notifier = GetNotifier();
	if (NULL != notifier)
	{
		notifier->ResetTrackers();
		appendedTracker = notifier->GetAppendedTracker();
				
		appendedTracker->Reserve(count + 1);
		appendedTracker->Add(first, last);
	}
	else
		appendedTracker = NULL;

	if (0 == count || insertRange.first > (list[count-1].last + 1))
	{
		list.push_back(insertRange);
		totalCount += (1 + insertRange.last - insertRange.first);
	}	
	else
	{
		for(index = 0; index < count; )
		{
			range = &list[index];
			if ((insertRange.last + 1) < range->first)
			{
				list.insert(index, insertRange);
				break;
			}
			
			if (insertRange.first <= (range->last + 1))
			{
				if (insertRange.first > range->first)
					insertRange.first = range->first;

				if (insertRange.last < range->last)
					insertRange.last = range->last;
					
				totalCount -= (1 + range->last - range->first);

				if (NULL != appendedTracker)
					appendedTracker->Remove(range->first, range->last);

				list.eraseAt(index);
				count--;
			}
			else
				index++;
		}

		if (index >= count)
			list.push_back(insertRange);

		totalCount += (1 + insertRange.last - insertRange.first);
	}

	if (NULL != notifier)
		notifier->Notify(this, ifc_viewselectionevent::Selection_Add);

	return S_OK;
}

HRESULT Selection::Remove(size_t first, size_t last)
{
	size_t index, count;
	IndexRange *range, insertRange;
	Selection *removedTracker;
	SelectionNotifier *notifier;
	
	if (FALSE == editAllowed) 
		return SELECTION_E_WRITE_PROTECT;

	if (first > last)
	{
		size_t temp;
		temp = last;
		last = first;
		first = temp;
	}

	count = list.size();

	if (0 == count || 
		last < list[0].first || 
		first > list[count-1].last)
	{		
		return S_OK;
	}	
	
	notifier = GetNotifier();
	if (NULL != notifier)
	{
		notifier->ResetTrackers();
		removedTracker = notifier->GetRemovedTracker();
		removedTracker->Reserve(count);
	}
	else
		removedTracker = NULL;

	for(index = 0; index < count; index++)
	{
		range = &list[index];

		if (last < range->first)
			break;
		
		if (first > range->last)
			continue;
		
		if (first <= range->first && last >= range->last)
		{
			if (NULL != removedTracker)
				removedTracker->Add(range->first, range->last);

			totalCount -= (1 + range->last - range->first);
			list.eraseAt(index);
			index--;
			count--;

			continue;
		}
		
		if (first <= range->first && 
			last < range->last)
		{
			if (NULL != removedTracker)
				removedTracker->Add(last, range->first);

			totalCount -= (1 + last - range->first);
			range->first = (last + 1);
			continue;
		}

		if (last >= range->last && 
			first > range->first)
		{
			if (NULL != removedTracker)
				removedTracker->Add(range->last, first);

			totalCount -= (1 + range->last - first);
			range->last = (first - 1);
			continue;
		}
				

		if (NULL != removedTracker)
			removedTracker->Add(first, last);

		insertRange.first = last + 1;
		insertRange.last = range->last;
		range->last = first - 1;

		list.insert(++index, insertRange);
		totalCount -= (1 + last - first);
	}

	if (NULL != notifier)
		notifier->Notify(this, ifc_viewselectionevent::Selection_Remove);

	return S_OK;
}

HRESULT Selection::RemoveAll()
{	
	SelectionNotifier *notifier;
	
	if (FALSE == editAllowed) 
		return SELECTION_E_WRITE_PROTECT;

	notifier = GetNotifier();
	if (NULL != notifier)
	{
		size_t count;
		IndexRange *range;
		Selection *removedTracker; 

		notifier->ResetTrackers();

		removedTracker = notifier->GetRemovedTracker();
		
		count = list.size();
		removedTracker->Reserve(count);
		
		while(count--)
		{
			range = &list[count];
			removedTracker->Add(range->first, range->last);
		}
	}

	list.clear();
	totalCount = 0;

	if (NULL != notifier)
		notifier->Notify(this, ifc_viewselectionevent::Selection_RemoveAll);

	return S_OK;
}

HRESULT Selection::Shift(size_t start, int length)
{
	HRESULT hr;
	size_t index, count;
	int countDelta;
	IndexRange *range;
	SelectionNotifier *notifier;

	if (FALSE == editAllowed) 
		return SELECTION_E_WRITE_PROTECT;

	if (0 == length || 0 == list.size())
		return S_OK;
	
	if (length > 0)
	{
		count = list.size();
		for(index = 0; index < count; index++)
		{
			range = &list[index];
			if (range->first >= start)
				break;
			
			if (range->last >= start)
			{
				IndexRange temp;
				temp.first = start;
				temp.last = range->last;
				range->last = start - 1;
				index++;
				if (index < count)
					list.insert(index, temp);
				else 
					list.push_back(temp);
				break;
			}
		}
	}
	
	notifier = GetNotifier();
	if (NULL != notifier)
	{
		notifier->ResetTrackers();
		hr = NotifyDifferenceEx(notifier, list.begin(), list.size(), start, length);
		if (SUCCEEDED(hr) && S_OK == notifier->IsTrackersEmpty())
			return S_OK;
	}
	else
		hr = S_OK;

	count = list.size();
	for(index = 0; index < count; index++)
	{
		range = &list[index];
		if (NULL == TransformRange(range, start, length, &countDelta))
		{
			list.eraseAt(index);
			index--;
			count--;
			totalCount += countDelta;
			continue;
		}
		totalCount += countDelta;
	}

	if (NULL != notifier)
		notifier->Notify(this, ifc_viewselectionevent::Selection_Shift);

	return hr;
}

HRESULT Selection::IsEditAllowed()
{
	return (FALSE != editAllowed) ? S_OK : S_FALSE;
}

size_t Selection::GetCount()
{
	return totalCount;
}

HRESULT Selection::IsSelected(size_t index)
{
	size_t i, count;
	const IndexRange *range;
	
	count = list.size();
	
	for(i = 0; i < count; i++)
	{
		range = &list[i];
		if (index >= range->first && index <= range->last)
			return S_OK;
	}
	return S_FALSE;
}

HRESULT Selection::Enumerate(ifc_viewselectionenum **enumerator)
{
	return SelectionEnum::CreateInstance(list.begin(), list.size(), (SelectionEnum**)enumerator);
}

HRESULT Selection::CreateTransaction(ifc_viewselectiontransaction **transaction)
{
	return SelectionTransaction::CreateInstance(this, (SelectionTransaction**)transaction);
}

HRESULT Selection::RegisterEventHandler(ifc_viewselectionevent *eventHandler)
{
	if (NULL == notifierInstance)
	{
		notifierInstance = new (std::nothrow) SelectionNotifier();
		if (NULL == notifierInstance)
			return E_OUTOFMEMORY;

	}

	return notifierInstance->RegisterHandler(eventHandler);
}

HRESULT Selection::UnregisterEventHandler(ifc_viewselectionevent *eventHandler)
{
	HRESULT hr;

	if (NULL == notifierInstance)
		return S_FALSE;

	hr = notifierInstance->UnregisterHandler(eventHandler);
	if (S_OK == hr && 0 == notifierInstance->GetHandlerCount())
	{
		delete notifierInstance;
		notifierInstance = NULL;
	}

	return hr;
}

void Selection::Reserve(size_t size)
{
	list.reserve(list.size() + size);
}

size_t Selection::GetListSize()
{
	return list.size();
}

size_t Selection::CopyTo(IndexRange *buffer, size_t bufferSize)
{
	if (NULL == buffer)
		return 0;

	if (bufferSize > list.size())
		bufferSize = list.size();

	if (0 == bufferSize)
		return 0;

	memcpy(buffer, list.begin(), bufferSize * sizeof(IndexRange));
	return bufferSize;
}

HRESULT Selection::NotifyDifference(SelectionNotifier *notifier, const IndexRange *buffer, size_t bufferCount)
{
	size_t count, temp;
	size_t indexNew, indexOld;
	IndexRange rangeOld, rangeNew;
	const IndexRange *range;
	Selection *appendedTracker, *removedTracker;
	
	if (NULL == buffer)
		return E_INVALIDARG;

	if (NULL == notifier)
		return E_INVALIDARG;

	
	appendedTracker = notifier->GetAppendedTracker();
	removedTracker = notifier->GetRemovedTracker();

	count = list.size();
	indexNew = 0;
	indexOld = 0;

	if (0 == bufferCount)
	{
		if(NULL != appendedTracker)
		{
			while(indexNew < count)
			{
				range = &list[indexNew++];
				appendedTracker->Add(range->first, range->last);
			}
		}
		return S_OK;
	}

	if (0 == count)
	{
		if(NULL != removedTracker)
		{
			while(indexOld < bufferCount)
			{
				range = &buffer[indexOld++];
				removedTracker->Add(range->first, range->last);
			}
		}
		return S_OK;
	}

	
	rangeNew = list[indexNew];

	for(;indexOld < bufferCount; indexOld++)
	{
		rangeOld = buffer[indexOld];
				
		while(indexNew < count)
		{			
			if (rangeNew.first > rangeOld.last)
				break;

			if (rangeNew.first < rangeOld.first)
			{
				temp = rangeOld.first - 1;
				if (rangeNew.last < temp)
					temp = rangeNew.last;
				appendedTracker->Add(rangeNew.first, temp);
			}
			else if (rangeNew.first > rangeOld.first)
			{
				removedTracker->Add(rangeOld.first, rangeNew.first -1);
				rangeOld.first = rangeNew.first;
			}
			
			if (rangeNew.last < rangeOld.last)
			{
				temp = rangeNew.last + 1;
				if (rangeOld.first < temp)
					rangeOld.first = temp;
				
				rangeNew = list[++indexNew];
			}
			else
			{
				if (rangeNew.last == rangeOld.last)
					rangeNew = list[++indexNew];
				else
					rangeNew.first = rangeOld.last + 1;

				rangeOld.last = -1;
				break;
			}
		}

		if (rangeOld.first <= rangeOld.last && (size_t)-1 != rangeOld.last)
		{
			removedTracker->Add(rangeOld.first, rangeOld.last);
		}
	}

	range = &rangeNew;
	while(indexNew < count)
	{		
		appendedTracker->Add(range->first, range->last);
		range = &list[++indexNew];
	}
	
	return S_OK;
}

const IndexRange *Selection::TransformRange(IndexRange *range, size_t shiftStart, int shiftLength, int *countDelta)
{
	if (shiftLength  < 0)
	{
		size_t shiftLow;

		shiftLow = shiftStart + shiftLength + 1;

		if (range->first >= shiftLow)
		{
			if (range->first > shiftStart)
			{
				range->first += shiftLength;
				range->last += shiftLength;

				SetPtrValSafe(countDelta, 0);
			}
			else
			{				
				if (range->last > shiftStart)
					range->last += shiftLength;
				else
				{
					// whole range was in removed area
					SetPtrValSafe(countDelta, -(int)(1 + range->last - range->first));
					return NULL;
				}
				
				SetPtrValSafe(countDelta, -(int)(1 + range->first - shiftLow));

				range->first = shiftLow;
			}
		}
		else if (range->last >= shiftLow)
		{
			if (range->last > shiftStart)
			{
				range->last += shiftLength;
				SetPtrValSafe(countDelta, shiftLength);
			}
			else
			{
				SetPtrValSafe(countDelta, -(int)(1 + range->last - shiftLow));
				range->last = shiftLow - 1;
			}
		}
		else
		{
			SetPtrValSafe(countDelta, 0);
		}
	}
	else
	{		
		if (range->first >= shiftStart)
		{
			range->first += shiftLength;
			range->last += shiftLength;
			SetPtrValSafe(countDelta, 0);
		}
		else if (range->last >= shiftStart)
		{
			// range brake 
			range->last += shiftStart;
			SetPtrValSafe(countDelta, shiftStart);
		}
		else
		{
			SetPtrValSafe(countDelta, 0);
		}
	}

	return range;
}

IndexRange Selection::GetNextTransformedRange(size_t *index, size_t shiftStart, int shiftLength)
{
	IndexRange range;
	
	for (;*index < list.size(); (*index)++)
	{
		range = list[*index];
		if (NULL != TransformRange(&range, shiftStart, shiftLength, NULL))
			return range;
	}
	
	range.first = -1;
	range.last = -1;
	return range;
}

HRESULT Selection::NotifyDifferenceEx(SelectionNotifier *notifier, const IndexRange *buffer, size_t bufferCount,
									  size_t shiftStart, int shiftLength)
{
	size_t count, temp;
	size_t indexNew, indexOld;
	IndexRange rangeOld, rangeNew;
	const IndexRange *range;
	Selection *appendedTracker, *removedTracker;

	if (NULL == buffer)
		return E_INVALIDARG;

	if (NULL == notifier)
		return E_INVALIDARG;

	appendedTracker = notifier->GetAppendedTracker();
	removedTracker = notifier->GetRemovedTracker();

	count = list.size();
	indexNew = 0;
	indexOld = 0;

	rangeNew = GetNextTransformedRange(&indexNew, shiftStart, shiftLength);

	for(;indexOld < bufferCount; indexOld++)
	{
		rangeOld = buffer[indexOld];
						
		while(indexNew < count)
		{			
			if (rangeNew.first > rangeOld.last)
				break;

			if (rangeNew.first < rangeOld.first)
			{
				temp = rangeOld.first - 1;
				if (rangeNew.last < temp)
					temp = rangeNew.last;
				appendedTracker->Add(rangeNew.first, temp);
			}
			else if (rangeNew.first > rangeOld.first)
			{
				removedTracker->Add(rangeOld.first, rangeNew.first -1);
				rangeOld.first = rangeNew.first;
			}
			
			if (rangeNew.last < rangeOld.last)
			{
				temp = rangeNew.last + 1;
				if (rangeOld.first < temp)
					rangeOld.first = temp;
				
				indexNew++;
				rangeNew = GetNextTransformedRange(&indexNew, shiftStart, shiftLength);
			}
			else
			{
				if (rangeNew.last == rangeOld.last)
				{
					indexNew++;
					rangeNew = GetNextTransformedRange(&indexNew, shiftStart, shiftLength);
				}
				else
					rangeNew.first = rangeOld.last + 1;

				rangeOld.last = -1;
				break;
			}
		}

		if (rangeOld.first <= rangeOld.last && (size_t)-1 != rangeOld.last)
		{
			removedTracker->Add(rangeOld.first, rangeOld.last);
		}
	}

	range = &rangeNew;
	while(indexNew < count)
	{		
		appendedTracker->Add(range->first, range->last);
		range = &list[++indexNew];
	}

	return S_OK;
}

SelectionNotifier *Selection::GetNotifier()
{
	return ( 0 == notificationBlock) ? notifierInstance : NULL;
}

void Selection::BlockNotifications()
{
	InterlockedIncrement(&notificationBlock);
}

void Selection::UnblockNotifications()
{
	InterlockedDecrement(&notificationBlock);
}

void Selection::AllowEdit(BOOL allowEdit)
{
	editAllowed = allowEdit;
}

#define CBCLASS Selection
START_DISPATCH;
CB(ADDREF, AddRef)
CB(RELEASE, Release)
CB(QUERYINTERFACE, QueryInterface)
CB(API_SET, Set)
CB(API_ADD, Add)
CB(API_REMOVE, Remove)
CB(API_REMOVEALL, RemoveAll)
CB(API_ISEDITALLOWED, IsEditAllowed)
CB(API_GETCOUNT, GetCount)
CB(API_ISSELECTED, IsSelected)
CB(API_ENUMERATE, Enumerate)
CB(API_CREATETRANSACTION, CreateTransaction)
CB(API_REGISTEREVENTHANDLER, RegisterEventHandler)
CB(API_UNREGISTEREVENTHANDLER, UnregisterEventHandler)
END_DISPATCH;
#undef CBCLASS
