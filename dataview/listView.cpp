#include "main.h"
#include "./listView.h"
#include "./listViewHeader.h"
#include "./columnSorter.h"
#include "./viewColumnSerializer.h"

#include "./contextMenu.h"
#include "./menuGroup.h"
#include "./menuItem.h"
#include "./actionContextEnum.h"

#include "./testGroupAction.h"
#include "./testItemAction.h"

#include <strsafe.h>


static unsigned int LISTVIEW_WM_COMMITTRANSACTION = WM_NULL;

ListView::ListView(HWND _hwnd, const char *_name, ifc_viewcontents *_contents)
	: ViewWindow(_hwnd, _name, _contents), listFlags(ListFlag_None), updateLock(0),
	  sortColumn(NULL), sortOrder(SortOrder_Undefined), selectionTransaction(NULL),
	  sortedCount(0), listCount(0)
{
	if (NULL != contents)
	{
		if (FAILED(contents->GetSortColumn(&sortColumn)))
			sortColumn = NULL;
		
		if (NULL == sortColumn)
		{
			if (FAILED(contents->GetPrimaryColumn(&sortColumn)))
				sortColumn = NULL;
		}
			
		contents->GetSortOrder(&sortOrder);
	}

	localeId = Plugin_GetUserLocaleId();

	ifc_dataobject *summary;
	if (S_OK == GetSummaryObject(&summary))
	{
		listFlags |= ListFlag_SummaryEnabled;
		summary->Release();
	}
}

ListView::~ListView()
{
	size_t index;

	SafeRelease(selectionTransaction);

	index = columnList.size();
	while(index--)
	{
		columnList[index]->Release();
	}

	SafeRelease(sortColumn);
}

HWND ListView::CreateInstance(ifc_viewcontents *contents, 
							  unsigned int windowStyleEx, unsigned int windowStyle,
							  HWND parentWindow, int x, int y, int width, int height, int controlId)
{
	HWND hwnd;
	unsigned long styleEx;
	INITCOMMONCONTROLSEX iccex;
	ListView *self;
	
	if (NULL == contents)
		return NULL;

	iccex.dwSize = sizeof(iccex);
	iccex.dwICC = ICC_LISTVIEW_CLASSES;
	if (FALSE == InitCommonControlsEx(&iccex))
		return NULL;

	hwnd = CreateWindowEx(windowStyleEx, WC_LISTVIEW, NULL,
						  windowStyle |
						  LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA, 
						  x, y, width, height, 
						  parentWindow, (HMENU)controlId, 
						  Plugin_GetInstance(), NULL);

	if (NULL == hwnd)
		return NULL;

	SendMessage(hwnd, CCM_SETVERSION, 5, 0L);
	SendMessage(hwnd, CCM_SETUNICODEFORMAT, TRUE, 0);

	Plugin_SetControlTheme(hwnd, SKINNEDWND_TYPE_LISTVIEW,
						   SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | 
						   SWLVS_FULLROWSELECT | SWLVS_DOUBLEBUFFER | SWLVS_ALTERNATEITEMS);

	styleEx = LVS_EX_HEADERDRAGDROP;
	SendMessageW(hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx, styleEx);

	self = new (std::nothrow) ListView(hwnd, "ListView", contents);
	if (NULL == self)
	{
		DestroyWindow(hwnd);
		return NULL;
	}

	if (FAILED(self->AttachWindow()))
	{
		self->Release();
		return NULL;
	}

	return hwnd;
}

HRESULT ListView::AttachWindow()
{
	HRESULT hr;
	ListViewHeader *header;
		
	hr = __super::AttachWindow();
	if (FAILED(hr))
		return hr;
	
	if (WM_NULL == LISTVIEW_WM_COMMITTRANSACTION)
		LISTVIEW_WM_COMMITTRANSACTION = RegisterWindowMessage(L"LISTVIEW_WM_COMMITTRANSACTION");

	ReloadColumns();
	UpdateHeader();

	if (SUCCEEDED(ListViewHeader::CreateInstance(this, &header)))
		header->Release();
		
	UpdateItems(FALSE);
	UpdateSort(FALSE);


	return S_OK;
}

void ListView::OnRedrawEnabled(BOOL enabled)
{
	if (0 != (ListFlag_BypassSetRedraw & listFlags))
		return;

	if (FALSE == enabled)
	{
		LockUpdates();
	}
	else
	{
		UnlockUpdates();
	}

	__super::OnRedrawEnabled(enabled);
}

void ListView::OnContextMenu(HWND targetWindow, long cursor)
{
	POINT pt;
	LVHITTESTINFO hitTest;
	size_t selectedCount;
	BOOL extendedMode;

	POINTSTOPOINT(pt, cursor);

	CommitSelectionTransaction();

	selectedCount = (size_t)SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0L);

	if (-1 == pt.x && -1 == pt.y)
	{
		int iMark;
		RECT clientRect;

		if (FALSE == GetClientRect(hwnd, &clientRect))
			return;

		pt.x = clientRect.left;
		pt.y = clientRect.top;

		if (0 != selectedCount)
		{
			iMark = (int)SendMessage(hwnd, LVM_GETSELECTIONMARK, 0, 0L);
			if (-1 != iMark)
			{
				if (0 == (unsigned int)SendMessage(hwnd, LVM_GETITEMSTATE, (WPARAM)iMark, (LPARAM)LVIS_SELECTED))
					iMark = -1;
			}

			if (-1 == iMark)
			{
				int iSelected;

				iSelected = (int)SendMessage(hwnd, LVM_GETNEXTITEM, (WPARAM)-1, 
								(LPARAM)(LVNI_ALL | LVNI_SELECTED));
				
				if (-1 != iSelected)
				{
					
					iMark = (int)SendMessage(hwnd, LVM_GETNEXTITEM, (WPARAM)iSelected, 
								(LPARAM)(LVNI_ALL | LVNI_SELECTED));
					if (-1 == iMark)
						iMark = iSelected;
				}
			}
			
			if (-1 != iMark)
			{
				RECT itemRect;
				
				itemRect.left = LVIR_ICON;
				if (FALSE != SendMessage(hwnd, LVM_GETITEMRECT, iMark, (LPARAM)&itemRect))
				{
					pt.x = itemRect.left + RECTWIDTH(itemRect)/2;
					pt.y = itemRect.top + RECTHEIGHT(itemRect)/2;

					if (pt.x < clientRect.left || pt.x > clientRect.right)
						pt.x = clientRect.left;
					
					if (pt.y < clientRect.top || pt.y > clientRect.bottom)
						pt.y = clientRect.top;
				}
			}
		}
		MapWindowPoints(hwnd, HWND_DESKTOP, &pt, 1);
	}

	hitTest.pt = pt;
	MapWindowPoints(HWND_DESKTOP, hwnd, &hitTest.pt, 1);

	extendedMode = (0 != (0x8000 & GetKeyState(VK_SHIFT)));

	if (0 != selectedCount &&
		-1 != (int)SendMessage(hwnd, LVM_HITTEST,  0, (LPARAM)&hitTest))
	{
		ShowItemContextMenu(pt, extendedMode);
	}
	else
	{
		ShowViewContextMenu(pt, extendedMode);
	}

}

