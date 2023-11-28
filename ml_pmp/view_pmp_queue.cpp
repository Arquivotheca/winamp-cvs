/* almost the same as view_pmp_devices, but only one device*/
#include "main.h"
#include <windows.h> 
#include <windowsx.h>
#include <stdio.h>
#include <shlobj.h>
#include "../gen_ml/ml.h"
#include "../gen_ml/itemlist.h"
#include "../nu/listview.h"
#include "../gen_ml/childwnd.h"
#include "../winamp/wa_ipc.h"
#include "../winamp/wa_dlg.h"
#include "resource1.h"
#include "SkinnedListView.h"
#include "DeviceView.h"
#include "api.h"
#include "./graphics.h"
#include <strsafe.h>

#define BUTTON_MIN_HEIGHT_PX		18	
#define BUTTON_MIN_WIDTH_PX			72

static DeviceView *device;
static SkinnedListView * listTransfers=NULL;
ValueMap<DeviceView *, bool> device_update_map;

typedef struct TransferView
{
	HFONT font;
	SIZE unitSize;
	HRGN updateRegion;
	POINT updateOffset;
} TransferView;

#define TRANSFERVIEW(_hwnd) ((TransferView*)GetViewData(_hwnd))
#define TRANSFERVIEW_RET_VOID(_self, _hwnd) { (_self) = TRANSFERVIEW((_hwnd)); if (NULL == (_self)) return; }
#define TRANSFERVIEW_RET_VAL(_self, _hwnd, _error) { (_self) = TRANSFERVIEW((_hwnd)); if (NULL == (_self)) return (_error); }

#define TRANSFERVIEW_DLU_TO_HORZ_PX(_self, _dlu) MulDiv((_dlu), (_self)->unitSize.cx, 4)
#define TRANSFERVIEW_DLU_TO_VERT_PX(_self, _dlu) MulDiv((_dlu), (_self)->unitSize.cy, 8)

void handleContextMenuResult(int r, C_ItemList * items=NULL, DeviceView * dev=NULL);
int showContextMenu(int context,HWND hwndDlg, Device * dev, POINT pt);

class TransferItemShadowShort
{
public:
	CopyInst * c;
	wchar_t * status, * type, * track, * sourceDevice, * destDevice, * lastChanged, * sourceFile;
	bool changed;
	TransferItemShadowShort(CopyInst * c)
	{
		changed = false;
		this->c = c;
		status = _wcsdup(c->statusCaption);
		type = _wcsdup(c->typeCaption);
		track = _wcsdup(c->trackCaption);
		lastChanged = _wcsdup(c->lastChanged);
		sourceDevice = _wcsdup(c->sourceDevice);
		destDevice = _wcsdup(c->destDevice);
		sourceFile = _wcsdup(c->sourceFile);
	}
	~TransferItemShadowShort()
	{
		if (status) free(status);
		if (type) free(type);
		if (track) free(track);
		if (lastChanged) free(lastChanged);
		if (sourceDevice) free(sourceDevice);
		if (destDevice) free(destDevice);
		if (sourceFile) free(sourceFile);
	}
	bool Equals(TransferItemShadowShort * a)
	{
		if (!a) return false;
		return (c == a->c) && !wcscmp(track,a->track) && !wcscmp(status,a->status) && !wcscmp(type,a->type);
	}
};

static C_ItemList *getTransferListShadow()
{
	C_ItemList * list = new C_ItemList;
	LinkedQueue * txQueue = (device->isCloudDevice ? &cloudTransferQueue : &device->transferQueue);
	LinkedQueue * finishedTX = (device->isCloudDevice ? &cloudFinishedTransfers : &device->finishedTransfers);

	txQueue->lock();
	int l = txQueue->GetSize();

	for (int j=0; j<l; j++)
		list->Add(new TransferItemShadowShort((CopyInst*)txQueue->Get(j)));
	txQueue->unlock();

	finishedTX->lock();
	l = finishedTX->GetSize();

	for (int j=0; j<l; j++)
		list->Add(new TransferItemShadowShort((CopyInst*)finishedTX->Get(j)));
	finishedTX->unlock();

	return list;
}

class TransferContents : public ListContents
{
public:
	TransferContents()
	{
		oldSize = 0;
		listShadow = 0;
		InitializeCriticalSection(&cs);
	}

