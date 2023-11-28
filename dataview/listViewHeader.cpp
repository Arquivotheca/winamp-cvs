#include "main.h"
#include "./listViewHeader.h"


ListViewHeader::ListViewHeader(HWND _hwnd, ListView *_listView, ifc_viewcontents *_contents)
	: ViewWindow(_hwnd, "ListViewHeader", _contents), listView(_listView)
{
	if (NULL != listView)
		listView->AddRef();
}


ListViewHeader::~ListViewHeader()
{
	SafeRelease(listView);
}

HRESULT ListViewHeader::CreateInstance(ListView *listView, ListViewHeader **instance)
{
	HRESULT hr;
	HWND listWindow;
	HWND headerWindow;
	ListViewHeader *self;
	ifc_viewcontents *contents;

	if (NULL == instance)
		return E_POINTER;

	if (NULL == listView)
		return E_INVALIDARG;

	listWindow = listView->GetWindow();
	if (NULL == listWindow)
		return E_FAIL;

	headerWindow = (HWND)SendMessage(listWindow, LVM_GETHEADER, 0, 0L);
	if (NULL == headerWindow)
		return E_FAIL;

	if (FAILED(listView->GetContents(&contents)))
		return E_FAIL;

	self = new (std::nothrow) ListViewHeader(headerWindow, listView, contents);
	
	contents->Release();

	if (NULL == self)
		return E_OUTOFMEMORY;
	
	hr = self->AttachWindow();
	if (FAILED(hr))
	{
		self->Release();
		return hr;
	}

	self->AddRef();
	*instance = self;

	return S_OK;
}

HRESULT ListViewHeader::SwapColumns(int iOrder1, int iOrder2)
{
	int iItem1, iItem2;
	
	iItem1 = (int)SendMessage(hwnd, HDM_ORDERTOINDEX, (WPARAM)iOrder1, 0L);
	iItem2 = (int)SendMessage(hwnd, HDM_ORDERTOINDEX, (WPARAM)iOrder2, 0L);
	
	return listView->SwapColumns(iItem1, iItem2);
}

void ListViewHeader::OnContextMenu(HWND targetWindow, long cursor)
{
	aTRACE_LINE("ListViewHeader ContextMenu called");
}

BOOL ListViewHeader::OnItemChanging(int iItem, int iButton, HDITEM *item)
{
	if (0 != (HDI_WIDTH & item->mask))
	{
		ViewColumn *column;
			
		column = listView->GetColumn(iItem);
		if (NULL != column)
		{
			LengthUnit unit;
			
			long length;

			if (SUCCEEDED(column->GetMaxWidth(&unit)))
			{
				length = (long)(LengthUnit_GetHorzPx(&unit, &lengthConverter) + 0.5f);
				if (0 != length && item->cxy > length)
				item->cxy = length;
			}

			if (SUCCEEDED(column->GetMinWidth(&unit)))
			{
				length = (long)(LengthUnit_GetHorzPx(&unit, &lengthConverter) + 0.5);
				if (0 != length && item->cxy < length)
				item->cxy = length;
			}
		}
	}

	return FALSE;
}

void ListViewHeader::OnItemChanged(int iItem, int iButton, HDITEM *item)
{
	if (0 != (HDI_WIDTH & item->mask))
	{
		ViewColumn *column;
			
		column = listView->GetColumn(iItem);
		if (NULL != column)
		{
			LengthUnit unit;
			LengthUnit_Set(&unit, (float)item->cxy, UnitType_Pixel);

			column->SetWidth(&unit);
		}
	}
}

BOOL ListViewHeader::OnBeginTrack(int iItem, int iButton, HDITEM *item)
{
	ViewColumn *column;
	BOOL result;

	if (FALSE == LengthConverter_InitFromWindow(&lengthConverter, hwnd))
		memset(&lengthConverter, 0, sizeof(lengthConverter));
	
	result = FALSE;
			
	column = listView->GetColumn(iItem);
	if (NULL != column)
	{
		LengthUnit unit;
		long min, max;

		if (SUCCEEDED(column->GetMaxWidth(&unit)))
		{
			max = (long)(LengthUnit_GetHorzPx(&unit, &lengthConverter) + 0.5f);
			if (SUCCEEDED(column->GetMinWidth(&unit)))
			{
				min = (long)(LengthUnit_GetHorzPx(&unit, &lengthConverter) + 0.5f);
				
				if (min == max && 0 != min) // prevent resizing 
					result = TRUE;
			}
		}
	}

	return result;
}

void ListViewHeader::OnEndTrack(int iItem, int iButton, HDITEM *item)
{
}

BOOL ListViewHeader::OnBeginDrag(int iItem, int iButton, HDITEM *item)
{
	return FALSE;
}

BOOL ListViewHeader::OnEndDrag(int iItem, int iButton, HDITEM *item)
{
	HDITEM itemInfo;
	int srcOrder, dstOrder; 

	itemInfo.mask = HDI_ORDER;
	if (FALSE == SendMessage(hwnd, HDM_GETITEM, (WPARAM)iItem, (LPARAM)&itemInfo))
		return TRUE;
	
	srcOrder = item->iOrder;
	dstOrder = itemInfo.iOrder;

	if (srcOrder < dstOrder)
	{
		for(;srcOrder < dstOrder; srcOrder++)
		{
			if (FAILED(SwapColumns(srcOrder, srcOrder + 1)))
				return TRUE;
		}
	}
	else if (srcOrder > dstOrder)
	{
		for(;srcOrder > dstOrder; srcOrder--)
		{
			if (FAILED(SwapColumns(srcOrder, srcOrder - 1)))
				return TRUE;
		}
	}
		
	return FALSE;
}

LRESULT ListViewHeader::OnNotification(NMHDR *notification)
{
	switch(notification->code)
	{		
		case HDN_ITEMCHANGING:
			{
				NMHEADER *headerInfo;
				headerInfo = (NMHEADER*)notification;
				return OnItemChanging(headerInfo->iItem, headerInfo->iButton, headerInfo->pitem);
			}
			return TRUE;
		case HDN_ITEMCHANGED:
			{
				NMHEADER *headerInfo;
				headerInfo = (NMHEADER*)notification;
				OnItemChanged(headerInfo->iItem, headerInfo->iButton, headerInfo->pitem);
			}
			return TRUE;
		case HDN_BEGINTRACK:
			{
				NMHEADER *headerInfo;
				headerInfo = (NMHEADER*)notification;
				return OnBeginTrack(headerInfo->iItem, headerInfo->iButton, headerInfo->pitem);
			}
			return TRUE;
		case HDN_ENDTRACK:
			{
				NMHEADER *headerInfo;
				headerInfo = (NMHEADER*)notification;
				OnEndTrack(headerInfo->iItem, headerInfo->iButton, headerInfo->pitem);
			}
			return TRUE;
		case HDN_BEGINDRAG:
			{
				NMHEADER *headerInfo;
				headerInfo = (NMHEADER*)notification;
				return OnBeginDrag(headerInfo->iItem, headerInfo->iButton, headerInfo->pitem);
			}
			return TRUE;
		case HDN_ENDDRAG:
			{
				NMHEADER *headerInfo;
				headerInfo = (NMHEADER*)notification;
				return OnEndDrag(headerInfo->iItem, headerInfo->iButton, headerInfo->pitem);
			}
			return TRUE;
	}
	return 0;
}

HRESULT ListViewHeader::ReflectedMessage(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
	switch(message)
	{
		case WM_NOTIFY:
			*result = OnNotification((NMHDR*)lParam);
			return S_OK;
	}
	return __super::ReflectedMessage(message, wParam, lParam, result);
}
