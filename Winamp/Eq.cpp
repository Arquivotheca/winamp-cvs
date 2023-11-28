#include "Main.h"
#include "api.h"
#include "../nu/AutoChar.h"
#include "../nu/ns_wc.h"

unsigned char eq_tab[10] = {31, 31, 31, 31, 31, 31, 31, 31, 31, 31};

static int writeEQfile(wchar_t *file, char *name);
static int readEQfile(wchar_t *file, char *name);
static void deleteEQfile(wchar_t *file, char *name);
static void addEQtolistbox(wchar_t *file, HWND listbox);

static BOOL CALLBACK loadpresetProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK delpresetProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK savepresetProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK loadmp3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK savemp3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK delmp3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK eqProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static int EQ_OnRButtonUp(HWND hwnd, int x, int y, UINT flags);
static int EQ_OnLButtonDblClk(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if ((config_dsize && config_eqdsize ? 1 : 0)) { x /= 2; y /= 2; }
	if (y <= 14 && x < 252)
	{
		SendMessage(hMainWindow, WM_COMMAND, WINAMP_OPTIONS_WINDOWSHADE_EQ, 0);
	}
	return 1;
}

static int EQ_OnLButtonUp(HWND hwnd, int x, int y, UINT flags);
static int EQ_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
static int EQ_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
static BOOL EQ_OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized);

static char last_preset[128] = "Default";
static char sig[] = "Winamp EQ library file v1.1\x1A!--";

void eq_autoload(const char *mp3fn)
{
	if (!config_autoload_eq) return ;
	if (readEQfile(EQDIR2, scanstr_back((char*)mp3fn, "\\", (char*)mp3fn - 1) + 1))
		readEQfile(EQDIR1, "Default");

	PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
}

void eq_dialog(HWND hwnd, int init_state)
{
	if (!hEQWindow) return ;
	if (Ipc_WindowToggle(IPC_CB_WND_EQ, !config_eq_open))
	{
	if (!config_eq_open)
	{
		if(!init_state & !config_minimized) ShowWindow(hEQWindow, SW_SHOWNA);
		CheckMenuItem(main_menu, WINAMP_OPTIONS_EQ, MF_CHECKED);
		config_eq_open = 1;
		if (config_eq_ws) draw_eq_tbar(GetForegroundWindow() == hEQWindow ? 1 : (config_hilite ? 0 : 1));
		set_aot(1);
	}
	else
	{
		if (GetForegroundWindow() == hEQWindow || IsChild(hEQWindow, GetForegroundWindow()))
		{
			SendMessage(hMainWindow, WM_COMMAND, WINAMP_NEXT_WINDOW, 0);
		}
		CheckMenuItem(main_menu, WINAMP_OPTIONS_EQ, MF_UNCHECKED);
		ShowWindow(hEQWindow, SW_HIDE);
		config_eq_open = 0;
	}
	draw_eqplbut(config_eq_open, 0, config_pe_open, 0);
	}
	return ;
}


static void
EqWindow_OnMouseWheel(HWND hwnd, INT virtualKeys, INT distance, LONG pointer_s)
{
	SendMessageW(hMainWindow, WM_MOUSEWHEEL, MAKEWPARAM(virtualKeys, distance), (LPARAM)pointer_s);
}

