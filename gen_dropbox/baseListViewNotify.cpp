#include "main.h"
#include "./baseListView.h"
#include "./document.h"
#include "./fileInfoInterface.h"

#define BVM_NOTIFY_STRING		TEXT("WA_DROPBOX_BVM_NOTIFY")

static UINT BVM_NOTIFY = 0;

typedef enum
{
	NotifyModeDirect = 0,
	NotifyModeAsync = 1,
} NotifyMode;

typedef struct
{
	WPARAM	wParam;
	LPARAM	lParam;
	UINT	mode;
} NOTIFYPARAM;


BOOL BaseListViewNotify_RegisterMessage()
{
	if (0 == BVM_NOTIFY)
		BVM_NOTIFY = RegisterWindowMessage(BVM_NOTIFY_STRING);
	return (0 != BVM_NOTIFY);
}

static void* BaseListViewNotify_DuplicateMemory(const void *block, size_t blockSize)
{
	void *copy = NULL;
	if (NULL != block)
	{
		copy = malloc(blockSize);
		if (NULL != copy)
			CopyMemory(copy, block, blockSize);
	}
	return copy;
}

static void BaseListViewNotify_ReleaseMemory(void *block)
{
	if (NULL != block)
		free(block);
}

static Document::ITEMREAD* BaseListViewNotify_DuplicateItemRead(const Document::ITEMREAD *readData)
{
	Document::ITEMREAD *copy = NULL;
	if (NULL != readData)
	{
		copy = (Document::ITEMREAD*)malloc(sizeof(Document::ITEMREAD));
		if (NULL != copy)
		{
			CopyMemory(copy, readData, sizeof(Document::ITEMREAD));
			if (NULL != copy->pItem)
				copy->pItem->AddRef();
		}
	}
	return copy;
}

static void BaseListViewNotify_ReleaseItemRead(Document::ITEMREAD *readData)
{
	if (NULL != readData)
	{
		if (NULL != readData->pItem)
			readData->pItem->Release();

		free(readData);
	}
}

static void BaseListViewNotify_BeginMarshaling(UINT notifyCode, NOTIFYPARAM *param)
{
	if (NotifyModeDirect == param->mode)
		return;

	switch(notifyCode)
	{
		case BaseListView::NotifyUpdateMetrics:
			param->lParam = (LPARAM)BaseListViewNotify_DuplicateItemRead((const Document::ITEMREAD*)param->lParam);
			break;
		case BaseListView::NotifyItemShifted:
			param->lParam = (LPARAM)BaseListViewNotify_DuplicateMemory((const void*)param->lParam, sizeof(Document::ITEMSHIFT));
			break;
	}
}

static void BaseListViewNotify_FinishMarshaling(UINT notifyCode, NOTIFYPARAM *param)
{
	if (NotifyModeDirect == param->mode)
		return;

	switch(notifyCode)
	{
		case BaseListView::NotifyUpdateMetrics:
			BaseListViewNotify_ReleaseItemRead((Document::ITEMREAD*)param->lParam);
			break;
		case BaseListView::NotifyItemShifted:
			BaseListViewNotify_ReleaseMemory((void*)param->lParam);
			break;
	}
	free(param);
}


LRESULT BaseListView::SendNotification(UINT notifyCode, WPARAM wParam, LPARAM lParam)
{
	return BaseListView_SendNotification(hwnd, notifyCode, wParam, lParam);
}

BOOL BaseListView::SendNotificationMT(UINT notifyCode, WPARAM wParam, LPARAM lParam)
{
	return BaseListView_SendNotificationMT(hwnd, notifyCode, wParam, lParam);
}

BOOL BaseListView::PostNotification(UINT notifyCode, WPARAM wParam, LPARAM lParam)
{
	return BaseListView_PostNotification(hwnd, notifyCode, wParam, lParam);
}

BOOL BaseListView::ProcessNotifications(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (BVM_NOTIFY != uMsg || 0 == BVM_NOTIFY)
		return FALSE;
	
	NOTIFYPARAM *param = (NOTIFYPARAM*)lParam;

	switch(wParam)
	{
		case NotifyReadSelection:
			OnSelectionRead((BOOL)param->wParam); 
			break;
		case NotifyUpdateMetrics:
			OnUpdateMetrics((INT)param->wParam, (Document::ITEMREAD*)param->lParam);
			break;
		case NotifyCacheModified:
			OnCacheModified();
			break;
		case NotifyItemShifted:
			OnItemShifted((Document::ITEMSHIFT*)param->lParam);
			break;
		case NotifyRangeReversed:
			OnRangeReversed((INT)param->wParam, (INT)param->lParam);
			break;
		case NotifyRangeReordered:
			OnRangeReordered((INT)param->wParam, (INT)param->lParam);
			break;
		case NotifyRangeRemoved:
			OnRangeRemoved((INT)param->wParam, (INT)param->lParam);
			break;
	}

	BaseListViewNotify_FinishMarshaling((UINT)wParam, param);
	return TRUE;
}

LRESULT BaseListView_SendNotification(HWND hwnd, UINT notifyCode, WPARAM wParam, LPARAM lParam)
{
	if (0 == BVM_NOTIFY && !BaseListViewNotify_RegisterMessage())
		return FALSE;

	NOTIFYPARAM notifyParam;
	notifyParam.wParam = wParam;
	notifyParam.lParam = lParam;
	notifyParam.mode = NotifyModeDirect;

	return SendMessage(hwnd, BVM_NOTIFY, notifyCode, (LPARAM)&notifyParam);
}

BOOL BaseListView_SendNotificationMT(HWND hwnd, UINT notifyCode, WPARAM wParam, LPARAM lParam)
{

	if (GetCurrentThreadId() == GetWindowThreadProcessId(hwnd, NULL))
		return (BOOL)BaseListView_SendNotification(hwnd, notifyCode, wParam, lParam);

	if (0 == BVM_NOTIFY && !BaseListViewNotify_RegisterMessage())
		return FALSE;


	NOTIFYPARAM *param = (NOTIFYPARAM*)malloc(sizeof(NOTIFYPARAM));
	if (NULL == param)
		return FALSE;

	param->wParam = wParam;
	param->lParam = lParam;
	param->mode = NotifyModeAsync;

	BaseListViewNotify_BeginMarshaling(notifyCode, param);

	BOOL result = SendNotifyMessage(hwnd, BVM_NOTIFY, notifyCode, (LPARAM)param);
	
	if (0 == result)
		BaseListViewNotify_FinishMarshaling(notifyCode, param);

	return result;
}


BOOL BaseListView_PostNotification(HWND hwnd, UINT notifyCode, WPARAM wParam, LPARAM lParam)
{
	if (0 == BVM_NOTIFY && !BaseListViewNotify_RegisterMessage())
		return FALSE;
	
	NOTIFYPARAM *param = (NOTIFYPARAM*)malloc(sizeof(NOTIFYPARAM));
	if (NULL == param)
		return FALSE;

	param->wParam = wParam;
	param->lParam = lParam;
	param->mode = NotifyModeAsync;

	BaseListViewNotify_BeginMarshaling(notifyCode, param);

	BOOL result = PostMessage(hwnd, BVM_NOTIFY, notifyCode, (LPARAM)param);
	
	if (0 == result)
		BaseListViewNotify_FinishMarshaling(notifyCode, param);

	return result;
}