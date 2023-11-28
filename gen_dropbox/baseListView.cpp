#include "./main.h"
#include "./baseListView.h"
#include "./plugin.h"
#include "./wasabiApi.h"
#include "./fileInfoInterface.h"
#include "./configIniSection.h"
#include "./configManager.h"
#include "./resource.h"
#include "./dataObject.h"
#include "./fileEnumInterface.h"
#include "./cfpInterface.h"
#include "./dropSource.h"
#include "./listDragData.h"
#include "./guiObjects.h"
#include "./skinWindow.h"
#include "./document.h"
#include "../winamp/wa_ipc.h"
#include "../nu/menuHelpers.h"
#include "./explorePath.h"

//#include <windowsx.h>
#include <strsafe.h>


#define ANIMATION_TIME			200

#define REDRAWTIMER_ID			81
#define REDRAWTIMER_DELAY		40/*USER_TIMER_MINIMUM*/

#define READSELECTION_TIMER		105
#define READSELECTION_DELAY		200

static BOOL viewRegistered = FALSE;
static HACCEL viewAccelTable = NULL;

static UINT WAML_NOTIFY_DRAGDROP = 0;


static int _cdecl CompareUnsignedInt(const void *elem1, const void *elem2)
{
	return (*(UINT*)elem1) - (*(UINT*)elem2);
}


static void CALLBACK UninitializeBaseListView(void)
{
	viewRegistered = FALSE;
	if (NULL != viewAccelTable)
	{
		DestroyAcceleratorTable(viewAccelTable);
		viewAccelTable = NULL;
	}
}

static void InitialiewBaseListView(void)
{
	if (viewRegistered) 	return;

	if (NULL != viewAccelTable)
		DestroyAcceleratorTable(viewAccelTable);

	viewAccelTable = WASABI_API_LOADACCELERATORSW(IDR_ITEMVIEW_ACCELERATORS);
	Plugin_RegisterUnloadCallback(UninitializeBaseListView);
	
	WAML_NOTIFY_DRAGDROP = RegisterWindowMessageW(WAMLM_DRAGDROP);
	viewRegistered = TRUE;
}

BaseListView::BaseListView(HWND hView) :
	DropboxView(hView), enableSelectionRead(TRUE), enableCachedRedraw(TRUE), pDragData(NULL)
{
	InitialiewBaseListView();
	
	if (NULL != WASABI_API_APP)
	{		
		if (NULL != viewAccelTable) 
			WASABI_API_APP->app_addAccelerators(hView, &viewAccelTable, 1, TRANSLATE_MODE_NORMAL);
	}

	uiState = (WORD)CallPrevWndProc(WM_QUERYUISTATE, 0, 0L);
	SendMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_SET, UISF_HIDEACCEL | UISF_HIDEFOCUS), 0L);

	frameCache.RegisterModifiedCallback(BaseListView_OnCacheModified, (ULONG_PTR)this);

	ZeroMemory(&selectionMetrics, sizeof(Document::METRICS));
	selectionMetrics.cbSize = sizeof(Document::METRICS);
	selectionMetrics.flags = Document::FlagMetricLength | Document::FlagMetricSize;

	RegisterDragDrop();
	OnSkinChanged();
}


BaseListView::~BaseListView()
{	
	frameCache.RegisterModifiedCallback(NULL, 0);
	
	RevokeDragDrop();

	if (NULL != WASABI_API_APP)
	{
		WASABI_API_APP->app_removeAccelerators(hwnd);
	}

	if (NULL != pDragData)
	{
		delete(pDragData);
		pDragData = NULL;
	}
	
}

STDMETHODIMP BaseListView::ProcessNotification(NMHDR *pnmh, LRESULT *pResult)
{
	switch(pnmh->code)
	{
		case LVN_ODCACHEHINT:		OnCacheHint((NMLVCACHEHINT*)pnmh); return S_OK;
		case LVN_KEYDOWN:			OnKeyDown((NMLVKEYDOWN*)pnmh); return S_OK;
		case LVN_BEGINDRAG:			OnBeginDrag(((NMLISTVIEW*)pnmh)->iItem); return S_OK;
		case NM_DBLCLK:				OnItemDblClick((NMITEMACTIVATE*)pnmh); return S_OK;
		case NM_RETURN:				OnEnterKey(pnmh); return S_OK;
		case LVN_ODSTATECHANGED:	OnStateChanged((NMLVODSTATECHANGE*)pnmh); return S_OK;
		case LVN_ITEMCHANGED:		OnItemChanged((NMLISTVIEW*)pnmh); return S_OK;
	}
	return E_NOTIMPL;
}


STDMETHODIMP BaseListView::DrawItem(DRAWITEMSTRUCT *pdis)
{	
	return E_NOTIMPL;
}

STDMETHODIMP BaseListView::MeasureItem(MEASUREITEMSTRUCT *pmis)
{	
	return E_NOTIMPL;
}



BOOL BaseListView::FetchItems(INT first, INT last)
{
	if (NULL == pActiveDocument)
		return FALSE;
	
	pActiveDocument->ReadItems(first, last, metaKeyList.data(), (INT)metaKeyList.size(), FALSE);

	return TRUE;
}

static BOOL CALLBACK BaseListView_OnFetchItems(INT first, INT last, ULONG_PTR param)
{
	BaseListView *view = (BaseListView*)param;
	if (NULL == view)
		return FALSE;
	view->FetchItems(first, last);
	return TRUE;
}

void BaseListView::OnCacheHint(NMLVCACHEHINT *pch)
{
	if (NULL != pActiveDocument)
	{
		INT top = (INT)CallPrevWndProc(LVM_GETTOPINDEX, 0, 0L);
		frameCache.SetTop(top, TRUE);
		frameCache.EnumerateEx(pch->iFrom, pch->iTo, FrameCache::ItemStateClear, BaseListView_OnFetchItems, (ULONG_PTR)this);
	}
}