	void Init()
	{
		lock();

		if (!listShadow)
			listShadow = getTransferListShadow();

		unlock();
	}

	virtual ~TransferContents()
	{
		DeleteCriticalSection(&cs); 
		delete listShadow;
	}

	virtual int GetNumColumns()
	{
		return (device->isCloudDevice ? 6 : 3);
	}

	virtual int GetNumRows()
	{
		return (listShadow ? listShadow->GetSize() : 0);
	}

	virtual wchar_t * GetColumnTitle(int num)
	{
		switch (num + device->isCloudDevice)
		{
			case 0: return WASABI_API_LNGSTRINGW(IDS_TYPE);
			case 1: return WASABI_API_LNGSTRINGW(IDS_TRACK);
			case 2: return WASABI_API_LNGSTRINGW(IDS_STATUS);
			case 3: return WASABI_API_LNGSTRINGW(IDS_LAST_CHANGED);
			case 4: return WASABI_API_LNGSTRINGW(IDS_SOURCE);
			case 5: return WASABI_API_LNGSTRINGW(IDS_DESTINATION);
			case 6: return WASABI_API_LNGSTRINGW(IDS_SOURCE_FILE);
		}
		return L"";
	}

	virtual int GetColumnWidth(int num)
	{
		switch (num)
		{
			case 0: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col0_width" : L"cloud_col0_width"), 300);
			case 1: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col1_width" : L"cloud_col1_width"), 150);
			case 2: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col2_width" : L"cloud_col2_width"), 100);
			case 3: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col3_width" : L"cloud_col3_width"), 100);
			case 4: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col4_width" : L"cloud_col4_width"), 100);
			case 5: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col5_width" : L"cloud_col5_width"), 100);
			case 6: return global_config->ReadInt((!device->isCloudDevice ? L"transfers_col6_width" : L"cloud_col6_width"), 300);
			default: return 0;
		}
	}

	virtual void ColumnResize(int col, int newWidth)
	{
		if (NULL != global_config &&
			col >= 0 && 
			col < GetNumColumns())
		{
			wchar_t buffer[64];	

			if (FAILED(StringCchPrintf(buffer, ARRAYSIZE(buffer), (!device->isCloudDevice ? L"transfers_col%d_width" : L"cloud_col%d_width"), col)))
				return;

			global_config->WriteInt(buffer, newWidth);
		}
	}

	void lock()
	{
		EnterCriticalSection(&cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&cs);
	}

	virtual void GetCellText(int row, int col, wchar_t * buf, int buflen)
	{
		if (NULL == buf)
			return;

		buf[0] = L'\0';

		lock();

		if (row < listShadow->GetSize())
		{
			TransferItemShadowShort * t = (TransferItemShadowShort *)listShadow->Get(row);
			if (NULL != t)
			{
				switch (col + device->isCloudDevice)
				{
					case 0: StringCchCopy(buf, buflen, t->type); break;
					case 1: StringCchCopy(buf, buflen, t->track); break;
					case 2: StringCchCopy(buf, buflen, t->status); break;
					case 3: StringCchCopy(buf, buflen, t->lastChanged); break;
					case 4: StringCchCopy(buf, buflen, t->sourceDevice); break;
					case 5: StringCchCopy(buf, buflen, t->destDevice); break;
					case 6: StringCchCopy(buf, buflen, t->sourceFile); break;
				}
			}
		}
		unlock();
	}

	void PushPopItem(CopyInst *c)
	{
		lock();
		int size = listShadow->GetSize();
		for (int i=0; i<size; i++)
		{
			TransferItemShadowShort * t = (TransferItemShadowShort *)listShadow->Get(i);
			if (c == t->c)
			{
				t->changed=true;
				listShadow->Del(i);
				listShadow->Add(t);
				if (listTransfers)
				{
					HWND hwnd = listTransfers->listview.getwnd();
					PostMessage(hwnd, LVM_REDRAWITEMS, i, size);
				}
				unlock();
				return;
			}
		}
		unlock();
	}

	void ItemUpdate(CopyInst * c)
	{
		lock();
		int size = listShadow->GetSize();
		for (int i=0; i<size; i++)
		{
			TransferItemShadowShort * t = (TransferItemShadowShort *)listShadow->Get(i);
			if (c == t->c)
			{
				TransferItemShadowShort * n = new TransferItemShadowShort(c);
				n->changed=true;
				listShadow->Set(i,n);
				delete t;
				if (listTransfers)
				{
					HWND hwnd = listTransfers->listview.getwnd();
					PostMessage(hwnd,LVM_REDRAWITEMS,i,i);
				}
				unlock();
				return;
			}
		}
		unlock();
	}

