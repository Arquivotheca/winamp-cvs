#include "main.h"
#include "api.h"

#include "Downloaded.h"
#include "./navigation.h"
#include "DownloadStatus.h"
#include "Defaults.h"

#include "../nu/listview.h"
#include "../gen_ml/ml_ipc.h"
#include "../gen_ml/ml_ipc_0313.h"
#include "../nu/Vector.h"
#include "../nu/menushortcuts.h"
#include <commctrl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <strsafe.h>
#include <algorithm>

#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#define HDF_SORTDOWN            0x0200
#endif // !HDF_SORTUP

using namespace Nullsoft::Utility;

enum
{
    COL_CHANNEL = 0,
    COL_ITEM,
	COL_PROGRESS,
    COL_PATH,
    NUM_COLUMNS,
};

int downloadsChannelWidth=DOWNLOADSCHANNELWIDTHDEFAULT;
int downloadsItemWidth=DOWNLOADSITEMWIDTHDEFAULT;
int downloadsProgressWidth=DOWNLOADSPROGRESSWIDTHDEFAULT;
int downloadsPathWidth=DOWNLOADSPATHWIDTHDEFAULTS;

W_ListView downloadList;
int downloadsItemSort = -1; // -1 means no sort active
bool downloadsSortAscending = true;

enum
{
    DOWNLOADSDIALOG_TIMER_UPDATESTATUSBAR = 0,
};

class DownloadListItem {
public:
	DownloadedFile *f;
	DownloadToken token;
	wchar_t *channel, *item, *path;
	wchar_t status[20];
	DownloadListItem(DownloadedFile *fi) : token(0), channel(0), item(0), path(0) { f = new DownloadedFile(*fi); }
	DownloadListItem(DownloadToken token, const wchar_t *channel0, const wchar_t *item0, const wchar_t *path0, size_t downloaded, size_t maxSize) : token(token), f(0) {
		if(maxSize) StringCchPrintf(status,20,WASABI_API_LNGSTRINGW(IDS_DOWNLOADING_PERCENT),(int)(downloaded/(maxSize/100)));
		else WASABI_API_LNGSTRINGW_BUF(IDS_DOWNLOADING,status,20);
		channel = channel0?_wcsdup(channel0):NULL;
		item = item0?_wcsdup(item0):NULL;
		path = path0?_wcsdup(path0):NULL;
	}
	~DownloadListItem() {
		if(channel) free(channel);
		if(item) free(item);
		if(path) free(path);
		if(f) delete f;
	}
};

static Vector<DownloadListItem*> listContents;

static ChildWndResizeItem downloadsItems[] = {
  /* download group */
  {IDC_PLAY, DockToBottom },
  {IDC_ENQUEUE, DockToBottom},
  {IDC_REMOVE, DockToBottom},
  {IDC_CLEANUP, DockToBottom},
  {IDC_DOWNLOADLIST, ResizeRight | ResizeBottom},
  {IDC_STATUS, DockToBottom | ResizeRight},
};

bool GetDownload(int &download)
{
	download = ListView_GetNextItem(downloadList.getwnd(), download, LVNI_ALL | LVNI_SELECTED);
	if (download == -1)
		return false;
	else
		return true;
}

void Downloads_Play(bool enqueue=false)
{
	int download=-1;
	AutoLock lock (downloadedFiles);
	while (GetDownload(download))
	{
		if(!enqueue) {
			if(listContents[download]->f)
				mediaLibrary.PlayFile(listContents[download]->f->path);
			else if(listContents[download]->path)
				mediaLibrary.PlayFile(listContents[download]->path);
			enqueue=true;
		} else {
			if(listContents[download]->f)
				mediaLibrary.EnqueueFile(listContents[download]->f->path);
			else if(listContents[download]->path)
				mediaLibrary.EnqueueFile(listContents[download]->path);
		}
	}
}

void DownloadsUpdated(const DownloadStatus::Status& s, DownloadToken token) 
{
	listContents.push_back(new DownloadListItem(token,s.channel,s.item,s.path,s.downloaded,s.maxSize));
	downloadList.SetVirtualCountAsync(listContents.size());
}