void BaseListView::OnKeyDown(NMLVKEYDOWN *pkd)
{
	switch(pkd->wVKey)
	{
		case VK_CONTROL:
		case VK_SHIFT:
			return;
		case VK_MENU:
			if (UISF_HIDEACCEL & uiState)
				CallPrevWndProc(WM_CHANGEUISTATE, MAKELONG(UIS_CLEAR, UISF_HIDEACCEL), 0L);
			return;
	}
	
	if (UISF_HIDEFOCUS & uiState)
		CallPrevWndProc(WM_CHANGEUISTATE, MAKELONG(UIS_CLEAR, UISF_HIDEFOCUS), 0L);
}

void BaseListView::OnBeginDrag(INT iItem)
{
	PerformDragDrop(iItem);
}

void BaseListView::OnStateChanged(NMLVODSTATECHANGE *pstateChanged)
{
	if ((LVIS_SELECTED & pstateChanged->uOldState) != (LVIS_SELECTED & pstateChanged->uNewState))
	{
		selectionCursor = -1;

		Document::METRICS metrics;
		metrics.cbSize = sizeof(Document::METRICS);
		metrics.flags = Document::FlagMetricLength | Document::FlagMetricSize;
		if (NULL != pActiveDocument &&
			pActiveDocument->GetMetrics(pstateChanged->iFrom, pstateChanged->iTo, &metrics))
		{
			if (0 != (LVIS_SELECTED & pstateChanged->uNewState))
			{
				selectionMetrics.length += metrics.length;
				selectionMetrics.size += metrics.size;
				selectionMetrics.unknownData += metrics.unknownData;
			}
			else
			{
				selectionMetrics.length -= metrics.length;
				selectionMetrics.size -= metrics.size;
				selectionMetrics.unknownData -= metrics.unknownData;
			}
		}
		
		Notify(EventSelectionChanged, NULL);

		ScheduleSelectionRead();

	//	aTRACE_FMT("state changed: from: %d, to: %d\r\n", pstateChanged->iFrom, pstateChanged->iTo);
	}
}

void BaseListView::OnItemChanged(NMLISTVIEW *plv)
{	
	if (0 != (LVIF_STATE & plv->uChanged))
	{
		if ((LVIS_SELECTED & plv->uOldState) != (LVIS_SELECTED & plv->uNewState))
		{
			selectionCursor = -1;
			if (-1 == plv->iItem)
			{
				if (0 == (LVIS_SELECTED & plv->uNewState) ||
					NULL == pActiveDocument || 
					FALSE == pActiveDocument->GetMetrics((size_t)-1, (size_t)-1, &selectionMetrics))
				{					
					selectionMetrics.length = 0;
					selectionMetrics.size = 0;
					selectionMetrics.unknownData = 0;
				}
			}
			else
			{
				Document::METRICS metrics;
				metrics.cbSize = sizeof(Document::METRICS);
				metrics.flags = Document::FlagMetricLength | Document::FlagMetricSize;
				if (NULL != pActiveDocument &&
					pActiveDocument->GetMetrics(plv->iItem, plv->iItem, &metrics))
				{
					if (0 != (LVIS_SELECTED & plv->uNewState))
					{
						selectionMetrics.length += metrics.length;
						selectionMetrics.size += metrics.size;
						selectionMetrics.unknownData += metrics.unknownData;
					}
					else
					{
						selectionMetrics.length -= metrics.length;
						selectionMetrics.size -= metrics.size;
						selectionMetrics.unknownData -= metrics.unknownData;
					}
				}
			}		

		
			Notify(EventSelectionChanged, NULL);

			ScheduleSelectionRead();
		
		/*	aTRACE_FMT("item state changed: item %d %s\r\n", 
				plv->iItem,
				((0 != (LVIS_SELECTED & plv->uNewState)) ? "selected" : "deselected"));*/
		}
	}
}

void BaseListView::OnItemDblClick(NMITEMACTIVATE *pnma)
{
	if (-1 != pnma->iItem)
	{
		SENDCMD(hwnd, ID_ACTION, 1, NULL);
	}
}

void BaseListView::OnEnterKey(NMHDR *pnmh)
{	
	SENDCMD(hwnd, ID_ACTION, 1, NULL);
}



HRESULT BaseListView::OnSelectAll(BOOL bSelect)
{
	INT itemCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
	if (itemCount < 1) return S_FALSE;

	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if ((bSelect && selectedCount == itemCount) || (!bSelect && selectedCount == 0)) 
		return S_OK;

	KillTimer(hwnd, 42);
	
	LVITEM lvi;
	lvi.stateMask = LVIS_SELECTED;
	lvi.state = ((bSelect) ? LVIS_SELECTED : 0);
	if (CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi))
		return S_OK;
	else 
		return E_FAIL;
}

HRESULT BaseListView::OnDeleteSelection()
{
	if (NULL == pActiveDocument)
		return E_POINTER;

	KillTimer(hwnd, 42);

	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (selectedCount < 1) return S_FALSE;

	INT iCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
	if (selectedCount == iCount)
	{	
		pActiveDocument->RemoveAll();
		return S_OK;
	}
	
	INT iPage = (INT)CallPrevWndProc(LVM_GETCOUNTPERPAGE, 0, 0L);

	BOOL updateFrame = (iCount >= iPage);

	DWORD windowStyle = GetWindowStyle(hwnd);
	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);	
		
	size_t *selection = (size_t*)malloc(selectedCount * sizeof(size_t));
	if (NULL != selection)
	{
		INT iFile = -1;
		size_t *cursor = selection;

		while (-1 != (iFile = (INT)CallPrevWndProc(LVM_GETNEXTITEM, iFile, (LPARAM) LVNI_SELECTED)))
		{
			*cursor = iFile;
			cursor++;
		}
		INT iTop = (INT)CallPrevWndProc(LVM_GETTOPINDEX, 0, 0L);
		if (iTop < 0) iTop = 0;

		pActiveDocument->RemoveItems(selection, selectedCount);

		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state = 0;
		CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
		CallPrevWndProc(LVM_SETSELECTIONMARK, 0, (LPARAM)-1);

		iCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
		iFile = (INT)selection[0];

		if (iFile >= iCount) 
			iFile = iCount - 1;
		if (iFile >= 0)
		{	
			lvi.stateMask = LVIS_SELECTED | LVIS_FOCUSED;
			lvi.state = LVIS_SELECTED | LVIS_FOCUSED;
			CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)iFile, (LPARAM)&lvi);
			CallPrevWndProc(LVM_SETSELECTIONMARK, 0, (LPARAM)iFile);
		}

				
		if (iFile > iTop)
		{
			if (iFile >= (iTop + iPage))
				CallPrevWndProc(LVM_ENSUREVISIBLE, (WPARAM)iFile, (LPARAM)FALSE);
		}
		else if (iFile < iTop)
		{	
			INT iLast = (INT)selection[selectedCount - 1];
			
			if (iLast < iTop || iLast >= (iTop + iPage))
				iTop = iFile - 1;
			else
				iTop = iFile - (iLast - iTop);
			
			if (iTop < 0) iTop = 0;
			CallPrevWndProc(LVM_ENSUREVISIBLE, (WPARAM)iTop, (LPARAM)FALSE);
		}
		
		free(selection);
	}
	
	if (0 != (WS_VISIBLE & windowStyle))
	{
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
		
		UINT redrawFlags = RDW_INVALIDATE;
		if (updateFrame)
			redrawFlags |= RDW_FRAME | RDW_ALLCHILDREN | RDW_ERASE;

		RedrawWindow(hwnd, NULL, NULL, redrawFlags);
	}

	return S_OK;
}


