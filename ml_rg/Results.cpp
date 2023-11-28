#include "main.h"
#include "resource.h"
#include "../nu/listview.h"
#include "api.h"
#include <shlwapi.h>
#include <strsafe.h>

// this isn't nice but it localises the values for display as they're saved in "C" locale
enum { GAIN_MODE=0, PEAK_MODE };
wchar_t* LocaliseNumericText(wchar_t str[64], int mode){
static wchar_t tmp[64];
double value;
	tmp[0]=0;
	value = _wtof_l(str,WASABI_API_LNG->Get_C_NumericLocale());
	StringCchPrintfW(tmp,64,(mode==GAIN_MODE?L"%-+.2f dB":L"%-.9f"),value);
	return tmp;
}

static void AddQueueToListView(W_ListView *listView, RGWorkAlbum *queue)
{
	int i=listView->GetCount();
	for(RGWorkAlbum::RGWorkFiles::iterator itr = queue->files.begin(); itr!= queue->files.end(); itr++)
	{
		listView->InsertItem(i, PathFindFileNameW((*itr)->filename), (int)&*itr);
		listView->SetItemText(i, 1, LocaliseNumericText((*itr)->track_gain,GAIN_MODE));
		listView->SetItemText(i, 2, LocaliseNumericText((*itr)->track_peak,PEAK_MODE));
		listView->SetItemText(i, 3, LocaliseNumericText(queue->album_gain,GAIN_MODE));
		listView->SetItemText(i, 4, LocaliseNumericText(queue->album_peak,PEAK_MODE));
		i++;
	}
}

INT_PTR WINAPI ReplayGainDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
<<<<<<< Results.cpp
		W_ListView listView(GetDlgItem(hwndDlg, IDC_RGLIST));
		listView.setwnd(GetDlgItem(hwndDlg, IDC_RGLIST));

		listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_FILENAME), 200);
		listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_TRACK_GAIN), 65);
		listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_TRACK_PEAK), 80);
		listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_ALBUM_GAIN), 65);
		listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_ALBUM_PEAK), 80);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		RGWorkAlbum *queue = (RGWorkAlbum *)lParam;
=======
	case WM_INITDIALOG:
		{
			W_ListView listView(GetDlgItem(hwndDlg, IDC_RGLIST));
			listView.setwnd(GetDlgItem(hwndDlg, IDC_RGLIST));
>>>>>>> 1.3

<<<<<<< Results.cpp
		AddQueueToListView(&listView, queue);
		
	}
	break;
case WM_COMMAND:
	switch(LOWORD(wParam))
	{
	case IDOK:
		{
			RGWorkAlbum *queue = (RGWorkAlbum *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			WriteAlbum(*queue);
			EndDialog(hwndDlg, 0);
=======
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_FILENAME), 200);
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_TRACK_GAIN), 65);
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_TRACK_PEAK), 80);
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_ALBUM_GAIN), 65);
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_ALBUM_PEAK), 80);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			WorkQueue::RGWorkQueue *queue = (WorkQueue::RGWorkQueue *)lParam;

			AddQueueToListView(&listView, queue);

			POINT pt = {GetPrivateProfileInt("ml_rg", "res_x", -1, iniFile),
						GetPrivateProfileInt("ml_rg", "res_y", -1, iniFile)};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDOK:
				{
					WorkQueue::RGWorkQueue *queue = (WorkQueue::RGWorkQueue *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					WriteAlbum(*queue);
					EndDialog(hwndDlg, 0);
				}
			case IDCANCEL:
				{
					RECT rect = {0};
					GetWindowRect(hwndDlg, &rect);
					char buf[16] = {0};
					StringCchPrintfA(buf, 16, "%d", rect.left);
					WritePrivateProfileStringA("ml_rg", "res_x", buf, iniFile);
					StringCchPrintfA(buf, 16, "%d", rect.top);
					WritePrivateProfileStringA("ml_rg", "res_y", buf, iniFile);
				}
				break;
			case IDC_SAVETRACK:
				{
					WorkQueue::RGWorkQueue *queue = (WorkQueue::RGWorkQueue *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					WriteTracks(*queue);
					EndDialog(hwndDlg, 0);
				}
				break;
>>>>>>> 1.3
		}
<<<<<<< Results.cpp
		break;
	case IDC_SAVETRACK:
		{
			RGWorkAlbum *queue = (RGWorkAlbum *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
			WriteTracks(*queue);
			EndDialog(hwndDlg, 0);
		}
		break;
	case IDCANCEL:
		EndDialog(hwndDlg, 0);
		break;
=======
>>>>>>> 1.3
	}
	return 0;
}

INT_PTR WINAPI ReplayGainDialogProcAll(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			W_ListView listView(GetDlgItem(hwndDlg, IDC_RGLIST));
			listView.setwnd(GetDlgItem(hwndDlg, IDC_RGLIST));

			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_FILENAME), 200);
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_TRACK_GAIN), 65);
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_TRACK_PEAK), 80);
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_ALBUM_GAIN), 65);
			listView.AddCol(WASABI_API_LNGSTRINGW(IDS_COL_ALBUM_PEAK), 80);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

			WorkQueue *queue = (WorkQueue *)lParam;

			AddQueueToListView(&listView, &queue->unclassified);
			for (WorkQueue::AlbumMap::iterator mapItr=queue->albums.begin();mapItr!=queue->albums.end();mapItr++)
			{
				AddQueueToListView(&listView, &mapItr->second);
			}
			
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			{
				WorkQueue *queue = (WorkQueue  *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				WriteAlbum(queue->unclassified);
				for (WorkQueue::AlbumMap::iterator mapItr=queue->albums.begin();mapItr!=queue->albums.end();mapItr++)
			{
				WriteAlbum(mapItr->second);
			}
				EndDialog(hwndDlg, 0);
			}
			break;
		case IDC_SAVETRACK:
			{
				WorkQueue *queue = (WorkQueue *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				WriteTracks(queue->unclassified);
				for (WorkQueue::AlbumMap::iterator mapItr=queue->albums.begin();mapItr!=queue->albums.end();mapItr++)
			{
				WriteTracks(mapItr->second);
			}
				EndDialog(hwndDlg, 0);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			break;
		}
	}
	return 0;
}

void DoResults(RGWorkAlbum &queue)
{
	if (!queue.files.empty())
	WASABI_API_DIALOGBOXPARAM(IDD_RESULTS, GetDialogBoxParent(), ReplayGainDialogProc, (LPARAM)&queue);
}

void DoResults(WorkQueue &queue)
{
	WASABI_API_DIALOGBOXPARAM(IDD_RESULTS, GetDialogBoxParent(), ReplayGainDialogProcAll, (LPARAM)&queue);
}