int ListView::OnFindItem(int iStart, LVFINDINFO *findInfo)
{
	return -1;
}

void ListView::OnGetDisplayInfo(LVITEM *item)
{
	if (0 != (LVIF_TEXT & item->mask))
	{
		ViewColumn *column;
		
		item->pszText[0]= L'\0';
		
		column = GetColumn(item->iSubItem);
		if (NULL != column)	
		{			
			ifc_dataobject *object;
			if (SUCCEEDED(GetItem(item->iItem, &object)))
			{				
				column->Format(localeId, object, item->pszText, item->cchTextMax);
				object->Release();
			}
		}
	}
}

void ListView::OnColumnClick(int iColumn)
{
	ViewColumn *viewColumn;
	BOOL columnChanged;
	ifc_viewcolumn *column;
	
	viewColumn = GetColumn(iColumn);
	if (FAILED(viewColumn->GetBase(&column)))
		column = NULL;
	
	if (NULL == column)
		return;

	columnChanged = TRUE;

	if (NULL != sortColumn && 
		0 == Column_CompareByName(column, sortColumn))
	{
		columnChanged = FALSE;
		if (SortOrder_Ascending == sortOrder)
			sortOrder = SortOrder_Descending;
		else
			sortOrder = SortOrder_Ascending;
	}
	else
	{
		SafeRelease(sortColumn);

		sortColumn = column;
		sortColumn->AddRef();
		sortOrder = SortOrder_Ascending;

		if (NULL != contents)
			contents->SetSortColumn(Column_GetName(sortColumn));
	}

	if (NULL != contents)
		contents->SetSortOrder(sortOrder);
	
	column->Release();

	InvalidateSort(0);
	UpdateSort(FALSE);
	SyncSortArrow();
}

unsigned int ListView::OnCustomDraw(int iItem, int iSubItem, NMCUSTOMDRAW *drawData, COLORREF backColor, COLORREF textColor)
{
	return CDRF_DODEFAULT;
}

void ListView::OnGetInfoTip(int iItem, int iSubItem, wchar_t *buffer, size_t bufferMax, BOOL unfolded)
{
}

void ListView::OnKeyDown(unsigned short vKey)
{
}

void ListView::OnReturnPressed()
{
}

void ListView::OnDoubleClick(POINT pt)
{
}

void ListView::OnStateChange(int iFrom, int iTo, unsigned int newState, unsigned int oldState)
{
	if (0 != (LVIS_SELECTED & (newState ^ oldState)) &&
		0 == (ListFlag_UpdatingSelection & listFlags))
	{
		BOOL transactionStarted;
		
		transactionStarted = FALSE;

		aTRACE_FMT("%s(0x%08X) selection change: view(%d,%d,%s)\r\n", GetName(), hwnd, 
						iFrom, iTo, ((0 != (LVIS_SELECTED & newState)) ? "+" : "-"));

		if (FALSE == FilterSelectionChange(&iFrom, &iTo, (0 != (LVIS_SELECTED & newState))))
			return;

		if (NULL == selectionTransaction)
		{
			ifc_viewselection *selection;
			ifc_viewgroupfilter *groupFilter;

			if (SUCCEEDED(GetSelectionTracker(&selection)))
			{
				if (FAILED(selection->CreateTransaction(&selectionTransaction)))
					selectionTransaction = NULL;
				else
					transactionStarted = TRUE;

				selection->Release();
			}

			filterBypassTransaction = FALSE;
			if (SUCCEEDED(GetGroupFilter(&groupFilter)))
			{
				if (S_OK == groupFilter->IsBypassEnabled())
					filterBypassTransaction = TRUE;
				groupFilter->Release();
			}
		}
		
		

		NotifySelectionChange(iFrom, iTo, (0 != (LVIS_SELECTED & newState)));

		if (FALSE != transactionStarted)
		{
			if (WM_NULL == LISTVIEW_WM_COMMITTRANSACTION ||
				FALSE == PostMessage(hwnd, LISTVIEW_WM_COMMITTRANSACTION, 0, 0L))
			{
				CommitSelectionTransaction();
			}
		}
	}
}

void ListView::OnFocusChanged(BOOL focusReceived)
{
}

BOOL ListView::OnMarqueeBegin()
{
	listFlags |= ListFlags_MarqueeSelection;
	return FALSE;
}

void ListView::OnReleasedCapture()
{
	if (0 != (ListFlags_MarqueeSelection & listFlags))
	{
		listFlags &= ~ListFlags_MarqueeSelection;
		
		if (0 == (int)SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0L))
		{
			ifc_viewgroupfilter *groupFilter;
			
			if (SUCCEEDED(GetGroupFilter(&groupFilter)))
			{
				groupFilter->EnableBypass(TRUE);
				groupFilter->Release();
			}
		}
	}
}