	void FullUpdate()
	{
		C_ItemList * newListShadow = getTransferListShadow();
		int newSize = newListShadow->GetSize();
		lock();
		oldSize = listShadow->GetSize();
		for (int i=0; i<newSize; i++)
		{
			TransferItemShadowShort * newt = (TransferItemShadowShort *)newListShadow->Get(i);
			TransferItemShadowShort * oldt = i<oldSize?(TransferItemShadowShort *)listShadow->Get(i):NULL;
			newt->changed = !newt->Equals(oldt);
		}

		C_ItemList * oldListShadow = listShadow;
		listShadow = newListShadow;
		for (int i=0; i<oldListShadow->GetSize(); i++) delete(TransferItemShadowShort *)oldListShadow->Get(i);
		delete oldListShadow;
		if (listTransfers)
		{
			HWND hwnd = listTransfers->listview.getwnd();
			if (newSize != oldSize) PostMessage(hwnd,LVM_SETITEMCOUNT,newSize, 0);
			for (int i=0; i<newSize; i++)
			{
				TransferItemShadowShort * t = (TransferItemShadowShort *)listShadow->Get(i);
				if (t->changed) PostMessage(hwnd,LVM_REDRAWITEMS,i,i);
			}
		}
		unlock();
	}

	virtual songid_t GetTrack(int pos) { return 0; }

private:
	CRITICAL_SECTION cs;
	C_ItemList * listShadow;
	int oldSize;
};

static TransferContents transferListContents;

static void updateStatus(HWND hwnd)
{	
	HWND statusWindow;
	int size;
		
	statusWindow = GetDlgItem(hwnd, IDC_STATUS);
	if (NULL == statusWindow)
		return;

	LinkedQueue * txQueue = (device->isCloudDevice ? &cloudTransferQueue : &device->transferQueue);
	LinkedQueue * finishedTX = (device->isCloudDevice ? &cloudFinishedTransfers : &device->finishedTransfers);
	int txProgress = (device->isCloudDevice ? cloudTransferProgress : device->currentTransferProgress);
	size = txQueue->GetSize();
	if (size > 0)
	{
		wchar_t buffer[256], format[256];
		int pcnum, time, pc, total;

		pcnum = (size * 100) - txProgress;
		total = size * 100;
		total += 100 * finishedTX->GetSize();
	
		time = (int)(device->transferRate * (((double)pcnum)/100.0));
		pc = ((total-pcnum)*100)/total;

		WASABI_API_LNGSTRINGW_BUF((time > 0 ? IDS_TRANFERS_PERCENT_REMAINING : IDS_TRANFERS_PERCENT_REMAINING_NOT_TIME), format, ARRAYSIZE(format));
		if (SUCCEEDED(StringCchPrintf(buffer, ARRAYSIZE(buffer), format, size, pc, time/60, time%60)))
		{
			if (0 == SendMessage(statusWindow, WM_GETTEXT, (WPARAM)ARRAYSIZE(format), (LPARAM)format) ||
				CSTR_EQUAL != CompareString(LOCALE_USER_DEFAULT, 0, format, -1, buffer, -1))
			{		
				SendMessage(statusWindow, WM_SETTEXT, 0, (LPARAM)buffer);
			}
		}
	}
	else 
	{
		int length;
		length = (int)SendMessage(statusWindow, WM_GETTEXTLENGTH, 0, 0L);
		if (0 != length)
			SendMessage(statusWindow, WM_SETTEXT, 0, 0L);
	}
}

void TransfersListUpdateItem(CopyInst * item, DeviceView *view)
{
	if (view == device)
		transferListContents.ItemUpdate(item);
}

void TransfersListPushPopItem(CopyInst * item, DeviceView *view)
{
	if (view == device)
		transferListContents.PushPopItem(item);
}