void DownloadsUpdated(DownloadToken token, const DownloadedFile *f) {
	for(size_t i=0; i<listContents.size(); i++) {
		if(listContents[i]->token == token) {
			listContents[i]->token=0;
			if(f) {
				listContents[i]->f = new DownloadedFile(*f);
				if(listContents[i]->channel) free(listContents[i]->channel); listContents[i]->channel=0;
				if(listContents[i]->item) free(listContents[i]->item); listContents[i]->item=0;
				if(listContents[i]->path) free(listContents[i]->path); listContents[i]->path=0;
			} else lstrcpyn(listContents[i]->status,L"Error",20);
			PostMessage(downloadList.getwnd(),LVM_REDRAWITEMS,i,i);
		}
	}
}

void DownloadsUpdated()
{
	for(size_t i=0; i<listContents.size(); i++) delete listContents[i];
	listContents.clear();

	for(size_t i=0; i<downloadedFiles.downloadList.size(); i++) {
		listContents.push_back(new DownloadListItem(&downloadedFiles.downloadList[i]));
	}
	{
		AutoLock lock(downloadStatus.statusLock);
		for(DownloadStatus::Downloads::iterator itr = downloadStatus.downloads.begin(); itr != downloadStatus.downloads.end(); itr++) {
			listContents.push_back(new DownloadListItem(itr->first,itr->second.channel,itr->second.item,itr->second.path,itr->second.downloaded,itr->second.maxSize));
		}
	}
	downloadList.SetVirtualCountAsync(listContents.size());
	Navigation_ShowService(SERVICE_DOWNLOADS, SHOWMODE_AUTO);
}

static void CleanupDownloads()
{
	{
		AutoLock lock (downloadedFiles);
		DownloadList::DownloadedFileList &downloads = downloadedFiles.downloadList;
		DownloadList::iterator itr, next;
		for (itr = downloads.begin();itr != downloads.end();)
		{
			next = itr;
			next++;
			if (!PathFileExists(itr->path))
				downloads.erase(itr);
			else
				itr = next;
		}
	}
	Navigation_ShowService(SERVICE_DOWNLOADS, SHOWMODE_AUTO);
}

void Downloads_UpdateStatusBar(HWND hwndDlg)
{
	wchar_t status[256]=L"";
	downloadStatus.GetStatusString(status, 256);
	SetWindowText(GetDlgItem(hwndDlg, IDC_STATUS), status);
}

void Downloads_Paint(HWND hwndDlg)
{
	int tab[] = { IDC_DOWNLOADLIST | DCW_SUNKENBORDER, };
	dialogSkinner.Draw(hwndDlg, tab, sizeof(tab) / sizeof(tab[0]));
}

void Downloads_Size(HWND hwndDlg, WPARAM wParam)
{
	if (wParam != SIZE_MINIMIZED)
		childSizer.Resize(hwndDlg, downloadsItems, sizeof(downloadsItems) / sizeof(downloadsItems[0]));
}

void Downloads_DisplayChange(HWND hwndDlg)
{
	ListView_SetTextColor(downloadList.getwnd(), dialogSkinner.Color(WADLG_ITEMFG));
	ListView_SetBkColor(downloadList.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
	ListView_SetTextBkColor(downloadList.getwnd(), dialogSkinner.Color(WADLG_ITEMBG));
	downloadList.SetFont(dialogSkinner.GetFont());
}

static void DownloadsDialog_SkinControls(HWND hwnd, const INT *itemList, INT itemCount, UINT skinType, UINT skinStyle)
{
	MLSKINWINDOW skinWindow;
	skinWindow.style = skinStyle;
	skinWindow.skinType = skinType;

	for(INT i = 0; i < itemCount; i++)
	{
		skinWindow.hwndToSkin = GetDlgItem(hwnd, itemList[i]);
		if (NULL != skinWindow.hwndToSkin)
		{
			MLSkinWindow(plugin.hwndLibraryParent, &skinWindow);
		}
	}
}

static void DownloadDialog_InitializeList(HWND hwnd)
{
	HWND hControl = GetDlgItem(hwnd, IDC_DOWNLOADLIST);
	if (NULL == hControl) return;

	UINT styleEx = (UINT)GetWindowLongPtr(hControl, GWL_EXSTYLE);
	SetWindowLongPtr(hControl, GWL_EXSTYLE, styleEx & ~WS_EX_NOPARENTNOTIFY);

	styleEx = LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | /*LVS_EX_HEADERDRAGDROP |*/ LVS_EX_INFOTIP;
	SendMessage(hControl, LVM_SETEXTENDEDLISTVIEWSTYLE, styleEx, styleEx);
	SendMessage(hControl, LVM_SETUNICODEFORMAT, (WPARAM)TRUE, 0L);

	MLSKINWINDOW skinWindow;
	skinWindow.style = SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS | SWLVS_ALTERNATEITEMS;
	skinWindow.skinType = SKINNEDWND_TYPE_LISTVIEW;
	skinWindow.hwndToSkin = hControl;
	MLSkinWindow(plugin.hwndLibraryParent, &skinWindow);
}

bool COL_CHANNEL_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
											 (item1->f?item1->f->channel:item1->channel), -1,
											 (item2->f?item2->f->channel:item2->channel), -1));
}