LRESULT ListView::OnNotification(NMHDR *notification)
{
	switch(notification->code)
	{
		case LVN_ODFINDITEM:
			{
				NMLVFINDITEM *findItem;
				findItem = (NMLVFINDITEM*)notification;
				return OnFindItem(findItem->iStart, &findItem->lvfi);
			}
			return TRUE;
		case LVN_GETDISPINFO:
			OnGetDisplayInfo(&((NMLVDISPINFO*)notification)->item);
			return TRUE;
		case NM_CUSTOMDRAW:
			{
				NMLVCUSTOMDRAW *customDraw;
				customDraw = (NMLVCUSTOMDRAW*)notification;
				return OnCustomDraw(customDraw->nmcd.dwItemSpec, customDraw->iSubItem,
									&customDraw->nmcd, customDraw->clrTextBk, customDraw->clrText);
			}
			return TRUE;
		case LVN_COLUMNCLICK:
			OnColumnClick(((NMLISTVIEW*)notification)->iSubItem);
			return TRUE;
		case LVN_GETINFOTIP:
			{
				NMLVGETINFOTIP *infoTip;
				infoTip = (NMLVGETINFOTIP*)notification;
				OnGetInfoTip(infoTip->iItem, infoTip->iSubItem, infoTip->pszText, 
							 infoTip->cchTextMax, 0 != (LVGIT_UNFOLDED & infoTip->dwFlags));
			}
			return TRUE;
		case LVN_KEYDOWN:
			OnKeyDown(((NMLVKEYDOWN*)notification)->wVKey);
			return FALSE;
		case LVN_ODSTATECHANGED:
			{
				NMLVODSTATECHANGE *stateChange;
				stateChange = (NMLVODSTATECHANGE*)notification;
				OnStateChange(stateChange->iFrom, stateChange->iTo, stateChange->uNewState, stateChange->uOldState);
			}
			return TRUE;
		case LVN_ITEMCHANGED:
			{
				NMLISTVIEW *listView;
				listView = (NMLISTVIEW*)notification;
				if (0 != (LVIF_STATE & listView->uChanged))	
					OnStateChange(listView->iItem, listView->iItem, listView->uNewState, listView->uOldState);
			}
			return TRUE;
		case NM_RETURN:
			OnReturnPressed();
			return TRUE;
		case NM_DBLCLK:
			OnDoubleClick(((NMITEMACTIVATE*)notification)->ptAction);
			return TRUE;
		case NM_KILLFOCUS:
			OnFocusChanged(FALSE);
			return TRUE;
		case NM_SETFOCUS:
			OnFocusChanged(TRUE);
			return TRUE;
		case LVN_MARQUEEBEGIN:
			return OnMarqueeBegin();
		case NM_RELEASEDCAPTURE:
			OnReleasedCapture();
			break;

	}
	return FALSE;
}

HRESULT ListView::ReflectedMessage(unsigned int message, WPARAM wParam, LPARAM lParam, LRESULT *result)
{	
	switch(message)
	{
		case WM_NOTIFY:
			*result = OnNotification((NMHDR*)lParam);
			return S_OK;
	}

	return __super::ReflectedMessage(message, wParam, lParam, result);
}

BOOL ListView::ReloadColumns()
{
	HRESULT hr;
	ViewColumnSerializer serializer;
	ViewColumnEnum *enumerator;
	ifc_viewconfig *config;
	char buffer[8192];

	columnList.clear();

	if (SUCCEEDED(GetConfig(&config)) && NULL != config)
	{
		config->ReadString("columns", NULL, buffer, ARRAYSIZE(buffer));
		config->Release();
	}
	else
		buffer[0] = '\0';

	if ('\0' == buffer[0])
	{
		ifc_viewcontroller *controller;
		if (SUCCEEDED(GetController(&controller)))
		{
			char *defaultColumns;
			if (SUCCEEDED(controller->GetDefaultColumns(this, &defaultColumns)) && 
				NULL != defaultColumns)
			{
				if (FAILED(StringCchCopyA(buffer, ARRAYSIZE(buffer), defaultColumns)))
					buffer[0] = '\0';

				controller->FreeString(defaultColumns);
			}
			controller->Release();
		}
	}
		
	hr = serializer.Deserialize(buffer, -1, &enumerator);
	if (SUCCEEDED(hr))
	{
		ViewColumn *column;
		ifc_viewcolumn *base;
		size_t count;

		if (SUCCEEDED(enumerator->GetCount(&count)))
			columnList.reserve(count);

		while(S_OK == enumerator->Next(&column, 1, NULL))
		{
			if (S_OK == contents->FindColumn(column->GetName(), &base))
			{
				column->SetBase(base);
				base->Release();
			}
			columnList.push_back(column);
		}

		enumerator->Release();
	}

	return SUCCEEDED(hr);
}

BOOL ListView::UpdateHeader()
{
	LVCOLUMN columnInfo;
	size_t index, count, inserted, columnIndex, dummyOffset;
	wchar_t buffer[512];
	ViewColumn *column;
	LengthUnit unit;
	LengthConverter lengthConverter;
	ifc_dataprovider *provider;
	HWND headerWindow;
	long length;
	unsigned long windowStyle;
		
	if (NULL == hwnd || NULL == contents)
		return FALSE;

	windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle))
		SetRedrawInternal(FALSE);
	
	headerMap.clear();
	while(FALSE != SendMessage(hwnd, LVM_DELETECOLUMN, (WPARAM)0, 0L));
		
	columnInfo.mask = LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
	columnInfo.fmt = LVCFMT_LEFT;
	
	count = columnList.size();
	inserted  = 0;

	columnInfo.cx = 0;
	columnInfo.pszText = L"";

	headerWindow = (HWND)SendMessage(hwnd, LVM_GETHEADER, 0, 0L);
	if (FALSE == LengthConverter_InitFromWindow(&lengthConverter, (NULL != headerWindow) ? headerWindow : hwnd))
		return FALSE;

	if (-1 != SendMessage(hwnd, LVM_INSERTCOLUMN, (WPARAM)0, (LPARAM)&columnInfo))
		dummyOffset = 1;
	else
		dummyOffset = 0;

	if (FAILED(contents->GetProvider(&provider)))
		provider = NULL;

	for (index = 0; index < count; index++) 
	{
		column = columnList[index];
		if (FALSE != column->IsVisible())
		{
			if (NULL == provider || 
				FAILED(provider->GetColumnDisplayName(column->GetName(), buffer, ARRAYSIZE(buffer))) || 
				L'\0' == buffer[0])
			{
				if (FAILED(column->GetDisplayName(buffer, ARRAYSIZE(buffer))))
					StringCchPrintf(buffer, ARRAYSIZE(buffer), L"[Error]");
			}
			
			columnInfo.fmt &= ~LVCFMT_JUSTIFYMASK;
			switch(column->GetAlignMode())
			{
				case ifc_viewcolumninfo::AlignMode_Center:
					columnInfo.fmt |= LVCFMT_CENTER;
					break;
				case ifc_viewcolumninfo::AlignMode_Right:
					columnInfo.fmt |= LVCFMT_RIGHT;
					break;
				default:
					columnInfo.fmt |= LVCFMT_LEFT;
					break;
			}

			columnInfo.pszText = buffer;

			if (FAILED(column->GetWidth(&unit)) && 
				FAILED(column->GetMinWidth(&unit)))
			{
				LengthUnit_Set(&unit, 0.0f, UnitType_Pixel);
			}

			columnInfo.cx = (int)(LengthUnit_GetHorzPx(&unit, &lengthConverter) + 0.5f);
			
			if (SUCCEEDED(column->GetMaxWidth(&unit)))
			{
				length = (long)(LengthUnit_GetHorzPx(&unit, &lengthConverter) + 0.5f);
				if (0 != length && columnInfo.cx > length)
					columnInfo.cx = length;
			}

			if (SUCCEEDED(column->GetMinWidth(&unit)))
			{
				length = (long)(LengthUnit_GetHorzPx(&unit, &lengthConverter) + 0.5);
				if (0 != length && columnInfo.cx < length)
					columnInfo.cx = length;
			}
		
			columnIndex = (size_t)SendMessage(hwnd, LVM_INSERTCOLUMN, (WPARAM)index + dummyOffset, (LPARAM)&columnInfo);
			if (-1 != columnIndex)
			{	
				headerMap.push_back(index);
				inserted++;
			}
		}
	}

	SafeRelease(provider);

	while(dummyOffset--)
		SendMessage(hwnd, LVM_DELETECOLUMN, (WPARAM)0, 0L);

	SyncSortArrow();

	if (0 != (WS_VISIBLE & windowStyle))
		SetRedrawInternal(TRUE);

	
	return TRUE;
}

