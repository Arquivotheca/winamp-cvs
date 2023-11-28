#include "Main.h"
#include "ipc_pe.h"
#include "buildType.h"
#include "../gen_ml/ml_ipc.h"
#include "../gen_ml/ml.h"
#include "../gen_ml/menufucker.h"
#include "api.h"
#include "ExplorerFindFile.h"
#include "../nu/AutoWide.h"
#include "../nu/AutoWideFn.h"
#include "resource.h"

int pledit_disp_offs;
static int pledit_delta_carryover;

void plEditRefresh(void)
{
	if (config_pe_open) InvalidateRect(hPLWindow, NULL, FALSE);
}

void plEditSelect(int song)
{
	if (hPLWindow) SendMessage(hPLWindow, WM_USER, 666, song);
}

int pe_startuphack;

void pleditDlg(HWND hwnd, int init_state)
{
	if (!hPLWindow) return ;
	int toggle = Ipc_WindowToggle(IPC_CB_WND_PE, !config_pe_open);
	KillTimer(hPLWindow, 3);
	if (!toggle) return;

	if (!config_pe_open)
	{
		CheckMenuItem(main_menu, WINAMP_OPTIONS_PLEDIT, MF_CHECKED);
		if (pe_startuphack == 1)
		{
			if(!GetParent(hPLWindow))
				SetTimer(hPLWindow, 3, 10, NULL);
			config_pe_open = 1;
		}
		else
		{
			if(!init_state && !config_minimized) ShowWindow(hPLWindow, SW_SHOWNA);
			SendMessage(hPLWindow, WM_USER, 666, PlayList_getPosition());
			config_pe_open = 1;

			// works around a quirk with Bento docked to the right-hand edge
			// where the pledit is left docked on reverting to classic skin (dro)
			if(!GetParent(hPLWindow)) set_aot(1);
		}
	}
	else
	{
		if (GetForegroundWindow() == hPLWindow || IsChild(hPLWindow, GetForegroundWindow()))
		{
			SendMessage(hMainWindow, WM_COMMAND, WINAMP_NEXT_WINDOW, 0);
		}
		CheckMenuItem(main_menu, WINAMP_OPTIONS_PLEDIT, MF_UNCHECKED);
		ShowWindow(hPLWindow, SW_HIDE);
		config_pe_open = 0;
	}
	draw_eqplbut(config_eq_open, 0, config_pe_open, 0);
	return ;
}

int playlist_open(HWND hwnd)
{
	if (!(loadpls(hwnd, -1))) return 0;
	PlayList_setposition(0);
	if (config_shuffle) PlayList_randpos(-100000);
	PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
	{
		RECT r = {0, 0, config_pe_width, config_pe_height - 37};
		InvalidateRect(hPLWindow, &r, FALSE);
	}
	return 1;
}

static wchar_t entryFN[FILENAME_SIZE];
static BOOL CALLBACK entryProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			SetDlgItemTextW(hwndDlg, IDC_OLD, entryFN);
			SetDlgItemTextW(hwndDlg, IDC_NEW, entryFN);

			// show ctrl+e window and restore last position as applicable
			POINT pt = {ctrle_rect.left, ctrle_rect.top};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, ctrle_rect.left, ctrle_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);

			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
					GetDlgItemTextW(hwndDlg, IDC_NEW, entryFN, FILENAME_SIZE);
					GetWindowRect(hwndDlg, &ctrle_rect);
					EndDialog(hwndDlg, 1);
					return 0;
				case IDCANCEL:
					GetWindowRect(hwndDlg, &ctrle_rect);
					EndDialog(hwndDlg, 0);
					return 0;
			}
			return 0;
	}
	return 0;
}

void PE_Cmd(windowCommand *wc)
{
	if (wc == NULL) return ;
	switch (wc->cmd)
	{
		case PLCMD_ADD:
			TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 1), wc->align, wc->x, wc->y, 0, hPLWindow, NULL);
			break;
		case PLCMD_REM:
			TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 2), wc->align, wc->x, wc->y, 0, hPLWindow, NULL);
			break;
		case PLCMD_SEL:
			TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 3), wc->align, wc->x, wc->y, 0, hPLWindow, NULL);
			break;
		case PLCMD_MISC:
			TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 0), wc->align, wc->x, wc->y, 0, hPLWindow, NULL);
			break;
		case PLCMD_LIST:
			TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 4), wc->align, wc->x, wc->y, 0, hPLWindow, NULL);
			break;
	}
}

static librarySendToMenuStruct mySendTo;