bool COL_ITEM_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
											 (item1->f?item1->f->item:item1->item), -1,
											 (item2->f?item2->f->item:item2->item), -1));
}

bool COL_PROGRESS_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
											(item1->f?WASABI_API_LNGSTRINGW(IDS_DONE):item1->status), -1,
											(item2->f?WASABI_API_LNGSTRINGW(IDS_DONE):item2->status), -1));
}

bool COL_PATH_Sort(const DownloadListItem* item1, const DownloadListItem* item2)
{
	return (CSTR_LESS_THAN == CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
											 (item1->f?item1->f->path:item1->path), -1,
											 (item2->f?item2->f->path:item2->path), -1));
}

static BOOL Downloads_SortItems(int sortColumn)
{	
	AutoLock lock (downloadedFiles);
	switch (sortColumn)
	{
		case COL_CHANNEL:
			std::sort(listContents.begin(), listContents.end(), COL_CHANNEL_Sort);
			return TRUE;
		case COL_ITEM:
			std::sort(listContents.begin(), listContents.end(), COL_ITEM_Sort);
			return TRUE;
		case COL_PROGRESS:
			std::sort(listContents.begin(), listContents.end(), COL_PROGRESS_Sort);
			return TRUE;
		case COL_PATH:
			std::sort(listContents.begin(), listContents.end(), COL_PATH_Sort);
			return TRUE;
	}
	return FALSE;
}

static void Downloads_SetListSortColumn(HWND hwnd, INT listId, INT index, BOOL fAscending)
{
	HWND hItems = GetDlgItem(hwnd, listId);
	if (NULL == hItems) return;
	
	HWND hHeader = (HWND)SNDMSG(hItems, LVM_GETHEADER, 0, 0L);
	if (NULL == hHeader) return;
		
	HDITEM item;
	item.mask = HDI_FORMAT;
	// reset first (ml req)
	INT count = (INT)SNDMSG(hHeader, HDM_GETITEMCOUNT, 0, 0L);
	for (INT i = 0; i < count; i++)
	{
		if (index != i && FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, i, (LPARAM)&item))
		{
			if (0 != ((HDF_SORTUP | HDF_SORTDOWN) & item.fmt))
			{	
				item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
				SNDMSG(hHeader, HDM_SETITEM, i, (LPARAM)&item);
			}
		}
	}

	if (FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, index, (LPARAM)&item))
	{
		INT fmt = item.fmt & ~(HDF_SORTUP | HDF_SORTDOWN);
		fmt |= (FALSE == fAscending) ? HDF_SORTDOWN : HDF_SORTUP;
		if (fmt != item.fmt)
		{
			item.fmt = fmt;
			SNDMSG(hHeader, HDM_SETITEM, index, (LPARAM)&item);
		}
	}
}

static BOOL Downloads_Sort(HWND hwnd, INT iColumn, bool fAscending)
{
	BOOL result = TRUE;
	Downloads_SortItems(iColumn);
	downloadsSortAscending = fAscending;
	Downloads_SetListSortColumn(hwnd, IDC_DOWNLOADLIST, iColumn, fAscending);

	if (FALSE != result)
	{
		HWND hItems = GetDlgItem(hwnd, IDC_DOWNLOADLIST);
		if (NULL != hItems) 
			InvalidateRect(hItems, NULL, TRUE);
	}
	
	return TRUE;
}