LRESULT CALLBACK EQ_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
		{
			LPTOOLTIPTEXT tt = (LPTOOLTIPTEXT)lParam;
			if(tt->hdr.hwndFrom = hEQTooltipWindow)
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
	case WM_DESTROY:
		if (eq_init)
			draw_eq_kill();
		if (NULL != WASABI_API_APP) WASABI_API_APP->app_unregisterGlobalWindow(hwnd);
		break;
	case WM_DISPLAYCHANGE: 
		InvalidateRect(hwnd, NULL, TRUE);
		break;

	case WM_SHOWWINDOW:
		if (wParam == TRUE && lParam == SW_PARENTOPENING && !config_eq_open)
			return 0;		
		break;

		HANDLE_MSG(hwnd, WM_QUERYNEWPALETTE, Main_OnQueryNewPalette);
		HANDLE_MSG(hwnd, WM_PALETTECHANGED, Main_OnPaletteChanged);
		HANDLE_MSG(hwnd, WM_LBUTTONUP, EQ_OnLButtonUp);
		HANDLE_MSG(hwnd, WM_RBUTTONUP, EQ_OnRButtonUp);
		HANDLE_MSG(hwnd, WM_LBUTTONDBLCLK, EQ_OnLButtonDblClk);
		HANDLE_MSG(hwnd, WM_LBUTTONDOWN, EQ_OnLButtonDown);
		HANDLE_MSG(hwnd, WM_MOUSEMOVE, EQ_OnMouseMove);
		HANDLE_MSG(hwnd, WM_NCACTIVATE, EQ_OnNCActivate);
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_SCREENSAVE || (wParam & 0xfff0) == SC_MONITORPOWER)
			return SendMessage(hMainWindow, uMsg, wParam, lParam);
		break;

	case WM_CLOSE:
		WASABI_API_APP->main_shutdown();
		return 0;
	case WM_PRINTCLIENT:
		draw_printclient_eq((HDC)wParam, lParam);
		return 0;
	case WM_PAINT:
		draw_paint_eq(hwnd);
		return 0;
	case WM_SETCURSOR:
		if (config_usecursors && !disable_skin_cursors)
		{
			if ((HWND)wParam == hEQWindow && HIWORD(lParam) == WM_MOUSEMOVE) eq_ui_handlecursor();
			return TRUE;
		}
		else SetCursor(LoadCursor(NULL, IDC_ARROW));
		break;
	case WM_CREATE:
		hEQWindow = hwnd;
		SetWindowLongPtr(hEQWindow, GWLP_USERDATA, (config_keeponscreen&2) ? 0x49474541 : 0);
		{
			int w = WINDOW_WIDTH << ((config_dsize && config_eqdsize) ? 1 : 0);
			int h = ((config_eq_ws ? 14 : WINDOW_HEIGHT)) << ((config_dsize && config_eqdsize) ? 1 : 0);
			SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE)&~(WS_CAPTION));
			SetWindowPos(hEQWindow, 0, config_eq_wx, config_eq_wy, w, h, SWP_NOACTIVATE | SWP_NOZORDER | SWP_HIDEWINDOW);

			HACCEL hAccel = LoadAcceleratorsW(language_pack_instance, MAKEINTRESOURCEW(IDR_ACCELERATOR_EQ));
			if (!hAccel && language_pack_instance != hMainInstance) hAccel = LoadAcceleratorsW(hMainInstance, MAKEINTRESOURCEW(IDR_ACCELERATOR_EQ));
			if (hAccel) WASABI_API_APP->app_addAccelerators(hwnd, &hAccel, 1, TRANSLATE_MODE_NORMAL);
		}
		if (NULL != WASABI_API_APP) WASABI_API_APP->app_registerGlobalWindow(hwnd);
		return 0;
	case WM_USER:
		{
			int x;
			draw_eq_slid(0, config_preamp, 0);
			for (x = 1; x <= 10; x ++)
				draw_eq_slid(x, eq_tab[x - 1], 0);
			draw_eq_graphthingy();
		}
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case EQ_PANLEFT:
			if (config_pan - 4 < -128) config_pan = -128;
			else
			{
				if (config_pan - 4 > 0 && config_pan - 4 < 20) config_pan = 0;
				else config_pan -= 4;
			}
			in_setpan(config_pan);
			draw_panbar(config_pan, 0);
			return 0;
		case EQ_PANRIGHT:
			if (config_pan + 4 > 127) config_pan = 127;
			else
			{
				if (config_pan + 4 < 0 && config_pan + 4 > -20) config_pan = 0;
				else config_pan += 4;
			}
			in_setpan(config_pan);
			draw_panbar(config_pan, 0);
			return 0;

		case WINAMP_OPTIONS_WINDOWSHADE:
			SendMessage(hMainWindow, WM_COMMAND, WINAMP_OPTIONS_WINDOWSHADE_EQ, 0);
			return 0;
		case IDM_EQ_LOADDEFAULT:
			readEQfile(EQDIR1, "Default");
			PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			return 0;
		case IDM_EQ_SAVEDEFAULT:
			{
				writeEQfile(EQDIR1, "Default");
			}
			return 0;
		case ID_SAVE_EQF:
		case ID_LOAD_EQF:
			{
				wchar_t filename[1024] = {0};
				size_t size, ns;
				wchar_t *filter, *sf;
				UINT operation;
				OPENFILENAMEW l = {sizeof(OPENFILENAMEW), 0};

				size = 1024;
				filter = (wchar_t*)malloc(size*sizeof(wchar_t));
				memset(filter, 0, size);
				sf = filter;
				ns = 1;

				getStringW(IDS_P_FILE_EQ, filter, size);
				StringCbLengthW(filter, size, &ns);
				size -= (++ns);
				filter += ns;
				StringCchCopyW(filter, size, L"*.eqf");
				StringCbLengthW(filter, size, &ns);
				size -= (++ns);
				filter += ns;
				getStringW(IDS_P_FILE_ALL, filter, size);
				StringCbLengthW(filter, size, &ns);
				size -= (++ns);
				filter += ns;
				StringCchCopyW(filter, size, L"*.*");
				StringCbLengthW(filter, size, &ns);
				size -= (ns + 2);

				l.hwndOwner = hwnd;
				l.lpstrFilter = sf;
				l.lpstrFile = filename;
				l.nMaxFile = sizeof(filename) - 1;

				operation = (LOWORD(wParam) == ID_SAVE_EQF) ? IDS_P_EQ_FILE_WRITE : IDS_P_EQ_FILE_READ;
				l.lpstrTitle = getStringW(operation, NULL, 0);
				l.lpstrDefExt = L"eqf";
				l.Flags = OFN_HIDEREADONLY | OFN_EXPLORER;
				UninitDragDrops();
				if ((LOWORD(wParam) == ID_SAVE_EQF && GetSaveFileNameW(&l)) ||
				    (LOWORD(wParam) != ID_SAVE_EQF && GetOpenFileNameW(&l)))
				{
					if (LOWORD(wParam) == ID_SAVE_EQF) writeEQfile(filename, "Entry1");
					else
					{
						readEQfile(filename, "Entry1");
						PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
					}
				}
				InitDragDrops();
				free(sf);
			}

			return 0;
		case IDM_EQ_SAVEPRE:
			LPDialogBoxW(IDD_SAVEPRESET, DIALOG_PARENT(hwnd), (WNDPROC)savepresetProc);
			return 0;
		case IDM_EQ_SAVEMP3:
			LPDialogBoxW(IDD_SAVEPRESET, DIALOG_PARENT(hwnd), (WNDPROC)savemp3Proc);
			return 0;
		case IDM_EQ_LOADPRE:
			LPDialogBoxW(IDD_LOADPRESET, DIALOG_PARENT(hwnd), (WNDPROC)loadpresetProc);
			return 0;
		case IDM_EQ_DELPRE:
			LPDialogBoxW(IDD_LOADPRESET, DIALOG_PARENT(hwnd), (WNDPROC)delpresetProc);
			return 0;
		case IDM_EQ_DELMP3:
			LPDialogBoxW(IDD_LOADPRESET, DIALOG_PARENT(hwnd), (WNDPROC)delmp3Proc);
			return 0;
		case IDM_EQ_LOADMP3:
			LPDialogBoxW(IDD_LOADPRESET, DIALOG_PARENT(hwnd), (WNDPROC)loadmp3Proc);
			return 0;
		case EQ_PRESETS:
			{
				extern HMENU top_menu;
				POINT p = {218, 19};
				if ((config_dsize && config_eqdsize))
				{
					p.x *= 2;
					p.y *= 2;
				}
				ClientToScreen(hEQWindow, &p);
				TrackPopupMenu(GetSubMenu(top_menu, 1), TPM_LEFTALIGN, p.x, p.y, 0, hEQWindow, NULL);
			}
			return 0;
		case EQ_AUTO:
			config_autoload_eq = !config_autoload_eq;
			draw_eq_onauto(config_use_eq, config_autoload_eq, 0, 0);
			PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			return 0;
		case EQ_ENABLE:
			config_use_eq = !config_use_eq;
			eq_set(config_use_eq, (char*)eq_tab, config_preamp);
			draw_eq_onauto(config_use_eq, config_autoload_eq, 0, 0);
			PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			return 0;
		case WINAMP_NEXT_WINDOW:
			return SendMessage(hMainWindow, uMsg, wParam, lParam);
		case ID_PE_CLOSE:
			SendMessage(hMainWindow, WM_COMMAND, WINAMP_OPTIONS_EQ, 0);
			return 0;
		default:
			{
				int id = LOWORD(wParam);
				if (id == EQ_INCPRE || id == EQ_DECPRE ||
				    (id >= EQ_DEC1 && id <= EQ_DEC10) ||
				    (id >= EQ_INC1 && id <= EQ_INC10))
				{
					int x;
					int addsub = ((id >= EQ_DEC1 && id <= EQ_DEC10) || id == EQ_DECPRE) ? -2 : 2;
					if (id == EQ_INCPRE || id == EQ_DECPRE)
					{
						config_preamp -= addsub;
						if (config_preamp < 0) config_preamp = 0;
						if (config_preamp > 63) config_preamp = 63;
					}
					else
					{
						int o = (id - (addsub > 0 ? EQ_INC1 : EQ_DEC1));
						int p = eq_tab[o];
						p -= addsub;
						if (p < 0) p = 0;
						if (p > 63) p = 63;
						eq_tab[o] = p;
					}
					draw_eq_slid(0, config_preamp, 0);
					for (x = 1; x <= 10; x ++)
						draw_eq_slid(x, eq_tab[x - 1], 0);
					draw_eq_graphthingy();
					eq_set(config_use_eq, (char*)eq_tab, config_preamp);
					PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
				}
				else
				{
					SendMessage(hMainWindow, uMsg, wParam, lParam);
					if (GetForegroundWindow() == hMainWindow) SetForegroundWindow(hEQWindow);
				}
			}
			return 0;
		}
		return 0;
	case WM_MOUSEWHEEL:
		EqWindow_OnMouseWheel(hwnd, LOWORD(wParam), (SHORT)HIWORD(wParam), (LONG)lParam); 
		return 0;
	}

	if (FALSE != IsDirectMouseWheelMessage(uMsg))
	{
		SendMessageW(hwnd, WM_MOUSEWHEEL, wParam, lParam);
		return TRUE;
	}

	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

