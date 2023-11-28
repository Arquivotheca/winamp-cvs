#include "./main.h"
#include "./resource.h"
#include "./detailsView.h"
#include "./wasabiApi.h"
#include "./fileInfoInterface.h"
#include "./fileMetaInterface.h"
#include "./configIniSection.h"
#include "./configManager.h"
#include "./listSelectHook.h"
#include "./document.h"
#include "./imageLoader.h"




#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200
#endif // !HDF_SORTUP


DetailsView::DetailsView(HWND hView) :
	BaseListView(hView)
{

	DWORD styleEx = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP;
	SendMessage(hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx, styleEx);
	SendMessage(hwnd, LVM_SETUNICODEFORMAT, (WPARAM)TRUE, 0L);
	
	HWND hParent = GetParent(hwnd);
	Profile *profile = (NULL != hParent) ? DropboxWindow_GetProfile(hParent) : NULL;
    if (NULL != profile)
		Load(profile);

	if (-1 == columns[0].id)
	{
		for (int i = 0; i < ARRAYSIZE(szRegisteredColumns); i++)
		{
			columns[i].id = szRegisteredColumns[i].id;
			columns[i].order = i;
			columns[i].width = szRegisteredColumns[i].width;
		}
		columns[ARRAYSIZE(szRegisteredColumns)].id = -1;
	}

	METAKEY szMetaKeys[] = { METAKEY_FORMATTEDTITLE, METAKEY_TRACKLENGTH, METAKEY_TRACKTITLE, METAKEY_TRACKARTIST, METAKEY_TRACKALBUM};
	for (int i = 0; i < ARRAYSIZE(szMetaKeys); i++)
		metaKeyList.push_back(szMetaKeys[i]);

	UpdateListColumns();
}

DetailsView::~DetailsView()
{
	
}

void DetailsView::UpdateColumnsData()
{
	HWND hHeader =  (HWND)SendMessage(hwnd, LVM_GETHEADER, 0, 0L);
	if (NULL == hHeader) return;
	
	INT orders[ARRAYSIZE(columns)];
	HDITEM item;
	item.mask = HDI_WIDTH;
	INT count = (INT)SendMessage(hHeader, HDM_GETITEMCOUNT, 0, 0L);
	
	//ASSERT(count > ARRAYSIZE(orders));
	
	if (0 != SendMessage(hHeader, HDM_GETORDERARRAY, (WPARAM)count, (LPARAM)orders))
	{
		for (int i = 0; i < count; i++)
		{
			columns[orders[i]].order = i;
			if (SendMessage(hHeader, HDM_GETITEM, (WPARAM)orders[i], (LPARAM)&item))
				columns[orders[i]].width = item.cxy;
		}
	}
	columns[count].id = -1;
}

void DetailsView::UpdateListColumns()
{
	DWORD windowStyle = GetWindowStyle(hwnd);
	
	if (0 != (WS_VISIBLE & windowStyle))
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle & ~WS_VISIBLE);
	
	HWND hHeader =  (HWND)SendMessage(hwnd, LVM_GETHEADER, 0, 0L);
	if (NULL != hHeader)
	{
		INT columnCount = (INT)SendMessage(hHeader, LVM_GETITEMCOUNT, 0, 0L);
		while(columnCount--) 
			SendMessage(hwnd, LVM_DELETECOLUMN, (WPARAM)columnCount, 0L);
	}
	
	LVCOLUMN lvc;
	LPCTSTR pszText;
	TCHAR szBuffer[512];
	
	for (int i = 0; i < ARRAYSIZE(columns) && -1 != columns[i].id; i++)
	{
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
		lvc.fmt = szRegisteredColumns[columns[i].id].format;
		lvc.cx = columns[i].width;
		if (lvc.cx < 0) lvc.cx = 0;
		pszText = szRegisteredColumns[columns[i].id].pszTitle;
		if (IS_INTRESOURCE(pszText))
		{
			WASABI_API_LNGSTRINGW_BUF((INT)(INT_PTR)pszText, szBuffer, ARRAYSIZE(szBuffer));
			pszText = szBuffer;
		}
		lvc.pszText = (LPTSTR)pszText;
		SendMessage(hwnd, LVM_INSERTCOLUMN, (WPARAM)i, (LPARAM)&lvc);
	}
	
	if (0 != (WS_VISIBLE & windowStyle))
	{
		SetWindowLongPtr(hwnd, GWL_STYLE, windowStyle);
		InvalidateRect(hwnd, NULL, TRUE);
	}
}