void Downloads_Init(HWND hwndDlg)
{
	HWND hLibrary = plugin.hwndLibraryParent;

	HACCEL accel = LoadAccelerators(plugin.hDllInstance, MAKEINTRESOURCE(IDR_VIEW_DOWNLOAD_ACCELERATORS));
	if (accel)
		WASABI_API_APP->app_addAccelerators(hwndDlg, &accel, 1, TRANSLATE_MODE_CHILD);

	MLSkinWindow2(hLibrary, hwndDlg, SKINNEDWND_TYPE_AUTO, SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	const INT szControls[] = { IDC_PLAY, IDC_ENQUEUE, IDC_REMOVE, IDC_CLEANUP, IDC_STATUS, };
	DownloadsDialog_SkinControls(hwndDlg, szControls, ARRAYSIZE(szControls), SKINNEDWND_TYPE_AUTO,
		SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	DownloadDialog_InitializeList(hwndDlg);

	CleanupDownloads();
	Downloads_UpdateStatusBar(hwndDlg);

	downloadList.setwnd(GetDlgItem(hwndDlg, IDC_DOWNLOADLIST));
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_CHANNEL), downloadsChannelWidth);
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_ITEM), downloadsItemWidth);
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_PROGRESS), downloadsProgressWidth);
	downloadList.AddCol(WASABI_API_LNGSTRINGW(IDS_PATH), downloadsPathWidth);

    DownloadsUpdated();

	downloadList.SetVirtualCount(listContents.size());
	Downloads_DisplayChange(hwndDlg);
	childSizer.Init(hwndDlg, downloadsItems, sizeof(downloadsItems) / sizeof(downloadsItems[0]));

	Downloads_Sort(hwndDlg, downloadsItemSort, downloadsSortAscending);

	SetTimer(hwndDlg, DOWNLOADSDIALOG_TIMER_UPDATESTATUSBAR , 1000, 0);
}

void Downloads_Timer(HWND hwndDlg, UINT timerId)
{
	switch (timerId)
	{
	case DOWNLOADSDIALOG_TIMER_UPDATESTATUSBAR:
		Downloads_UpdateStatusBar(hwndDlg);
		{
			AutoLock lock(downloadStatus.statusLock);
			for(size_t i=0; i<listContents.size(); i++) {
				if(listContents[i]->token) {
					size_t d = downloadStatus.downloads[listContents[i]->token].downloaded;
					size_t s = downloadStatus.downloads[listContents[i]->token].maxSize;
					if(s) StringCchPrintf(listContents[i]->status,20,WASABI_API_LNGSTRINGW(IDS_DOWNLOADING_PERCENT),(int)(d/(s/100)));
					else WASABI_API_LNGSTRINGW_BUF(IDS_DOWNLOADING,listContents[i]->status,20);
					PostMessage(downloadList.getwnd(),LVM_REDRAWITEMS,i,i);
				}
			}
		}
		break;
	}
}

static INT Downloads_GetListSortColumn(HWND hwnd, INT listId, bool *fAscending)
{
	HWND hItems = GetDlgItem(hwnd, listId);
	if (NULL != hItems)
	{
		HWND hHeader = (HWND)SNDMSG(hItems, LVM_GETHEADER, 0, 0L);
		if (NULL != hHeader)
		{
			HDITEM item;
			item.mask = HDI_FORMAT;

			INT count = (INT)SNDMSG(hHeader, HDM_GETITEMCOUNT, 0, 0L);
			for (INT i = 0; i < count; i++)
			{
				if (FALSE != (BOOL)SNDMSG(hHeader, HDM_GETITEM, i, (LPARAM)&item) &&
					0 != ((HDF_SORTUP | HDF_SORTDOWN) & item.fmt))
				{
					if (NULL != fAscending)
					{
						*fAscending = (0 != (HDF_SORTUP & item.fmt));
					}
					return i;
				}
			}
		}
	}
	return -1;
}

void Downloads_Destroy(HWND hwndDlg)
{
	downloadsChannelWidth = downloadList.GetColumnWidth(COL_CHANNEL);
	downloadsItemWidth = downloadList.GetColumnWidth(COL_ITEM);
	downloadsProgressWidth = downloadList.GetColumnWidth(COL_PROGRESS);
	downloadsPathWidth = downloadList.GetColumnWidth(COL_PATH);

	for(size_t i=0; i<listContents.size(); i++) delete listContents[i];
	listContents.clear();
	downloadList.setwnd(NULL); 

	bool fAscending;
	downloadsItemSort = Downloads_GetListSortColumn(hwndDlg, IDC_DOWNLOADLIST, &fAscending);
	downloadsSortAscending = (-1 != downloadsItemSort) ? (FALSE != fAscending) : true;
}