static BOOL CALLBACK loadpresetProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x;
	static unsigned char eqtbk[10];
	static int eqpa;
	static char lpp[512];
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			addEQtolistbox(EQDIR1, GetDlgItem(hwndDlg, IDC_LOADEQLIST));
			x = SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)last_preset);
			if (x != LB_ERR)
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, x, 0);
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, -1, 0);
			StringCchCopy(lpp, 512, last_preset);

			// show window and restore last position as applicable
			POINT pt = {load_rect.left, load_rect.top};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, load_rect.left, load_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);

			eqpa = config_preamp;
			memcpy(eqtbk, eq_tab, sizeof(eqtbk));
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETTEXT,
			            SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCURSEL, 0, 0),
			            (LPARAM) last_preset);
			readEQfile(EQDIR1, last_preset);
			PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 0);
			return FALSE;
		case IDCANCEL:
			config_preamp = eqpa;
			memcpy(eq_tab, eqtbk, sizeof(eqtbk));
			StringCchCopy(last_preset, 128, lpp);
			SendMessage(hEQWindow, WM_USER, 0, 0);
			eq_set(config_use_eq, (char*)eq_tab, config_preamp);
			PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 1);
			return FALSE;
		case IDC_LOADEQLIST:
			if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETTEXT,
				            SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCURSEL, 0, 0),
				            (LPARAM) last_preset);
				readEQfile(EQDIR1, last_preset);
				PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			}
			return FALSE;
		}
		return FALSE;
	}
	return 0;
}