HRESULT BaseListView::OnCopySelection()
{
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return E_POINTER;

	KillTimer(hwnd, 42);

	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (selectedCount < 1) return S_FALSE;
	
	IDataObject *pdo;
	HRESULT hr = MakeDataObject(-1, NULL, FALSE, &pdo, DATAOBJECT_HDROP, NULL, NULL);
	if (SUCCEEDED(hr))
	{	
		hr = Plugin_SetClipboard(pdo);
		pdo->Release();
	}
	return hr;
}

HRESULT BaseListView::OnPasteItems()
{
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return E_POINTER;

	KillTimer(hwnd, 42);

	IDataObject *pObject;

	HRESULT hr = OleGetClipboard(&pObject);
	if (FAILED(hr))
		return hr;
	IClipboardFormatProcessor *pProcessor;
	hr = CreateDataObectProcessor(pObject, hParent, &pProcessor, DATAOBJECT_HDROP);
	if(SUCCEEDED(hr))
	{
		INT count = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
		pProcessor->Process(count);
		pProcessor->Release();
	}
	pObject->Release();
	return hr;
}

HRESULT BaseListView::OnCutSelection()
{
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return E_POINTER;

	KillTimer(hwnd, 42);

	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (selectedCount < 1) return S_FALSE;
	
	INT iFirst = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);

	IDataObject *pdo;
	size_t *dropItems, dropCount;
	HRESULT hr = MakeDataObject(-1, NULL, FALSE, &pdo, DATAOBJECT_HDROP, &dropItems, &dropCount);
	if (SUCCEEDED(hr))
	{	
		hr = Plugin_SetClipboard(pdo);
		pdo->Release();
		
		if (dropCount > 0)
		{
			qsort(dropItems, dropCount, sizeof(size_t), CompareUnsignedInt);

			LVITEM lvi;
			lvi.state = 0;
			lvi.stateMask = LVIS_SELECTED | LVIS_CUT | LVIS_DROPHILITED;
			for(size_t i = 0; i < dropCount; i++)
				CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)dropItems[i], (LPARAM)&lvi);
			
			pActiveDocument->RemoveItems(dropItems, dropCount);
		}
	}

	INT iFocused = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_FOCUSED);
	if (-1 == iFocused)
	{
		iFocused = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
		if (-1 == iFocused) 
			iFocused = iFirst;

		INT iCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
		if (iFocused >= iCount) iFocused = iCount - 1;

		if (iFocused >= 0)
		{
			LVITEM lvi;
			lvi.stateMask = LVIS_FOCUSED;
			lvi.state = LVIS_FOCUSED;
			CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)iFocused, (LPARAM)&lvi);
		}
	}

	if (-1 != iFocused)
		CallPrevWndProc(LVM_ENSUREVISIBLE, (WPARAM)iFocused, FALSE);
	
	return hr;
}

HRESULT BaseListView::OnInvertSelection()
{
	INT iCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
	if (iCount < 1) return S_OK;

	INT iSelectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (0 == iSelectedCount || iSelectedCount == iCount)
		return OnSelectAll(0 == iSelectedCount);

	HRESULT hr = S_OK;
	LVITEM item;
	item.stateMask = LVIS_SELECTED;
	for (INT iItem  = 0; iItem < iCount; iItem++)
	{
		item.state = ((INT)CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)iItem, (LPARAM)LVIS_SELECTED)) ^ LVIS_SELECTED;
		if (!CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&item))
			hr = E_FAIL;
	}
	return hr;
}

HRESULT BaseListView::OnShowFileInfo()
{
	if (NULL == pActiveDocument)
		return E_FAIL;

	INT iFocused = (INT)CallPrevWndProc(LVM_GETNEXTITEM, -1, (LPARAM)LVNI_FOCUSED);
	if (iFocused < 0 || (size_t)iFocused >= pActiveDocument->GetItemCount())
		return E_INVALIDARG;
	
	IFileInfo *pItem = pActiveDocument->GetItemSafe(iFocused);
	if (NULL == pItem)
		return E_OUTOFMEMORY;

	LPCTSTR pszPath;
	if (SUCCEEDED(pItem->GetPath(&pszPath)))
	{
		infoBoxParamW infoBox;
		infoBox.parent = hwnd;
		infoBox.filename = pszPath;
		
		if (!SENDWAIPC(plugin.hwndParent, IPC_INFOBOXW, (WPARAM)&infoBox))
		{			
			if (NULL != pActiveDocument)
				pActiveDocument->InvalidateItem(pItem); 
		}
	}

	pItem->Release();

	return S_OK;
}
HRESULT BaseListView::OnPlay()
{
	EnqueueSelection(-1, TRUE);
	return S_OK;
}
HRESULT BaseListView::OnEnqueue()
{
	EnqueueSelection(-1, FALSE);
	return S_OK;
}