BOOL ListView::UpdateItems(BOOL immediate)
{
	size_t count;
	unsigned long windowStyle;

	if (0 != updateLock && FALSE == immediate)
	{
		listFlags |= ListFlag_UpdateItems;
		return FALSE;
	}
	else
	{
		listFlags &= ~ListFlag_UpdateItems;
	}

	count = 0;
		
	if (NULL != contents)
	{
		ifc_dataobjectlist *list;
		if (S_OK == contents->GetObjects(&list))
		{
			count = list->GetCount();
			list->Release();
		}
	}

	listCount = (size_t)SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0L);
	if (0 != (ListFlag_SummaryEnabled & listFlags) && listCount > 0)
		listCount--;

	if (count == listCount)
	{
		if (0 != (ListFlag_SummaryEnabled & listFlags))
			count++;

		return SendMessage(hwnd, LVM_REDRAWITEMS, 0, (LPARAM)count);
	}

	if (0 != (ListFlag_SummaryEnabled & listFlags))
		count++;
		
	windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle))
		SetRedrawInternal(FALSE);
	
//	SetItemState(-1, 0, /*LVIS_SELECTED | */LVIS_FOCUSED);
//	SetItemCount(0, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	SetItemCount(count, 0);
	
	//SendMessage(hwnd, LVM_SETSELECTIONMARK, 0, (LPARAM)-1);

	if (0 != (WS_VISIBLE & windowStyle))
	{
		SetRedrawInternal(TRUE);
		InvalidateRect(hwnd, NULL, TRUE);
	}

	return TRUE;
}

BOOL ListView::SyncSortArrow()
{	
	int iColumn;
	const char *name;
	BOOL sortAscending;

	iColumn = -1;

	name = Column_GetName(sortColumn);
	if (NULL != name)
	{
		size_t index;
		index = headerMap.size();
		
		while(index--)
		{
			if (0 == ColumnInfo_CompareNames(columnList[headerMap[index]]->GetName(), -1, name, -1))
			{
				iColumn = index;
				break;
			}
		}
	}

	sortAscending = (SortOrder_Descending == sortOrder) ? FALSE : TRUE;
	return MLSkinnedListView_DisplaySort(hwnd, iColumn, sortAscending);

}

void ListView::UpdateSelection(BOOL immediate)
{
	ifc_viewselection *selection;
	ifc_viewselectionenum *enumerator;
	IndexRange range;
	int iItem, offset;
	LVITEM itemInfo;
	BOOL selectionUpdated;


	if (0 != updateLock && FALSE == immediate)
	{
		listFlags |= ListFlag_UpdateSelection;
		return;
	}
	else
	{
		listFlags &= ~ListFlag_UpdateSelection;
	}

	listFlags |= ListFlag_UpdatingSelection;

	if (0 != (ListFlag_SummaryEnabled & listFlags))
		offset = 1;
	else
		offset = 0;

	selectionUpdated = FALSE;

	
	if (0 != (ListFlag_SummaryEnabled & listFlags))
	{
		ifc_viewgroupfilter *groupFilter;
		if (SUCCEEDED(GetGroupFilter(&groupFilter)))
		{
			if (S_OK == groupFilter->IsBypassEnabled())
			{
				itemInfo.state = 0;
				itemInfo.stateMask = LVIS_SELECTED;
				SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&itemInfo);

				itemInfo.state = LVIS_SELECTED;
				SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)0, (LPARAM)&itemInfo);

				selectionUpdated = TRUE;
			}

			groupFilter->Release();
		}
	}

	if (FALSE == selectionUpdated && 
		SUCCEEDED(GetSelectionTracker(&selection)))
	{
		if (listCount == selection->GetCount())
		{
			itemInfo.state = LVIS_SELECTED;
			itemInfo.stateMask = LVIS_SELECTED;

			SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&itemInfo);
			if (0 != (ListFlag_SummaryEnabled & listFlags))
			{				
				itemInfo.state = 0;
				SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)0, (LPARAM)&itemInfo);
			}

			selectionUpdated = TRUE;
		}
		else
		{
			if (SUCCEEDED(selection->Enumerate(&enumerator)))
			{
				size_t index, mapSize;

				itemInfo.state = 0;
				itemInfo.stateMask = LVIS_SELECTED;
				SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&itemInfo);

				itemInfo.state = LVIS_SELECTED;
				
				mapSize = indexMap.size();

				while (S_OK == enumerator->Next(&range, 1, NULL))
				{
					for(index = range.first; index <= range.last; index++)
					{
						if (index < mapSize)
						{
							iItem = indexMap[index] + offset;
							SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&itemInfo);
						}
					}
				}
				enumerator->Release();

				selectionUpdated = TRUE;
			}
		}

		selection->Release();
	}

	if (FALSE == selectionUpdated)
	{
		itemInfo.state = 0;
		itemInfo.stateMask = LVIS_SELECTED;
		SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&itemInfo);
	}

	listFlags &= ~ListFlag_UpdatingSelection;
}

