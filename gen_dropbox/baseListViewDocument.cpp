#include "main.h"
#include "./baseListView.h"
#include "./document.h"

void BaseListView::OnDocumentCountChanged()
{
	SendNotifyMessage(hwnd, LVM_SETITEMCOUNT, 
				(WPARAM)((NULL != pActiveDocument) ? pActiveDocument->GetItemCount() : 0),
				LVSICF_NOSCROLL);
}

void BaseListView::OnDocumentReadScheduled(Document::ITEMREAD *readData)
{
	if (-1 != readData->index)
	{
		frameCache.SetItemState((INT)readData->index, FrameCache::ItemStateFetching, FALSE);
	}
}

void BaseListView::OnDocumentReadCompleted(Document::ITEMREAD *readData)
{
	if (-1 != readData->index)
	{
		if (SUCCEEDED(readData->result))
		{
			frameCache.SetItemState((INT)readData->index, FrameCache::ItemStateFetched, TRUE); 
            SendNotificationMT(NotifyUpdateMetrics, (WPARAM)((INT)readData->index), (LPARAM)readData);
		}
		else
		{
			INT itemState = FrameCache::ItemStateClear;
			switch(readData->result)
			{
				case E_META_NOTHINGTOREAD:
				case E_NOTIMPL:
					itemState = FrameCache::ItemStateCached;
					break;
			}
			frameCache.SetItemState((INT)readData->index, itemState, FALSE); 
		}
									
	}
}

void BaseListView::OnDocumentItemShifted(Document::ITEMSHIFT *pShiftData)
{
	frameCache.SetModified(FALSE);
	frameCache.Reset();

	if (NULL == pShiftData || 0 == pShiftData->delta)
		return;
	
	SendNotificationMT(NotifyItemShifted, 0, (LPARAM)pShiftData);
}

void BaseListView::OnDocumentReadQueueEmpty()
{
	if (0 != selectionMetrics.unknownData && enableSelectionRead)
	{
		PostNotification(NotifyReadSelection, FALSE, 0L);
	}
}

void BaseListView::OnDocumentAsyncFinished()
{
	ScheduleSelectionRead();
}

void BaseListView::OnDocumentItemInvalidated(INT index)
{
	frameCache.SetItemState(index, FrameCache::ItemStateClear, FALSE);
	selectionCursor = -1;
	selectionMetrics.length = 0;
	selectionMetrics.size = 0;
	selectionMetrics.unknownData = 0;
	Notify(EventSelectionChanged, NULL);
	PostMessage(hwnd, LVM_REDRAWITEMS, index, index);
}

void BaseListView::OnDocumentRangeReversed(size_t first, size_t last)
{
	frameCache.Reset();
	frameCache.SetModified(FALSE);
	
	SendNotificationMT(NotifyRangeReversed, (WPARAM)first, (LPARAM)last);
}

void BaseListView::OnDocumentRangeReordered(size_t first, size_t last)
{
	frameCache.Reset();
	frameCache.SetModified(FALSE);

	SendNotificationMT(NotifyRangeReordered, (WPARAM)first, (LPARAM)last);
}

void BaseListView::OnDocumentRangeRemoved(size_t first, size_t last)
{
	frameCache.Reset();
	frameCache.SetModified(FALSE);

	SendNotificationMT(NotifyRangeRemoved, (WPARAM)first, (LPARAM)last);
}

void BaseListView::OnDocumentNotify(UINT eventId, LONG_PTR param)
{
	switch(eventId)
	{
		case Document::EventCountChanged:
			OnDocumentCountChanged();
			break;
		case Document::EventItemReadScheduled:
			OnDocumentReadScheduled((Document::ITEMREAD*)param);
			break;
		case Document::EventItemReadCompleted:
			OnDocumentReadCompleted((Document::ITEMREAD*)param);
			break;
		case Document::EventItemShifted:
			OnDocumentItemShifted((Document::ITEMSHIFT*)param);
			break;
		case Document::EventReadQueueEmpty:
			OnDocumentReadQueueEmpty();
			break;
		case Document::EventAsyncFinished:
			OnDocumentAsyncFinished();
			break;
		case Document::EventItemInvalidated:
			OnDocumentItemInvalidated((INT)param);
			break;
		case Document::EventRangeReversed:
			OnDocumentRangeReversed(((Document::ITEMRANGE*)param)->first, ((Document::ITEMRANGE*)param)->last);
			break;
		case Document::EventRangeReordered:
			OnDocumentRangeReordered(((Document::ITEMRANGE*)param)->first, ((Document::ITEMRANGE*)param)->last);
			break;
		case Document::EventRangeRemoved:
			OnDocumentRangeRemoved(((Document::ITEMRANGE*)param)->first, ((Document::ITEMRANGE*)param)->last);
			break;
	}
}