static BOOL CALLBACK loadmp3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static unsigned char eqtbk[10];
	static int eqpa;
	char buf[1024];
	switch (uMsg)
	{
	case WM_INITDIALOG:
		addEQtolistbox(EQDIR2, GetDlgItem(hwndDlg, IDC_LOADEQLIST));
		SetWindowText(hwndDlg, getString(IDS_LOADAUTOLOAD, NULL, 0));
		{
			int x;
			const wchar_t *filespec = PathFindFileNameW(FileName);
			WideCharToMultiByteSZ(CP_ACP, 0, filespec, -1, buf, 1024, 0, 0);
			SetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), buf);
			x = SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)buf);
			if (x != LB_ERR)
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, x, 0);
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, -1, 0);

			// show window and restore last position as applicable
			POINT pt = {load_rect.left, load_rect.top};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, load_rect.left, load_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
		}
		eqpa = config_preamp;
		memcpy(eqtbk, eq_tab, sizeof(eqtbk));
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETTEXT,
			            SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCURSEL, 0, 0),
			            (LPARAM) buf);
			readEQfile(EQDIR2, buf);
			PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 0);
			return FALSE;
		case IDCANCEL:
			config_preamp = eqpa;
			memcpy(eq_tab, eqtbk, sizeof(eqtbk));
			SendMessage(hEQWindow, WM_USER, 0, 0);
			eq_set(config_use_eq, (char*)eq_tab, config_preamp);
			PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 1);
			return FALSE;
		case IDC_LOADEQLIST:
			if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETTEXT,
				            SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCURSEL, 0, 0),
				            (LPARAM) buf);
				readEQfile(EQDIR2, buf);
				PostMessage(hMainWindow, WM_WA_IPC, IPC_CB_MISC_EQ, IPC_CB_MISC);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), 1);
			}
			return FALSE;
		}
		return FALSE;
	}
	return 0;
}