HRESULT BaseListView::OnAction()
{
	INT activeCommand = ID_PLAY;

	IConfiguration *pConfig;

	Profile *profile = DropboxWindow_GetProfile(hwnd);
	if (NULL != profile && SUCCEEDED(profile->QueryConfiguration(mediaLibrarySettingsGuid, &pConfig)))
	{
		INT mlAction;
		if (S_OK == pConfig->ReadInt(CFG_ACTIONTYPE, &mlAction) && mlAction == ACTIONTYPE_ENQUEUE)
			activeCommand = ID_ENQUEUE;
		pConfig->Release();
	}
		
	SENDCMD(hwnd, activeCommand, 1, NULL);
	return S_OK;
}

HRESULT BaseListView::OnReverseOrder()
{
	if (NULL == pActiveDocument)
		return E_NOINTERFACE;

	HCURSOR hc = SetCursor(LoadCursor(NULL, IDC_WAIT));
	HRESULT hr = pActiveDocument->Reverse(0, -1);
	SetCursor(hc);

	return S_OK;
}

HRESULT BaseListView::OnExploreFolder()
{
	if (NULL == pActiveDocument)
		return E_FAIL;

	INT iFocused = (INT)CallPrevWndProc(LVM_GETNEXTITEM, -1, (LPARAM)LVNI_FOCUSED);
	if (iFocused < 0 || (size_t)iFocused >= pActiveDocument->GetItemCount())
		return E_INVALIDARG;
	
	IFileInfo *pItem = pActiveDocument->GetItemSafe(iFocused);
	if (NULL == pItem)
		return E_OUTOFMEMORY;
	
	LPCTSTR pszPath;
	TCHAR szFilePath[MAX_PATH];
	szFilePath[0] = TEXT('\0');

	if (SUCCEEDED(pItem->GetPath(&pszPath)) && 
		NULL != pszPath &&
		TEXT('\0') != *pszPath)
	{
		if (S_OK != StringCchCopy(szFilePath, ARRAYSIZE(szFilePath), pszPath))
			szFilePath[0] = TEXT('\0');
	}
	pItem->Release();

	if (TEXT('\0') != szFilePath[0])
		ExploreFile(hwnd, szFilePath, TRUE);

	return S_OK;
}

STDMETHODIMP BaseListView::ProcessCommand(INT commandId)
{
	switch (commandId)
	{	
		case ID_COPY:		
		case ID_COPYWIN:
			OnCopySelection();
			return S_OK;
		case ID_CUT:		
		case ID_CUTWIN:
			OnCutSelection();
			return S_OK;;
		case ID_PASTE:		
		case ID_PASTEWIN:
			OnPasteItems();
			return S_OK;;
		case ID_DELETE:	
			OnDeleteSelection();	
			return S_OK;;
		case ID_SELECTALL: 
			OnSelectAll(TRUE);
			return S_OK;;
		case ID_INVERTSELECTION:
			OnInvertSelection();
			return S_OK;;
		case ID_SHOWFILEINFO:
			OnShowFileInfo();
			return S_OK;;
		case ID_PLAY:
			OnPlay();
			return S_OK;;
		case ID_ENQUEUE:
			OnEnqueue();
			return S_OK;;
		case ID_ACTION:
			OnAction();
			return S_OK;;
		case ID_REVERSEORDER:
			OnReverseOrder();
			return S_OK;;
		case ID_EXPLOREFOLDER:
			OnExploreFolder();
			return S_OK;
	}

	return E_NOTIMPL;
}
void BaseListView::OnCommand(INT ctrlId, INT eventId, HWND hwndCtrl)
{
	if (S_OK != ProcessCommand(ctrlId))
	{
		HWND hParent = GetParent(hwnd);
		if (NULL != hParent)
			DropboxWindow_BroadcastCommand(hParent, ctrlId, hwnd);
	}

}

void BaseListView::OnUpdateUiState(WORD action, WORD state)
{
	CallPrevWndProc(WM_UPDATEUISTATE, MAKEWPARAM(action, state), 0L);
	uiState = (WORD)CallPrevWndProc(WM_QUERYUISTATE, 0, 0L);
}


HRESULT BaseListView::EnqueueSelection(INT iFirst, BOOL bPlay)
{
	INT iSelected = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (iSelected < 1) 
		return S_OK;

	HWND hParent = GetParent(hwnd);
	if (NULL == hParent)
		return E_FAIL;

	size_t *indices = NULL, count = 0, index;
	DWORD enqueueFlags = 0;
	INT iCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
	if (iCount == iSelected)
	{
		enqueueFlags = Document::EF_ENQUEUEALL;
	}
	else
	{
		INT iFile = iFirst;
		if (iFile > -1) iFile--;
		
		if (1 == iSelected)
		{
			iFile = (INT)CallPrevWndProc(LVM_GETNEXTITEM, iFile, (LPARAM)LVNI_SELECTED);
			if (-1 != iFile)
			{
				index = iFile;
				indices = &index;
				count = 1;
			}
		}
		else
		{
			indices = (size_t*)malloc(sizeof(size_t) * iSelected);
			if (NULL == indices)
				return E_OUTOFMEMORY;

			while (-1 != (iFile = (INT)CallPrevWndProc(LVM_GETNEXTITEM, iFile, (LPARAM)LVNI_SELECTED)))
			{
				indices[count] = iFile;
				count++;
			}

			if (iFirst > -1)
			{
				iFile = -1;
				while (-1 != (iFile = (INT)CallPrevWndProc(LVM_GETNEXTITEM, iFile, (LPARAM)LVNI_SELECTED)) &&
						iFile < iFirst)
				{
					indices[count] = iFile;
					count++;
				}
			}
		}
	}
	
	HRESULT hr;
	if (NULL != pActiveDocument)
	{
		if (bPlay)
			hr = pActiveDocument->PlayItems(indices, count, enqueueFlags, NULL, 0);
		else
			hr = pActiveDocument->EnqueueItems(indices, count, enqueueFlags, NULL);
	}
	else 
		hr = E_POINTER;

	if (NULL != indices && iSelected > 1)
		free(indices);
	return hr;
	
}
STDMETHODIMP BaseListView::SetSkinned(BOOL bSkinned)
{
	HRESULT hr = __super::SetSkinned(bSkinned);
	OnSkinChanged();
	return hr;
}
void BaseListView::OnSkinChanged()
{
	if (GetSkinned())
	{
		GetThemeColor = GetSkinColor;
		GetThemeBrush = GetSkinBrush;
	}
	else
	{
		GetThemeColor = GetSystemColor;
		GetThemeBrush = GetSystemBrush;
	}

	SendMessage(hwnd, LVM_SETBKCOLOR, 0, GetThemeColor(COLOR_WINDOW));
	SendMessage(hwnd, LVM_SETTEXTBKCOLOR, 0, GetThemeColor(COLOR_WINDOW));
	SendMessage(hwnd, LVM_SETTEXTCOLOR, 0, GetThemeColor(COLOR_WINDOWTEXT));
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
}