void ListView::InvalidateSort(size_t validCount)
{
	if (validCount < sortedCount)
		sortedCount = validCount;
}

HRESULT ListView::UpdateSort(BOOL immediate)
{
	HRESULT hr;
	size_t index, count;
	ifc_dataobjectlist *objectList;
	size_t focusedItem;
	
	if (sortedCount == itemMap.size() &&
		listCount == itemMap.size())
	{
		return S_FALSE;
	}

	if (0 != updateLock && FALSE == immediate)
	{
		listFlags |= ListFlag_UpdateSort;
		return S_OK;
	}
	else
	{
		listFlags &= ~ListFlag_UpdateSort;
	}
		
	focusedItem = (size_t)SendMessage(hwnd, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_FOCUSED);
	if (0 != (ListFlag_SummaryEnabled & listFlags))
		focusedItem--;

	if (focusedItem < itemMap.size())
		focusedItem = itemMap[focusedItem];

	if (NULL == contents)
		return E_UNEXPECTED;

	hr = contents->GetObjects(&objectList);
	if (S_OK != hr)
		return hr;
	
	count = objectList->GetCount();
	if (listCount != count)
		UpdateItems(immediate);
	
	itemMap.resize(count);
	indexMap.resize(count);
	
	for (index = sortedCount; index < count; index++)
		itemMap[index] = index;
	
	indexMap.resize(count);

	if (NULL != sortColumn)
	{
		ColumnSorter columnSorter(sortColumn, sortOrder, contents);

		/* performance test */
		//	LARGE_INTEGER start, stop;
		//	char buffer[128];
		//	QueryPerformanceCounter(&start);
		/* performance test */

		columnSorter.Reorder2(objectList, itemMap.begin(), itemMap.size(), sortedCount);

		/* performance test */
		//	QueryPerformanceCounter(&stop);
		//	StringCchPrintfA(buffer, ARRAYSIZE(buffer), "%s sort2: %I64u\r\n", 
		//				((NULL != contents) ? contents->GetName() : name), 
		//				stop.QuadPart - start.QuadPart);
		//	OutputDebugStringA(buffer);
		/* performance test */
	}

	objectList->Release();

	for (index = 0; index < count; index++)
		indexMap[itemMap[index]] = index;

	sortedCount = itemMap.size();

	UpdateSelection(immediate);
	
	if (focusedItem < indexMap.size())
	{
		int iItem;

		iItem = indexMap[focusedItem];
		if (0 != (ListFlag_SummaryEnabled & listFlags))
			iItem++;

		SetFocusItem(iItem, TRUE);
	}
	else if (0 != (ListFlag_SummaryEnabled & listFlags) && 
			 (size_t)-1 == focusedItem)
	{
		SetFocusItem(0, TRUE);
	}

	SendMessage(hwnd, LVM_REDRAWITEMS, 0, (LPARAM)count);
	return hr;
}

HRESULT ListView::SetItemCount(size_t count, unsigned int flags)
{
	listCount = count;
	if (0 != (ListFlag_SummaryEnabled & listFlags) && 
		listCount > 0)
	{
		listCount--;
	}

	if (0 == SendMessage(hwnd, LVM_SETITEMCOUNT, (WPARAM)count, (LPARAM)flags))
		return E_FAIL;
	return S_OK;
}

HRESULT ListView::SetItemState(int iItem, unsigned int state, unsigned int stateMask)
{
	LVITEM itemInfo;

	itemInfo.state = state;
	itemInfo.stateMask = stateMask;

	if (FALSE == SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&itemInfo))
		return E_FAIL;

	return S_OK;
}
HRESULT ListView::SetFocusItem(int iItem, BOOL focused)
{
	unsigned int state;

	state = (FALSE != focused) ? LVIS_FOCUSED : 0;
	return SetItemState(iItem, state, LVIS_FOCUSED);
	
}

HRESULT ListView::SetSelectedItem(int iItem, BOOL selected, BOOL silentMode)
{
	HRESULT hr;
	BOOL resetFlag;
	unsigned int state;

	if (FALSE != silentMode &&
		0 == (ListFlag_UpdatingSelection & listFlags))
	{
		listFlags |= ListFlag_UpdatingSelection;
		resetFlag = TRUE;
	}
	else
		resetFlag = FALSE;

	state = (FALSE != selected) ? LVIS_SELECTED : 0;
	hr = SetItemState(iItem, state, LVIS_SELECTED);

	if (FALSE != resetFlag)
		listFlags &= ~ListFlag_UpdatingSelection;

	return hr;
}

void ListView::SetRedrawInternal(BOOL enableRedraw)
{
	listFlags |= ListFlag_BypassSetRedraw;
	SendMessage(hwnd, WM_SETREDRAW, (WPARAM)enableRedraw, 0L);
	listFlags &= ~ListFlag_BypassSetRedraw;
}