static int PE_OnRButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	POINT p;

	GetCursorPos(&p);

	if ((flags & MK_LBUTTON)) return 1;

	if (peui_isrbuttoncaptured()) return 1;

	if (config_pe_height != 14 && config_pe_width >= 350 &&
	    (x >= config_pe_width - 150 - 75 && y >= config_pe_height - 26 && x <= config_pe_width - 150 && y <= config_pe_height - 8))
	{
		extern HMENU g_submenus_vis;
		TrackPopupMenu(g_submenus_vis, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
	}
	else if (config_pe_height != 14 &&
	         (x >= 14 && y >= config_pe_height - 30 && x <= 32 && y <= config_pe_height - 12))
	{
		TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 1), TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
	}
	else if (config_pe_height != 14 &&
	         (x >= 43 && y >= config_pe_height - 30 && x <= 64 && y <= config_pe_height - 12))
	{
		TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 2), TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
	}
	else if (config_pe_height != 14 &&
	         (x >= 72 && y >= config_pe_height - 30 && x <= 93 && y <= config_pe_height - 12))
	{
		TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 3), TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
	}
	else if (config_pe_height != 14 &&
	         (x >= 101 && y >= config_pe_height - 30 && x <= 122 && y <= config_pe_height - 12))
	{
		TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
	}
	else if (config_pe_height != 14 &&
	         (x >= config_pe_width - 44 && y >= config_pe_height - 30 && x <= config_pe_width - 44 + 22 && y <= config_pe_height - 12))
	{
		TrackPopupMenu(GetSubMenu(GetSubMenu(top_menu, 2), 4), TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
	}
	else if (config_pe_height != 14 && y >= 20 && y <= config_pe_height - 38 && x >= 12 && x <= config_pe_width - 20)
	{		// right click in list box
		int wh = (y - 22) / pe_fontheight + pledit_disp_offs;
		int s = (flags & MK_CONTROL);
		if (!s && !PlayList_getselect(wh))
		{
			int x, t = PlayList_getlength();
			for (x = 0; x < t; x ++)
				PlayList_setselect(x, 0);
		}
		if (PlayList_getselect(wh) && s) PlayList_setselect(wh, 0);
		else
		{
			int y = wh - pledit_disp_offs;
			if (y < PlayList_getlength() - pledit_disp_offs && y < (config_pe_height - 38 - 20 - 2) / pe_fontheight)
			{
				PlayList_setselect(wh, 1);
			}
		}
		{
			RECT r1 = {12, 22, config_pe_width - 20, config_pe_height - 38};
			//RECT r2 = {12, 20 + (wh - pledit_disp_offs) * pe_fontheight, config_pe_width - 20, 20 + (wh + 1 - pledit_disp_offs) * pe_fontheight};
			//			if (!s)
			InvalidateRect(hwnd, &r1, FALSE);
			//			else
			//				InvalidateRect(hPLWindow,&r2,FALSE);
			UpdateWindow(hwnd);
		}
#ifdef WINAMP_FINAL_BUILD
		if (!PlayList_gethidden(wh) && !PlayList_hasanycurtain(wh))
#endif
		{
			int ret;
			LRESULT IPC_LIBRARY_SENDTOMENU = SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
			HMENU ourmenu = GetSubMenu(GetSubMenu(top_menu, 2), 5);
			HMENU menu = 0;
			memset(&mySendTo, 0, sizeof(mySendTo));
			if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)0, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
			{
				MENUITEMINFOW mii = {sizeof(mii), MIIM_SUBMENU | MIIM_TYPE, MFT_STRING, };
				mii.hSubMenu = menu = CreatePopupMenu();
				mii.dwTypeData = getStringW(IDS_SENDTO_STR,NULL,0);
				mii.cch = wcslen(mii.dwTypeData);

				InsertMenuItemW(ourmenu, 1, TRUE, &mii);

				mySendTo.mode = 1;
				mySendTo.hwnd = hwnd;
				mySendTo.data_type = 7; // ML_TYPE_FILENAMESW
				mySendTo.build_hMenu = menu;
			}

			menufucker_t mf = {sizeof(mf),MENU_PLAYLIST,ourmenu,0x3000,0x4000,0};

			pluginMessage message_build = {SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&"menufucker_build", IPC_REGISTER_WINAMP_IPCMESSAGE),(intptr_t)&mf,0};
			sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_build);

			ret = TrackPopupMenu(ourmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hwnd, NULL);

			pluginMessage message_result = {SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&"menufucker_result", IPC_REGISTER_WINAMP_IPCMESSAGE),(intptr_t)&mf,ret,0};
			sendMlIpc(ML_IPC_SEND_PLUGIN_MESSAGE,(WPARAM)&message_result);

			if (menu)
			{
				if (mySendTo.mode == 2)
				{
					mySendTo.menu_id = ret;
					if (SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&mySendTo, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
					{
						size_t buf_size = 4096;
						wchar_t *buf = (wchar_t*)malloc(buf_size*sizeof(wchar_t));						
						size_t buf_pos = 0;

						int i, l = PlayList_getlength();

						for (i = 0;i < l;i++)
						{
							if (PlayList_getselect(i)
#ifdef WINAMP_FINAL_BUILD
								&& !PlayList_gethidden(i) && !PlayList_hasanycurtain(i)
#endif
								)
							{
								wchar_t itrFilename[FILENAME_SIZE];
								size_t newsize;

								PlayList_getitem(i, itrFilename, NULL);
								newsize = buf_pos + wcslen(itrFilename) + 1;
								if (newsize < buf_size)
								{
									buf_size = newsize + 4096;
									buf = (wchar_t*)realloc(buf, buf_size*sizeof(wchar_t));
								}
								StringCchCopyW(buf + buf_pos, buf_size - buf_pos, itrFilename);
								buf_pos = newsize;
							}
						}
						if (buf_pos)
						{
							buf[buf_pos] = 0;
							mySendTo.mode = 3;
							mySendTo.data = buf;
							mySendTo.data_type = 7; // ML_TYPE_FILENAMESW
							SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&mySendTo, IPC_LIBRARY_SENDTOMENU);
						}

						free(buf);

						ret = 0;
					}
				}
				// remove sendto
				DeleteMenu(ourmenu, 1, MF_BYPOSITION);
			}
			if (mySendTo.mode)
			{
				mySendTo.mode = 4;
				SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&mySendTo, IPC_LIBRARY_SENDTOMENU); // cleanup
				memset(&mySendTo, 0, sizeof(mySendTo));
			}
			if (ret) SendMessage(hwnd, WM_COMMAND, ret, 0);
		}
	}
	else
	{
		TrackPopupMenu(main_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hMainWindow, NULL);
	}

	return 1;
}

static int PE_OnLButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	ReleaseCapture();
	peui_handlemouseevent(hwnd, x, y, -1, flags);
	return 1;
}

static int PE_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	SetCapture(hwnd);
	peui_handlemouseevent(hwnd, x, y, 1, keyFlags);
	SetFocus(hwnd);
	return 1;
}

static int PE_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	peui_handlemouseevent(hwnd, x, y, 0, keyFlags);
	return 1;
}

static BOOL PE_OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized)
{
	if (fActive == FALSE)
	{
		draw_pe_tbar(hwnd, NULL, config_hilite ? 0 : 1);
		peui_reset(hwnd);
	}
	else
	{
		draw_pe_tbar(hwnd, NULL, 1);
	}
	return TRUE;
}

static int PE_OnLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if (y <= 14 && x < config_pe_width - 20)
	{
		SendMessage(hMainWindow, WM_COMMAND, WINAMP_OPTIONS_WINDOWSHADE_PL, 0);
	}
	else if (config_pe_height != 14)
	{
		if (x >= 12 && y >= 20 && x <= config_pe_width - 20 && y <= config_pe_height - 38)
		{
			int t = (y - 22) / pe_fontheight;
			if (t < PlayList_getlength() - pledit_disp_offs && t < (config_pe_height - 38 - 20 - 2) / pe_fontheight)
			{
				t += pledit_disp_offs;
				PlayList_setposition(t);
				PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
				StartPlaying();
				SetForegroundWindow(hwnd);
			}
		}
		else if (x >= config_pe_width - 44 && y >= config_pe_height - 30 && x < config_pe_width - 44 + 22 && y < config_pe_height - 12)
		{
			InvalidateRect(hwnd, NULL, 0);
			SendMessage(hwnd, WM_COMMAND, ID_PE_OPEN, 0);
		}
		else if (x >= 101 && y >= config_pe_height - 30 && x < 122 && y < config_pe_height - 12)
		{
			{
				POINT p = {122, config_pe_height - 30};
				ClientToScreen(hwnd, &p);
				TrackPopupMenu(GetSubMenu(GetSubMenu(GetSubMenu(top_menu, 2), 0), 2), TPM_LEFTALIGN, p.x, p.y, 0, hwnd, NULL);
			}
			peui_reset(hwnd);
		}
		else if (x >= 72 && y >= config_pe_height - 30 && x < 93 && y < config_pe_height - 12)
		{
			peui_reset(hwnd);
			SendMessage(hwnd, WM_COMMAND, ID_PE_SELECTALL, 0);
		}
		else if (x >= 43 && y >= config_pe_height - 30 && x < 64 && y < config_pe_height - 12)
		{
			peui_reset(hwnd);
			SendMessage(hwnd, WM_COMMAND, IDC_PLAYLIST_REMOVEMP3, 0);
		}
		else if (x >= 14 && y >= config_pe_height - 30 && x < 35 && y < config_pe_height - 12)
		{
			peui_reset(hwnd);
			SendMessage(hwnd, WM_COMMAND, IDC_PLAYLIST_ADDMP3, 0);
		}
	}
	return 1;
}