void Downloads_Remove(bool del=false, HWND parent=NULL)
{
	int download,d=-1;
	int r=0;
	while(GetDownload(d))
	{
		download = d - r;
		DownloadListItem * item = listContents[download];
		if(item->f) {
			AutoLock lock (downloadedFiles);
			int j=0;
			for(DownloadList::iterator i=downloadedFiles.begin(); i!=downloadedFiles.end(); i++) {
				if(!wcscmp(i->path,item->f->path)) {
					if(del)	
					{
						if(!downloadedFiles.RemoveAndDelete(j))
							MessageBox(parent,WASABI_API_LNGSTRINGW(IDS_DELETEFAILED),downloadedFiles.downloadList[j].path,0);
					}
					else downloadedFiles.Remove(j);
					delete item;
					listContents.eraseAt(download);
					r++;
					break;
				}
				j++;
			}
		} else if(item->token) {
			AutoLock lock (downloadStatus.statusLock);
			downloadStatus.downloads[item->token].killswitch=1;
			delete item;
			listContents.eraseAt(download);
			r++;
		} else {
			delete item;
			listContents.eraseAt(download);
			r++;
		}
	}
	downloadList.SetVirtualCountAsync(listContents.size());
	Navigation_ShowService(SERVICE_DOWNLOADS, SHOWMODE_AUTO);
}

void Downloads_Delete(HWND parent)
{
	wchar_t message[256];
	int c=downloadList.GetSelectedCount();
	if(!c) return;
	else if(c==1) WASABI_API_LNGSTRINGW_BUF(IDS_PERM_DELETE_ARE_YOU_SURE,message,256);
	else StringCchPrintf(message, 256, WASABI_API_LNGSTRINGW(IDS_PERM_DELETE_THESE_ARE_YOU_SURE),c);
	if(MessageBox(NULL, message, WASABI_API_LNGSTRINGW(IDS_DELETION), MB_ICONWARNING|MB_YESNO) == IDNO) return;
	Downloads_Remove(true,parent);
}

void Downloads_CleanUp(HWND hwndDlg)
{
	wchar_t titleStr[64];
	if(MessageBox(hwndDlg,WASABI_API_LNGSTRINGW(IDS_CLEAR_ALL_FINISHED_DOWNLOADS),
				  WASABI_API_LNGSTRINGW_BUF(IDS_CLEAN_UP_LIST,titleStr,64),
				  MB_ICONWARNING|MB_YESNO) == IDNO) return;
	{
		AutoLock lock (downloadedFiles);
		downloadedFiles.downloadList.clear();
	}
	DownloadsUpdated();
}

void Downloads_InfoBox(HWND parent) {
	int download=-1;
	if(GetDownload(download)) {
		const wchar_t *fn;
		if(listContents[download]->f) fn=listContents[download]->f->path;
		else fn = listContents[download]->path;
		if(fn) {
			infoBoxParamW p={parent,fn};
			SendMessage(plugin.hwndWinampParent, WM_WA_IPC, (WPARAM)&p, IPC_INFOBOXW);
		}
	}
}

void Downloads_SelectAll() {
	int l = downloadList.GetCount();
	for (int i = 0;i < l;i++) downloadList.SetSelected(i);
}

/*static void exploreItemFolder(HWND hwndDlg)
{
	if (downloadList.GetSelectionMark() >= 0)
	{
		int download=-1;
		if(GetDownload(download)) {
			const wchar_t *file;
			if(listContents[download]->f) file=listContents[download]->f->path;
			else file = listContents[download]->path;

			// abort if not valid or if it's a stream-based filename (may not handle some cases correctly)
			if (!file || wcsstr(file,L"://")) return ;

			// open an explorer window with the file selected in it & if shift is down then open a new window
			// won't reselect if the explorer view is already open and there was a selection (annoying :( )
			wchar_t tmp[1024];
			StringCchPrintfW(tmp, 1024, L"%s/select,\"%s\"",(GetAsyncKeyState(VK_SHIFT)?L"/n,":L""), file);
			ShellExecuteW(hwndDlg, NULL, L"explorer.exe", tmp, NULL, SW_SHOWNORMAL);
		}
	}
}*/
static void exploreItemFolder(HWND hwndDlg)
{
	if (downloadList.GetSelectionMark() >= 0)
	{
		int download=-1;
		while (GetDownload(download))
		{
			wchar_t *file;
			if(listContents[download]->f) file=listContents[download]->f->path;
			else file = listContents[download]->path;
			WASABI_API_EXPLORERFINDFILE->AddFile(file);
		}
		WASABI_API_EXPLORERFINDFILE->ShowFiles();
	}
}

