#include "main.h"
extern void AccessingGracenoteHack(int);
extern HWND subWnd;

static void WndProc_DropFiles(HWND hwndDlg, HDROP hdrop)
{
	wchar_t temp[1024];

	int y = DragQueryFileW(hdrop, 0xffffffff, temp, 1024);
	//    wsprintf(temp,"Add %d item%s to local media?",y,y==1?"":"s");
	if (y > 0) //&& MessageBox(hwndDlg,temp,"Winamp Library",MB_YESNO)==IDYES)
	{
		int x;
		wchar_t **paths = (wchar_t **)malloc(sizeof(wchar_t *) * y);
		int *guesses = (int *)malloc(sizeof(int) * y);
		int *metas = (int *)malloc(sizeof(int) * y);
		int *recs= (int *)malloc(sizeof(int) * y);
		if (paths && guesses && metas && recs)
		{
			size_t count=0;
			for (x = 0; x < y; x ++)
			{
				DragQueryFileW(hdrop, x, temp, 1024);
				int guess = -1, meta = -1, rec = 1;
				// do this for normal media drops
				PLCallBackW plCB;
				if (AGAVE_API_PLAYLISTMANAGER && PLAYLISTMANAGER_SUCCESS != AGAVE_API_PLAYLISTMANAGER->Load(temp, &plCB))
				{
					autoscan_add_directory(temp, &guess, &meta, &rec, 0);
					if (guess == -1) guess = g_config->ReadInt("guessmode", 0);
					if (meta == -1)	meta = g_config->ReadInt("usemetadata", 1);
					paths[count] = _wcsdup(temp);
					guesses[count]=guess;
					metas[count]=meta;
					recs[count]=rec;
					count++;
				}
			}
			DragFinish(hdrop);
			Scan_ScanFolders(hwndDlg, count, paths, guesses, metas, recs);
			if (m_curview_hwnd) SendMessage(m_curview_hwnd, WM_APP + 1, 0, 0); //update current view
		}
		else
		{
			free(paths);
			free(guesses);
			free(metas);
			free(recs);
		}
	}
	else DragFinish(hdrop);
}
// TODO: benski> a lot of things don't need to be part of gen_ml window - they could easily be done with a hidden window
LRESULT APIENTRY ml_newWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_USER+641:
		{
			AccessingGracenoteHack(wParam);
			break;
		}
	case WM_DROPFILES:
		WndProc_DropFiles(hwndDlg, (HDROP)wParam);
		break;
	case WM_ML_IPC:
		{
			INT_PTR ret = HandleIpcMessage((INT_PTR)lParam, (INT_PTR)wParam);
			if (ret != 0)
			{
				SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, ret);
				return ret; // supposed to return TRUE but thus is not working for me :(
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_DOSHITMENU_ADDNEWVIEW:
			addNewQuery(hwndDlg);
			return 0;
		case IDM_ADD_PLEDIT:
			add_pledit_to_library();
			return 0;
		case IDM_ADD_DIRS:
			add_to_library(hwndDlg);
			return 0;
		case IDM_REMOVE_UNUSED_FILES:
			Scan_RemoveFiles(hwndDlg);
			if (m_curview_hwnd) SendMessage(m_curview_hwnd, WM_APP + 1, 0, 0); //update current view
			return 0;
		case IDM_RESCANFOLDERSNOW:
			if (!g_bgscan_scanning) SendMessage(hwndDlg, WM_USER + 575, 0xffff00dd, 0);
			return 0;
		}
		break;
	case WM_USER + 575:        //sent by prefs to start scanning
		if (wParam == 0xffff00dd && !lParam)
		{
			if (!g_bgscan_scanning)
			{
				Scan_BackgroundScan();
			}
		}
		break;
	case WM_TIMER:
		{
			static int in_timer;
			if (in_timer) return 0;
			in_timer = 1;
			if (wParam == 200) // decide if it is time to scan yet
			{
				if (!g_bgscan_scanning)
				{
					if (g_bgrescan_force || (g_bgrescan_do && (time(NULL) - g_bgscan_last_rescan) > g_bgrescan_int*60))
					{
						// send to the prefs page so it'll show the status if it's open
						// (makes it easier to see if things are working with the rescan every x option)
						if (IsWindow(subWnd)) SendMessage(subWnd, WM_USER+101, 0, 0);
						Scan_BackgroundScan();
					}
				}
				in_timer = 0;
				return 0;
			}
			in_timer = 0;
		}
		break;
	}
	return CallWindowProc(ml_oldWndProc, hwndDlg, uMsg, wParam, lParam);
}