static BOOL CALLBACK savepresetProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[1024];
	switch (uMsg)
	{
	case WM_INITDIALOG:
		addEQtolistbox(EQDIR1, GetDlgItem(hwndDlg, IDC_LOADEQLIST));
		{
			int x;
			StringCchCopy(buf, 1024, last_preset);
			SetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), buf);
			x = SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)buf);
			if (x != LB_ERR)
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, x, 0);
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, -1, 0);

			// show window and restore last position as applicable
			POINT pt = {load_rect.left, load_rect.top};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, load_rect.left, load_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
		}

		SendMessage(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), EM_LIMITTEXT, _MAX_FNAME, 0);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), last_preset, sizeof(last_preset) - 1);
			writeEQfile(EQDIR1, last_preset);
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 0);
			return FALSE;
		case IDCANCEL:
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 1);
			return FALSE;
		case IDC_LOADEQLIST:
			if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETTEXT,
				            SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCURSEL, 0, 0),
				            (LPARAM) buf);
				SetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), buf);
			}
			return FALSE;
		case IDC_SAVEPRESET_EDIT:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				int x;
				GetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), buf, sizeof(buf) - 1);
				if (lstrlen(buf)) EnableWindow(GetDlgItem(hwndDlg, IDOK), 1);
				else EnableWindow(GetDlgItem(hwndDlg, IDOK), 0);

				x = SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)buf);
				if (x != LB_ERR)
					SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, x, 0);
				else
					SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, -1, 0);
			}
		}
		return FALSE;
	}
	return 0;
}