int we_are_drag_and_dropping = 0;

static void Downloads_OnColumnClick(HWND hwnd, NMLISTVIEW *plv)
{
	bool fAscending;
	INT iSort = Downloads_GetListSortColumn(hwnd, IDC_DOWNLOADLIST, &fAscending);
	fAscending = (-1 != iSort && iSort == plv->iSubItem) ? (!fAscending) : true;
	Downloads_Sort(hwnd, plv->iSubItem, fAscending);
}

LRESULT DownloadList_Notify(LPNMHDR l, HWND hwndDlg)
{
	switch (l->code)
	{
	case LVN_COLUMNCLICK:
		Downloads_OnColumnClick(hwndDlg, (NMLISTVIEW*)l);
		break;
	//case NM_RETURN:
	case NM_DBLCLK:
		Downloads_Play(((!!(GetAsyncKeyState(VK_SHIFT)&0x8000)) ^ ML_ENQDEF_VAL()));
		break;
	case LVN_BEGINDRAG:
		we_are_drag_and_dropping=1;
		SetCapture(hwndDlg);
		break;
	case LVN_GETDISPINFO:
		NMLVDISPINFO *lpdi = (NMLVDISPINFO*) l;
		size_t item = lpdi->item.iItem;

		if (item < 0 || item >= listContents.size()) return 0;
		if (FALSE == downloadsSortAscending) item = listContents.size() - item - 1;

		DownloadListItem *l = listContents[item];
		if (lpdi->item.mask & LVIF_TEXT)
		{
			lpdi->item.pszText[0] = 0;
			switch (lpdi->item.iSubItem)
			{
			case COL_CHANNEL:
				if(!l->token && l->f) lstrcpyn(lpdi->item.pszText, l->f->channel, lpdi->item.cchTextMax);
				else lstrcpyn(lpdi->item.pszText, l->channel, lpdi->item.cchTextMax);
				break;
			case COL_ITEM:
				if(!l->token && l->f) lstrcpyn(lpdi->item.pszText, l->f->item, lpdi->item.cchTextMax);
				else lstrcpyn(lpdi->item.pszText, l->item, lpdi->item.cchTextMax);
				break;
			case COL_PROGRESS:
				if(!l->token && l->f) WASABI_API_LNGSTRINGW_BUF(IDS_DONE,lpdi->item.pszText,lpdi->item.cchTextMax);
				else lstrcpyn(lpdi->item.pszText, l->status, lpdi->item.cchTextMax);
				break;
			case COL_PATH:
				if(!l->token && l->f) lstrcpyn(lpdi->item.pszText, l->f->path, lpdi->item.cchTextMax);
				else lstrcpyn(lpdi->item.pszText, l->path, lpdi->item.cchTextMax);
				break;
			}
		}
		break;
	}
	return 0;
}

void listbuild(wchar_t **buf, int &buf_size, int &buf_pos, const wchar_t *tbuf)
{
	if(!*buf)
	{
		*buf = (wchar_t*)malloc(4096*sizeof(wchar_t));
		buf_size = 4096;
		buf_pos = 0;
	}
	int newsize = buf_pos + lstrlenW(tbuf) + 1;
	if (newsize < buf_size)
	{
		buf_size = newsize + 4096;
		*buf = (wchar_t*)realloc(*buf, (buf_size + 1) * sizeof(wchar_t));
	}
	StringCchCopyW(*buf + buf_pos, buf_size, tbuf);
	buf_pos = newsize;
}

wchar_t * getSelectedList()
{
	wchar_t* path=NULL;
	int buf_pos=0, buf_size=0;
	int download=-1;
	while (GetDownload(download))
	{
		if(listContents[download]->f)
			listbuild(&path,buf_size,buf_pos,listContents[download]->f->path);
	}
	if(path) path[buf_pos] = 0;
	return path;
}