void ListView::ShowItemContextMenu(POINT pt, BOOL extendedMode)
{
	MenuGroup *group;
	ActionContextEnum *contextEnum;

	TestItemAction *testAction;
	
	if (FAILED(ActionContextEnum::CreateInstance(&contextEnum)))
		return;

	contextEnum->Add((ifc_viewwindow*)this);
	if (NULL != contents)
	{
		ifc_dataobjectenum *selection;

		contextEnum->Add(contents);
		if (S_OK == contents->GetSelection(&selection))
		{
			size_t selectionCount;
			if (SUCCEEDED(selection->GetCount(&selectionCount)))
				contextEnum->Add(selection);
			
			selection->Release();
		}
	}

	if (FAILED(TestItemAction::CreateInstance(L"Simple Menu Action", &IFC_ViewActionContextEnum, &testAction)))
		testAction = NULL;

	if (SUCCEEDED(MenuGroup::CreateInstance("root", L"Root", NULL, NULL, &group)))
	{
		MenuGroup_InsertItem(group, 0, "item1", L"Item1", MenuStyle_Normal, MenuState_Normal, testAction);
		MenuGroup_InsertItem(group, 1, "item2", L"Item2", MenuStyle_Normal, MenuState_Normal, testAction);
		MenuGroup_InsertItem(group, 2, "item3", L"Item3 (Disabled)", MenuStyle_Normal, MenuState_Disabled, testAction);
		MenuGroup_InsertItem(group, 3, "item4", L"Item4 (Checked)", MenuStyle_Normal, MenuState_Checked, testAction);
		MenuGroup_InsertItem(group, 4, "item5", L"Item5 (RadioChecked)", MenuStyle_RadioCheck, MenuState_Checked, testAction);
		MenuGroup_InsertItem(group, 5, "item6", L"Separator", MenuStyle_Separator, MenuState_Normal, NULL);
		MenuGroup_InsertItem(group, 6, "item7", L"Item7 (Last)", MenuStyle_Normal, MenuState_Normal, testAction);

		
		TestGroupAction *groupAction;
		if (SUCCEEDED(TestGroupAction::CreateInstance(extendedMode, &groupAction)))
		{
			MenuGroup *group2;
			if (SUCCEEDED(MenuGroup::CreateInstance("subgroup", L"SubGroup", NULL, groupAction, &group2)))
			{
				group->Insert(7, group2);
				group2->Release();
			}
			groupAction->Release();
		}
			

		if (0 != group->GetCount())
		{
			ContextMenu *contextMenu;
			if (SUCCEEDED(ContextMenu::CreateInstance(contextEnum, group, extendedMode, &contextMenu)))
			{
				contextMenu->Show(0, pt.x, pt.y, hwnd, NULL);
				contextMenu->Release();
			}
		}

		group->Release();
	}
	
	if (NULL != testAction)
		testAction->Release();

	contextEnum->Release();
}

void ListView::ShowViewContextMenu(POINT pt, BOOL extendedMode)
{
	ShowItemContextMenu(pt, extendedMode);
}

size_t ListView::LockUpdates()
{
	return InterlockedIncrement(&updateLock);
	
}
size_t ListView::UnlockUpdates()
{
	size_t result;

	if (0 == updateLock)
		return 0;

	result = InterlockedDecrement(&updateLock);
	if (0 == result)
	{
		if (0 != (ListFlag_UpdateItems & listFlags))
			UpdateItems(FALSE);

		if (0 != (ListFlag_UpdateSort & listFlags))
			UpdateSort(FALSE);

		if (0 != (ListFlag_UpdateSelection & listFlags))
			UpdateSelection(FALSE);
	}

	return result;
}

HRESULT ListView::GetItem(size_t iItem, ifc_dataobject **object)
{		
	ifc_dataobjectlist *objectList;

	if (0 != (ListFlag_SummaryEnabled & listFlags))
	{
		if (0 == iItem)
		{
			if (S_OK == GetSummaryObject(object))
				return S_OK;

			return E_FAIL;
		}
		
		iItem--;
	}

	if (NULL == object)
		return E_POINTER;

	if (NULL == contents)
		return E_UNEXPECTED;
			
	if (FAILED(UpdateSort(TRUE)))
		return E_FAIL;

	if (iItem >= itemMap.size())
		return E_INVALIDARG;

	if (S_OK != contents->GetObjects(&objectList))
		return E_FAIL;

	*object = objectList->GetItem(itemMap[iItem]);
	if (NULL != *object)
		(*object)->AddRef();

	objectList->Release();
	
	return S_OK;
}

ViewColumn *ListView::GetColumn(size_t iColumn)
{
	if (iColumn >= headerMap.size())
		return NULL;

	return columnList[headerMap[iColumn]];
}

HRESULT ListView::SwapColumns(int iColumn1, int iColumn2)
{
	size_t count;
	size_t index1, index2;
	ViewColumn *column;

	count = headerMap.size() ;
	if ((size_t)iColumn1 >= count || (size_t)iColumn2 >= count)
		return E_INVALIDARG;

	index1 = headerMap[iColumn1];
	index2 = headerMap[iColumn2];

	count = columnList.size();
	if (index1 >= count || index2 >= count)
		return E_FAIL;
	
	column = columnList[index1];
	columnList[index1] = columnList[index2];
	columnList[index2] = column;

	index1 = headerMap[iColumn1];
	headerMap[iColumn1] = headerMap[iColumn2];
	headerMap[iColumn2] = index1;

	return S_OK;
}

void ListView::ContentsEvent_ObjectListChanged(ifc_viewcontents *contents, ifc_dataobjectlist *newObjects, ifc_dataobjectlist *prevObjects)
{
	__super::ContentsEvent_ObjectListChanged(contents, newObjects, prevObjects);
	InvalidateSort(0);
	UpdateItems(FALSE);
	UpdateSort(FALSE);
}

void ListView::ContentsEvent_ObjectsAdded(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **added, size_t count, size_t startIndex)
{
	__super::ContentsEvent_ObjectsAdded(contents, list, added, count, startIndex);
	UpdateItems(FALSE);
	UpdateSort(FALSE);
}

void ListView::ContentsEvent_ObjectsRemoved(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **removed, size_t removedCount, size_t removedOffset)
{
	size_t indexR, indexW, mapSize, removedMax, val;

	__super::ContentsEvent_ObjectsRemoved(contents, list, removed, removedCount, removedOffset);

	if (0 == list->GetCount())
	{
		InvalidateSort(0);
		UpdateItems(FALSE);
		return;
	}

	if (S_FALSE != UpdateSort(FALSE))
		return;
	
	mapSize = indexMap.size();
	removedMax = removedOffset + removedCount;

	for (indexR = 0, indexW = 0; indexR < mapSize; indexR++)
	{
		val = itemMap[indexR];
		if (val >= removedOffset)
		{
			if (val < removedMax)
				continue;

			indexMap[val] = indexW;
			val -= removedCount;
		}
		else
			indexMap[val] = indexW;

		itemMap[indexW] = val;
		indexW++;
	}

	if (mapSize > removedMax)
	{
		memmove(indexMap.begin() + removedOffset, 
				indexMap.begin() + removedMax,  
				(mapSize - removedMax) * sizeof(size_t));
	}
	
	indexMap.resize(mapSize - removedCount);
	itemMap.resize(mapSize - removedCount);
	InvalidateSort(indexMap.size());
	listCount = indexMap.size();
	
	UpdateItems(FALSE);
	UpdateSelection(FALSE);
}