static BOOL CALLBACK savemp3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char buf[1024];
	switch (uMsg)
	{
	case WM_INITDIALOG:
		addEQtolistbox(EQDIR2, GetDlgItem(hwndDlg, IDC_LOADEQLIST));
		{
			int x;
			const wchar_t *filespec = PathFindFileNameW(FileName);
						WideCharToMultiByteSZ(CP_ACP, 0, filespec, -1, buf, 1024, 0, 0);
			SetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), buf);
			x = SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)buf);
			if (x != LB_ERR)
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, x, 0);
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, -1, 0);
			SetWindowText(hwndDlg, getString(IDS_SAVEAUTOLOAD, NULL, 0));

			// show window and restore last position as applicable
			POINT pt = {load_rect.left, load_rect.top};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, load_rect.left, load_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
		}
		SendMessage(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), EM_LIMITTEXT, _MAX_FNAME, 0);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), buf, sizeof(buf) - 1);
			writeEQfile(EQDIR2, buf);
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 0);
			return FALSE;
		case IDCANCEL:
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 1);
			return FALSE;
		case IDC_LOADEQLIST:
			if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETTEXT,
				            SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCURSEL, 0, 0),
				            (LPARAM) buf);
				SetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), buf);
			}
			return FALSE;
		case IDC_SAVEPRESET_EDIT:
			if (HIWORD(wParam) == EN_CHANGE)
			{
				int x;
				GetWindowText(GetDlgItem(hwndDlg, IDC_SAVEPRESET_EDIT), buf, sizeof(buf) - 1);
				x = SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)buf);
				if (x != LB_ERR)
					SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, x, 0);
				else
					SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, -1, 0);
			}
		}
		return FALSE;
	}
	return 0;
}

static BOOL CALLBACK delpresetProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x;
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			addEQtolistbox(EQDIR1, GetDlgItem(hwndDlg, IDC_LOADEQLIST));
			if (SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCOUNT, 0, 0) == 0)
				EnableWindow(GetDlgItem(hwndDlg, IDOK), 0);
			SetWindowText(hwndDlg, getString(IDS_DELETEPRE1, NULL, 0));
			SetWindowText(GetDlgItem(hwndDlg, IDOK), getString(IDS_DELETEAUTOLOAD2, NULL, 0));
			x = SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)last_preset);
			if (x != LB_ERR)
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, x, 0);
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, -1, 0);

			// show window and restore last position as applicable
			POINT pt = {load_rect.left, load_rect.top};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, load_rect.left, load_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETTEXT,
			            SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCURSEL, 0, 0),
			            (LPARAM) last_preset);
			deleteEQfile(EQDIR1, last_preset);
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 0);
			return FALSE;
		case IDCANCEL:
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 1);
			return FALSE;
		case IDC_LOADEQLIST:
			if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			return FALSE;
		}
		return FALSE;
	}
	return 0;
}

static BOOL CALLBACK delmp3Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int x;
	char buf[1024];
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			addEQtolistbox(EQDIR2, GetDlgItem(hwndDlg, IDC_LOADEQLIST));
			if (SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCOUNT, 0, 0) == 0)
				EnableWindow(GetDlgItem(hwndDlg, IDOK), 0);
			SetWindowText(hwndDlg, getString(IDS_DELETEAUTOLOAD1, NULL, 0));
			SetWindowText(GetDlgItem(hwndDlg, IDOK), getString(IDS_DELETEAUTOLOAD2, NULL, 0));
			const wchar_t *filespec = PathFindFileNameW(FileName);
			WideCharToMultiByteSZ(CP_ACP, 0, filespec, -1, buf, 1024, 0, 0);
			x = SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)buf);
			if (x != LB_ERR)
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, x, 0);
			else
				SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_SETCURSEL, -1, 0);

			// show window and restore last position as applicable
			POINT pt = {load_rect.left, load_rect.top};
			if (!windowOffScreen(hwndDlg, pt))
				SetWindowPos(hwndDlg, HWND_TOP, load_rect.left, load_rect.top, 0, 0, SWP_NOSIZE | SWP_NOSENDCHANGING);

			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETTEXT,
			            SendMessage(GetDlgItem(hwndDlg, IDC_LOADEQLIST), LB_GETCURSEL, 0, 0),
			            (LPARAM) buf);
			deleteEQfile(EQDIR2, buf);
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 0);
			return FALSE;
		case IDCANCEL:
			GetWindowRect(hwndDlg, &load_rect);
			EndDialog(hwndDlg, 1);
			return FALSE;
		case IDC_LOADEQLIST:
			if (HIWORD(wParam) == LBN_DBLCLK)
				SendMessage(hwndDlg, WM_COMMAND, IDOK, 0);
			return FALSE;
		}
		return FALSE;
	}
	return 0;
}