static bool AddSelectedItems(C_ItemList *items, W_ListView *listview, LinkedQueue *transfer_queue, int &row, DeviceView *&dev)
{
	transfer_queue->lock();
	int l = transfer_queue->GetSize();
	for (int j=0; j<l; j++)
	{
		if (listview->GetSelected(row++))
		{
			CopyInst * c = (CopyInst *)transfer_queue->Get(j);
			if (c->songid)
			{
				if (!dev && c->dev) 
					dev = c->dev;
				if (dev)
				{
					if (c->dev != dev)
					{
						transfer_queue->unlock();
						return false;
					}
					else
						items->Add((void*)c->songid);
				}
			}
		}
	}
	transfer_queue->unlock();
	return true;
}

static void RemoveSelectedItems(DeviceView *device, W_ListView *listview, LinkedQueue *transfer_queue, int &row, bool finished_queue)
{
	transfer_queue->lock();
	int j = transfer_queue->GetSize();
	while (j-- > 0)
	{
		if (listview->GetSelected(--row))
		{
			if (j == 0 && !finished_queue)
			{
				CopyInst * d = (CopyInst *)transfer_queue->Get(j);
				if (d && (d->status == STATUS_WAITING || d->status == STATUS_CANCELLED) && device->transferContext.IsPaused())
				{
					transfer_queue->Del(j);
					d->Cancelled();
					delete d;
				} // otherwise don't bother
			}
			else
			{
				CopyInst * d = (CopyInst*)transfer_queue->Del(j);
				if (d)
				{
					if ((d->status == STATUS_WAITING || d->status == STATUS_CANCELLED) && !finished_queue)
						d->Cancelled();
					delete d;
				}
			}
		}
	}
	transfer_queue->unlock();
}

static void CancelSelectedItems(DeviceView *device, W_ListView *listview, LinkedQueue *transfer_queue, int &row)
{
	transfer_queue->lock();
	int j = transfer_queue->GetSize();
	int sel = listview->GetSelectedCount();

	for (int i = 0, q = 0; i <= j; i++)
	{
		if (listview->GetSelected(i) || !sel)
		{
			CopyInst * d = (CopyInst *)transfer_queue->Get(q);
			if (d && d->status == STATUS_WAITING)
			{
				transfer_queue->Del(q);
				d->Cancelled();
				delete d;
			}
			else if (d && d->status == STATUS_TRANSFERRING)
			{
				d->Cancelled();
			}
			else
			{
				q++;
			}
		}
	}

	transfer_queue->unlock();
}

static void RetrySelectedItems(DeviceView *device, W_ListView *listview, LinkedQueue *transfer_queue, LinkedQueue *finished_transfer_queue, int &row)
{
	transfer_queue->lock();
	finished_transfer_queue->lock();
	int j = finished_transfer_queue->GetSize(), jj = j;
	int sel = listview->GetSelectedCount();

	LinkedQueue retryTransferQueue;
	int i = 0;
	for (int /*i = 0,*/ q = 0; i <= j; i++)
	{
		if (listview->GetSelected(i) || !sel)
		{
			CopyInst * d = (CopyInst *)finished_transfer_queue->Get(q);
			if (d && (d->status == STATUS_DONE || d->status == STATUS_CANCELLED || d->status == STATUS_ERROR))
			{
				// due to STATUS_DONE being applied in most cases, have to look at the
				// status message and use as the basis on how to proceed with the item
				if (lstrcmpi(d->statusCaption, WASABI_API_LNGSTRINGW(IDS_UPLOADED)))
				{
					retryTransferQueue.Offer(d);
					finished_transfer_queue->Del(q);
				}
				else
				{
					q++;
				}
			}
			else
			{
				q++;
			}
		}
	}

	i = 0;
	for (; i <= retryTransferQueue.GetSize(); i++)
	{
		CopyInst * d = (CopyInst *)retryTransferQueue.Get(i);
		if (d)
		{
			WASABI_API_LNGSTRINGW_BUF(IDS_WAITING, d->statusCaption, sizeof(d->statusCaption)/sizeof(wchar_t));

			SYSTEMTIME system_time;
			GetLocalTime(&system_time);
			GetTimeFormat(LOCALE_INVARIANT, NULL, &system_time, NULL, d->lastChanged, sizeof(d->lastChanged)/sizeof(wchar_t));

			d->dev->AddTrackToTransferQueue(d);
		}
	}

	transfer_queue->unlock();
	finished_transfer_queue->unlock();
}