int setCurrentRating(int rating)
{
	LRESULT ret = sendMlIpc(ML_IPC_SETRATING, rating);
	if (!ret && !got_ml)
	{
		wchar_t fn[FILENAME_SIZE];
		if (!PlayList_getitem2W(PlayList_getPosition(), fn, NULL))
		{
			wchar_t buf[64];
			if (rating > 0)
				StringCchPrintfW(buf, 64, L"%d", rating);
			else
				buf[0] = 0;

			if(config_noml_ratings_prompt)
			{
				config_noml_ratings = (LPMessageBox(hMainWindow, IDS_NOML_SAVE_RATING, IDS_NOML_SAVE_RATING_TITLE, MB_YESNO | MB_ICONQUESTION) == IDYES);
				config_noml_ratings_prompt = 0;
			}

			if (config_noml_ratings)
			{
				in_set_extended_fileinfoW(fn, L"rating", buf);
				in_write_extended_fileinfo();
			}
		}
	}
	SendMessage(hMainWindow,WM_TIMER,UPDATE_DISPLAY_TIMER+4,0);
	return ret;
}

int setPlRating(int rating)
{
	int r = 0;
	int i, l = PlayList_getlength();

	for (i = 0;i < l;i++)
	{
		if (PlayList_getselect(i))
		{
			pl_set_rating psr;
			psr.plentry = i;
			psr.rating = rating;
			r |= sendMlIpc(ML_IPC_PL_SETRATING, (WPARAM) & psr);
			if (!r && !got_ml)
			{
				wchar_t buf[64];
				if (rating > 0)
					StringCchPrintfW(buf, 64, L"%d", rating);
				else
					buf[0] = 0;

				if(config_noml_ratings_prompt)
				{
					config_noml_ratings = (LPMessageBox(hMainWindow, IDS_NOML_SAVE_RATING, IDS_NOML_SAVE_RATING_TITLE, MB_YESNO | MB_ICONQUESTION) == IDYES);
					config_noml_ratings_prompt = 0;
				}

				wchar_t fn[FILENAME_SIZE];
				if (config_noml_ratings && !PlayList_getitem2W(i, fn, NULL))
				{
					in_set_extended_fileinfoW(fn, L"rating", buf);
					in_write_extended_fileinfo();
				}
			}
			g_need_titleupd = 1; // force a title update (%rating% may be in the ATF string)
			PlayList_updateitem(i);
		}
	}
	plEditRefresh();
	SendMessage(hMainWindow,WM_TIMER,UPDATE_DISPLAY_TIMER+4,0);
	return r;
}