static int EQ_OnRButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	POINT p;
	extern HMENU top_menu;
	GetCursorPos(&p);
	if ((config_dsize && config_eqdsize))
	{
		x /= 2;
		y /= 2;
	}

	if ((flags & MK_LBUTTON)) return 1;

	if (x >= 14 && y >= 18 && x <= 14 + 25 && y <= 18 + 12)
	{
		HMENU hmenu = GetSubMenu(GetSubMenu(top_menu, 4), 0);
		CheckMenuItem(hmenu, EQ_ENABLE, config_use_eq ? MF_CHECKED : MF_UNCHECKED);
		TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
	}
	else if (x >= 14 + 25 && y >= 18 && x <= 14 + 25 + 33 && y <= 18 + 12)
	{
		HMENU hmenu = GetSubMenu(GetSubMenu(top_menu, 4), 1);
		CheckMenuItem(hmenu, EQ_AUTO, config_autoload_eq ? MF_CHECKED : MF_UNCHECKED);
		TrackPopupMenu(hmenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hwnd, NULL);
	}
	else
	{
		TrackPopupMenu(main_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, p.x, p.y, 0, hMainWindow, NULL);
	}
	return 1;
}

static int EQ_OnLButtonUp(HWND hwnd, int x, int y, UINT flags)
{
	ReleaseCapture();
	if ((config_dsize && config_eqdsize))
	{
		equi_handlemouseevent(x / 2, y / 2, -1, flags);
	}
	else
	{
		equi_handlemouseevent(x, y, -1, flags);
	}
	return 1;
}

// Mousedown handler. Just passes to routines in ui.c, scaling if in doublesize mode
static int EQ_OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
	if ((config_dsize && config_eqdsize))
	{
		x /= 2;
		y /= 2;
	}

	SetCapture(hwnd);
	equi_handlemouseevent(x, y, 1, keyFlags);
	return 1;
}

// Mousemove handler. Just passes to routines in ui.c, scaling if in doublesize mode
static int EQ_OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
	if ((config_dsize && config_eqdsize))
	{
		x /= 2;
		y /= 2;
	}

	equi_handlemouseevent(x, y, 0, keyFlags);
	return 1;
}

static BOOL EQ_OnNCActivate(HWND hwnd, BOOL fActive, HWND hwndActDeact, BOOL fMinimized)
{
	if (fActive == FALSE)
	{
		draw_eq_tbar(config_hilite ? 0 : 1);
	}
	else
	{
		draw_eq_tbar(1);
	}
	return TRUE;
}

int writeEQfile_init(wchar_t *file, char *name, unsigned char *tab)
{
	unsigned char preamp = 31;
	HANDLE hFile;
	char s[2048];
	DWORD a;
	hFile = CreateFileW(file, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return 1;
	ReadFile(hFile, s, lstrlen(sig), &a, NULL);
	if (memcmp(s, sig, lstrlen(sig)))
	{
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		WriteFile(hFile, sig, lstrlen(sig), &a, NULL);
		SetEndOfFile(hFile);
	}
	while (1)
	{
		ReadFile(hFile, s, _MAX_FNAME + 1 + 10 + 1, &a, NULL);
		if (a != _MAX_FNAME + 1 + 10 + 1)
		{
			SetFilePointer(hFile, 0, NULL, FILE_END);
			break;
		}
		if (!_stricmp(name, s))
		{
			CloseHandle(hFile);
			return 0; // don't write it
		}
	}
	StringCchCopyEx(s, _MAX_FNAME+1, name, 0, 0, STRSAFE_FILL_BEHIND_NULL);
	memcpy(s + _MAX_FNAME + 1, tab, 10);
	memcpy(s + _MAX_FNAME + 1 + 10, &preamp, 1);
	WriteFile(hFile, s, _MAX_FNAME + 1 + 10 + 1, &a, NULL);
	CloseHandle(hFile);
	return 0;
}

static int writeEQfile(wchar_t *file, char *name)
{
	HANDLE hFile;
	char s[2048];
	DWORD a;
	hFile = CreateFileW(file, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return 1;
	ReadFile(hFile, s, lstrlen(sig), &a, NULL);
	if (memcmp(s, sig, lstrlen(sig)))
	{
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		WriteFile(hFile, sig, lstrlen(sig), &a, NULL);
		SetEndOfFile(hFile);
	}
	while (1)
	{
		ReadFile(hFile, s, _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp), &a, NULL);
		if (a != _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp))
		{
			SetFilePointer(hFile, 0, NULL, FILE_END);
			break;
		}
		if (!_stricmp(name, s))
		{
			SetFilePointer(hFile, -(signed)a, NULL, FILE_CURRENT);
			break;
		}
	}
	StringCchCopyEx(s, _MAX_FNAME+1, name, 0, 0, STRSAFE_FILL_BEHIND_NULL);
	memcpy(s + _MAX_FNAME + 1, eq_tab, sizeof(eq_tab));
	memcpy(s + _MAX_FNAME + 1 + sizeof(eq_tab), &config_preamp, sizeof(config_preamp));
	WriteFile(hFile, s, _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp), &a, NULL);
	CloseHandle(hFile);
	return 0;
}