static BOOL TransferView_UpdateLayout(HWND hwnd, BOOL redraw)
{
	TransferView *self;
	HWND elementWindow;
	RECT clientRect, elementRect;
	HDWP hdwp;
	unsigned int positionFlags, elementFlags;
	long buttonsTop, elementRight;

	const int controlList[] = 
	{
		IDC_BUTTON_PAUSETRANSFERS,
		IDC_BUTTONCANCELSELECTED,
		IDC_BUTTON_CLEARFINISHED,
		IDC_BUTTON_REMOVESELECTED,
		IDC_BUTTON_RETRYSELECTED,
		IDC_STATUS,
		IDC_LIST_TRANSFERS,
	};

	TRANSFERVIEW_RET_VAL(self, hwnd, FALSE);

	if (FALSE == GetClientRect(hwnd, &clientRect))
		return FALSE;

	hdwp = BeginDeferWindowPos(ARRAYSIZE(controlList));
	if (NULL == hdwp)
		return FALSE;

	positionFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	if (FALSE == redraw)
		positionFlags |= SWP_NOREDRAW;

	buttonsTop = clientRect.bottom;
	elementRight = clientRect.left;

	for(size_t index = 0; index < ARRAYSIZE(controlList); index++)
	{
		elementWindow = GetDlgItem(hwnd, controlList[index]);
		if (NULL == elementWindow)
			continue;

		elementFlags = positionFlags;

		switch(controlList[index])
		{
			case IDC_BUTTONCANCELSELECTED:
			case IDC_BUTTON_PAUSETRANSFERS:
			case IDC_BUTTON_CLEARFINISHED:
			case IDC_BUTTON_REMOVESELECTED:
			case IDC_BUTTON_RETRYSELECTED:
			{					
				SIZE buttonSize;
				GetWindowRect(elementWindow, &elementRect);
				buttonSize.cx = RECTWIDTH(elementRect);
				buttonSize.cy = RECTHEIGHT(elementRect);
				
				if (buttonSize.cx < BUTTON_MIN_WIDTH_PX)
					buttonSize.cx = BUTTON_MIN_WIDTH_PX;

				if (buttonSize.cy < BUTTON_MIN_HEIGHT_PX)
					buttonSize.cy = BUTTON_MIN_HEIGHT_PX;
				
				elementRect.left = elementRight;
				if (elementRect.left != clientRect.left)
					elementRect.left += 4;

				elementRect.top = clientRect.bottom - buttonSize.cy;
				elementRect.right =	elementRect.left + buttonSize.cx;
				elementRect.bottom = elementRect.top + buttonSize.cy;
				
				elementRight = elementRect.right;

				if (buttonsTop > elementRect.top)
					buttonsTop = elementRect.top;
			}
			break;

			case IDC_STATUS:
			{
				long statusHeight;
				GetWindowRect(elementWindow, &elementRect);
				statusHeight = RECTHEIGHT(elementRect);

				elementRect.left = elementRight;
				if (elementRect.left != clientRect.left)
					elementRect.left += 4;

				elementRect.right = clientRect.right;
				if (elementRect.right < elementRect.left)
					elementRect.right = elementRect.left;

				elementRect.top = buttonsTop + ((clientRect.bottom - buttonsTop) - statusHeight + 1)/2;
				elementRect.bottom = elementRect.top + statusHeight;

				elementRight = elementRect.right;
			}
			break;

			case IDC_LIST_TRANSFERS:
				SetRect(&elementRect, clientRect.left, clientRect.top, clientRect.right - 2, buttonsTop);
				if (elementRect.bottom != clientRect.bottom)
				{
					elementRect.bottom -= 3;
					if (elementRect.bottom < elementRect.top)
						elementRect.bottom = elementRect.top;
				}
			break;

			default:
			continue;
		}

		hdwp = DeferWindowPos(hdwp, elementWindow, NULL, 
							elementRect.left, elementRect.top, 
							elementRect.right - elementRect.left, 
							elementRect.bottom - elementRect.top, 
							elementFlags);

		if (NULL == hdwp)
			break;
	}

	return (NULL != hdwp) ? 
			EndDeferWindowPos(hdwp) : FALSE;
}