LRESULT CALLBACK PE_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int se_lp, se_a, se_t;
	if (uMsg == g_scrollMsg)
	{
		wParam <<= 16; uMsg = WM_MOUSEWHEEL;
	}
	switch (uMsg)
	{
		case WM_SHOWWINDOW:
		if (wParam == TRUE && lParam == SW_PARENTOPENING && !config_pe_open)
			return 0;		
		break;

		case WM_DISPLAYCHANGE:
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		case WM_INITMENUPOPUP:
			if (wParam && (HMENU)wParam == mySendTo.build_hMenu && mySendTo.mode == 1)
			{
				int IPC_LIBRARY_SENDTOMENU = SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&"LibrarySendToMenu", IPC_REGISTER_WINAMP_IPCMESSAGE);
				if (IPC_LIBRARY_SENDTOMENU > 65536 && SendMessage(hMainWindow, WM_WA_IPC, (WPARAM)&mySendTo, IPC_LIBRARY_SENDTOMENU) == 0xffffffff)
					mySendTo.mode = 2;
			}
			// handles the ratings menu and is localised happy now
			// (compared to the IDS_RATEITEM_STR based code) - DO 26/09/07
			if (wParam && (HMENU)wParam == GetSubMenu(GetSubMenu(top_menu, 2), 5))
			{
				int i;
				int nitems;
				HMENU menu = NULL, tmenu = NULL;
				nitems = GetMenuItemCount((HMENU)wParam);
				for (i = 0;i < nitems;i++)
				{
					// test for it being a popup
					if(GetMenuItemID((HMENU)wParam, i) == -1){
						if((tmenu = GetSubMenu((HMENU)wParam, i)))
						{
							// and if so then see what the last popup menu item is
							if(GetMenuItemID(tmenu, GetMenuItemCount(tmenu)-1) == ID_PL_RATING0)
							{
								menu = tmenu;
								break;
							}
						}
					}
				}

				if (menu)
				{
					int darating = -1;
					int i, l = PlayList_getlength();

					for (i = 0;i < l;i++)
					{
						if (PlayList_getselect(i) 
#ifdef WINAMP_FINAL_BUILD
							&& !PlayList_gethidden(i) && !PlayList_hasanycurtain(i)
#endif
							)
						{
							int rating = sendMlIpc(ML_IPC_PL_GETRATING, i);

							// deal with no ml being present and querying the rating of a file from the tag (if possible)
							// as well as getting a zero rating which could mean that it's not present in the library
							if (!rating || !got_ml)
							{
								wchar_t fn[FILENAME_SIZE], buf[64];
								if (!PlayList_getitem2W(i, fn, NULL))
								{
									in_get_extended_fileinfoW(fn, L"rating", buf, 64);
									rating = _wtoi(buf);
								}
							}

							if (darating == -1)
							{
								darating = rating;
							}
							else
							{
								if (darating != rating) darating = -2;
							}
						}
					}

					CheckMenuItem(menu, ID_PL_RATING5, (darating == 5) ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(menu, ID_PL_RATING4, (darating == 4) ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(menu, ID_PL_RATING3, (darating == 3) ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(menu, ID_PL_RATING2, (darating == 2) ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(menu, ID_PL_RATING1, (darating == 1) ? MF_CHECKED : MF_UNCHECKED);
					CheckMenuItem(menu, ID_PL_RATING0, (darating == 0) ? MF_CHECKED : MF_UNCHECKED);
				}
			}
			return 0;
			HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE, Main_OnQueryNewPalette);
			HANDLE_MSG(hwnd, WM_PALETTECHANGED, Main_OnPaletteChanged);
			HANDLE_MSG(hwnd, WM_LBUTTONUP, PE_OnLButtonUp);
			HANDLE_MSG(hwnd, WM_RBUTTONUP, PE_OnRButtonUp);
			HANDLE_MSG(hwnd, WM_LBUTTONDOWN, PE_OnLButtonDown);
			HANDLE_MSG(hwnd, WM_MOUSEMOVE, PE_OnMouseMove);
			HANDLE_MSG(hwnd, WM_NCACTIVATE, PE_OnNCActivate);
			HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, PE_OnLButtonDblClk);
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_SCREENSAVE || (wParam & 0xfff0) == SC_MONITORPOWER)
				return SendMessage(hMainWindow, uMsg, wParam, lParam);
			break;

		case WM_CLOSE:
			WASABI_API_APP->main_shutdown();
			return 0;
		case WM_PAINT:
			draw_paint_pe(hwnd);
			return 0;
		case WM_PRINTCLIENT:
			draw_printclient_pe(hwnd, (HDC)wParam, lParam);
			return 0;
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *p = (MINMAXINFO *)lParam;
			p->ptMaxTrackSize.x = 16384;
			p->ptMaxTrackSize.y = 16384;
		}
			return 0;
		case WM_NOTIFY:
		{
			LPTOOLTIPTEXT tt = (LPTOOLTIPTEXT)lParam;
			/*if(tt->hdr.hwndFrom = hPL2TooltipWindow)
			{
				switch (tt->hdr.code)
				{
					case TTN_SHOW:*/
						/*if(tt->hdr.idFrom == 17)
						{
							POINT pt = {0};
							GetCursorPos(&pt);
							ScreenToClient(hPLWindow,&pt);
							RECT rc = {0};
							EstPLWindowRect(&rc);
							rc.left += 12;

							int t = (pt.y - 20) / pe_fontheight;
							if (t < 0) t = 0;
							else if (t > (config_pe_height - 38 - 20 - 2) / pe_fontheight) t = PlayList_getlength();
							rc.top += 22+(pe_fontheight*t);

							SendMessage(tt->hdr.hwndFrom,TTM_ADJUSTRECT,TRUE,(LPARAM)&rc);
							SetWindowPos(tt->hdr.hwndFrom,HWND_TOPMOST,rc.left,rc.top,0,0,SWP_NOSIZE|SWP_NOACTIVATE);
							return TRUE;
						}
						else*/
						/*{
							SetWindowPos(tt->hdr.hwndFrom,HWND_TOPMOST,0,0,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
						}
						break;
				}
			}
			else*/ if(tt->hdr.hwndFrom = hPLTooltipWindow)
			{
				switch (tt->hdr.code)
				{
					case TTN_SHOW:
						SetWindowPos(tt->hdr.hwndFrom,HWND_TOPMOST,0,0,0,0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
						break;
				}
			}
		}
		break;
		case WM_SIZE:
			if (wParam == SIZE_RESTORED)
			{
				// do an up down with position update (if applicable on processing) so
				// when we go to a modern skin we aren't left with a massive gap at the
				// end from the pledit thinking it is still in the classic window size
				// (dro 30/05/08)
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_PE_SCROLLDOWN,0), 0);
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_PE_SCROLLUP,0), 0);
				SendMessage(hwnd, WM_USER, 666, PlayList_getPosition());
			}
			return 0;
		case WM_WINDOWPOSCHANGED:
			if (GetParent(hwnd)) // fix the stupid pl bug?
			{
				WINDOWPOS *lpwp = (WINDOWPOS*)lParam;
				config_pe_width = lpwp->cx;
				if (config_pe_height != 14)
				{
					config_pe_height = lpwp->cy;
					if (hPLVisWindow)
					{
						int x, y, w, h;
						x = config_pe_width - 150 - 75 + 2;
						y = config_pe_height - 26;
						w = (config_pe_width >= 350 && config_pe_height != 14 ? 72 : 0);
						h = 16;
						SetWindowPos(hPLVisWindow, 0, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
					}
				}
				break;
			} else {
				// update the position of the tooltips on window resize
				set_pl_wnd_tooltip();
			}
			break;
		case WM_CREATE:
			hPLWindow = hwnd;
			SetTimer(hwnd, 1, 100, NULL);
			draw_reinit_plfont(0);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (config_keeponscreen&2) ? 0x49474541 : 0);
			if (NULL != WASABI_API_APP) WASABI_API_APP->app_registerGlobalWindow(hwnd);
			{
				if (!config_embedwnd_freesize) config_pe_width -= config_pe_width % 25;
				if (config_pe_width < 275) config_pe_width = 275;
				if (config_pe_height != 14)
				{
					if (!config_embedwnd_freesize) config_pe_height -= config_pe_height % 29;
					if (config_pe_height < 116) config_pe_height = 116;
					config_pe_height_ws = 0;
				}
				else
				{
					if (!config_embedwnd_freesize) config_pe_height_ws -= config_pe_height_ws % 29;
					if (config_pe_height_ws < 116) config_pe_height_ws = 116;
				}
				SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE)&~(WS_CAPTION));
				SetWindowPos(hwnd, 0, config_pe_wx, config_pe_wy, config_pe_width, config_pe_height, SWP_NOACTIVATE | SWP_NOZORDER);

				HACCEL hAccel = LoadAcceleratorsW(language_pack_instance, MAKEINTRESOURCEW(IDR_ACCELERATOR_PL));
				if (!hAccel && language_pack_instance != hMainInstance) hAccel = LoadAcceleratorsW(hMainInstance, MAKEINTRESOURCEW(IDR_ACCELERATOR_PL));
				if (hAccel) WASABI_API_APP->app_addAccelerators(hwnd, &hAccel, 1, TRANSLATE_MODE_NORMAL);
			}

			return 0;
		case WM_DESTROY:
			KillTimer(hwnd, 1);
			KillTimer(hwnd, 3);
			if (pe_init)
				draw_pe_kill();
			if (NULL != WASABI_API_APP) WASABI_API_APP->app_unregisterGlobalWindow(hwnd);
			return 0;
		case WM_TIMER:
			if (wParam == 3)
			{
				KillTimer(hwnd, 3);
				if (!IsMinimized(hMainWindow))
				{
					ShowWindow(hwnd, SW_SHOWNA);
				}
				SendMessage(hwnd, WM_USER, 666, PlayList_getPosition());

				// works around a quirk with Bento docked to the right-hand edge
				// where the pledit is left docked on reverting to classic skin (dro)
				if(!GetParent(hwnd)) set_aot(1);
			}
			else if (wParam == 1)
			{
				static int a;
				if (!a && config_pe_height != 14 && config_pe_open && (config_riol == 0 || config_riol == 4) && !peui_isrbuttoncaptured())
				{
					int start, end, view_start, pl_len;
					a = 1;
					start = pledit_disp_offs;
					end = start + (config_pe_height - 38 - 20 - 2) / pe_fontheight;
					if (start < 0) start = 0;
					pl_len = PlayList_getlength();
					if (end > pl_len || config_riol == 4) end = pl_len;
					view_start = start;

					while (start < end)
					{
						if (!PlayList_getcached(start))
						{
							RECT r1 = {12, 20 + (start - pledit_disp_offs) * pe_fontheight,
							           config_pe_width - 20, 20 + (start + 1 - pledit_disp_offs) * pe_fontheight};

							PlayList_updateitem(start);
							if (PlayList_getcached(start))
							{
								InvalidateRect(hwnd, &r1, FALSE);
								if(config_riol == 4)
									InvalidateRect(hwnd, NULL, FALSE);
								break;
							}
						}
						start++;

						// if we got to the end of the viewable area / playlist contents
						// then we loop to the start so we can try to read all the titles
						// and back to the point just before the current top of list point
						if(config_riol == 4 && start == end && end == pl_len){
							start = 0;
							end = view_start;
						}
					}
					a = 0;
				}
			}
			return 0;
		case WM_USER:
			switch (wParam)
			{
				case 666:
				{
					RECT r = {12, 22, config_pe_width - 3, config_pe_height - 37};
					int num_songs = (config_pe_height - 38 - 20 - 2) / pe_fontheight;
					int p = (lParam & ((1 << 30) - 1)), t;
					int brep = !(lParam & (1 << 30));
					int needrepaint = 1;
					t = PlayList_getlength();
					if (config_pe_height == 14)
					{
						RECT r2 = {0, 0, config_pe_width, 14};
						r = r2;
						num_songs = (config_pe_height_ws - 38 - 20 - 2) / pe_fontheight;
					}

					if (p >= pledit_disp_offs + num_songs || p < pledit_disp_offs)
					{
						if (brep)
						{
							pledit_disp_offs = max(p - num_songs / 2, 0);
							if (pledit_disp_offs + num_songs > t)
								pledit_disp_offs = t - num_songs;
						}
						else needrepaint = 0;
					}
					if (brep)
					{
						if (num_songs >= t || pledit_disp_offs < 0)
						{
							pledit_disp_offs = 0;
						}
					}
					if (needrepaint && IsWindowVisible(hwnd)) InvalidateRect(hwnd, &r, FALSE);
				}
				break;
				case IPC_PE_GETDIRTY:
					return plmodified;
				case IPC_PE_SETCLEAN:
					plmodified = 0;
					break;
				case IPC_PE_GETNEXTSELECTED:
					return PlayList_GetNextSelected(lParam);
				case IPC_PE_GETSELECTEDCOUNT:
					return PlayList_GetSelectedCount();
				case IPC_PE_GETCURINDEX:
					return PlayList_getPosition();
				case IPC_PE_GETINDEXTOTAL:
					return PlayList_getlength();
				case IPC_PE_DELETEINDEX:
				{
					if (lParam < PlayList_getlength())
					{
						PlayList_deleteitem(lParam);
					}
					InvalidateRect(hwnd, NULL, FALSE);
					if (!g_has_deleted_current)
					{
						PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
						draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
					}
					break;
				}
				case IPC_PE_SWAPINDEX:
				{
					PlayList_swap(lParam & 0xFFFF, (lParam & 0xFFFF0000) >> 16);
					InvalidateRect(hwnd, NULL, FALSE);
					if (!g_has_deleted_current)
					{
						PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
						draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
					}
					break;
				}
				case IPC_PE_GETIDXFROMPOINT:
				{
					POINT *p = (POINT *) lParam;
					int t;

					if (config_pe_height == 14) return PlayList_getlength();
					else
					{
						t = (p->y - 20) / pe_fontheight;
						if (t < 0) t = 0;
						else if (t > (config_pe_height - 38 - 20 - 2) / pe_fontheight) t = PlayList_getlength();
						else t += pledit_disp_offs;
					}
					return t;
				}
				case IPC_PE_SAVEEND:
					se_a = PlayList_getlength();
					PlayList_saveend(se_t = (int)lParam);
					se_lp = PlayList_getPosition();

					break;
				case IPC_PE_RESTOREEND:
					PlayList_restoreend();
					if (se_t <= PlayList_getPosition())
					{
						PlayList_setposition(se_lp + PlayList_getlength() - se_a);
						if (!g_has_deleted_current)
						{
							PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
							draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
						}
					}
					plEditRefresh();
					break;
				case IPC_PE_GETINDEXTITLE:
					if (lParam)
					{
						fileinfo2 *fi = (fileinfo2 *)lParam;
						return PlayList_getitem3(fi->fileindex, fi->filetitle, fi->filelength);
					}
					break;
				case IPC_PE_GETINDEXTITLEW:
					if (lParam)
					{
						fileinfo2W *fi = (fileinfo2W *)lParam;
						return PlayList_getitem3W(fi->fileindex, fi->filetitle, fi->filelength);
					}
					break;
				case IPC_PE_GETINDEXINFO_INPROC:
					if (lParam)
					{
						fileinfo *fi = (fileinfo *)lParam;
						char tempfile[FILENAME_SIZE]="";
						PlayList_getitem2(fi->index, tempfile, NULL);
						StringCbCopy(fi->file, MAX_PATH, tempfile);
					}
					break;
				case IPC_PE_GETINDEXINFOW_INPROC:
					if (lParam)
					{
						fileinfoW *fi = (fileinfoW *)lParam;
						wchar_t tempfile[FILENAME_SIZE]=L"";
						PlayList_getitem2W(fi->index, tempfile, NULL);
						StringCbCopyW(fi->file, sizeof(fi->file), tempfile);
					}
					break;
			}
			return 0;


		case WM_COPYDATA:
		{
			COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;
			switch (cds->dwData)
			{
				case IPC_PE_INSERTFILENAME:
				{
					fileinfo *f = (fileinfo *)(cds->lpData);
					if (f && *f->file)
					{
						int oldposition = PlayList_getPosition();
						int oldSize = PlayList_getlength();
						PlayList_saveend(f->index);
						PlayList_appendthing(AutoWideFn(f->file), 0, 0);
						PlayList_restoreend();
						if (oldposition >= f->index)
						{
							oldposition += (PlayList_getlength() - oldSize);
							PlayList_setposition(oldposition);
						}

						InvalidateRect(hwnd, NULL, FALSE);
						if (!g_has_deleted_current)
						{
							PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
							draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
						}
					}
					return TRUE;
				}
				case IPC_PE_INSERTFILENAMEW:
				{
					fileinfoW *f = (fileinfoW *)(cds->lpData);
					if (f && *f->file)
					{
						int oldposition = PlayList_getPosition();
						int oldSize = PlayList_getlength();
						PlayList_saveend(f->index);
						PlayList_appendthing(f->file, 0, 0);
						PlayList_restoreend();
						if (oldposition >= f->index)
						{
							oldposition += (PlayList_getlength() - oldSize);
							PlayList_setposition(oldposition);
						}

						InvalidateRect(hwnd, NULL, FALSE);
						if (!g_has_deleted_current)
						{
							PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
							draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
						}
					}
					return TRUE;
				}
				case IPC_PE_GETINDEXINFO:
				{
					callbackinfo *i = (callbackinfo *)(cds->lpData);
					if (i && i->callback)
					{
						COPYDATASTRUCT cdsr;
						fileinfo f;
						cdsr.dwData = IPC_PE_GETINDEXINFORESULT;
						char tempfile[FILENAME_SIZE]="";
						PlayList_getitem2(i->index, tempfile, NULL);
						StringCbCopy(f.file, sizeof(f.file), tempfile);
						f.index = i->index;
						cdsr.lpData = (void *) & f;
						cdsr.cbData = sizeof(fileinfo);
						SendMessage(i->callback, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cdsr);
						return 0;
					}
					return TRUE;
				}
				case IPC_PE_GETINDEXINFO_TITLE:
				{
					callbackinfo *i = (callbackinfo *)(cds->lpData);
					if (i && i->callback)
					{
						COPYDATASTRUCT cdsr;
						fileinfo f;
						cdsr.dwData = IPC_PE_GETINDEXINFORESULT_TITLE;
						PlayList_getitem3(i->index, f.file, NULL);
						f.index = i->index;
						cdsr.lpData = (void *) & f;
						cdsr.cbData = sizeof(fileinfo);
						SendMessage(i->callback, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cdsr);
						return 0;
					}
					return TRUE;
				}
			}
		}
		return FALSE;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case ID_PE_SHOWPLAYING:
					plEditSelect(PlayList_getPosition());
					return 0;
				case WINAMP_OPTIONS_WINDOWSHADE:
					SendMessage(hMainWindow, WM_COMMAND, WINAMP_OPTIONS_WINDOWSHADE_PL, 0);
					return 0;
				case ID_PE_FFOD:
				{
					int x, v;
					wchar_t fn[FILENAME_SIZE];
					v = PlayList_getlength();
					for (x = 0; x < v; x++)
					{
						if (PlayList_getselect(x) 
							#ifdef WINAMP_FINAL_BUILD
							&& !PlayList_gethidden(x) && !PlayList_hasanycurtain(x)
							#endif
							)
						{
							if (!PlayList_getitem2W(x, fn, NULL))
							{
								// show the playlist file entry of the selected item(s) if there were any
								explorerFindFileManager->AddFile(fn);
							}
						}
					}
					explorerFindFileManager->ShowFiles();
					return 0;
				}
				case ID_PE_BOOKMARK:
				{
					int x, v;
					wchar_t fn[FILENAME_SIZE], ft[FILETITLE_SIZE];
					v = PlayList_getlength();
					for (x = 0; x < v; x++)
					{
						if (PlayList_getselect(x) 
							#ifdef WINAMP_FINAL_BUILD
							&& !PlayList_gethidden(x) && !PlayList_hasanycurtain(x)
							#endif
							)
						{
							PlayList_getitem2W(x, fn, ft);
							Bookmark_additem(fn, ft);
						}
					}
				}
				return 0;
				case ID_PE_EXTINFO:
				{
					int x, v;
					wchar_t ft[FILETITLE_SIZE];
					RECT r = {0, 0, config_pe_width, config_pe_height - 37};
					v = PlayList_getlength();
					for (x = 0; x < v; x++)
					{
						if (PlayList_getselect(x) 
							#ifdef WINAMP_FINAL_BUILD
							&& !PlayList_gethidden(x) && !PlayList_hasanycurtain(x)
#endif
							)
						{
							wchar_t fn[FILENAME_SIZE];
							PlayList_getitem(x, fn, ft);
							PlayList_setitem(x, fn, PlayList_gettitle(fn, 1));
							PlayList_setlastlen(x);
						}
					}
					InvalidateRect(hwnd, &r, FALSE);
				}
				return 0;
				case ID_PE_PRINT:
					doHtmlPlaylist();
					return 0;
				case WINAMP_NEXT_WINDOW:
					return SendMessage(hMainWindow, uMsg, wParam, lParam);
				case ID_PE_OPEN:
				{
					//int isvisible = IsWindowVisible(hwnd);
					if (!playlist_open(/*isvisible ? hwnd :*/ (g_dialog_box_parent ? g_dialog_box_parent : hMainWindow))) break;
				}
				return 0;
				case ID_PE_ID3:
				{
					int x, v;
					wchar_t ft[FILETITLE_SIZE];
					v = PlayList_getlength();
					for (x = 0; x < v; x++)
					{
						if (PlayList_getselect(x) 
							#ifdef WINAMP_FINAL_BUILD
							&& !PlayList_gethidden(x) && !PlayList_hasanycurtain(x)
#endif
							)
						{
							wchar_t fn[FILENAME_SIZE];

							PlayList_getitem(x, fn, ft);

							if (in_infobox(hwnd, fn)) break;
							PlayList_setitem(x, fn, PlayList_gettitle(fn, 1));
							PlayList_setlastlen(x);
						}
					}
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
				}
				return 0;
				case ID_PE_CLEAR:
					PlayList_delete();
					PlayList_randpos(0);
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
					return 0;
				case IDC_SELECTINV:
				{
					int x, v;
					v = PlayList_getlength();
					for (x = 0; x < v; x ++)
					{
						PlayList_setselect(x, !PlayList_getselect(x));
					}
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
				}
				return 0;
				case ID_PE_SELECTALL:
				{
					int x, v;
					v = PlayList_getlength();
					for (x = 0; x < v; x ++)
					{
						PlayList_setselect(x, 1);
					}
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
				}
				return 0;
				case ID_PE_NONE:
				{
					int x, v;
					v = PlayList_getlength();
					for (x = 0; x < v; x ++)
					{
						PlayList_setselect(x, 0);
					}
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
				}
				return 0;
				case ID_PE_ENTRY:
				{
					int x, v;
					wchar_t ft[FILETITLE_SIZE];
					v = PlayList_getlength();
					for (x = 0; x < v; x ++)
					{
						if (PlayList_getselect(x) 
#ifdef WINAMP_FINAL_BUILD
							&& !PlayList_gethidden(x) && !PlayList_hasanycurtain(x)
#endif
							)
						{
							wchar_t itrFilename[FILENAME_SIZE];
							PlayList_getitem(x, itrFilename, ft);
							StringCchCopyW(entryFN, FILENAME_SIZE, itrFilename);
							if (!LPDialogBoxW(IDD_EDITENTRY, DIALOG_PARENT(hwnd), (WNDPROC)entryProc))
							{
								SetFocus(hwnd);
								break;
							}
							SetFocus(hwnd);
							PlayList_setitem(x, entryFN, PlayList_gettitle(entryFN, 1));
						}
					}

					RECT r = {0, 0, config_pe_width, config_pe_height - 37};
					InvalidateRect(hwnd, &r, FALSE);
				}
				return 0;
				case ID_PE_SAVEAS:
					savepls(hwnd);
					return 0;
				case IDC_PLAYLIST_CROP:
				{
					int v = PlayList_getlength();
					int x;
					for (x = 0; x < v; x ++)
						if (PlayList_getselect(x)) break;
					if (x != v) for (x = v - 1; x >= 0; x--)
						{
							if (!PlayList_getselect(x))
							{
								PlayList_deleteitem(x);
							}
						}
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
				}
				return 0;
				case IDC_PLAYLIST_REMOVEMP3:
				{
					int v = PlayList_getlength(), x;
					for (x = v - 1; x >= 0; x--)
					{
						if (PlayList_getselect(x))
						{
							PlayList_deleteitem(x);
						}
					}
					InvalidateRect(hwnd, NULL, FALSE);
					if (!g_has_deleted_current)
					{
						PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
						draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
					}
				}
				return 0;
				case ID_PEPLAYLIST_PLAYLISTPREFERENCES:
					prefs_last_page = 23;
					prefs_dialog(1);
					return 0;
				case ID_PE_SCUP:
				case ID_PE_SCDOWN:
					// TODO: need to make the shift action more like Windows so it'll shrink the selection down rather than making it grow
					//       ideally need to keep a record of the 'current selection' so we can then adjust from there-will be interesting
					if (config_pe_height != 14)
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						int which;
						int x;
						int num_songs = (config_pe_height - 38 - 20 - 2) / pe_fontheight;
						if (LOWORD(wParam) == ID_PE_SCUP)
						{
							for (x = pledit_disp_offs; x < pledit_disp_offs + num_songs; x ++) if (PlayList_getselect(x)) break;
							if (x != pledit_disp_offs + num_songs) which = x - 1;
							else which = pledit_disp_offs;
						}
						else
						{
							for (x = pledit_disp_offs + num_songs - 1; x >= pledit_disp_offs; x--) if (PlayList_getselect(x)) break;
							if (x >= pledit_disp_offs) which = x + 1;
							else which = pledit_disp_offs + num_songs - 1;
						}

						if (which < 0) which = 0;
						if (which >= PlayList_getlength()) which = PlayList_getlength() - 1;
						if (!(GetAsyncKeyState(VK_SHIFT)&0x8000))
						{
							for (x = PlayList_getlength() - 1; x >= 0; x --) PlayList_setselect(x, 0);
						}
						PlayList_setselect(which, 1);
						if (which < pledit_disp_offs) pledit_disp_offs = which;
						else if (which >= pledit_disp_offs + num_songs)
							pledit_disp_offs = which - num_songs + 1;
						InvalidateRect(hwnd, &r, FALSE);
					}
					else
					{
						if (LOWORD(wParam) == ID_PE_SCUP)
							SendMessage(hMainWindow, WM_COMMAND, WINAMP_BUTTON1, 0);
						else
							SendMessage(hMainWindow, WM_COMMAND, WINAMP_BUTTON5, 0);
					}
					return 0;
				case ID_PE_SCROLLDOWN:
				{
					RECT r = {0, 0, config_pe_width, config_pe_height - 37};
					int num_songs = (config_pe_height - 38 - 20 - 2) / pe_fontheight;
					int t = PlayList_getlength();
					pledit_disp_offs += config_plscrollsize;
					if (pledit_disp_offs >= t - num_songs) pledit_disp_offs = t - num_songs;
					InvalidateRect(hwnd, &r, FALSE);
				}
				return 0;

				case ID_PE_SCROLLUP:
					if (pledit_disp_offs > 0)
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						pledit_disp_offs -= config_plscrollsize;
						if (pledit_disp_offs < 0) pledit_disp_offs = 0;
						InvalidateRect(hwnd, &r, FALSE);
					}
					return 0;

				case ID_PE_MOVEDOWN:
				{
					extern int shiftsel_1;
					int x, v = PlayList_getlength();
					RECT r =
					  {
					    0, 0, config_pe_width, config_pe_height - 37
					  };
					if (!PlayList_getselect(v - 1)) for (x = v - 2; x >= 0; x --)
						{
							if (PlayList_getselect(x))
							{
								if (shiftsel_1 == x) shiftsel_1 = x + 1;
								PlayList_swap(x, x + 1);
								if (x == PlayList_getPosition()) PlayList_advance(1);
								else if (x + 1 == PlayList_getPosition()) PlayList_advance(-1);
							}
						}
					InvalidateRect(hwnd, &r, FALSE);
					if (!g_has_deleted_current)
					{
						PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
						draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
					}
				}
				return 0;

				case ID_PE_MOVEUP:
				{
					extern int shiftsel_1;
					int x, v = PlayList_getlength();
					RECT r =
					  {
					    0, 0, config_pe_width, config_pe_height - 37
					  };
					if (!PlayList_getselect(0)) for (x = 1; x < v; x ++)
						{
							if (PlayList_getselect(x))
							{
								if (shiftsel_1 == x) shiftsel_1 = x + 1;
								PlayList_swap(x, x - 1);
								if (x == PlayList_getPosition()) PlayList_advance(-1);
								else if (x - 1 == PlayList_getPosition()) PlayList_advance(1);
							}
						}
					InvalidateRect(hwnd, &r, FALSE);
					if (!g_has_deleted_current)
					{
						PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
						draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
					}
					return 0;
				}
				case ID_PE_CLOSE:
					SendMessage(hMainWindow, WM_COMMAND, WINAMP_OPTIONS_PLEDIT, 0);
					return 0;
				case IDC_PLAYLIST_ADDMP3:
					//getNewFile(0, hwnd, 0); // changed May-10-2005 benski
					getNewFile(0,   /*NULL*/DIALOG_PARENT(hMainWindow), 0);
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
					SetFocus(hwnd);
					SetForegroundWindow(hwnd);
					return 0;
				case IDC_PLAYLIST_ADDLOC:
					getNewLocation(0, hwnd);
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
					return 0;
				case IDC_PLAYLIST_ADDDIR:
				{
					BROWSEINFOW bi;
					ITEMIDLIST *idlist;
					wchar_t name[MAX_PATH];
					bi.hwndOwner = hwnd;
					bi.pidlRoot = 0;
					bi.pszDisplayName = name;
					bi.lpszTitle = L"__foo";
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
					bi.lpfn = BrowseCallbackProc;
					bi.lParam = 1;
					idlist = SHBrowseForFolderW(&bi);
					if (idlist)
					{
						int s = PlayList_getlength();
						wchar_t path[MAX_PATH];
						SHGetPathFromIDListW(idlist, path);
						Shell_Free(idlist);
						WASABI_API_APP->path_setWorkingPath(path);
						//SetCurrentDirectoryW(path);
						PlayList_adddir(path, (config_rofiob&2) ? 0 : 1);
						if (config_rofiob&1)
							PlayList_sort(2, s);

						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);

					}
				}
				return 0;
				case IDC_PLAYLIST_PLAY:
				{
					int x, v = PlayList_getlength();
					for (x = 0; x < v; x ++)
					{
						if (PlayList_getselect(x)) break;
					}
					if (x != v)
					{
						PlayList_setposition(x);
						PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
						StartPlaying();
						SetForegroundWindow(hwnd);
					}
				}
				return 0;
				case ID_PE_S_TITLE:
				case ID_PE_S_FILENAME:
				case ID_PE_S_PATH:
				case ID_PE_S_RANDOM:
				case ID_PE_S_REV:
				{
					int num_songs = (config_pe_height - 38 - 20 - 2) / pe_fontheight;
					int w = 0;
					if (LOWORD(wParam) == ID_PE_S_TITLE) w = 1;
					else if (LOWORD(wParam) == ID_PE_S_PATH) w = 2;
					else if (LOWORD(wParam) == ID_PE_S_FILENAME) w = 0;
					else if (LOWORD(wParam) == ID_PE_S_RANDOM) w = 3;
					else if (LOWORD(wParam) == ID_PE_S_REV) w = 4;
					if (w == 3) PlayList_randomize();
					else if (w == 4) PlayList_reverse();
					else if (w <= 2) PlayList_sort(w, 0);
					pledit_disp_offs = PlayList_getPosition();
					if (pledit_disp_offs + num_songs > PlayList_getlength())
						pledit_disp_offs = PlayList_getlength() - num_songs;
					if (pledit_disp_offs < 0) pledit_disp_offs = 0;
					InvalidateRect(hwnd, NULL, FALSE);
					if (!g_has_deleted_current)
					{
						PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
						draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
					}
				}
				break;
				case ID_PE_DELETEFROMDISK:
				{
					int v, x;
					v = PlayList_getlength();
					for (x = v - 1; x >= 0; x --)
					{
						if (PlayList_getselect(x))
						{
							wchar_t fn[FILENAME_SIZE];

							PlayList_getitem(x, fn, NULL);
							if (!PathIsURLW(fn))
							{
								SHFILEOPSTRUCTW fileOp;
								wchar_t file[FILENAME_SIZE];
								StringCchCopyW(file, FILENAME_SIZE - 1, fn);
								file[lstrlenW(fn) + 1] = 0; // double null terminate;

								fileOp.hwnd = hwnd;
								fileOp.wFunc = FO_DELETE;
								fileOp.pFrom = file;
								fileOp.pTo = 0;
								fileOp.fFlags = config_playlist_recyclebin ? FOF_ALLOWUNDO : 0;
								fileOp.fAnyOperationsAborted = 0;
								fileOp.hNameMappings = 0;
								fileOp.lpszProgressTitle = 0;

								if (SHFileOperationW(&fileOp))
								{
									wchar_t mes[4096];
									StringCchPrintfW(mes, 4096, getStringW(IDS_ERROR_DELETING,NULL,0), fn);
									MessageBoxW(hwnd, mes, getStringW(IDS_ERROR,NULL,0), MB_OK);
								}
								else
								{
									// only remove if the deletion actually went ahead
									if(!fileOp.fAnyOperationsAborted)
										PlayList_deleteitem(x);
								}
							}
						}
					}
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
				}
				break;
				case ID_PE_NONEXIST:
				{
					int x, v;
					v = PlayList_getlength();
					for (x = v - 1; x >= 0; x --)
					{
						wchar_t fn[FILENAME_SIZE];
						FILE *fp = NULL;
						PlayList_getitem(x, fn, NULL);
						fp = _wfopen(fn, L"rb");
						if (!PathIsURLW(fn) && !(fp/* = _wfopen(fn, L"rb")*/))
						{
							PlayList_deleteitem(x);
						}
						else
						{
							if (fp) fclose(fp);
						}
					}
					{
						RECT r = {0, 0, config_pe_width, config_pe_height - 37};
						InvalidateRect(hwnd, &r, FALSE);
					}
				}
				break;
				case ID_PE_TOP:
					if (config_pe_height != 14)
					{
						pledit_disp_offs = 0;
						{
							RECT r = {0, 0, config_pe_width, config_pe_height - 37};
							InvalidateRect(hwnd, &r, FALSE);
						}
					}
					break;
				case ID_PE_BOTTOM:
					if (config_pe_height != 14)
					{
						pledit_disp_offs = PlayList_getlength() - (config_pe_height - 38 - 20 - 2) / pe_fontheight;
						if (pledit_disp_offs < 0) pledit_disp_offs = 0;
						{
							RECT r = {0, 0, config_pe_width, config_pe_height - 37};
							InvalidateRect(hwnd, &r, FALSE);
						}
					}
					break;
				case ID_PE_FONTBIGGER:
					config_pe_fontsize++;
					if (hMainWindow)
					{
						draw_reinit_plfont(1);
						InvalidateRect(hwnd, NULL, FALSE);
					}
					UpdatePlaylistFontSizeText();
					_w_i("pe_fontsize", config_pe_fontsize);
					break;
				case ID_PE_FONTSMALLER:
					if (config_pe_fontsize > 2) config_pe_fontsize--;
					if (hMainWindow)
					{
						draw_reinit_plfont(1);
						InvalidateRect(hwnd, NULL, FALSE);
					}
					UpdatePlaylistFontSizeText();
					_w_i("pe_fontsize", config_pe_fontsize);
					break;
				default:
					SendMessage(hMainWindow, uMsg, wParam, lParam);
					if (GetForegroundWindow() == hMainWindow) SetForegroundWindow(hEQWindow);
					return 0;
			}
			return 0;
		case WM_USER + 0xebe:
			if (lParam != 0xdeadbeef) break;
		case WM_DROPFILES:
		{
			wchar_t temp[FILENAME_SIZE];
			int x, y, a, t, lp;
			HDROP h = (HDROP)(HANDLE) wParam;
			POINT dp;
			DragQueryPoint(h, &dp);
			if (config_pe_height == 14) t = PlayList_getlength();
			else
			{
				t = (dp.y - 20) / pe_fontheight;
				if (t < 0) t = 0;
				else if (t > (config_pe_height - 38 - 20 - 2) / pe_fontheight) t = PlayList_getlength();
				else t += pledit_disp_offs;
			}

			y = DragQueryFileW(h, 0xffffffff, temp, FILENAME_SIZE);

			a = PlayList_getlength();
			PlayList_saveend(t);
			lp = PlayList_getPosition();

			for (x = 0; x < y; x ++)
			{
				DragQueryFileW(h, x, temp, FILENAME_SIZE);
				PlayList_appendthing(temp, 0, 0);
			}
			PlayList_restoreend();
			if (t <= PlayList_getPosition())
			{
				PlayList_setposition(lp + PlayList_getlength() - a);
				if (!g_has_deleted_current)
				{
					PlayList_getcurrent(FileName, FileTitle, FileTitleNum);
					draw_songname(FileTitle, &ui_songposition, playing ? in_getlength() : PlayList_getcurrentlength());
				}
			}
			if (uMsg == WM_DROPFILES) DragFinish(h);
			plEditRefresh();
		}
		return 0;
		case WM_SETCURSOR:
			if (config_usecursors && !disable_skin_cursors)
			{
				if ((HWND)wParam == hwnd && HIWORD(lParam) == WM_MOUSEMOVE) 
					pe_ui_handlecursor(hwnd);
				return TRUE;
			}
			else SetCursor(LoadCursor(NULL, IDC_ARROW));
			break;
		case WM_MOUSEWHEEL:
		{
			int zDelta = GET_WHEEL_DELTA_WPARAM(wParam), dLines;
			// if the delta changes then ignore prior carryover
			// hopefully this will go with the expected action.
			if(zDelta < 0 && pledit_delta_carryover > 0 ||
			   zDelta > 0 && pledit_delta_carryover < 0)
			{
				pledit_delta_carryover = 0;
			}
			// otherwise add on the carryover from the prior message
			else zDelta += pledit_delta_carryover;

			if (0 == (MK_MBUTTON & LOWORD(wParam)) &&
				config_plmw2xscroll)
			{
				zDelta *= 2;
			}

			dLines = zDelta / WHEEL_DELTA;
			pledit_delta_carryover = zDelta - dLines * WHEEL_DELTA;

			if (!dLines)
			{
				if (zDelta > 0) dLines = 1;
				else if (zDelta < 0) dLines = 0;
			}
			zDelta = (dLines >= 0) ? dLines : -dLines;

			if (config_pe_height == 14)
			{
				if (dLines >= 0) dLines = WINAMP_BUTTON1;
				else dLines = WINAMP_BUTTON5;
			}
			else
			{
				if (0 != (MK_MBUTTON & LOWORD(wParam)))
				{
					if (dLines >= 0) dLines = ID_PE_MOVEUP;
					else dLines = ID_PE_MOVEDOWN;
				}
				else
				{
					if (dLines >= 0) dLines = ID_PE_SCROLLUP;
					else dLines = ID_PE_SCROLLDOWN;
				}
			}

			while (zDelta--)
			{
				SendMessage(hwnd, WM_COMMAND, dLines, 0);
			}
		}
		return 1; //FG> or gen_ff gets it too
	}
	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{
		SendMessageW(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}