static int readEQfile(wchar_t *file, char *name)
{
	HANDLE hFile;
	char s[2048];
	DWORD a;
	hFile = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return 1;
	ReadFile(hFile, s, lstrlen(sig), &a, NULL);
	if (memcmp(s, sig, lstrlen(sig)))
	{
		CloseHandle(hFile);
		return 1;
	}
	while (1)
	{
		ReadFile(hFile, s, _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp), &a, NULL);
		if (a != _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp))
		{
			CloseHandle(hFile);
			return 1;
		}
		if (!_stricmp(name, s))
			break;
	}
	CloseHandle(hFile);
	memcpy(eq_tab, s + _MAX_FNAME + 1, sizeof(eq_tab));
	memcpy(&config_preamp, s + _MAX_FNAME + 1 + sizeof(eq_tab), sizeof(config_preamp));
	eq_set(config_use_eq, (char*)eq_tab, config_preamp);
	if (hEQWindow) SendMessage(hEQWindow, WM_USER, 0, 0);
	return 0;
}

static void deleteEQfile(wchar_t *file, char *name)
{
	HANDLE hFile, hFileOut;
	wchar_t tmpfile[2048];
	char s[2048];
	DWORD a;
	hFile = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return ;
	StringCchPrintfW(tmpfile, 2048, L"%s.TMP", file);
	hFileOut = CreateFileW(tmpfile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFileOut == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return ;
	}
	ReadFile(hFile, s, lstrlen(sig), &a, NULL);
	WriteFile(hFileOut, s, lstrlen(sig), &a, NULL);
	while (1)
	{
		ReadFile(hFile, s, _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp), &a, NULL);
		if (a != _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp))
		{
			break;
		}
		if (_stricmp(name, s))
			WriteFile(hFileOut, s, a, &a, NULL);
	}
	CloseHandle(hFile);
	CloseHandle(hFileOut);
	DeleteFileW(file);
	StringCchPrintfW(tmpfile, 2048, L"%s.TMP", file);
	MoveFileW(tmpfile, file);
	return ;
}

static void addEQtolistbox(wchar_t *file, HWND listbox)
{
	HANDLE hFile;
	char s[2048];
	DWORD l;
	hFile = CreateFileW(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) return ;
	ReadFile(hFile, s, lstrlen(sig), &l, NULL);
	if (memcmp(s, sig, lstrlen(sig)))
	{
		CloseHandle(hFile);
		return ;
	}
	while (1)
	{
		DWORD a;
		ReadFile(hFile, s, _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp), &a, NULL);
		if (a != _MAX_FNAME + 1 + sizeof(eq_tab) + sizeof(config_preamp))
			break;
		SendMessage(listbox, LB_ADDSTRING, 0, (LPARAM)s);
	}
	CloseHandle(hFile);
}