STDMETHODIMP DetailsView::ProcessNotification(NMHDR *pnmh, LRESULT *pResult)
{
	switch(pnmh->code)
	{
		case LVN_GETDISPINFO:	OnGetDispInfo((NMLVDISPINFO*)pnmh); return S_OK;
		case LVN_COLUMNCLICK:	OnColumnClick((NMLISTVIEW*)pnmh); return S_OK;
	}
	return BaseListView::ProcessNotification(pnmh, pResult);
}



STDMETHODIMP_(DropboxViewMeta*) DetailsView::GetMeta()
{
	return detailsViewMeta;
}
STDMETHODIMP DetailsView::ConfigChanged(void)
{
	return S_OK;
}

void DetailsView::OnGetDispInfo(NMLVDISPINFO *pdi)
{	
	if (NULL == pActiveDocument || 
		((size_t)pdi->item.iItem) >= pActiveDocument->GetItemCount())
	{
		pdi->item.pszText = TEXT("");
		return;
	}

	IFileInfo *pFileInfo = pActiveDocument->GetItemDirect(pdi->item.iItem);
	if (NULL == pFileInfo)
	{
		pdi->item.pszText = TEXT("<< Bad Item Data>>");
		return;
	}
	
	UINT columnId = (UINT)columns[pdi->item.iSubItem].id;
	if (columnId >= COLUMN_LAST)
	{
		pdi->item.pszText = TEXT("Bad Column Index");
		return;
	}

	const LISTCOLUMN *pColumn = &szRegisteredColumns[columnId];
		
	if ((LVIF_TEXT & pdi->item.mask))
	{
		LPCTSTR pszText = NULL;
		if (NULL != pColumn->fnFormatter)
		{			
			pszText = (LPTSTR)pColumn->fnFormatter(pFileInfo, pdi->item.pszText, pdi->item.cchTextMax);
		}
		pdi->item.pszText = (NULL != pszText) ? pszText : TEXT("");
	}
}

void DetailsView::OnColumnClick(NMLISTVIEW *pnmv)
{
	HWND hParent = GetParent(hwnd);
	if (NULL == hParent) return;

	INT columnId = (pnmv->iSubItem >= 0 && pnmv->iSubItem < ARRAYSIZE(columns)) ? 
		columns[pnmv->iSubItem].id : -1;

	DropboxWindow_ArrangeBy(hParent, columnId);
}

void DetailsView::OnLButtonDown(UINT uFlags, POINTS pts)
{

	LVHITTESTINFO ht;
	POINTSTOPOINT(ht.pt, pts);
	INT index = (INT)SendMessage(hwnd, LVM_HITTEST, 0, (LPARAM)&ht);
	if (-1 == index || 
		0 == SendMessage(hwnd, LVM_GETITEMSTATE, (WPARAM)index, 
					(LPARAM)(LVIS_SELECTED | LVIS_DROPHILITED | LVIS_CUT)))
	{
		MouseHook *pHook = ListViewSelectionHook::CreateHook(hwnd);
		if (NULL != pHook) 
		{
			if (hwnd !=GetFocus())
				SetFocus(hwnd);
			return;
		}
	}
	CallPrevWndProc(WM_LBUTTONDOWN, (WPARAM)uFlags, *((LPARAM*)&pts));
}


LRESULT DetailsView::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_LBUTTONDOWN:	 OnLButtonDown((UINT)wParam, MAKEPOINTS(lParam)); return 0;
	}


	return BaseListView::WindowProc(uMsg, wParam, lParam);
}