void ListView::ContentsEvent_ObjectsRemovedAll(ifc_viewcontents *contents, ifc_dataobjectlist *list)
{
	__super::ContentsEvent_ObjectsRemovedAll(contents, list);

	InvalidateSort(0);
	UpdateItems(FALSE);
	UpdateSort(FALSE);
}


void ListView::ContentsEvent_ObjectsChanged(ifc_viewcontents *contents, ifc_dataobjectlist *list, ifc_dataobject **changed, size_t count, size_t startIndex)
{
	__super::ContentsEvent_ObjectsChanged(contents, list, changed, count, startIndex);
	InvalidateSort(0);
	UpdateItems(FALSE);
	UpdateSort(FALSE);
}

void ListView::ContentsEvent_ObjectsUpdateStarted(ifc_viewcontents *contents,  ifc_dataobjectlist *list)
{
	__super::ContentsEvent_ObjectsUpdateStarted(contents, list);
	LockUpdates();
}

void ListView::ContentsEvent_ObjectsUpdateFinished(ifc_viewcontents *contents, ifc_dataobjectlist *list)
{
	__super::ContentsEvent_ObjectsUpdateFinished(contents, list);

	UnlockUpdates();
	UpdateWindow(hwnd);
}

void ListView::ContentsEvent_SelectionChanged(ifc_viewcontents *contents, ifc_viewselection *selection, ifc_viewselection *appended, ifc_viewselection *removed, ifc_viewselectionevent::Reason reason)
{
	if (0 == (ListFlag_PublishingSelection & listFlags) && 
		0 == (ifc_viewselectionevent::Selection_Shift & reason))
	{
		ifc_viewselectionenum *enumerator;
		IndexRange range;
		size_t index;
		int iItem, offset;
		LVITEM itemInfo;

		listFlags |= ListFlag_UpdatingSelection;

		if (0 != (ListFlag_SummaryEnabled & listFlags))
			offset = 1;
		else
			offset = 0;

		if (NULL != appended && 
			SUCCEEDED(appended->Enumerate(&enumerator)))
		{
			itemInfo.state = LVIS_SELECTED;
			itemInfo.stateMask = LVIS_SELECTED;

			while (S_OK == enumerator->Next(&range, 1, NULL))
			{
				for(index = range.first; index <= range.last; index++)
				{
					iItem = indexMap[index] + offset;
					
					SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&itemInfo);
				}
			}
			enumerator->Release();
		}

		if (NULL != removed && 
				SUCCEEDED(removed->Enumerate(&enumerator)))
		{
			itemInfo.state = 0;
			itemInfo.stateMask = LVIS_SELECTED;

			while (S_OK == enumerator->Next(&range, 1, NULL))
			{
				for(index = range.first; index <= range.last; index++)
				{
					iItem = indexMap[index] + offset;
					SendMessage(hwnd, LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&itemInfo);
				}
			}
			enumerator->Release();
		}

		listFlags &= ~ListFlag_UpdatingSelection;

	}
	__super::ContentsEvent_SelectionChanged(contents, selection, appended, removed, reason);
}

void ListView::ContentsEvent_ColumnsChanged(ifc_viewcontents *contents)
{
	ReloadColumns();
	UpdateHeader();

	__super::ContentsEvent_ColumnsChanged(contents);
}

void ListView::GroupFilterEvent_BypassModeChanged(ifc_viewgroupfilter *instance, BOOL bypassEnabled)
{	
	if (0 != (ListFlag_SummaryEnabled & listFlags))
	{
		SetSelectedItem(0, bypassEnabled, TRUE);
	}
	
	__super::GroupFilterEvent_BypassModeChanged(instance, bypassEnabled);
}

LRESULT ListView::WindowProc(unsigned int uMsg, WPARAM wParam, LPARAM lParam)
{
	if (LISTVIEW_WM_COMMITTRANSACTION == uMsg && 
		WM_NULL != LISTVIEW_WM_COMMITTRANSACTION)
	{
		CommitSelectionTransaction();
		return 0;
	}
	return __super::WindowProc(uMsg, wParam, lParam);
}
HRESULT ListView::CommitSelectionTransaction()
{
	HRESULT hr;
	ifc_viewgroupfilter *groupFilter;

	if (NULL == selectionTransaction)
		return S_FALSE;

	if (NULL != performanceTimer)
		performanceTimer->Start();

	listFlags |= ListFlag_PublishingSelection;
	
	if (SUCCEEDED(GetGroupFilter(&groupFilter)))
	{
		if (FALSE != filterBypassTransaction)
		{
			if (S_OK != groupFilter->IsBypassEnabled())
				groupFilter->EnableBypass(TRUE);
			else if (0 != (ListFlag_SummaryEnabled & listFlags) &&
					 0 == (unsigned int)SendMessage(hwnd, LVM_GETITEMSTATE, 0, (LPARAM)LVIS_SELECTED))
			{
				SetSelectedItem(0, TRUE, TRUE);
			}
		}
	}
	else
		groupFilter = NULL;


	hr = selectionTransaction->Commit();
	selectionTransaction->Release();
	selectionTransaction = NULL;
	

	if (NULL != groupFilter)
	{
		if (FALSE == filterBypassTransaction)
		{
			if (S_OK == groupFilter->IsBypassEnabled())
				groupFilter->EnableBypass(FALSE);
			else if (0 != (ListFlag_SummaryEnabled & listFlags) &&
					 0 != (unsigned int)SendMessage(hwnd, LVM_GETITEMSTATE, 0, (LPARAM)LVIS_SELECTED))
			{
				SetSelectedItem(0, FALSE, TRUE);
			}
		}

		groupFilter->Release();
	}
			
	listFlags &= ~ListFlag_PublishingSelection;

	if (NULL != performanceTimer)
		performanceTimer->Stop();

	return hr;
}