static void
TransferView_UpdateFont(HWND hwnd, BOOL redraw)
{
	TransferView *self;
	HWND elementWindow;
	HDWP hdwp;

	const int buttonList[] = 
	{
		IDC_BUTTON_PAUSETRANSFERS,
		IDC_BUTTONCANCELSELECTED,
		IDC_BUTTON_CLEARFINISHED,
		IDC_BUTTON_REMOVESELECTED,
		IDC_BUTTON_RETRYSELECTED
	};

	TRANSFERVIEW_RET_VOID(self, hwnd);

	if (FALSE == Graphics_GetWindowBaseUnits(hwnd, &self->unitSize.cx, &self->unitSize.cy))
	{
		self->unitSize.cx = 6;
		self->unitSize.cy = 13;
	}	

	elementWindow = GetDlgItem(hwnd, IDC_LIST_TRANSFERS);
	if (NULL != elementWindow)
	{
		elementWindow = (HWND)SendMessage(elementWindow, LVM_GETHEADER, 0, 0L);
		if (NULL != elementWindow)
			MLSkinnedHeader_SetHeight(elementWindow, -1);
	}

	hdwp = BeginDeferWindowPos(ARRAYSIZE(buttonList) + 1);
	if (NULL != hdwp)
	{
		LRESULT idealSize;
		SIZE buttonSize;

		elementWindow = GetDlgItem(hwnd, IDC_STATUS);
		if (NULL != elementWindow)
		{
			hdwp = DeferWindowPos(hdwp, elementWindow, NULL, 0, 0, 100, self->unitSize.cy,
							SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
		}

		for(size_t index = 0; index < ARRAYSIZE(buttonList) && NULL != hdwp; index++)
		{
			elementWindow = GetDlgItem(hwnd, buttonList[index]);
			if (NULL == elementWindow)
				continue;

			if (IDC_BUTTON_PAUSETRANSFERS == buttonList[index])
			{
				wchar_t buffer[128];

				WASABI_API_LNGSTRINGW_BUF(IDS_RESUME, buffer, ARRAYSIZE(buffer));
				idealSize = MLSkinnedButton_GetIdealSize(elementWindow, buffer);
				buttonSize.cx = LOWORD(idealSize);
				buttonSize.cy = HIWORD(idealSize);

				WASABI_API_LNGSTRINGW_BUF(IDS_PAUSE, buffer, ARRAYSIZE(buffer));
				idealSize = MLSkinnedButton_GetIdealSize(elementWindow, buffer);

				if (buttonSize.cx < LOWORD(idealSize))
					buttonSize.cx = LOWORD(idealSize);

				if (buttonSize.cy < HIWORD(idealSize))
					buttonSize.cy = HIWORD(idealSize);
			}
			else
			{
				idealSize = MLSkinnedButton_GetIdealSize(elementWindow, NULL);
				buttonSize.cx = LOWORD(idealSize);
				buttonSize.cy = HIWORD(idealSize);
			}

			hdwp = DeferWindowPos(hdwp, elementWindow, NULL, 0, 0, buttonSize.cx, buttonSize.cy,
							SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOREDRAW);
		}

		if (NULL != hdwp)
			EndDeferWindowPos(hdwp);
	}

	TransferView_UpdateLayout(hwnd, redraw);
}

static int 
TransferView_OnInitDialog(HWND hwnd, HWND focusWindow, LPARAM param)
{
	TransferView *self;
	HWND controlWindow;

	const int skinList[] = 
	{
		IDC_BUTTON_PAUSETRANSFERS,
		IDC_BUTTONCANCELSELECTED,
		IDC_BUTTON_CLEARFINISHED,
		IDC_BUTTON_REMOVESELECTED,
		IDC_BUTTON_RETRYSELECTED,
		IDC_STATUS,
	};

	self = (TransferView*)malloc(sizeof(TransferView));
	if (NULL != self) 
	{
		ZeroMemory(self, sizeof(TransferView));
		if (FALSE == SetViewData(hwnd, self))
		{
			free(self);
			self = NULL;
		}
	}

	if (NULL == self)
	{
		DestroyWindow(hwnd);
		return 0;
	}

	device = (DeviceView *)param;

	transferListContents.Init();
	transferListContents.lock();

	transferListContents.FullUpdate();

	listTransfers = new SkinnedListView(&transferListContents,IDC_LIST_TRANSFERS,plugin.hwndLibraryParent, hwnd);
	listTransfers->DialogProc(hwnd,WM_INITDIALOG, (WPARAM)focusWindow, param);

	transferListContents.unlock();
	SetDlgItemText(hwnd,IDC_BUTTON_PAUSETRANSFERS,
				   WASABI_API_LNGSTRINGW((device->transferContext.IsPaused()?IDS_RESUME:IDS_PAUSE)));

	MLSkinWindow2(plugin.hwndLibraryParent, hwnd, SKINNEDWND_TYPE_DIALOG, 
				SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);

	for(size_t index = 0; index < ARRAYSIZE(skinList); index++)
	{
		controlWindow = GetDlgItem(hwnd, skinList[index]);
		if (NULL != controlWindow)
		{
			MLSkinWindow2(plugin.hwndLibraryParent, controlWindow, SKINNEDWND_TYPE_AUTO, 
					SWS_USESKINFONT | SWS_USESKINCOLORS | SWS_USESKINCURSORS);
		}
	}

	TransferView_UpdateFont(hwnd, FALSE);

	SetTimer(hwnd,1,500,NULL);
	updateStatus(hwnd);

	return 0;
}

static void
TransferView_OnDestroy(HWND hwnd)
{
	TransferView *self;

	self = (TransferView *)RemoveViewData(hwnd);
	if (NULL == self)
		return;

	KillTimer(hwnd, 1);
	device = 0;

	SkinnedListView * lt = listTransfers;
	if (NULL != lt)
	{
		transferListContents.lock();
		listTransfers=NULL;
		transferListContents.unlock();
		delete lt;
	}

	free(self);
}

static void
TransferView_OnWindowPosChanged(HWND hwnd, WINDOWPOS *windowPos)
{
	if (NULL == windowPos)
		return;

	if (SWP_NOSIZE != ((SWP_NOSIZE | SWP_FRAMECHANGED) & windowPos->flags)) 
	{
		TransferView_UpdateLayout(hwnd, 0 == (SWP_NOREDRAW & windowPos->flags));   
	}
}

static void
TransferView_OnDisplayChanged(HWND hwnd, INT bpp, INT dpi_x, INT dpi_y)
{
	TransferView_UpdateFont(hwnd, FALSE);
}

static void
TransferView_OnSetFont(HWND hwnd, HFONT font, BOOL redraw)
{
	TransferView *self;
	TRANSFERVIEW_RET_VOID(self, hwnd);

	self->font = font;
}

static HFONT
TransferView_OnGetFont(HWND hwnd)
{
	TransferView *self;
	TRANSFERVIEW_RET_VAL(self, hwnd, NULL);

	return self->font;
}

static void 
TransferView_OnSetUpdateRegion(HWND  hwnd, HRGN updateRegion, POINTS regionOffset)
{
	TransferView *self;
	TRANSFERVIEW_RET_VOID(self, hwnd);

	self->updateRegion = updateRegion;
	self->updateOffset.x = regionOffset.x;
	self->updateOffset.y = regionOffset.y;
}

INT_PTR CALLBACK pmp_queue_dlgproc(HWND hwndDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
	if (NULL != listTransfers)
	{
		INT_PTR processed = listTransfers->DialogProc(hwndDlg,uMsg,wParam,lParam);
		if (0 != processed) 
			return processed;
	}

	switch (uMsg)
	{
		case WM_INITDIALOG:			return TransferView_OnInitDialog(hwndDlg, (HWND)wParam, lParam);
		case WM_DESTROY:			TransferView_OnDestroy(hwndDlg); break;
		case WM_WINDOWPOSCHANGED:	TransferView_OnWindowPosChanged(hwndDlg, (WINDOWPOS*)lParam); return TRUE;
		case WM_DISPLAYCHANGE:		TransferView_OnDisplayChanged(hwndDlg, (INT)wParam, LOWORD(lParam), HIWORD(lParam)); return TRUE;
		case WM_GETFONT:			DIALOG_RESULT(hwndDlg, TransferView_OnGetFont(hwndDlg));
		case WM_SETFONT:			TransferView_OnSetFont(hwndDlg, (HFONT)wParam, LOWORD(lParam)); return TRUE;
		case WM_TIMER:
			if (wParam == 1)
			{
				updateStatus(hwndDlg);
				/*if (device_update_map[device] == true)
				{
					device_update_map[device] = false;*/
					transferListContents.FullUpdate();
				/*}*/
			}
			break;
		case WM_NOTIFY:
			{
				LPNMHDR l=(LPNMHDR)lParam;
				if (l->idFrom==IDC_LIST_TRANSFERS)
				{
					switch (l->code)
					{
						case NM_RETURN: // enter!
						//case NM_RCLICK: // right click!
						case LVN_KEYDOWN:
						{
							C_ItemList items;
							int row = 0;

							LinkedQueue * txQueue = (device->isCloudDevice ? &cloudTransferQueue : &device->transferQueue);
							LinkedQueue * finishedTX = (device->isCloudDevice ? &cloudFinishedTransfers : &device->finishedTransfers);

							if (!AddSelectedItems(&items, &listTransfers->listview, txQueue, row, device))
								return 0;
							if (!AddSelectedItems(&items, &listTransfers->listview, finishedTX, row, device))
								return 0;

							if (items.GetSize())
							{
								/*if (l->code == NM_RCLICK)
								{
									LPNMITEMACTIVATE lva=(LPNMITEMACTIVATE)lParam;
									handleContextMenuResult(showContextMenu(7,l->hwndFrom,device->dev,lva->ptAction),&items,device);
								}
								else*/ if (l->code == NM_RETURN)
								{
									handleContextMenuResult((!GetAsyncKeyState(VK_SHIFT)?ID_TRACKSLIST_PLAYSELECTION:ID_TRACKSLIST_ENQUEUESELECTION),&items,device);
								}
								else if (l->code == LVN_KEYDOWN)
								{
									switch (((LPNMLVKEYDOWN)lParam)->wVKey)
									{
									case VK_DELETE:
										{
											if (!(GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_SHIFT)&0x8000))
											{
												handleContextMenuResult(ID_TRACKSLIST_DELETE,&items,device);
											}
										}
										break;
									case 0x45: //E
										if ((GetAsyncKeyState(VK_CONTROL)&0x8000) && !(GetAsyncKeyState(VK_SHIFT)&0x8000))
										{
											handleContextMenuResult(ID_TRACKSLIST_EDITSELECTEDITEMS,&items,device);
										}
										break;
									}
								}
							}
							break;
						}
					}
				}
			}
			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON_CLEARFINISHED:
				{
					LinkedQueue * finishedTX = (device->isCloudDevice ? &cloudFinishedTransfers : &device->finishedTransfers);
					finishedTX->lock();
					int j=finishedTX->GetSize();
					while (j-- > 0) delete(CopyInst*)finishedTX->Del(j);
					finishedTX->unlock();
					transferListContents.FullUpdate();
				}
				break;
				case IDC_BUTTON_REMOVESELECTED:
				{
					int row = transferListContents.GetNumRows();
					LinkedQueue * txQueue = (device->isCloudDevice ? &cloudTransferQueue : &device->transferQueue);
					LinkedQueue * finishedTX = (device->isCloudDevice ? &cloudFinishedTransfers : &device->finishedTransfers);
					RemoveSelectedItems(device, &listTransfers->listview, txQueue, row, false);
					RemoveSelectedItems(device, &listTransfers->listview, finishedTX, row, true);
					transferListContents.FullUpdate();
				}
				break;
				case IDC_BUTTON_PAUSETRANSFERS:
				{
					if (false != device->transferContext.IsPaused())
						device->transferContext.Resume();
					else
						device->transferContext.Pause();

					SetDlgItemText(hwndDlg,IDC_BUTTON_PAUSETRANSFERS,WASABI_API_LNGSTRINGW((device->transferContext.IsPaused()?IDS_RESUME:IDS_PAUSE)));
				}
				break;
				case IDC_BUTTONCANCELSELECTED:
				{
					int row = transferListContents.GetNumRows();
					CancelSelectedItems(device, &listTransfers->listview, &cloudTransferQueue, row);
					transferListContents.FullUpdate();
				}
				break;
				case IDC_BUTTON_RETRYSELECTED:
				{
					int row = transferListContents.GetNumRows();
					RetrySelectedItems(device, &listTransfers->listview, &cloudTransferQueue, &cloudFinishedTransfers, row);
					transferListContents.FullUpdate();
				}
				break;
			}
			break;

		// gen_ml flickerless drawing
		case WM_USER + 0x200: DIALOG_RESULT(hwndDlg, 1);
		case WM_USER + 0x201: TransferView_OnSetUpdateRegion(hwndDlg, (HRGN)lParam, MAKEPOINTS(wParam)); return TRUE;
	}

	return 0;
}