LRESULT BaseListView::OnGetDlgCode(UINT vKey, MSG *pMsg)
{
	LRESULT result = CallPrevWndProc(WM_GETDLGCODE, (WPARAM)vKey, (LPARAM)pMsg);
	result |= DLGC_WANTALLKEYS;
	return result;
}

void BaseListView::OnWindowPosChanged(WINDOWPOS *pwp)
{
	
	CallPrevWndProc(WM_WINDOWPOSCHANGED, 0, (LPARAM)pwp);

	if (SWP_NOSIZE != ((SWP_NOSIZE & SWP_FRAMECHANGED) & pwp->flags)) 
	{
		INT frameSize = (INT)CallPrevWndProc(LVM_GETCOUNTPERPAGE, 0, 0L);
		frameSize += 1;

		frameCache.SetSize(frameSize, TRUE);

		if (NULL != pActiveDocument)
			pActiveDocument->SetQueueLimit(frameSize);
	}
}

STDMETHODIMP BaseListView::SetDocument(Document *pDoc)
{
	frameCache.SetModified(FALSE);
	frameCache.Reset();
	
	DropboxView::SetDocument(pDoc);
	if (NULL != pActiveDocument)
		pActiveDocument->SetQueueLimit((INT)CallPrevWndProc(LVM_GETCOUNTPERPAGE, 0, 0L) + 1);
	
	size_t itemCount = 0;
	if (NULL != pActiveDocument)
		itemCount = pActiveDocument->GetItemCount();

	SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_TOP, 0), NULL);
	SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, 0), NULL);
	SendMessage(hwnd, LVM_SETITEMCOUNT, (WPARAM)itemCount, 0L);
	
	return S_OK;
}


LRESULT BaseListView::OnSetItemCount(INT cItems, DWORD dwFlags)
{	
	frameCache.SetModified(FALSE);
	frameCache.Reset();

	if (0 == cItems)
	{
		selectionCursor = -1;
		selectionMetrics.length = 0;
		selectionMetrics.size = 0;
		selectionMetrics.unknownData = 0;
		Notify(EventSelectionChanged, NULL);
	}

	LRESULT result = CallPrevWndProc(LVM_SETITEMCOUNT, (WPARAM)cItems, (LPARAM)dwFlags);
	return result;
}

void BaseListView::OnUpdateMetrics(INT index, Document::ITEMREAD *readData)
{
	if (NULL != readData->pItem && 
		NULL != pActiveDocument &&
		pActiveDocument->GetItemDirect(index) == readData->pItem &&
		CallPrevWndProc(LVM_GETITEMSTATE, index, (LPARAM)(LVIS_SELECTED)))
	{	
		BOOL notify = FALSE;
		if (readData->newUnknown != readData->oldUnknown)
		{
			if (0 != readData->newUnknown) 
				selectionMetrics.unknownData++;
			else if (selectionMetrics.unknownData > 0)
				selectionMetrics.unknownData--;
		}
		if (readData->newLength != readData->oldLength)
		{
			selectionMetrics.length += (readData->newLength - readData->oldLength);
		}
		Notify(EventSelectionLengthChanged, NULL);
	}

}

void BaseListView::OnCacheModified()
{
	if (NULL == pActiveDocument)
		return;
		
	if (0 != pActiveDocument->GetQueueSize())
	{
		SetTimer(hwnd, REDRAWTIMER_ID, REDRAWTIMER_DELAY, BaseListView_RedrawTimerProc);
	}
	else
	{
		KillTimer(hwnd, REDRAWTIMER_ID);
		RedrawCachedItems();
	}
}

void BaseListView::OnItemShifted(Document::ITEMSHIFT *pShiftData)
{
	//aTRACE_FMT("items shifted (first: %d, last: %d, delta: %d)\r\n", pShiftData->first + 1, pShiftData->last + 1, pShiftData->delta);
	
	UINT state;
	LVITEM lvi;

	INT iFocused = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_FOCUSED);
	if (-1 != iFocused &&
		((size_t)iFocused) >= pShiftData->first && 
		((size_t)iFocused) <= pShiftData->last)
	{
		lvi.stateMask = LVIS_FOCUSED;
		lvi.state = 0;
		CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)iFocused, (LPARAM)&lvi);
		lvi.state = LVIS_FOCUSED;
		CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)iFocused + pShiftData->delta, (LPARAM)&lvi);
	}
	
	INT iMark = (INT)CallPrevWndProc(LVM_GETSELECTIONMARK, 0, 0L);
	if (-1 != iMark &&
		((size_t)iFocused) >= pShiftData->first &&
		((size_t)iFocused) <= pShiftData->last)
	{
		CallPrevWndProc(LVM_SETSELECTIONMARK, 0, (LPARAM)iMark + pShiftData->delta);
	}

	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (0 == selectedCount)
	{
		return;
	}
	
	lvi.stateMask = LVIS_SELECTED;
	lvi.state = 0;


	if (pShiftData->delta > 0)
	{
		for (size_t i = pShiftData->last; i >= pShiftData->first; i--)
		{
			state = (DWORD)CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)i, (LPARAM)lvi.stateMask);
			lvi.state = 0;
			CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)&lvi);
			lvi.state = state;
			CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)i + pShiftData->delta, (LPARAM)&lvi);
			if (0 == i) break;
		}
	}
	else if (pShiftData->delta < 0)
	{
		for (size_t i = pShiftData->first; i <= pShiftData->last; i++)
		{
			state = (DWORD)CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)i, (LPARAM)lvi.stateMask);
			lvi.state = 0;
			CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)i, (LPARAM)&lvi);
			lvi.state = state;
			CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)i + pShiftData->delta, (LPARAM)&lvi);
		}
	}

}