static void SwapPlayEnqueueInMenu(HMENU listMenu)
{
	int playPos=-1, enqueuePos=-1;
	MENUITEMINFOW playItem={sizeof(MENUITEMINFOW), 0,}, enqueueItem={sizeof(MENUITEMINFOW), 0,};

	int numItems = GetMenuItemCount(listMenu);

	for (int i=0;i<numItems;i++)
	{
		UINT id = GetMenuItemID(listMenu, i);
		if (id == IDC_PLAY)
		{
			playItem.fMask = MIIM_ID;
			playPos = i;
			GetMenuItemInfoW(listMenu, i, TRUE, &playItem);
		}
		else if (id == IDC_ENQUEUE)
		{
			enqueueItem.fMask = MIIM_ID;
				enqueuePos= i;
			GetMenuItemInfoW(listMenu, i, TRUE, &enqueueItem);
		}
	}

	playItem.wID = IDC_ENQUEUE;
	enqueueItem.wID = IDC_PLAY;		
	SetMenuItemInfoW(listMenu, playPos, TRUE, &playItem);
	SetMenuItemInfoW(listMenu, enqueuePos, TRUE, &enqueueItem);
}

static void SyncMenuWithAccelerators(HWND hwndDlg, HMENU menu)
{
	HACCEL szAccel[24];
	INT c = WASABI_API_APP->app_getAccelerators(hwndDlg, szAccel, sizeof(szAccel)/sizeof(szAccel[0]), FALSE);
	AppendMenuShortcuts(menu, szAccel, c, MSF_REPLACE);
}

static int IPC_LIBRARY_SENDTOMENU=0;
static librarySendToMenuStruct s={0};

static void DownloadList_RightClick(HWND hwndDlg, HWND listHwnd, int xPos, int yPos)
{
	if (xPos == -1 || yPos == -1) // xPos and yPos are -1 if the user invoked a shift-f10 popup menu
	{
		RECT itemRect;
		int selected = downloadList.GetNextSelected();
		if (selected != -1) // if something is selected we'll drop the menu from there
		{
			downloadList.GetItemRect(selected, &itemRect);
			ClientToScreen(listHwnd, (POINT *)&itemRect);

		}
		else // otherwise we'll drop it from the top-left corner of the listview
		{
			GetWindowRect(listHwnd, &itemRect);
		}
		xPos = itemRect.left;
		yPos = itemRect.top;
	}

	LVHITTESTINFO hitTest;
	hitTest.pt.x = xPos;
	hitTest.pt.y = yPos;
	ScreenToClient(listHwnd, &hitTest.pt);
	hitTest.flags = LVHT_ONITEM;
	hitTest.iSubItem = -1;
	hitTest.flags = -1;
	int index = ListView_HitTest(listHwnd, &hitTest);

	HMENU baseMenu = WASABI_API_LOADMENU(IDR_MENU1);
	if (NULL == baseMenu) return;

	HMENU menu = GetSubMenu(baseMenu, 2);
	if (NULL != menu)
	{
		UINT enableExtras = MF_BYCOMMAND | MF_ENABLED;
		if (index == -1)
			enableExtras |= (MF_GRAYED | MF_DISABLED);
		
		EnableMenuItem(menu, IDC_PLAY, enableExtras);
		EnableMenuItem(menu, IDC_ENQUEUE, enableExtras);
		EnableMenuItem(menu, IDC_REMOVE, enableExtras);
		EnableMenuItem(menu, IDC_DELETE, enableExtras);
		EnableMenuItem(menu, IDC_INFOBOX, enableExtras);
		EnableMenuItem(menu, ID_DOWNLOADS_EXPLORERITEMFOLDER, enableExtras);
		EnableMenuItem(menu, 2, MF_BYPOSITION|(index == -1?(MF_GRAYED | MF_DISABLED):MF_ENABLED));

		{ // send-to menu shit...
			ZeroMemory(&s,sizeof(s));
			IPC_LIBRARY_SENDTOMENU=SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&"LibrarySendToMenu",IPC_REGISTER_WINAMP_IPCMESSAGE);
			if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)0,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
			{
				s.mode=1;
				s.hwnd=hwndDlg;
				s.data_type=ML_TYPE_FILENAMESW;
				s.build_hMenu=GetSubMenu(menu,2);
			}
		}

		bool swapPlayEnqueue=false;
		if (ML_ENQDEF_VAL())
		{
			SwapPlayEnqueueInMenu(menu);
			swapPlayEnqueue=true;
		}

		SyncMenuWithAccelerators(hwndDlg, menu);
		if (swapPlayEnqueue)
			SwapPlayEnqueueInMenu(menu);

		int r = Menu_TrackPopup(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTBUTTON, xPos, yPos, hwndDlg, NULL);
		if(!SendMessage(hwndDlg,WM_COMMAND,r,0))
		{
			s.menu_id = r; // more send to menu shit...
			if (s.mode == 2 && SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
			{
				s.mode=3;
				wchar_t* path=getSelectedList();
				if(path)
				{
					s.data=path;
					SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU);
					free(path);
				}
			}
		}

		if (s.mode)
		{ // yet more send to menu shit...
			s.mode=4;
			SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU); // cleanup
		}
	}

	DestroyMenu(baseMenu);
}