BOOL ListView::IsSummaryItem(size_t iItem)
{
	return (0 == iItem && 0 != (ListFlag_SummaryEnabled & listFlags));
}

BOOL ListView::FilterSelectionChange(int *_iFrom, int *_iTo, BOOL selected)
{
	int iFrom, iTo;

	if (0 == (ListFlag_SummaryEnabled & listFlags))
		return TRUE;

	iFrom = *_iFrom;
	iTo = *_iTo;
	

	if (-1 == iFrom && -1 == iTo)
	{
		if (FALSE != selected)
			SetSelectedItem(0, FALSE, TRUE);

		return TRUE;
	}

	if (0 == iFrom)
	{
		if (FALSE != selected)
		{
			if (iTo > 0)
				iFrom = 1;
			else if (1 < (int)SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0L))
				iFrom = -1;

			if (0 != iFrom)
			{
				SetSelectedItem(0, FALSE, TRUE);
				
				if (-1 == iFrom)
					return FALSE;

				*_iFrom = iFrom;
			}
		}
		else
		{
			if (iTo == iFrom &&
				0 == (int)SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0L))
			{
				SetSelectedItem(0, TRUE, TRUE);
				return FALSE;
			}
		}
	}
	else
	{
		if (FALSE != selected)
		{
			if (0 != (unsigned int)SendMessage(hwnd, LVM_GETITEMSTATE, (WPARAM)0, (LPARAM)LVIS_SELECTED))
				SetSelectedItem(0, FALSE, TRUE);
		}
	}
			
	return TRUE;
}

void ListView::NotifySelectionChange(int iFrom, int iTo, BOOL selected)
{
	BOOL publishSelection;

	publishSelection = TRUE;

	filterBypassTransaction = FALSE;

	if (0 != (ListFlag_SummaryEnabled & listFlags))
	{
		if (0 == iFrom && 0 == iTo)
		{
			if (FALSE != selected ||
				0 == (int)SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0L))
			{
				filterBypassTransaction = TRUE;
			}

			publishSelection = FALSE;
		}
		else
		{
			iTo -= 1;
			if (0 != iFrom)
				iFrom -= 1;
		}
	}
	
	if (FALSE == selected &&
		0 == (ListFlags_MarqueeSelection & listFlags) &&
		0 == (int)SendMessage(hwnd, LVM_GETSELECTEDCOUNT, 0, 0L))
	{
		filterBypassTransaction = TRUE;
	}
		
	if (FALSE != publishSelection)
		PublishSelection(iFrom, iTo, selected);
}

static int 
ListView_OrderSelectionCb(const void *elem1, const void *elem2)
{
	return (*((size_t*)elem1) - *((size_t*)elem2));
}

BOOL ListView::PublishSelection(int iFrom, int iTo, BOOL selected)
{	
	BOOL result;
		
	result = TRUE;
	
	if (NULL == selectionTransaction)
		return FALSE;
	
	
	if (iFrom < 0 || iTo < 0)
	{
		if (FALSE == selected)
			selectionTransaction->RemoveAll();
		else
		{
			size_t count;
			ifc_dataobjectlist *list;
			if (S_OK == contents->GetObjects(&list))
			{
				count = list->GetCount();
				list->Release();
			}
			else 
				count = 0;
			
			if (count > 0)
				selectionTransaction->Set(0, count - 1);
		}
	}
	else if (iFrom == iTo)
	{
		size_t index;

		UpdateSort(TRUE);

		index = itemMap[iFrom];

		if (FALSE != selected)
			selectionTransaction->Add(index, index);
		else
			selectionTransaction->Remove(index, index);
	}
	else
	{
		size_t *selection, selectionSize;
		size_t index, count, from, to;
		BOOL freeSelection;


		selectionSize = (iTo - iFrom + 1);
		if (selectionSize <= 1024)
		{
			__try
			{
				selection = (size_t*)_alloca(selectionSize * sizeof(size_t));
			}
			__except(STATUS_STACK_OVERFLOW == GetExceptionCode())
			{
				_resetstkoflw();
				selection = NULL;
			}
		}
		else
			selection = NULL;

		if (NULL == selection)
		{
			selection = (size_t*)malloc(selectionSize * sizeof(size_t));
			freeSelection = TRUE;
		}
		else
			freeSelection = FALSE;
			
		if (NULL != selection)
		{
			UpdateSort(TRUE);

			count = 0;
			while (iFrom <= iTo)
			{
				selection[count++] = itemMap[iFrom];
				iFrom++;
			}

			qsort(selection, count, sizeof(size_t), ListView_OrderSelectionCb);

			from = *selection;
			to = from;

			for(index = 1; index <= count; index++)
			{		
				if (index == count || selection[index] != (to + 1))
				{			
					if (FALSE != selected)
						selectionTransaction->Add(from, to);
					else
						selectionTransaction->Remove(from, to);
					
					if (index == count)
						break;
					
					from = selection[index];
					to = from;
				}
				else
					to++;
			}

			if (FALSE != freeSelection)
				free(selection);
		}

	}

	return result;
}

BOOL ListView::PublishEntireSelection()
{	
	int iSelected;
	int iFrom, iTo;
		
	if (NULL != selectionTransaction)
		selectionTransaction->RemoveAll();
	
	if (0 != (ListFlag_SummaryEnabled & listFlags) &&
		0 != SendMessageW(hwnd, LVM_GETITEMSTATE, 0, LVIS_SELECTED))
	{
		return PublishSelection(-1, -1, TRUE);

	}			
		
	iFrom = -1;
	iSelected = -1;

	for(;;)
	{
		iSelected = (int)SendMessage(hwnd, LVM_GETNEXTITEM, (WPARAM)iSelected, 
									(LPARAM)(LVNI_ALL | LVNI_SELECTED));
		if (-1 == iFrom)
		{
			iFrom = iSelected;
			iTo = iFrom;
		}
		else 
		{
			if (iSelected == (iTo + 1))
				iTo = iSelected;
			else
			{
				if (0 != (ListFlag_SummaryEnabled & listFlags))
				{
					iFrom--;
					iTo--;
				}

				PublishSelection(iFrom, iTo, TRUE);
	
				iFrom = iSelected;
				iTo = iSelected;
			}
		}
		if (-1 == iSelected)
			break;
	}
	
	return TRUE;
}