void BaseListView::OnRangeReversed(INT first, INT last)
{
	LVITEM lvi;
	lvi.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;

	INT iCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
	
	if (iCount > 0 && iCount > last)
	{
		DWORD windowStyle = GetWindowStyle(hwnd);
		if (0 != (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
		
		INT l = (INT)first;
		INT r = (INT)last;
		if (l < 0) l = 0;
		if (r < 0) r = 0;
		UINT stateStore;
		for (; l < r; l++, r--)
		{
			stateStore = (UINT)CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)l, (LPARAM)lvi.stateMask);
			lvi.state = (UINT)CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)r, (LPARAM)lvi.stateMask);
			CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)l, (LPARAM)&lvi);
			lvi.state = stateStore;
			CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)r, (LPARAM)&lvi);
		}
	
		if (0 != (WS_VISIBLE & windowStyle))
			SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);

		INT iMark = (INT)CallPrevWndProc(LVM_GETSELECTIONMARK, 0, 0L);
		if (iMark >= 0 && iMark >= (INT)first && iMark <= last)
			CallPrevWndProc(LVM_SETSELECTIONMARK, 0, (LPARAM)(last - (iMark - first)));
		
	}
	else
	{
		lvi.state = 0;
		CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);
	}

	InvalidateRect(hwnd, NULL, TRUE);

}
void BaseListView::OnRangeReordered(INT first, INT last)
{
	LVITEM lvi;
	lvi.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
	lvi.state = 0;
	CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)-1, (LPARAM)&lvi);

	INT iItem = 0;
	
	lvi.stateMask = LVIS_FOCUSED;
	lvi.state = LVIS_FOCUSED;
	CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)iItem, (LPARAM)&lvi);
	CallPrevWndProc(LVM_SETSELECTIONMARK, 0, (LPARAM)iItem);
	
	CallPrevWndProc(LVM_ENSUREVISIBLE, (WPARAM)iItem, FALSE);
	
	InvalidateRect(hwnd, NULL, TRUE);
}

void BaseListView::OnRangeRemoved(INT first, INT last)
{
	LVITEM lvi;
	lvi.stateMask = LVIS_CUT | LVIS_DROPHILITED | LVIS_FOCUSED | LVIS_SELECTED;
	lvi.state = 0;
		
	INT offset = (last - first) + 1;
	INT count = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);
	
	for (INT i = last + 1; i < count; i++)
	{
		lvi.state = (UINT)CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)i, (LPARAM)lvi.stateMask);
		CallPrevWndProc(LVM_SETITEMSTATE, (WPARAM)(i - offset), (LPARAM)&lvi);
	}

	INT iMark = (INT)CallPrevWndProc(LVM_GETSELECTIONMARK, 0, 0L);
	if (iMark >= 0 && iMark >= (INT)first)
	{
		iMark -= offset;
		if (iMark < first) iMark = first;
		CallPrevWndProc(LVM_SETSELECTIONMARK, 0, (LPARAM)iMark);
	}
}

void BaseListView::OnSetFont(HFONT hFont, BOOL bRedraw)
{
	CallPrevWndProc(WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE);
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}
LRESULT BaseListView::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch(uMsg)
	{		
		case WM_WINDOWPOSCHANGED:	OnWindowPosChanged((WINDOWPOS*)lParam); return 0;
		case WM_COMMAND:			OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return 0;
		case WM_UPDATEUISTATE:	OnUpdateUiState(LOWORD(wParam), HIWORD(wParam)); return 0;
		case DBM_SKINCHANGED:	OnSkinChanged(); return 0;
		case WM_GETDLGCODE:		return OnGetDlgCode((UINT)wParam, (MSG*)lParam);
		case LVM_SETITEMCOUNT:	return OnSetItemCount((INT)wParam, (DWORD)lParam);
		case WM_VSCROLL:			OnVScroll(LOWORD(wParam), HIWORD(wParam), (HWND)lParam); return 0;
		case WM_CONTEXTMENU:		OnContextMenu((HWND)wParam, MAKEPOINTS(lParam)); return 0;
		case WM_SETFONT:			OnSetFont((HFONT)wParam, (BOOL)lParam); return 0;
	}

	if (WAML_NOTIFY_DRAGDROP == uMsg && 0 != WAML_NOTIFY_DRAGDROP)
	{
		OnMediaLibraryDragDrop((INT)wParam, (mlDropItemStruct*)lParam);
		return TRUE;
	}
	
	if (ProcessNotifications(uMsg, wParam, lParam))
		return 0;
	
	return CallPrevWndProc(uMsg, wParam, lParam);
}