static void Downloads_ContextMenu(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	HWND sourceWindow = (HWND) wParam;
	if (sourceWindow == downloadList.getwnd())
		DownloadList_RightClick(hwndDlg, sourceWindow, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
}

static BOOL WINAPI DownloadDialog_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITMENUPOPUP: // yet yet more send to menu shit...
		if (wParam && (HMENU)wParam == s.build_hMenu && s.mode==1)
		{
			if (SendMessage(plugin.hwndWinampParent,WM_WA_IPC,(WPARAM)&s,IPC_LIBRARY_SENDTOMENU)==0xffffffff)
				s.mode=2;
		}
			break;

		case WM_CONTEXTMENU:
			Downloads_ContextMenu(hwndDlg, wParam, lParam);
			return TRUE;

		case WM_NOTIFYFORMAT:
			return NFR_UNICODE;

		case WM_INITDIALOG:
			Downloads_Init(hwndDlg);
			break;

		case WM_NOTIFY:
		{
			LPNMHDR l = (LPNMHDR)lParam;
			if (l->idFrom == IDC_DOWNLOADLIST)
				return DownloadList_Notify(l,hwndDlg);
		}
			break;

		case WM_DESTROY:
			Downloads_Destroy(hwndDlg);
			return 0;

		case WM_SIZE:
			Downloads_Size(hwndDlg, wParam);
			break;

		case WM_DISPLAYCHANGE:
			Downloads_DisplayChange(hwndDlg);
			return 0;

		case WM_TIMER:
			Downloads_Timer(hwndDlg, wParam);
			break;

		case WM_MOUSEMOVE:
			if (we_are_drag_and_dropping && GetCapture() == hwndDlg)
			{
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				ClientToScreen(hwndDlg, &p);
				mlDropItemStruct m;
				ZeroMemory(&m, sizeof(mlDropItemStruct));
				m.type = ML_TYPE_FILENAMESW;
				m.p = p;
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDRAG);
			}
			break;

		case WM_LBUTTONUP:
			if (we_are_drag_and_dropping && GetCapture() == hwndDlg)
			{
				we_are_drag_and_dropping = 0;
				ReleaseCapture();
				POINT p = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
				ClientToScreen(hwndDlg, &p);
				mlDropItemStruct m = {0};
				m.type = ML_TYPE_FILENAMESW;
				m.p = p;
				m.flags = ML_HANDLEDRAG_FLAG_NOCURSOR;
				SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDRAG);
				if (m.result > 0)
				{
					m.flags = 0;
					m.result = 0;
					wchar_t* path = getSelectedList();
					if(path)
					{
						m.data = path;
						SendMessage(plugin.hwndLibraryParent,WM_ML_IPC,(WPARAM)&m,ML_IPC_HANDLEDROP);
						free(path);
					}
				}
			}
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_PLAY:
				case IDC_ENQUEUE:
				{
					bool action = (HIWORD(wParam) == 1)?ML_ENQDEF_VAL():(LOWORD(wParam)==IDC_ENQUEUE);
					Downloads_Play(action);
				}
					break;
				case IDC_REMOVE:
					Downloads_Remove();
					break;
				case IDC_DELETE:
					Downloads_Delete(hwndDlg);
					break;
				case IDC_CLEANUP:
					Downloads_CleanUp(hwndDlg);
					break;
				case IDC_INFOBOX:
					Downloads_InfoBox(hwndDlg);
					break;
				case IDC_SELECTALL:
					Downloads_SelectAll();
					break;
				case ID_DOWNLOADS_EXPLORERITEMFOLDER:
					exploreItemFolder(hwndDlg);
					break;
				default:
					return 0;
			}
			return 1;
	}
	return 0;
}

HWND CALLBACK DownloadDialog_Create(HWND hParent, OmService *service)
{
	return WASABI_API_CREATEDIALOGPARAMW(IDD_DOWNLOADS, hParent, DownloadDialog_DlgProc, (LPARAM)service);
}