void BaseListView::OnSelectionRead(BOOL bRestart)
{	
	if (FALSE == enableSelectionRead || 
		0 == selectionMetrics.unknownData || 
		NULL == pActiveDocument || 
		0 != pActiveDocument->GetQueueSize() ||
		FALSE != pActiveDocument->QueryAsyncOpInfo(NULL))
		return;
		
	if (FALSE != bRestart)
		selectionCursor = -1;
	
	BOOL bTimeout = FALSE;
	UINT startMs = GetTickCount();

	selectionCursor = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)selectionCursor, (LPARAM)LVNI_SELECTED);
	INT iPage = (INT)CallPrevWndProc(LVM_GETCOUNTPERPAGE, 0, 0L);

	if (-1 == selectionCursor)
		selectionCursor = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
	
	size_t first(-1), last(-1);
	while (-1 != selectionCursor)
	{
		Document::METRICS metrics;
		metrics.cbSize = sizeof(Document::METRICS);
		metrics.flags = Document::FlagMetricLength;

        if (pActiveDocument->GetMetrics(selectionCursor, selectionCursor, &metrics) &&
			0 != metrics.unknownData)
		{
			if (-1 == first)
			{
				first = selectionCursor;
				last = first;
			}
			else if (selectionCursor == (last + 1) && 
					(last - first) < (size_t)iPage)
			{
				last++;
			}
			else
			{
				selectionCursor = (INT)last;
				break;
			}
		}
		selectionCursor = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)selectionCursor, (LPARAM)LVNI_SELECTED);
		
		if (((UINT)(GetTickCount() - startMs)) > 10U)
		{
			bTimeout = TRUE;
			break;
		}
	}
	
	if (((size_t)-1) == first || first > last ||
		!pActiveDocument->ReadItems(first, last, metaKeyList.data(), (INT)metaKeyList.size(), FALSE))
	{
		selectionCursor = -1;
	}

	if (bTimeout)
		PostNotification(NotifyReadSelection, FALSE, 0L);
	
}

static BOOL CALLBACK BaseListView_OnRedrawFetchedItems(INT first, INT last, ULONG_PTR param)
{
	BaseListView *view = (BaseListView*)param;
	if (NULL == view)
		return FALSE;

	view->CallPrevWndProc(LVM_REDRAWITEMS, (WPARAM)first, (LPARAM)last);	
	view->frameCache.SetItemStateEx(first, last, FrameCache::ItemStateCached, FALSE);
	return TRUE;
}
void BaseListView::RedrawCachedItems()
{
	frameCache.SetModified(FALSE);
	if (enableCachedRedraw && 
		(NULL == pDragData || pDragData->QueryIMarkActive()))
	{		
		frameCache.Enumerate(FrameCache::ItemStateFetched, BaseListView_OnRedrawFetchedItems, (ULONG_PTR)this);
	}

}

static void CALLBACK BaseListView_RedrawTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	BaseListView *view = (BaseListView*)DropBox_GetItemView(hwnd);
	if (NULL != view)
	{
		view->RedrawCachedItems();
	}
}


void BaseListView::OnVScroll(UINT scrollCode, UINT position, HWND hCtrl)
{
	switch(scrollCode)
	{
		case SB_PAGEUP:
		case SB_PAGEDOWN:
		{
			SCROLLINFO scrollInfo;
			scrollInfo.cbSize = sizeof(SCROLLINFO);
			scrollInfo.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
			if (GetScrollInfo(hwnd, SB_VERT, &scrollInfo))
			{
				INT delta = scrollInfo.nPage;
				if (SB_PAGEDOWN == scrollCode)
				{
					if ((scrollInfo.nPos + scrollInfo.nPage + delta) > (UINT)scrollInfo.nMax)
						delta = scrollInfo.nMax - (scrollInfo.nPos + scrollInfo.nPage);
				}
				else 
				{
					if ((scrollInfo.nPos - delta) < scrollInfo.nMin)
						delta =scrollInfo.nMin -  scrollInfo.nPos;
					else
						delta = -delta;
				}

				if (0 != delta)
				{				
					enableCachedRedraw = FALSE;
					CallPrevWndProc(WM_VSCROLL, MAKEWPARAM(scrollCode, position), (LPARAM)hCtrl);
					enableCachedRedraw = TRUE;
				}
					
				return;
			}
		}
		break;			
	}
	CallPrevWndProc(WM_VSCROLL, MAKEWPARAM(scrollCode, position), (LPARAM)hCtrl);
}

BOOL BaseListView::CanPlaySelection()
{
	if (NULL == pActiveDocument)
		return FALSE;

	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (selectedCount < 1)
		return FALSE;
	
	INT iItem = -1;
	IFileInfo *pItem;
	while(-1 != (iItem = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)iItem, (LPARAM)LVNI_SELECTED)))
	{
		pItem = pActiveDocument->GetItemDirect(iItem);
		if (S_OK == pItem->CanPlay())
			return TRUE;
	}
	return FALSE;
}

BOOL BaseListView::CanCopySelection()
{
	if (NULL == pActiveDocument)
		return FALSE;

	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (selectedCount < 1)
		return FALSE;
	
	INT iItem = -1;
	IFileInfo *pItem;
	while(-1 != (iItem = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)iItem, (LPARAM)LVNI_SELECTED)))
	{
		pItem = pActiveDocument->GetItemDirect(iItem);
		if (S_OK == pItem->CanCopy())
			return TRUE;
	}
	return FALSE;
}

void BaseListView::OnContextMenu(HWND hTarget, POINTS pts)
{
	if (hwnd != hTarget)
		return;

	if (NULL == pActiveDocument)
		return;
			
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	if (selectedCount < 0) selectedCount = 0;

	INT iCount = (INT)CallPrevWndProc(LVM_GETITEMCOUNT, 0, 0L);

	BOOL bEnable;

	UINT menuType;
	switch(selectedCount)
	{
		case 0:		menuType = DBMENU_VIEWCONTEXT; break;
		case 1:		menuType = DBMENU_ITEMCONTEXT; break;
		default:	menuType = DBMENU_SELECTIONCONTEXT; break;
	}
		
	HMENU hMenu = DropboxWindow_GetMenu(hParent, menuType);
	if (NULL == hMenu) return;
		
	UINT szPlayGroup[] = {ID_PLAY, ID_ENQUEUE,};
	MenuHelper_EnableGroup(hMenu, szPlayGroup, ARRAYSIZE(szPlayGroup), FALSE, CanPlaySelection());

	UINT szDeleteGroup[] = { ID_DELETE };
	MenuHelper_EnableGroup(hMenu, szDeleteGroup, ARRAYSIZE(szDeleteGroup), FALSE, (selectedCount > 0));

	UINT szItemOpGroup[] = { ID_SELECTALL, ID_INVERTSELECTION };
	MenuHelper_EnableGroup(hMenu, szItemOpGroup, ARRAYSIZE(szItemOpGroup), FALSE, (iCount > 0));

	UINT szCopyGroup[] = {ID_CUT, ID_COPY};
	MenuHelper_EnableGroup(hMenu, szCopyGroup, ARRAYSIZE(szCopyGroup), FALSE, CanCopySelection());

	UINT szPasteGroup[] = {ID_PASTE};
	
	bEnable = FALSE;
	IDataObject *pObject;
	if (SUCCEEDED(OleGetClipboard(&pObject)))
	{
		IClipboardFormatProcessor *pProcessor;
		if(SUCCEEDED(CreateDataObectProcessor(pObject, hParent, &pProcessor, DATAOBJECT_HDROP)))
		{
			bEnable = TRUE; 
			pProcessor->Release();
		}
		pObject->Release();
	}
	MenuHelper_EnableGroup(hMenu, szPasteGroup, ARRAYSIZE(szPasteGroup), FALSE, bEnable);

	UINT szInfoGroup[] = {ID_SHOWFILEINFO, ID_EXPLOREFOLDER};
	DWORD itemType = IItemType::itemTypeMissingFile;
	if (1 == selectedCount)
	{
		INT iSelected = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
		if (-1 != iSelected)
		{			
			IFileInfo *pItem = pActiveDocument->GetItemSafe(iSelected);
			if (FAILED(pItem->GetType(&itemType)))
				itemType = IItemType::itemTypeMissingFile;
			pItem->Release();
		}
	}
	INT groupSize = ARRAYSIZE(szInfoGroup);
	if (IItemType::itemTypeHttpStream == itemType)
	{
		MenuHelper_EnableGroup(hMenu, &szInfoGroup[1], 1, FALSE, FALSE);
		groupSize = 1;
	}
	MenuHelper_EnableGroup(hMenu, szInfoGroup, groupSize, FALSE, (IItemType::itemTypeMissingFile != itemType));
	
	POINT ptMenu;
	POINTSTOPOINT(ptMenu, pts);

	if (-1 == ptMenu.x && -1 == ptMenu.y)
	{
		INT iTop = -1;
		INT iFocus = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_FOCUSED);
		if (-1 != iFocus)
		{
			if (0 != (INT)CallPrevWndProc(LVM_GETITEMSTATE, (WPARAM)iFocus, (LPARAM)LVIS_SELECTED))
				iTop = iFocus;
		}

		if (-1 == iTop)
		{	
			INT iSelected = (INT)CallPrevWndProc(LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
			if (-1 != iSelected)
				iTop = iSelected;
		}
		
		RECT rcItem;
		rcItem.left = LVIR_BOUNDS;
		if (-1 != iTop &&
			CallPrevWndProc(LVM_GETITEMRECT, (WPARAM)iTop, (LPARAM)&rcItem))
		{
			INT offset = (rcItem.bottom - rcItem.top)/2 + (rcItem.bottom - rcItem.top)%2;
			ptMenu.x = rcItem.left + offset;
			ptMenu.y = rcItem.top + offset;
		}
		else
		{
			INT offset;
			if (CallPrevWndProc(LVM_GETITEMRECT, (WPARAM)0, (LPARAM)&rcItem))
				offset = (rcItem.bottom - rcItem.top)/2 + (rcItem.bottom - rcItem.top)%2;
			else
				offset = 6;
			ptMenu.x = offset;
			ptMenu.y = offset;
		}

		MapWindowPoints(hwnd, HWND_DESKTOP, &ptMenu, 1);
	}
	RECT rc;
	if (GetClientRect(hwnd, &rc))
	{
		MapWindowPoints(hwnd, HWND_DESKTOP, (POINT*)&rc, 2);
		if (ptMenu.x < rc.left) ptMenu.x = rc.left;
		else if (ptMenu.x > rc.right) ptMenu.x = rc.right;
		if (ptMenu.y < rc.top) ptMenu.y = rc.top;
		else if (ptMenu.y > rc.bottom) ptMenu.y = rc.bottom;
	}

	TrackPopup(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_VERPOSANIMATION, 
			ptMenu.x, ptMenu.y, hwnd, !GetSkinned());

	DropboxWindow_ReleaseMenu(hParent, DBMENU_ITEMCONTEXT, hMenu);
}

STDMETHODIMP BaseListView::GetSelectionMetrics(Document::METRICS *pMetrics)
{
	if (NULL == pMetrics)
		return E_POINTER;

	if (Document::FlagMetricSize & pMetrics->flags)
		pMetrics->size = selectionMetrics.size;

	if (Document::FlagMetricLength & pMetrics->flags)
	{
		pMetrics->length = selectionMetrics.length;
		pMetrics->unknownData = selectionMetrics.unknownData;
	}

	return S_OK;
}

STDMETHODIMP BaseListView::GetSelectionCount(size_t *pCount)
{
	if (NULL == pCount)
		return E_POINTER;
	
	INT selectedCount = (INT)CallPrevWndProc(LVM_GETSELECTEDCOUNT, 0, 0L);
	*pCount = (size_t)selectedCount;

	return S_OK;
}
STDMETHODIMP BaseListView::EnableSelectionRead(BOOL bEnable)
{
	enableSelectionRead = bEnable;

	if(	enableSelectionRead && 
		selectionMetrics.unknownData > 0 && 
		NULL != pActiveDocument &&
		FALSE == pActiveDocument->QueryAsyncOpInfo(NULL) &&
		0 == pActiveDocument->GetQueueSize())
	{
		PostNotification(NotifyReadSelection, TRUE, 0L);
	}
	return S_OK;
}

void BaseListView::ScheduleSelectionRead()
{
	if (0 != selectionMetrics.unknownData && enableSelectionRead)
	{
		SetTimer(hwnd, READSELECTION_TIMER, READSELECTION_DELAY,
					BaseListView_ReadSelectionTimerProc);
	}
}

static void CALLBACK BaseListView_ReadSelectionTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	BaseListView_SendNotification(hwnd, BaseListView::NotifyReadSelection, TRUE, 0L);
}



static void CALLBACK BaseListView_OnCacheModified(BOOL cacheModified, ULONG_PTR param)
{
	BaseListView *view = (BaseListView*)param;
	if (NULL != view)
		view->SendNotificationMT(BaseListView::NotifyCacheModified, cacheModified, 0L);
	
}